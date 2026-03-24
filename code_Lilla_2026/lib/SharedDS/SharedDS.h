/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>

// DIRECT SAMPLING

// bar VU-meter
static constexpr int BAR_ELEMENTS = 50; // barre del VU-meter stereo

static constexpr int DS_MV = 12; // elementi di Menu_DS[]
extern bool Menu_DS[DS_MV];
extern const char Menu_DS_char[DS_MV][19];
extern const uint8_t dimension_voice_Menu_DS[DS_MV]; // dimensione degli elementi di Menu_DS[] 
extern uint8_t X_position_Menu_DS[DS_MV]; 
extern uint8_t Y_position_Menu_DS[DS_MV]; 
extern uint8_t element_Menu_DS[DS_MV]; 
extern uint8_t position_Menu_DS[DS_MV];
extern uint8_t choice_DS_menu;

extern int recordings;
extern int recording;
extern int DS_gain;
extern elapsedMillis DS_blink_timer;
extern bool DS_blink_ON;
