/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedMM.h"

// MIDI MONITOR

uint8_t MM_midi_channel = 0;
uint8_t MM_note_number = 0;
uint8_t MM_velocity = 0;
uint8_t MM_midi_controller = 0;
uint8_t MM_midi_value = 0;
uint8_t MM_pitch_bend_least = 0;
uint8_t MM_pitch_bend_most = 0;
uint8_t MM_least_bits = 0;
uint8_t MM_most_bits = 0;
uint8_t midi_message_received = 0;
bool display_wait = false; // used in MIDI_MONITOR, CC_SETTING