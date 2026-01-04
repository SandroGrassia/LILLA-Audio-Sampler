/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "config.h"


// LIVE SAMPLING

// Virtual tape
/*
Dalla PSRAM totale si esclude spazio per i puntatori; lo spazio utile deve poter essere divisibile per  2 (L/R) poi per 256 (128 samples)
PSRAM_16MB --> 16777216 byte; escludiamo spazio per i puntatori (4*AUDIO_BLOCK_SAMPLES) --> 16776704 byte pari a 190sec; dedichiamo 1724*256=441344 byte (circa 5sec) per ciascun canale del Delay, restano 15894016 byte per Live Sampling
*/
static constexpr int LS_MONO_SAMPLES = 7946752;
static constexpr uint32_t LS_MONO_BYTES = LS_MONO_SAMPLES << 1; // 0xf28400 - 15.893.504
static constexpr int LS_STEREO_SAMPLES = 3973376; 
static constexpr uint32_t LS_STEREO_BYTES = LS_STEREO_SAMPLES << 1; // 0x794200 - decimale 7.946.752
static constexpr int FIRST_LIVE_SAMPLING_FILE = 320;

// Menu
constexpr int LS_MV = 4;
extern bool Menu_LS[LS_MV];
extern const char Menu_LS_char[][12];
extern const uint8_t dimension_voice_Menu_LS[LS_MV];
extern uint8_t X_position_Menu_LS[LS_MV]; // argument is position
extern uint8_t element_Menu_LS[LS_MV];    // argument is position
extern uint8_t position_Menu_LS[LS_MV];   // argument is element

// variabili
constexpr float LS_fbk_table[9] = {0, 0.01, 0.03, 0.07, 0.1, 0.2, 0.4, 0.8, 0.9};
extern int LS_state;
extern int LS_feedback; // feedback interno al Live Sampler
extern bool LS_stereo;
extern int LS_buffer_dim;
extern uint8_t LS_mode; // play mode
extern int LS_window_width; // samples from LS_window_A_sample to LS_window_B_sample

extern int LS_Q_sample; // ultima posizione registrata su LS_buffer_L/R
extern bool LS_XY_lock; // play bloccato sul virtual tape
extern int LS_X_step; // step di avanzamento
extern int LS_X_delta; // distanza tra LS_Q_sample e LS_X_sample
extern int LS_X_sample; // posizione di partenza play
extern int LS_Y_sample; // posizione di fine loop 
extern int LS_XY_delta; // distanza tra LS_X_sample e LS_Y_sample

// waveform
static constexpr int WAVE_WIDTH = 310;
int LS_constrain_position(int value);