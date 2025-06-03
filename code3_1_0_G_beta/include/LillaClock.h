/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include "SharedElements.h"
#include "MidiReader.h"
#include "FilterBiquadManager.h"
#include "DelayManager.h"

class LillaClock : public AudioStream
{
private:
    bool stop_flag = true;

public:
    LillaClock(void) : AudioStream(0, NULL) {}

    uint8_t identity;
    MidiReader *_Midi_reader = nullptr;
    FilterBiquadManager *_Filter_Biquad_Manager = nullptr;
    DelayManager *_Delay_Manager = nullptr;

    void Start(void);
    void Stop(void);
    virtual void update(void);
};