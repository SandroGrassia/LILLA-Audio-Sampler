/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

/*
*  questa classe utilizza receiveWritable quindi gli oggetti in ingresso
*  NON DOVREBBERO avere altri collegamenti
*/

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <utility/dspinst.h>
#include "SharedElements.h"
#include "Functions.h"
#include "config.h"

class StereoGain : public AudioStream
{
private:
    void UPDATE_gain(void);
    
    audio_block_t *inputQueueArray[2] = {nullptr};
    audio_block_t *out_block_L = nullptr;
    audio_block_t *out_block_R = nullptr;

    float gain_L = 0;
    float gain_R = 0;
    float gain_delta_L = 0;
    float gain_delta_R = 0;
    bool gain_flag_L = false;
    bool gain_flag_R = false;
    int gain_step_L = 0;
    int gain_step_R = 0;

    int32_t cache;    
    
public:
    StereoGain(void) : AudioStream(2, inputQueueArray) {}

    virtual void update(void);
    void Set_gain(float value);
    void set_gain_L(float value);
    void set_gain_R(float value);
};