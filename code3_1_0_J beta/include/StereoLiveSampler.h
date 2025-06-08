/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// StereoLiveSampler registra su file in PSRAM

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <SerialFlash.h>
#include "SharedLS.h"

class StereoLiveSampler : public AudioStream
{
private:
    audio_block_t *inputQueueArray[2] = {nullptr};
    audio_block_t *in_block_L = nullptr;
    audio_block_t *in_block_R = nullptr;
    bool cancel_beyond;
    volatile bool writing;
    bool attack_gain_flag = false;
    bool decay_gain_flag = false;
    bool book_stop_flag = false;
    int samples_counter = 0;
    static constexpr float Slope_samples = 3 * AUDIO_BLOCK_SAMPLES;
    bool stereo;
    int16_t Buffer_L[AUDIO_BLOCK_SAMPLES];
    int16_t Buffer_R[AUDIO_BLOCK_SAMPLES];
    // elapsedMicros microtimer;

public:
    // input: 0 (Left), 1 (Right)
    StereoLiveSampler(void) : AudioStream(2, inputQueueArray)
    {
        Begin();
    }
 
    virtual void update(void);
    void Begin(void);
    void Reset(void);
    void Start(bool stereo_in);
    void Stop(void);
    bool Is_writing(void);
    void Set_cancel_beyond(bool value);

    bool first_write_flag = true;
    int Q_sample;
    int16_t *LS_buffer_mono_ptr = nullptr;
    int16_t *LS_buffer_L_ptr = nullptr;
    int16_t *LS_buffer_R_ptr = nullptr;
};