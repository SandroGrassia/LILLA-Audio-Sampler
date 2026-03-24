/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <SerialFlash.h>
#include "LillaSerialFlash.h"
#include "SharedElements.h"
#include "DisplayPrimitives.h"
#include "SharedLS.h"
#include "SharedVFS.h"
#include "StereoLiveSampler.h"
#include "config.h"

class InfoMaster
{
private: 
    static constexpr int BASKET_INFO = 500; // samples_basket[] dimension
    int16_t samples_basket[BASKET_INFO];
    int16_t samples_620_array[2 * DisplayPrimitives::WAVEBOARD_WIDTH]; // for each of the 128 columns of display, the method gives the max positive value and the min negative value.

    // Live Sampling
    int16_t* FIFO;
    int FIFO_dim;
    bool LS_antiflicker = false;
    int last_A_window_sample;
    int last_samples_per_line;

    void Read_samples(int file_id, int16_t *destination, int seek_in, int samples_in); // samples_in <= BASKET_INFO

public:
    InfoMaster (void) {}

    StereoLiveSampler* LiveSampler_ptr = nullptr;  // Info._LiveSampler = &LiveSampler;
    int16_t* LS_buffer_mono_ptr = nullptr;
    int16_t* LS_buffer_L_ptr = nullptr;
    int16_t* LS_buffer_R_ptr = nullptr;

    // Samples in mono recording or in Left channel's recording
    int DS_recording_samples(int recording);
    int Raw_file_samples(int file_id);
    int16_t* Sound_620_samples_array(int file_id, uint32_t A, uint32_t B);
    int16_t* LS_620_samples_array(int file_id, int A_window_sample, int B_window_sample);
    void LS_restart_antiflicker(void); // chiamata da main.cpp quando si sceglie LS_XY_lock o quando con LS_XY_lock si modifica LS_A_window, o LS_window_width, o si 
};