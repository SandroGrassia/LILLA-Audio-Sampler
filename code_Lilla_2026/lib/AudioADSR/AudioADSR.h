/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <AudioStream.h> // contains AUDIO_SAMPLE_RATE definition
#include "config.h"
#include "SharedElements.h"

class AudioADSR
{
private:
    int identity;

    float Attack = 0;
    float Decay = 0;
    float Sustain = 0;
    float Release = 0;
    uint8_t Attack_type;

    enum ADSR_Phase
    {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
        STOP
    };
    ADSR_Phase phase; // 0: Attack, 1: Decay, 2: Sustain, 3: Release
    float ADSR_point;   // index, from 0.0 to 10.0
    float ADSR_point_0; // last index, from 0.0 to 10.0
    float ADSR_gain;
    float ADSR_gain_0;
    float J1;
    float J2;
    float DJ3_4; // indexes of samples for ADSR filter
    float K_Attack_step;
    float K_Dacay_step;
    float K_Sustain_delta;
    float K_Release_step;
    float K_Release_delta;

public:
    AudioADSR(void) {}
    void Set_identity(int id);
    int Get_identity(void);
    void Setup(const float &attack, const float &decay, const float &sustain, const float &release, const uint8_t &attack_type);
    void Set_parametrs(void);
    float Get_gain(void);
    void Release_note(void);
    void Fast_stop(void);
    uint8_t Get_phase(void);
    
};