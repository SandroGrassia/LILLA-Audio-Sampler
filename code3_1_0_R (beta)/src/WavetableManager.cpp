/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "WavetableManager.h"

int16_t *WavetableManager::get_pointer(void)
{
    return Wavetable;
}

bool WavetableManager::Make(int file_id, int8_t mode, int A_Flash_sample, int B_Flash_sample, uint16_t delta_Noclick, int16_t *p_Noclick)
{
    // A------(A+d-1)(A+d)-----------------(B-d)(B-d+1)------(B)
    // ********************************************************
    int length_max = B_Flash_sample - A_Flash_sample + 1;

    // A------(A+d-1)(A+d)-----------------(B-d)(B-d+1)------(B)
    //                 ***********************
    int length_min = B_Flash_sample - A_Flash_sample - (2 * delta_Noclick) + 1;

    // A------(A+d-1)(A+d)-----------------(B-d)(B-d+1)------(B)
    //                 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //                         oppure:
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    int length_mix = B_Flash_sample - A_Flash_sample - delta_Noclick + 1;

    if (mode == ONCE_FWD || mode == LOOP_REV_FWD)
    {
        mode = LOOP_FWD_REV;
    }

    switch (mode)
    {
    case ONCE_REV: // mode 1: B-->A than STOP
        length = length_max;
        // A>>>>>>>>>>>>>length_max>>>>>>>>>>>>>B
        // rawfile.seek(A_Flash_sample << 1);
        // rawfile.read(cache_1, length_max << 1);
        READ_Samples(file_id, cache_1, A_Flash_sample, length_max);

        // B>>>>>>>>>>>>length_max>>>>>>>>>>>>>>A
        for (auto i = 0; i < (length_max); ++i)
        {
            Wavetable[i] = cache_1[length_max - 1 - i];
        }

        break;

    case LOOP_FWD: // mode 2: loop A-->B A-->B.
        // forward play from (A_Flash_sample + delta_Noclick) to B_Flash_sample
        length = length_mix;

        // (A+d)******length_min*********(B-d)
        // rawfile.seek((A_Flash_sample + delta_Noclick) <<1);
        // rawfile.read(Wavetable, length_min<<1);
        READ_Samples(file_id, Wavetable, (A_Flash_sample + delta_Noclick), length_min);

        // (B-d+1)***delta_Noclick***B
        for (auto i = 0; i < delta_Noclick; ++i)
        {
            Wavetable[length_min + i] = *(p_Noclick + i);
        }

        // REWORK_Wavetable(1, 0.5);
        break;

    case LOOP_FWD_REV: // mode 3: loop A-->B-->A
        // play forward from A_Flash_sample to B_Flash_sample and reverse
        length = 2 * length_max - 2;

        // A>>>>>>>>>>>>>length_max>>>>>>>>>>>>>B
        // rawfile.seek(A_Flash_sample << 1);
        // rawfile.read(Wavetable, length_max << 1);
        READ_Samples(file_id, Wavetable, A_Flash_sample, length_max);

        // (A+1)<<<<<<<(length_max - 2)<<<<<<<(B-1)
        for (auto i = 0; i < (length_max - 2); ++i)
        {
            Wavetable[length_max + i] = Wavetable[(length_max - 2) - i];
        }

        // REWORK_Wavetable(3);
        break;

    case LOOP_REV: // mode 5: B-->A B-->A
        // reverse play from (A_Flash_sample) to (B_Flash_sample - delta_Noclick)
        length = length_mix;

        // (A)>>>>delta>>>>(A+d-1)
        for (auto i = 0; i < delta_Noclick; ++i)
        {
            cache_1[i] = *(p_Noclick + i);
        }

        // (A+d)>>>>>>>>>>>>>length_min>>>>>>>>>>>>>>>(B-d)
        // rawfile.seek((A_Flash_sample + delta_Noclick) <<1);
        // rawfile.read(cache_2, length_min<<1);
        READ_Samples(file_id, cache_2, (A_Flash_sample + delta_Noclick), length_min);

        // (A)>>>>>delta>>>>(A+d-1)(A+d)>>>>>>>>>>>>>>>>length_min>>>>>>>>>>>>(B-d)
        for (auto i = 0; i < length_min; ++i)
        {
            cache_1[delta_Noclick + i] = cache_2[i];
        }

        for (auto i = 0; i < length_mix; ++i)
        {
            Wavetable[i] = cache_1[(length_mix - 1) - i];
        }
        break;

    default:
        break;
    }
    return true;
}

void WavetableManager::READ_Samples(int file_id, int16_t *destination, int seek_in, int samples_in)
{
    int first_byte;
    int total_bytes = samples_in * 2;
    byte *destination_byte = (byte *)destination;
    LillaSerialFlashFile rawfile; // SerialFlashFile rawfile;
    int first_packet;

    // .RAW files
    if (file_id < FIRST_RECORDING_FILE)
    {
        first_byte = seek_in * 2;
        // const char* filename = name_file[file_id];
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
    // .REC files
    else if (file_id < FIRST_LIVE_SAMPLING_FILE)
    {
        first_byte = seek_in * 2;
        int recording = (file_id - FIRST_RECORDING_FILE) / 2;
        bool file_L_flag = ((file_id - FIRST_RECORDING_FILE) % 2 == 0);

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

        if (local_last_byte < PACKET_DIM) // 1 Block is needed
        {
            // timer = 0;
            rawfile.seek(local_first_byte);
            rawfile.read(destination_byte, total_bytes);
            rawfile.close();
            // Serial.println(timer);
        }
        else // 2 Blocks are needed - with T41@600MHz adds 40microseconds
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
}