/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <AudioStream.h>

class AudioPeakDetector : public AudioStream
{
public:
    AudioPeakDetector(void) : AudioStream(1, inputQueueArray)
    {
        min_sample = 32767;
        max_sample = -32768;
    }
    virtual void update(void);
    bool available(void);
    float read(void);
    void reset();

private:
    audio_block_t *inputQueueArray[1] = {nullptr};
    volatile bool new_output = false;
    int16_t min_sample = 0;
    int16_t max_sample = 0;
};