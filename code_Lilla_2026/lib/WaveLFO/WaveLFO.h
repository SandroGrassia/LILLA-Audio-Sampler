/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 /*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// WaveLFO.h does NOT inherit from AudioStream.h

#pragma once

#include <Arduino.h>
#include <utility/dspinst.h>
#include <AudioStream.h>
#include "SharedElements.h"
#include "config.h"

class WaveLFO
{
private:
    bool periodic = true;

    // sinus wave
    int phase_360 = 0;
    int phase_360_delta = 0;
    uint32_t phase_accumulator = 0;
    uint32_t phase_accumulator_delta = 0;
    float amplitude = 1000.0f;
    int32_t magnitude = 2000; // TEST max value
    bool step_magnitude = false;
    uint32_t phase_increment = 0;
    int J_counter = 0;

    // aperiodic wave
    float wave_time = 0.0;
    double wave_step = 0.0;
    double wave_point = 0.0;
    bool wave_end = false;
    double local_waveform[11] = {0};
    double local_m_waveform[10] = {0};

public:
    WaveLFO(void) {}

    int16_t block[AUDIO_BLOCK_SAMPLES];
    int identity = 100;

    void Update(void);
    void Set_phase(int angle);
    void Set_frequency(float freq);  // sinus wave
    void Set_amplitude(int max_value); // sinus wave
    void Update_amplitude(float max_value); // sinus wave
    void Setup_aperiodic_wave(float seconds, float max_value, int waveform); // aperiodic wave - period: seconds  n: waveform
};