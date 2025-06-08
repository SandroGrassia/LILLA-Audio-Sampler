/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "InfoMaster.h"

// Samples in mono recording or in Left channel's recording
int InfoMaster::DS_recording_samples(int recording)
{
    int first_packet = Recording[recording].first_packet;
    int packets = Recording[recording].packets;
    bool stereo = Recording[recording].stereo;
    SerialFlashFile rawfile;

    if (packets < 1 || first_packet < 0 || first_packet >= VFS_PACKETS_MAX)
    {
        return 0;
    }

    int value = 0; // bytes

    // Conta i campioni di tutti i packet tranne l'ultimo
    if (packets > 1)
    {
        value += (packets - 1) * PACKET_DIM;
    }

    // Quindi aggiungi l'ultimo packet
    // Look for the last packet of recording
    if (!stereo)
    {
        rawfile = SerialFlash.open(name_packet[first_packet + packets - 1]);
    }
    else
        // 0 1 2 3 4 5 6 7 8 9 10 11
        //         L R L R L R
        rawfile = SerialFlash.open(name_packet[first_packet + 2 * (packets - 1)]); // 4 + 2*(3 - 1) = 8

    uint16_t sample_value;
    int i = 0;
    for (i = (PACKET_DIM - 2); i >= 0; i -= 2)
    {
        rawfile.seek(i);
        rawfile.read(&sample_value, 2); // read 2 bytes (1 sample)
        if (sample_value != 0xFFFF)
        {
            break;
        }
    }
    if (i < 0)
    {
        i = 0;
    }

    value += i + 1;
    rawfile.close();

    Serial.print("InfoMaster - DS_recording_samples: ");
    Serial.println(value);

    return (value >> 1); // return samples_available;
}

int InfoMaster::Raw_file_samples(int file_id)
{
    int result;
    LillaSerialFlashFile rawfile; // SerialFlashFile rawfile;
    rawfile.fast_open(file_id);   // rawfile = SerialFlash.open(name_file[file_id]);
    if (!rawfile)
    {
        Serial.println(file_id);
        return 0;
    }
    else
    {
        result = rawfile.size() >> 1;
        rawfile.close();
        return result; // return samples_available;
    }
}

int16_t *InfoMaster::Sound_620_samples_array(int file_id, uint32_t A, uint32_t B)
{
    for (int i = 0; i < 2 * WAVE_WIDTH; ++i)
    {
        samples_620_array[i] = 0;
    }

    // (B - A + 1) >= 100
    float samples_per_pixel = (B - A + 1) / WAVE_WIDTH_F;                                     // fondamentale il "." nel divisore!!
    int Samples = (samples_per_pixel >= BASKET_INFO ? BASKET_INFO : ceil(samples_per_pixel)); // numero di campioni da leggere 1 <= Samples <= BASKET_INFO

    for (int i = 0; i < WAVE_WIDTH; ++i)
    {
        //    samples_per_pixel = 0.7
        //    file_id:   A          (A+1)      (A+2)      (A+3)      (A+5)         .......................B
        //    position:  0       0.7  |   1.4    2.1     2.8|    3.5   |
        //    position:  p        p   |    p     |p       p |     p    |

        float position = samples_per_pixel * i;
        Read_samples(file_id, samples_basket, A + position, (Samples <= 2 ? 2 : Samples));

        int16_t max_pos = 0;
        int16_t min_neg = 0;

        if (samples_per_pixel <= 1.0001)
        {
            float microposition = position - floor(position);
            int16_t value = samples_basket[0] + (samples_basket[1] - samples_basket[0]) * microposition;
            if (value >= 0)
            {
                max_pos = value;
            }
            else
            {
                min_neg = value;
            }
        }

        else
        {
            for (int j = 0; j < Samples; ++j)
            {
                if (samples_basket[j] >= 0 && samples_basket[j] > max_pos)
                {
                    max_pos = samples_basket[j];
                }
                if (samples_basket[j] < 0 && samples_basket[j] < min_neg)
                {
                    min_neg = samples_basket[j];
                }
            }
        }

        samples_620_array[i] = max_pos;
        samples_620_array[i + WAVE_WIDTH] = min_neg;
    }
    return samples_620_array;
}

// Live Sampling
void InfoMaster::LS_restart_antiflicker(void)
{
    LS_antiflicker = true;
}

// Live Sampling
int16_t *InfoMaster::LS_620_samples_array(int file_id, int A_window_sample, int B_window_sample)
{
    int A_window_sample_local = A_window_sample;
    int samples_to_read;  // numero di campioni da leggere per una riga verticale della window
    int samples_per_line; // numero di campioni associati ad una riga verticale della window
    float delta;
    int16_t max_pos;
    int16_t min_neg;

    if (file_id == FIRST_LIVE_SAMPLING_FILE)
    {
        FIFO = LS_buffer_mono_ptr;
        FIFO_dim = LS_MONO_SAMPLES; // samples
    }
    else if (file_id == FIRST_LIVE_SAMPLING_FILE + 1)
    {
        FIFO = LS_buffer_L_ptr;
        FIFO_dim = LS_STEREO_SAMPLES; // samples
    }
    else
    {
        FIFO = LS_buffer_R_ptr;
        FIFO_dim = LS_STEREO_SAMPLES; // samples
    }

    /*
        "samples_per_line" e' il numero di campioni associati ad 1 riga verticale della window; con ceil si garantisce la copertura della window contando
        per ogni riga della window lo stesso numero di campioni; esempio:
        B_window_sample - A_window_sample + 1 == 48
        WAVE_WIDTH_F == 10
        samples_per_line = ceil(48/10) = 5


                    0                                                                                                                             (LS_buffer_dim -1)
                    |.....................................................................................................................................|
                    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQPxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
                                                  A_window_sample                               B_window_sample
                                                        |                                              |
        window                                          ........................C.......................
        intervalli                                      00000111112222233333444445555566666777778888899999

    */

    samples_per_line = ceil((B_window_sample - A_window_sample + 1) / WAVE_WIDTH_F);

    // non si possono prelevare piu' di BASKET_INFO campioni; si definisce percio' samples_to_read
    samples_to_read = (samples_per_line <= BASKET_INFO ? samples_per_line : BASKET_INFO);

    // Serial.print("samples_to_read: ");
    // Serial.println(samples_to_read);

    /*
    Si devono eseguire WAVE_WIDTH (tante quante le linee verticali della window) letture, ciascuna di samples_to_read campioni, avanzando di samples_per_line elementi su
    LS_cache_Mono o LS_cache_L/R.

    Se LS_XY_locked == false e samples_per_line costante, si richiede che ad ogni refresh della wave gli stessi campioni siano sempre raggruppati nello stesso ciclo di lettura;
    in questo modo si garantisce che i valori visualizzati non saltino in continuazione (flickering).

    Esempio:
    (B_window_sample - A_window_sample + 1) == 50
    WAVE_WIDTH == 7
    samples_per_line = ceil(50/7) = 8
    BASKET_INFO == 4
    samples_to_read = (8 <= BASKET_INFO ? samples_per_line : BASKET_INFO) = 4

                0                                                                                                                             (LS_buffer_dim -1)
                |.....................................................................................................................................|
    tempo0      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQPxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
                                              A_window_sample                                  B_window_sample
                                                    |                                                |
    window0                                         ........................C.........................
    lettura0                                        0000----1111----2222----3333----4444----5555----6666----

    tempo1      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQPxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
                                                         A_window_sample                                  B_window_sample
                                                               |                                                |
    window1                                                    ........................C.........................
    lettura1                                                0000----1111----2222----3333----4444----5555----6666----

    tempo2      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQPxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
                                                                 A_window_sample                                  B_window_sample
                                                                       |                                                |
    window2                                                            ........................C.........................
    lettura2                                                        0000----1111----2222----3333----4444----5555----6666----
    */

    if (!LS_XY_lock)
    {
        if (LS_antiflicker)
        {
            last_A_window_sample = A_window_sample_local;
            last_samples_per_line = samples_per_line;
            LS_antiflicker = false;
        }

        if (samples_per_line == last_samples_per_line)
        {
            if (A_window_sample_local >= last_A_window_sample)
            {
                // si avanza finché si raggiunge il primo elemento
                // - multiplo di samples_per_line a partire da last_A_window_sample
                // - che non supera A_window_sample

                A_window_sample_local = last_A_window_sample;
                while ((A_window_sample_local + samples_per_line) < A_window_sample)
                {
                    A_window_sample_local += samples_per_line;
                }
            }
            else
            {
                A_window_sample_local = last_A_window_sample;
                while (A_window_sample_local > A_window_sample)
                {
                    A_window_sample_local -= samples_per_line;
                }
                if (A_window_sample_local < 0)
                {
                    A_window_sample_local += FIFO_dim;
                }
            }
            last_A_window_sample = A_window_sample_local;
        }
    }

    // si effettuano WAVE_WIDTH prelievi
    for (int i = 0; i < WAVE_WIDTH; ++i)
    {
        // si avanza di samples_per_line (per poi prelevare samples_to_read campioni)
        delta = samples_per_line * i;
        Read_samples(file_id, samples_basket, A_window_sample_local + delta, samples_to_read);
        max_pos = 0;
        min_neg = 0;
        for (int j = 0; j < samples_to_read; ++j)
        {
            if (samples_basket[j] > max_pos)
            {
                max_pos = samples_basket[j];
            }
            else if (samples_basket[j] < min_neg)
            {
                min_neg = samples_basket[j];
            }
        }
        samples_620_array[i] = max_pos;
        samples_620_array[i + WAVE_WIDTH] = min_neg;
    }
    return samples_620_array;
}

void InfoMaster::Read_samples(int file_id, int16_t *destination, int seek_in, int samples_in) // samples_in <= BASKET_INFO
{
    int first_byte;
    int total_bytes = samples_in * 2;
    byte *destination_byte = (byte *)destination;
    LillaSerialFlashFile rawfile; // SerialFlashFile rawfile;
    int first_packet;

    // .raw files
    if (file_id < FIRST_RECORDING_FILE)
    {
        first_byte = seek_in * 2;
        rawfile.fast_open(file_id); // rawfile = SerialFlash.open(filename);
        if (!rawfile)
        {
            return;
        }

        rawfile.seek(first_byte);
        rawfile.read(destination_byte, total_bytes);
        rawfile.close();
    }

    // Direct Sampling
    // .rec files; indicano solo una registrazione, i samples sono contenuti nei Packet (registrati con Direct Sampler)
    else if (file_id < FIRST_LIVE_SAMPLING_FILE)
    {
        first_byte = seek_in * 2;
        int recording = (file_id - FIRST_RECORDING_FILE) / 2;
        bool file_L_flag = ((file_id - FIRST_RECORDING_FILE) % 2 == 0); // 0.rec, 2.rec, 4.rec

        if (file_L_flag)
        {
            first_packet = Recording[recording].first_packet;
        }
        else
        {
            first_packet = Recording[recording].first_packet + 1;
        }

        int packet_delta = first_byte >> 16;
        int local_first_byte = first_byte % PACKET_DIM; // updated

        // Serial.print("needed_packet is: ");
        // Serial.println(needed_packet);

        rawfile.packet_fast_open(first_packet + packet_delta); // rawfile = SerialFlash.open(name_packet[first_packet + packet_delta]);
        if (!rawfile)
        {
            return;
        }

        // Serial.print(F("1 - Packet played is: "));
        // Serial.println(name_packet[first_packet + packet_delta]);

        int local_last_byte = local_first_byte + total_bytes - 1;

        if (local_last_byte < PACKET_DIM) // 1 only Packet is needed
        {
            // timer = 0;
            rawfile.seek(local_first_byte);
            rawfile.read(destination_byte, total_bytes);
            rawfile.close();
            // Serial.println(timer);
        }
        else // 2 Packets are needed - with T41@600MHz adds 40us
        {
            // timer = 0;
            int first_part = PACKET_DIM - local_first_byte;
            int second_part = total_bytes - first_part;

            rawfile.seek(local_first_byte);
            rawfile.read(destination_byte, first_part);
            rawfile.close();

            packet_delta += 2;
            rawfile.packet_fast_open(first_packet + packet_delta); // rawfile = SerialFlash.open(name_packet[first_packet + packet_delta]);
            if (!rawfile)
            {
                return;
            }

            rawfile.seek(0);
            rawfile.read(destination_byte + first_part, second_part);
            rawfile.close();

            // Serial.print(F("2 - Packet played is: "));
            // Serial.println(name_packet[first_packet + packet_delta]);
        }
    }

    // Live Sampling
    else
    {
        if (seek_in > FIFO_dim - 1)
        {
            seek_in -= FIFO_dim;
        }
        int last_sample = seek_in + samples_in - 1;

        if (last_sample <= FIFO_dim - 1)
        {
            memcpy(destination, (FIFO + seek_in), total_bytes);
        }

        else
        {
            int first_part_samples = FIFO_dim - seek_in; // lenght in samples
            int second_part_bytes = total_bytes - 2 * first_part_samples;
            memcpy(destination, (FIFO + seek_in), 2 * first_part_samples);
            memcpy(destination + first_part_samples, FIFO, second_part_bytes);
        }
    }
}