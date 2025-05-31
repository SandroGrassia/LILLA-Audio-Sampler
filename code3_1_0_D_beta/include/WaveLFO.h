/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// WaveLFO.h does NOT inherit from AudioStream.h

#pragma once

#include <Arduino.h>
#include <utility/dspinst.h>
#include <AudioStream.h>
#include "SharedElements.h"

class WaveLFO
{
private:
    bool periodic = true;

    // sinus wave
    int phase_360 = 0;
    int phase_360_delta = 0;
    uint32_t phase_accumulator = 0;
    uint32_t phase_accumulator_delta = 0;
    float amplitude = 1000.0f;
    int32_t magnitude = 2000; // TEST max value
    bool step_magnitude = false;
    uint32_t phase_increment = 0;
    int J_counter = 0;

    // aperiodic wave
    float wave_time = 0.0;
    double wave_step = 0.0;
    double wave_point = 0.0;
    bool wave_end = false;
    double local_waveform[11] = {0};
    double local_m_waveform[10] = {0};

public:
    WaveLFO(void) {}

    int16_t block[AUDIO_BLOCK_SAMPLES];
    int identity = 100;

    void Update(void);
    void Set_phase(int angle);
    void Set_frequency(float freq);                                          // sinus wave
    void Set_amplitude(int max_value);                                       // sinus wave
    void Update_amplitude(float max_value);                                  // sinus wave
    void Setup_aperiodic_wave(float seconds, float max_value, int waveform); // aperiodic wave - period: seconds  n: waveform
};