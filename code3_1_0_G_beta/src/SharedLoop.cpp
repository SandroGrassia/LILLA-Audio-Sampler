/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedLoop.h"



// MIDI LOOP

LOOP_struct LOOP_element[TRACKS][LOOP_EVENTS];
byte LOOP_events[TRACKS] = {0};
int LOOP_slide[TRACKS] = {0};
float LOOP_stretch = 1.0;

// Menu
int Loop_menu_max;
uint8_t choice_loop_menu;
const char Menu_Loop_char[4][12] = {{"NEW"}, {"SAVE"}, {"SAVE_AS_NEW"}, {"DELETE"}};
const uint8_t dimension_voice_Menu_Loop[4] = {3, 4, 11, 6};
uint8_t X_position_Menu_Loop[4]; // argument is position
bool Menu_Loop[4];
uint8_t element_Menu_Loop[4];  // argument is position
uint8_t position_Menu_Loop[4]; // argument is element

// LOOP play/stop
bool LOOP_track_run[TRACKS] = {false};
uint32_t LOOP_play_time[TRACKS] = {0};
int LOOP_play_event[TRACKS] = {0};
float LOOP_volume[TRACKS] = {0};
uint16_t LOOP_time;
int LOOP_pitch_int[TRACKS] = {0};

// LOOP learn
int LOOP_learning_track;
bool LOOP_learn_flag;
int LOOP_elements;
elapsedMillis LOOP_learn_clock;
int LOOP_clock_memo;
int LOOP_last_event;

// LOOP leds
bool LOOP_led_flag[TRACKS] = {false};

// LOOP_metronomo
bool LOOP_metronomo_run = false; // il metronomo e' running

// LOOP metronomo, richieste da MidiReader a Main
bool LOOP_metronomo_flag_IN[2] = {false, false}; // accendi led_0, switch led del metronomo

// LOOP metronomo, richieste da Main a MidiReader
// bool LOOP_metronomo_flag_OUT = {false}; // run metronomo
// unsigned long LOOP_metronomo_update_time = 0;  // prossimo istante di aggiornamento

// LOOP salvataggio su SD
int LOOP_id;