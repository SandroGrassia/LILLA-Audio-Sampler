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

// AudioVCF.h does NOT inherit from AudioStream.h (unlike original filter_biquad.h)

#pragma once

#include <Arduino.h>
#include <utility/dspinst.h>
#include <AudioStream.h>

class AudioVCF
{
private:
    // by default, the filter will pass NOTHING
    int32_t definition[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // up to 4 cascaded biquads

public:
    AudioVCF(void) {}

    int filter_type = 0; // 0:Lowpass  0 --> 3
    bool LFO_connect = false;
    float q_value = 0.7071;
    float *_frequency = NULL;
    int16_t *_block = NULL; // block[AUDIO_BLOCK_SAMPLES];

    void Update(void);
    void setCoefficients(uint32_t stage, const int *coefficients); // Set the biquad coefficients directly
    void setCoefficients(uint32_t stage, const double *coefficients);
    // http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
    // execution: 3micros @600MHz
    void Set_filter(uint32_t stage, float frequency);
};