/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
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
    MidiReader *Midi_reader_ptr = nullptr;
    FilterBiquadManager *Filter_Biquad_Manager_ptr = nullptr;
    DelayManager *Delay_Manager_ptr = nullptr;

    void Start(void);
    void Stop(void);
    virtual void update(void);
};