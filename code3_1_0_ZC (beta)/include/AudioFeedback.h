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
 */

// Lilla_LiveSampler_Feedback offers:
// - output0 = input0 + gain_float*input1
// - smooth gain_float changes

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <utility/dspinst.h>
#include "SharedElements.h"

class AudioFeedback : public AudioStream
{
private:
    static constexpr int MULTI_UNITYGAIN = 65536;
    audio_block_t *inputQueueArray[2];
    int32_t gain = 0;        // from 0 (nessun feedback) to MULTI_UNITYGAIN 65536 (feedback massimo / oscillatore)
    int32_t gain_target = 0; // nessun feedback
    int32_t gain_delta;
    int32_t gain_local;
    int gain_step;
    bool change_gain_flag = false;
    bool book_change_gain_flag = false;

    int32_t mult_1;
    int32_t mult_2;

    void Apply_gain_then_add(int16_t *data, const int16_t *in);
    void Apply_changing_gain_than_add(int16_t *data, const int16_t *in);
    void Get_mults(void);

public:
    AudioFeedback(void) : AudioStream(2, inputQueueArray) {}

    virtual void update(void);
    void value(float gain_in);
    uint8_t identity;
};