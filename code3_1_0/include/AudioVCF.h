/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// AudioVCF.h does NOT inherit from AudioStream.h

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