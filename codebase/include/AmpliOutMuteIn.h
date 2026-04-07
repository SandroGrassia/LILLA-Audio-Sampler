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

// Lilla_AmpliOut_MuteIn offers:
// - smooth output gain change
// - smooth Mute/unmute inputs
// - smooth inputs gain change

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <utility/dspinst.h>

// #define SAMPLES_VOLUME 1000 // deve essere pari

class AmpliOutMuteIn : public AudioStream
{
private:
    static constexpr uint32_t MULTI_UNITYGAIN = 65536;
    audio_block_t *inputQueueArray[4] = {nullptr};
    int32_t multiplier[4] = {MULTI_UNITYGAIN};
    bool gain_flag[4] = {false};
    float gain_runtime[4] = {1.0};
    float gain_runtime_mem[4] = {0};
    float gain_delta[4] = {0};
    int gain_step[4] = {0};
    int gain_samples[4] = {0};
    bool muted[4] = {false};

    int32_t multiplier_out = MULTI_UNITYGAIN;
    bool gain_flag_out = false;
    float gain_runtime_out = 1.0;
    float gain_delta_out = 0;
    int gain_step_out = 0;
    int gain_samples_out = 0;

    int32_t mult_1 = 0;
    int32_t mult_2 = 0;

    void Apply_gain(const int  channel, int16_t *data, const int32_t  mult);
    void Apply_gain_then_add(const int  channel, int16_t *data, const int16_t *in, const int32_t  mult);
    void Get_mults();
    void Get_mults(const int  channel);
    void Apply_gain(int16_t *data, const int32_t  mult);
    
    
public:
    AmpliOutMuteIn(void) : AudioStream(4, inputQueueArray) {}

    virtual void update(void);
    void gain(int channel, float value);
    void gain(float value);
    void Mute(int channel);
    void unmute(int channel);
};