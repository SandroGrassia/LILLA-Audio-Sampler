/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once
#include <Arduino.h>


// MIDI MONITOR

extern uint8_t MM_midi_channel;
extern uint8_t MM_note_number;
extern uint8_t MM_velocity;
extern uint8_t MM_midi_controller;
extern uint8_t MM_midi_value;
extern uint8_t MM_pitch_bend_least;
extern uint8_t MM_pitch_bend_most;
extern uint8_t MM_least_bits;
extern uint8_t MM_most_bits;
extern uint8_t midi_message_received;
extern bool display_wait; // used in MIDI_MONITOR, SETUP