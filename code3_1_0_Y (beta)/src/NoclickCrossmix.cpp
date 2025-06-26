/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// NoclickCrossmix.h crea il vettore di cross-mix per loop Forward o loop Reverse

#include "NoclickCrossmix.h"

int16_t *NoclickCrossmix::get_pointer()
{
    return Noclick; // &NoClick[0];
}

bool NoclickCrossmix::Make(int file_id, int32_t A_Flash_sample, int32_t B_Flash_sample, uint16_t delta_Noclick)
{
    // Serial.print("Delta_noclick: ");
    // Serial.println(delta_Noclick);

    int16_t cache[NOCLICK_DIM];
    Read_flash(file_id, cache, A_Flash_sample, delta_Noclick);
    Read_flash(file_id, Noclick, (B_Flash_sample - delta_Noclick + 1), delta_Noclick);

    // linear snubber and sum
    float h;

    for (auto sample = 0; sample < delta_Noclick; ++sample)
    {
         h = (float)sample / ((float)delta_Noclick - 1.0); // h: 0 --> 1.0
        cache[sample] = (float)cache[sample] * h;
        Noclick[sample] = ((float)Noclick[sample] * (1.0 - h)) + cache[sample];
    }
    
    return true;
}

void NoclickCrossmix::Read_flash(int file_id, int16_t *destination, int seek_in, int samples_in)
{
    LillaSerialFlashFile rawfile; // SerialFlashFile rawfile;
    int first_packet = 0;
    bool recording_flag = false;

    if (file_id >= FIRST_RECORDING_FILE)
    {
        recording_flag = true;
        recording = (file_id - FIRST_RECORDING_FILE) / 2;
        bool file_L_flag = ((file_id - FIRST_RECORDING_FILE) % 2 == 0);

        if (file_L_flag)
        {
            first_packet = Recording[recording].first_packet;
        }
        else
        {
            first_packet = Recording[recording].first_packet + 1;
        }
    }

    // Serial.print("seek_in: ");
    // Serial.print(seek_in);
    // Serial.print(" samples_in: ");
    // Serial.println(samples_in);

    int first_byte = seek_in * 2;
    int total_bytes = samples_in * 2;
    byte *destination_byte = (byte *)destination;

    if (recording_flag)
    {
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

    else
    {
        rawfile.fast_open(file_id); // rawfile = SerialFlash.open(filename);
        
        rawfile.seek(first_byte);
        if (!rawfile)
        {
            return;
        }

        rawfile.read(destination_byte, total_bytes);
        rawfile.close();
    }
}