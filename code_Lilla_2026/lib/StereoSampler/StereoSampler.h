/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// StereoSampler.h registra sul "virtual file system" (VFS) realizato nella Flash memory

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <SerialFlash.h>
#include <spi_interrupt.h>
#include "SharedDS.h"
#include "LillaSerialFlash.h"
#include "ArchivingManager.h"
#include "config.h"

class StereoSampler : public AudioStream
{
private:
    void stop(void);

    audio_block_t *inputQueueArray[2] = {nullptr};
    audio_block_t *in_block_L = nullptr;
    audio_block_t *in_block_R = nullptr;

    volatile bool recording;
    int recording_id = 0;          // equivalent to file_id
    LillaSerialFlashFile Packet_L; // SerialFlashFile Packet_L;
    LillaSerialFlashFile Packet_R; // SerialFlashFile Packet_R;

    bool increment_packets_flag = false;
    bool stereo_flag = false;

    byte RAM_buffer_L[256] = {0}; // contains 1 AUDIO_BLOCK_SAMPLES packets
    byte RAM_buffer_R[256] = {0}; // contains 1 AUDIO_BLOCK_SAMPLES packets
    int16_t *RAM_buffer_L_ptr = (int16_t *)RAM_buffer_L;
    int16_t *RAM_buffer_R_ptr = (int16_t *)RAM_buffer_R;

    int packet_open_L = 0;
    int last_packet_L = 0;
    int packet_open_R = 0;
    int last_packet_R = 0;

    bool attack_gain_flag = false;
    bool decay_gain_flag = false;
    bool book_stop_flag = false;
    int samples_counter = 0;
    static constexpr float Slope_blocks = 3.0;
    static constexpr float F_Slope_samples = Slope_blocks * AUDIO_BLOCK_SAMPLES;
    static constexpr int Slope_samples = Slope_blocks * AUDIO_BLOCK_SAMPLES;
    int countdown_blocks = 0;
    bool countdown_flag = false;
    elapsedMicros timer;

    // riferimenti esterni
    ArchivingManager &Archive;

public:
    StereoSampler(ArchivingManager &Archive) : AudioStream(2, inputQueueArray), Archive(Archive) // input: 0 (Left), 1 (Right)
    {
        Begin();
    }

    virtual void update(void);
    void Begin(void);
    bool Start(int from_packet, int last_packet, int recording_id_in, bool stereo_in);
    void Book_stop(void);
    bool Is_recording(void);
};