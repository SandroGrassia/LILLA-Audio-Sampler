/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "WaveSine.h"

void WaveSine::Frequency(float freq)
{
    if (freq < 0.0)
        freq = 0.0;
    else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2)
        freq = AUDIO_SAMPLE_RATE_EXACT / 2;
    phase_increment_new = freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT); // the new increment is applied after an interlude
}
void WaveSine::Amplitude(float gain_in)
{
    if (!playing) // if NOT playing the magnitude value is applied immediately
    {
        if (gain_in < 0)
            gain_magnitude = 0;
        else if (gain_in > 1.0)
            gain_magnitude = 1.0;
        else
            gain_magnitude = gain_in;
        magnitude = gain_magnitude * 65536.0;
    }
    else // while playing the magnitude value will be reached gradually
    {
        gain_magnitude_flag = true;
        gain_magnitude_target = gain_in;
        gain_magnitude_delta = (gain_magnitude_target - gain_magnitude) / 10000.0; // [gain]
    }
}
void WaveSine::Start(void)
{
    if (playing)
        gain_suspend_flag = true;
    else
    {
        go_on = true;
        gain_start_stop = 0.0;
        gain_start_stop_flag = true;
        playing = true;
        gain_magnitude_flag = false;
        gain_suspend_flag = false;
        gain_suspend = 1.0;
        phase_increment = phase_increment_new;
    }
}
void WaveSine::update(void)
{
    if (!playing)
        return;
    audio_block_t *block;
    uint32_t i, ph, inc, index, scale;
    int32_t val1, val2;
    block = allocate();
    if (block)
    {
        ph = phase_accumulator;
        inc = phase_increment;
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            index = ph >> 24;
            val1 = Sine_wave[index];
            val2 = Sine_wave[index + 1];
            scale = (ph >> 8) & 0xFFFF;
            val2 *= scale;
            val1 *= 0x10000 - scale;
            if (gain_suspend_flag)
            {
                if (gain_suspend > 0.001)
                    gain_suspend -= 0.001;
                else
                {
                    gain_suspend_flag = false;
                    gain_suspend = 1.0;
                    phase_increment = phase_increment_new;
                    gain_start_stop = 0.0;
                    gain_start_stop_flag = true;
                }
            }

            if (gain_magnitude_flag)
            {
                gain_magnitude += gain_magnitude_delta;
                magnitude = gain_magnitude * 65536.0;
                if (gain_magnitude_target - gain_magnitude < 0.00001)
                    gain_magnitude_flag = false;
            }
            if (gain_start_stop_flag)
                update_gain_start_stop();

            // Serial.print("gain_suspend :");  Serial.println(gain_suspend);

            block->data[i] = gain_start_stop * gain_suspend * ((((val1 + val2) >> 16) * magnitude) >> 16);
            ph += inc;
        }
        phase_accumulator = ph;
        transmit(block);
        release(block);
        return;
    }
    phase_accumulator += phase_increment * AUDIO_BLOCK_SAMPLES;
}
void WaveSine::Stop(void)
{
    go_on = false;
    gain_start_stop_flag = true;
}
void WaveSine::update_gain_start_stop(void)
{
    if (go_on)
    {
        if (gain_start_stop < 0.999)
            gain_start_stop += 0.001;
        else
        {
            gain_start_stop = 1.0;
            gain_start_stop_flag = false;
        }
    }
    else
    {
        if (gain_start_stop > 0.001)
            gain_start_stop -= 0.001;
        else
        {
            gain_start_stop = 0.0;
            gain_start_stop_flag = false;
            playing = false;
        }
    }
}