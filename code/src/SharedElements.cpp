/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedElements.h"

// SETUP
int key_step;
uint8_t optimization;
int8_t first_octave;

// GESTIONE DELLA MEMORIA FLASH ESTERNA
int flash_dimension_MB;

// LILLA STATE
uint8_t Lilla_state;
uint8_t Lilla_state_0;

// PATCH
Patch_struct Patch[PATCHES_MAX + 1]; // local copy of patches_number
Sound_struct Sound[SOUNDS_MAX + 2];

// PERFORMANCE
uint8_t Patch_id = 0;
int volume_patch = 29;
uint8_t map_instrument_for_note[16][128] = {0};
bool key_state[16][128] = {0}; // usato solo a fini statistici; key premuti su ciascun canale midi; rilevato attaverso il conteggio dei NoteOn
bool file_midi_ch_flag;
int8_t position_of_Instrument[INSTRUMENTS_MAX];
float pitch_from_note[128] = {0};
bool display_instrument_volume_flag;
uint8_t instrument_volume_changed;

// menu
bool instrument_editing_flag = false;
uint8_t choice_performance_menu;
const char Menu_P_char[5][12] = {{"EXIT"}, {"SAVE"}, {"CLONE"}, {"SAVE_AS_NEW"}, {"DROP"}};
const uint8_t dimension_voice_Menu_P[5] = {4, 4, 5, 11, 4};
bool Menu_P[5];
int X_position_Menu_P[5];   // argument is position
int Y_position_Menu_P[5];   // argument is position
uint8_t element_Menu_P[5];  // argument is position
uint8_t position_Menu_P[5]; // argument is element

// Tuning tone
int tuning_tone_volume = 10;
uint8_t tuning_tone_last_note = 0;
bool tuning_tone_flag = false;
bool TT_playing = false;
bool TT_led_flag = false;

// SOUND EDIT
uint8_t trim_speed = 5;
uint16_t Noclick_max;
bool solo_flag = false;
bool slicing_mode = true; // true: slicing AB  - false: slicing A-Samples
const char name_mode[6][8] = {{"FWD"}, {"REV"}, {"FWD"}, {"FWD-REV"}, {"REV-FWD"}, {"REV"}};
char loop_mode[6][5] = {{"once"}, {"once"}, {"loop"}, {"loop"}, {"loop"}, {"loop"}};

// Menu
uint8_t choice_sound_menu;
const char Menu_So_char[3][7] = {{"RETURN"}, {"CLONE"}, {"DROP"}};
const uint8_t dimension_voice_Menu_So[3] = {6, 5, 4};
uint8_t X_position_Menu_So[3]; // argument is position
uint8_t element_Menu_So[3];    // argument is position
uint8_t position_Menu_So[3];   // argument is element
bool Menu_So[3];

// array compilati al setup()
float m_exp_table[10];
float m_sin_table[10];
float m_decay_table[10];
float m_release_table[10];
float pan_gain_L_table[33];
float pan_gain_R_table[33];

// funzioni
uint8_t Get_midi_channel(int patch_id, int instrument_id)
{
    // .data contains midi channel in its bits: 7 6 5 M I D I 0
    return ((Sound[Patch[patch_id].Instrument[instrument_id].sound_id].data & 30) >> 1);
}

// PRESET
Preset_struct Preset[INSTRUMENTS_MAX];

// PLAYER
elapsedMicros security_timer = 0;
int16_t *Noclick_pointer[INSTRUMENTS_MAX] = {0};   // each Noclick instance contains 2 arrays
int16_t *Wavetable_pointer[INSTRUMENTS_MAX] = {0}; // each Wavetable instance contains 2 arrays

// funzioni
void Update_map_Instrument_for_notes(int from_note, int to_note, int instrument_id) // aggiorna la mappatura tra tutte Instrument e le coppie midi_channel/note_number e relative
{
    for (auto note = 0; note < 128; ++note)
    {
        if (note >= from_note && note <= to_note)
        {
            bitWrite(map_instrument_for_note[Get_midi_channel(Patch_id, instrument_id)][note], instrument_id, 1);
        }
        else
        {
            bitWrite(map_instrument_for_note[Get_midi_channel(Patch_id, instrument_id)][note], instrument_id, 0);
        }
    }
}

void Reset_keys_state()
{
    for (auto a = 0; a < 16; ++a)
    {
        for (auto b = 0; b < 128; ++b)
        {
            key_state[a][b] = false; // true: key pressed down
        }
    }
}

// LPF FILTER RESOLUTION DOWNSAMPLING
bool lowpass_flag;
bool lowpass_direction;
int lowpass;
int lowpass_target;
bool display_lowpass_flag;
int resolution;  // [0, 79] 0: risoluzione 16bit
int downsampling; // n. of repeated samples  1 = 44.1ksps

// PITCH BEND - AFTER TOUCH - VIBRATO
float pitch_bend_value[16] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
float after_touch_channel_value[16] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

// CONTROL CHANGE
uint8_t CC_Sound_gain[INSTRUMENTS_MAX] = {0};
uint8_t CC_lowpass_filter;
uint8_t CC_midi_controller;

// STAMPA
void PRINT(String who, String what, float value)
{
    Serial.print(who);
    Serial.print(" ");
    Serial.print(what);
    Serial.print(": ");
    Serial.println(value);
}