/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// Midi_Reader
// Credits: Francois Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)

#pragma once

#include <Arduino.h>
#include <MIDI.h>
#include "config.h"

extern MIDI_NAMESPACE::SerialMIDI<HardwareSerial> serialMIDI;                          // Define your transport - by Francois Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)
extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI; // Define the MIDI interface - by Francois Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)

class MidiOut
{
public:
    MidiOut(void) {}

    void NoteOn(const int &note, const int &velocity, const int &channel);
    void NoteOff(const int &note, const int &velocity, const int &channel);
    void ProgramChange(const int &programNumber, const int &channel);
    void ControlChange(const int &controlNumber, const int &controlValue, const int &channel);
    void PitchBend(const int &value, const int &channel); // -8192 <= value <= 8191
    void PolyPressure(const int &note, const int &pressure, const int &channel);
    void AfterTouch(const int &pressure, const int &channel);
};