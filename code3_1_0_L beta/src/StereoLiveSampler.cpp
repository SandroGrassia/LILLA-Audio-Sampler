/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// StereoLiveSampler registra su file in PSRAM

#include "StereoLiveSampler.h"

void StereoLiveSampler::Begin(void)
{
    writing = false;
}

void StereoLiveSampler::Reset(void)
{
    Q_sample = -1;
    first_write_flag = true;
}

void StereoLiveSampler::Start(bool stereo_in)
{
    stereo = stereo_in;
    decay_gain_flag = false;
    attack_gain_flag = true;
    samples_counter = Slope_samples;
    cancel_beyond = true;

    // Record!!
    writing = true;
}

void StereoLiveSampler::Stop(void)
{
    book_stop_flag = true;
}

void StereoLiveSampler::Set_cancel_beyond(bool value)
{
    cancel_beyond = value;
}
void StereoLiveSampler::update(void)
{
    // only update if we're writing
    if (!writing)
    {
        return;
    }

    if (book_stop_flag)
    {
        decay_gain_flag = true;
        samples_counter = Slope_samples;
        book_stop_flag = false;
    }

    in_block_L = receiveWritable(0);
    in_block_R = receiveWritable(1);

    if (!in_block_L || !in_block_R)
    {
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            in_block_L->data[i] = 0;
            in_block_R->data[i] = 0;
        }
        release(in_block_L);
        release(in_block_R);
        return;
    }

    // microtimer = 0;
    int Q_sample_cache = Q_sample; // ultimo Q_sample scritto al ciclo PRECEDENTE

    if (Q_sample == LS_buffer_dim - 1)
    {
        Q_sample = -1;
        first_write_flag = false;
    }
    if (stereo) // si copiano i sample entranti direttamente su PSRAM, array L e array R
    {
        memcpy((uint32_t *)(LS_buffer_L_ptr + Q_sample + 1), (uint32_t *)in_block_L->data, 256); // memcpy(destination pointer, origin pointer, bytes to be copied)
        memcpy((uint32_t *)(LS_buffer_R_ptr + Q_sample + 1), (uint32_t *)in_block_R->data, 256); // memcpy(destination pointer, origin pointer, bytes to be copied)
    }
    else // si copiano i sample entranti su due Buffer, poi il valore medio viene copiato su un unico array mono
    {
        memcpy((uint32_t *)(Buffer_L), (uint32_t *)in_block_L->data, 256); // memcpy(destination pointer, origin pointer, bytes to be copied)
        memcpy((uint32_t *)(Buffer_R), (uint32_t *)in_block_R->data, 256); // memcpy(destination pointer, origin pointer, bytes to be copied)
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
            LS_buffer_mono_ptr[Q_sample + 1 + i] = (Buffer_L[i] >> 1) + (Buffer_R[i] >> 1);
    }

    Q_sample += AUDIO_BLOCK_SAMPLES; // ultimo Q_sample scritto al ciclo ATTUALE

    if (cancel_beyond)
    {
        int start_sample = Q_sample + 1;

        if (start_sample == LS_buffer_dim)
        {
            start_sample = 0;
        }

        if (stereo)
        {
            for (int i = 0; i < (AUDIO_BLOCK_SAMPLES / 2); ++i)
            {
                *(uint32_t *)(LS_buffer_L_ptr + i) = 0;
                *(uint32_t *)(LS_buffer_R_ptr + i) = 0;
            }
        }

        else
        {
            for (int i = 0; i < (AUDIO_BLOCK_SAMPLES / 2); ++i)
            {
                *(uint32_t *)(LS_buffer_mono_ptr + i) = 0;
            }
        }
    }

    /*
    if (Q_sample < 10000 || Q_sample > LS_buffer_dim - 10000)
    {
        Serial.print("Q_sample ");
        Serial.println(Q_sample);
    }
    */

    if (attack_gain_flag)
    {
        if (stereo)
        {
            for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
            {
                float factor = (1.0 - samples_counter / Slope_samples);
                LS_buffer_L_ptr[Q_sample_cache + 1 + i] = LS_buffer_L_ptr[Q_sample_cache + 1 + i] * factor;
                LS_buffer_R_ptr[Q_sample_cache + 1 + i] = LS_buffer_R_ptr[Q_sample_cache + 1 + i] * factor;
                --samples_counter;
            }
        }

        else
        {
            for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
            {
                float factor = (1.0 - samples_counter / Slope_samples);
                LS_buffer_mono_ptr[Q_sample_cache + 1 + i] = LS_buffer_mono_ptr[Q_sample_cache + 1 + i] * factor;
                --samples_counter;
            }
        }

        if (samples_counter == 0)
        {
            attack_gain_flag = false;
        }
    }

    else if (decay_gain_flag)
    {
        if (stereo)
        {
            for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
            {
                float factor = samples_counter / Slope_samples;
                LS_buffer_L_ptr[Q_sample_cache + 1 + i] = LS_buffer_L_ptr[Q_sample_cache + 1 + i] * factor;
                LS_buffer_R_ptr[Q_sample_cache + 1 + i] = LS_buffer_R_ptr[Q_sample_cache + 1 + i] * factor;
                --samples_counter;
            }
        }

        else
        {
            for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
            {
                float factor = samples_counter / Slope_samples;
                LS_buffer_mono_ptr[Q_sample_cache + 1 + i] = LS_buffer_mono_ptr[Q_sample_cache + 1 + i] * factor;
                --samples_counter;
            }
        }

        if (samples_counter == 0)
        {
            decay_gain_flag = false;
            writing = false;
        }
    }

    release(in_block_L);
    release(in_block_R);
    return;
}

bool StereoLiveSampler::Is_writing(void)
{
    return writing;
}