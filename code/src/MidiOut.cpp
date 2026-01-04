/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// Midi_Reader
// Credits: Francois Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)

#include <MidiOut.h>

void MidiOut::NoteOn(const int &note, const int &velocity, const int &channel)
{
    MIDI.sendNoteOn(note, velocity, channel);
}

void MidiOut::NoteOff(const int &note, const int &velocity, const int &channel)
{
    MIDI.sendNoteOff(note, velocity, channel);
}

void MidiOut::ProgramChange(const int &programNumber, const int &channel)
{
    MIDI.sendProgramChange(programNumber, channel);
}

void MidiOut::ControlChange(const int &controlNumber, const int &controlValue, const int &channel)
{
    MIDI.sendControlChange(controlNumber, controlValue, channel);
}

void MidiOut::PitchBend(const int &value, const int &channel) // -8192 <= value <= 8191
{
    MIDI.sendPitchBend(value, channel);
}

void MidiOut::PolyPressure(const int &note, const int &pressure, const int &channel)
{
    MIDI.sendPolyPressure(note, pressure, channel);
}

void MidiOut::AfterTouch(const int &pressure, const int &channel)
{
    MIDI.sendAfterTouch(pressure, channel);
}
