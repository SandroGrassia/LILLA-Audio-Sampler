/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <utility/dspinst.h>

class AudioMultiplier : public AudioStream
{
public:
    AudioMultiplier(void) : AudioStream(2, inputQueueArray) {}
    
    virtual void update(void);

private:
    audio_block_t *inputQueueArray[2] = {nullptr};
};