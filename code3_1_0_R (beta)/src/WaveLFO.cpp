/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// WaveLFO.h does NOT inherit from AudioStream.h

#include "WaveLFO.h"

void WaveLFO::Update(void)
{
    if (periodic == 0) // aperiodic wave
    {
        for (auto i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            if (wave_end)
            {
                block[i] = local_waveform[10];
            }

            else
            {
                int I_wave_point = floor(wave_point);
                block[i] = local_waveform[I_wave_point] + (local_m_waveform[I_wave_point] * (wave_point - I_wave_point));
                
                wave_point += wave_step;
                if (wave_point >= 10.0)
                {
                    wave_end = true;
                }
            }
        }
        // Serial.print("LFO - block[0]: ");
        // Serial.println(block[0]);
    }

    else if (periodic == 1) // periodic wave (sinus)
    {
        // execution: 2micros @600MHz
        uint32_t phase;
        uint32_t inc;
        uint32_t index;
        uint32_t scale;
        int32_t val1;
        int32_t val2;

        // variazione di ampiezza massima
        if (J_counter > 0)
        {
            if (step_magnitude)
            {
                ++magnitude;
            }

            else
            {
                --magnitude;
            }

            J_counter--;
        }

        phase = phase_accumulator;
        inc = phase_increment;
        for (auto i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            index = phase >> 24;     // index prende gli 8 bit alti di phase
            val1 = Sine_wave[index]; // Sine_wave[] is defined in Lilla_Definitions.h
            val2 = Sine_wave[index + 1];
            scale = (phase >> 8) & 0xFFFF; // scale prende i 16 bit alti di phase
            val2 *= scale;
            val1 *= 0x10000 - scale;
            block[i] = multiply_32x32_rshift32(val1 + val2, magnitude); // ((((val1 + val2) >> 16) * LFO_magnitude) >> 16) ;
            phase += inc;
        }
        phase_accumulator = phase;
        // Serial.print("LFO id: ");
        // Serial.print(identity);
        // Serial.print("  - first value: ");
        // Serial.println(block[0]);
    }
}

void WaveLFO::Set_phase(int angle)
{
    angle = constrain(angle, 0, 359);
    if (angle != phase_360)
    {
        phase_360_delta = angle - phase_360;
        phase_accumulator_delta = (phase_360_delta / 360.0) * 4294967296.0;
        phase_accumulator += phase_accumulator_delta;
        phase_360 = angle;
    }
}

void WaveLFO::Set_frequency(float freq) // sinus wave
{
    periodic = true;
    if (freq < 0.0)
    {
        freq = 0.0;
    }
    else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2.0f)
    {
        freq = AUDIO_SAMPLE_RATE_EXACT / 2.0f;
    }
    phase_increment = freq * (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT); // the new increment is applied after an interlude
}

void WaveLFO::Set_amplitude(int max_value) // sinus wave
{
    // Serial.print("LFO id: ");
    // Serial.print(identity);
    periodic = true;
    phase_accumulator = 0;

    if (max_value < 0)
    {
        max_value = 0;
    }
    else if (max_value > 32767)
    {
        max_value = 32767;
    }

    magnitude = 2 * max_value; // peak to peak amplitude
}

void WaveLFO::Update_amplitude(float max_value) // sinus wave
{
    if (max_value < 0)
    {
        max_value = 0;
    }
    else if (max_value > 32767)
    {
        max_value = 32767;
    }

    int delta_magnitude = (2 * max_value) - magnitude;

    if (abs(delta_magnitude) >= 1)
    {
        step_magnitude = (delta_magnitude > 0);
        J_counter = abs(delta_magnitude);
    }
    else // if running, stop amplitude change
    {
        J_counter = 0;
    }
}

void WaveLFO::Setup_aperiodic_wave(float seconds, float max_value, int waveform) // aperiodic wave - period: seconds  n: waveform
{
    periodic = false;

    if (seconds < 0.1)
    {
        seconds = 0.1;
    }
    wave_time = seconds;                            // seconds
    double samples = wave_time * AUDIO_SAMPLE_RATE; // total sample from 0 (start point) to 10 (end point)
    wave_step = 10.0 / samples;                     // fraction of a step for each sample-out
    wave_point = 0.0;
    wave_end = false;

    if (max_value < 0)
    {
        max_value = 0;
    }
    else if (max_value > 32767)
    {
        max_value = 32767;
    }

    if (waveform == 1) // rising (positive values)
    {
        for (auto i = 0; i <= 10; ++i)
        {
            local_waveform[i] = max_value * waveform_attack[i];
        }
        for (auto i = 0; i < 10; ++i)
        {
            local_m_waveform[i] = max_value * (waveform_attack[i + 1] - waveform_attack[i]);
        }
    }

    if (waveform == 2) // falling (negative values)
    {
        for (auto i = 0; i <= 10; ++i)
        {
            local_waveform[i] = -max_value * waveform_attack[i];
        }
        for (auto i = 0; i < 10; ++i)
        {
            local_m_waveform[i] = -max_value * (waveform_attack[i + 1] - waveform_attack[i]);
        }
    }

    // Serial.print("LFO received --> seconds:");
    // Serial.print(seconds);
    // Serial.print(" amplitude:");
    // Serial.print(max_value);
    // Serial.print(" waveform:");
    // Serial.println(waveform);
}