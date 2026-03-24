/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedLS.h"

// LIVE SAMPLING

bool Menu_LS[LS_MV];
const char Menu_LS_char[LS_MV][12] = {{"REC"}, {"STOP"}, {"MONO/STEREO"}, {"ERASE"}};
const uint8_t dimension_voice_Menu_LS[LS_MV] = {3, 4, 11, 5};
uint8_t X_position_Menu_LS[LS_MV]; // argument is position
uint8_t element_Menu_LS[LS_MV];    // argument is position
uint8_t position_Menu_LS[LS_MV];   // argument is element


LS_States LS_state;
int LS_feedback;
uint8_t LS_mode; // playing mode
int LS_X_step;
int LS_window_width; // samples from LS_window_A_sample to LS_window_B_sample
bool LS_stereo;
int LS_Y_sample;
int LS_Q_sample;
int LS_buffer_dim;
bool LS_XY_lock;
int LS_X_sample;
int LS_X_delta;
int LS_XY_delta;

// waveform
int LS_constrain_position(int value)
{
    while (value < 0)
        value += LS_buffer_dim;
    while (value > LS_buffer_dim - 1)
        value -= LS_buffer_dim;
    return value;
}