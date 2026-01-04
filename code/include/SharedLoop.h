/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "config.h"

static constexpr int NEW_LOOP = -1;
static constexpr int NO_TRACK = -1; // track virtuale per note suonate da tastiera nello stato MIDI_LOOP
static constexpr int MASTER_TRACK = 0;
static constexpr int TRACKS = 6;        // numero di track
static constexpr int LOOP_EVENTS = 40; // numero massimo di eventi in un track
static constexpr int LOOP_UI_A = 1;    // primo encoder prima fila
static constexpr int LOOP_UI_B = 9;    // primo encoder seconda fila
static constexpr int LOOP_UI_C = 17;   // primo encoder terza fila
static constexpr int LOOP_metro_leds = 4; // 4 = 4/4
static constexpr int MIDI_LOOP_FILES = 1000;

struct LOOP_struct // verificata 12 byte 
{
    int time;    // ms
    uint8_t midi_channel;
    uint8_t note_number;
    uint8_t velocity;
    bool note_on;
};
extern LOOP_struct LOOP_element[TRACKS][LOOP_EVENTS];
extern byte LOOP_events[TRACKS];
extern int LOOP_slide[TRACKS]; // slittamento temporale in ms
extern int LOOP_pitch_int[TRACKS]; // slittamento pitch -400....0....+400
extern float LOOP_stretch; // stretch comune a tutti i track

// LOOP play/stop
extern bool LOOP_track_run[TRACKS]; // se "true" il track e' in esecuzione
extern int LOOP_play_event[TRACKS]; // indice del prossimo evento da eseguire
extern uint32_t LOOP_play_time[TRACKS]; // (ms) istante di esecuzione del prossimo evento da eseguire rispetto a LOOP_Clock
extern float LOOP_volume[TRACKS];
extern uint16_t LOOP_time; // (ms) durata del track master (0) comune a tutti i track

// LOOP learn
extern int LOOP_learning_track;
extern bool LOOP_learn_flag;
extern int LOOP_elements;
extern elapsedMillis LOOP_learn_clock; // utilizzato per calcolare la durata di track learn
extern int LOOP_clock_memo;
extern int LOOP_last_event;


// LOOP_metronomo
extern bool LOOP_metronomo_run; // se "true" i led del metronomo sono visualizzati

// LOOP metronomo, richieste da MidiReader a Main
extern bool LOOP_metronomo_flag_IN[2]; // accendi led_0, switch led del metronomo

// LOOP metronomo, richieste da Main a MidiReader
// extern bool LOOP_metronomo_flag_OUT; // run metronomo
// extern unsigned long LOOP_metronomo_update_time; // prossimo istante di aggiornamento

// LOOP Menu
static constexpr int LOOP_MV = 4;
extern int Loop_menu_max;
extern uint8_t choice_loop_menu;
extern const char Menu_Loop_char[LOOP_MV][12];
extern const uint8_t dimension_voice_Menu_Loop[LOOP_MV];
extern uint8_t X_position_Menu_Loop[LOOP_MV]; // argument is position
extern bool Menu_Loop[LOOP_MV];
extern uint8_t element_Menu_Loop[LOOP_MV]; // argument is position
extern uint8_t position_Menu_Loop[LOOP_MV]; // argument is element

// LOOP funzioni
unsigned long LOOP_Clock_time_from_virtual_time(int T_evento); // definita in main.cpp
unsigned long LOOP_Clock(void); // definita in main.cpp

// LOOP salvataggio su SD
extern int LOOP_id; // loop_id actually displayed