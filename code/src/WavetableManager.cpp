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
        READ_Samples(file_id, cache_1, A_Flash_sample, length_max);

        // B>>>>>>>>>>>>length_max>>>>>>>>>>>>>>A
        for (auto sample = 0; sample < (length_max); ++sample)
        {
            Wavetable[sample] = cache_1[length_max - 1 - sample];
        }

        break;

    case LOOP_FWD: // mode 2: loop A-->B A-->B.

        // forward play from (A_Flash_sample + delta_Noclick) to B_Flash_sample
        length = length_mix;

        // (A+d)******length_min*********(B-d)
        READ_Samples(file_id, Wavetable, (A_Flash_sample + delta_Noclick), length_min);

        // (B-d+1)***delta_Noclick***B
        for (auto sample = 0; sample < delta_Noclick; ++sample)
        {
            Wavetable[length_min + sample] = *(p_Noclick + sample);
        }

        // REWORK_Wavetable(1, 0.5);
        break;

    case LOOP_FWD_REV: // mode 3: loop A-->B-->A

        // play forward from A_Flash_sample to B_Flash_sample and reverse
        length = 2 * length_max - 2;

        // A>>>>>>>>>>>>>length_max>>>>>>>>>>>>>B
        READ_Samples(file_id, Wavetable, A_Flash_sample, length_max);

        // (A+1)<<<<<<<(length_max - 2)<<<<<<<(B-1)
        for (auto sample = 0; sample < (length_max - 2); ++sample)
        {
            Wavetable[length_max + sample] = Wavetable[(length_max - 2) - sample];
        }

        // REWORK_Wavetable(3);
        break;

    case LOOP_REV: // mode 5: B-->A B-->A
    
        // reverse play from (A_Flash_sample) to (B_Flash_sample - delta_Noclick)
        length = length_mix;

        // (A)>>>>delta>>>>(A+d-1)
        for (auto sample = 0; sample < delta_Noclick; ++sample)
        {
            cache_1[sample] = *(p_Noclick + sample);
        }

        // (A+d)>>>>>>>>>>>>>length_min>>>>>>>>>>>>>>>(B-d)
        READ_Samples(file_id, cache_2, (A_Flash_sample + delta_Noclick), length_min);

        // (A)>>>>>delta>>>>(A+d-1)(A+d)>>>>>>>>>>>>>>>>length_min>>>>>>>>>>>>(B-d)
        for (auto sample = 0; sample < length_min; ++sample)
        {
            cache_1[delta_Noclick + sample] = cache_2[sample];
        }

        for (auto sample = 0; sample < length_mix; ++sample)
        {
            Wavetable[sample] = cache_1[(length_mix - 1) - sample];
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
    LillaSerialFlashFile rawfile;
    int first_packet;

    // .RAW files
    if (file_id < FIRST_RECORDING_FILE)
    {
        first_byte = seek_in * 2;

        rawfile.fast_open(file_id);
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

        rawfile.packet_fast_open(first_packet + packet_delta); 
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