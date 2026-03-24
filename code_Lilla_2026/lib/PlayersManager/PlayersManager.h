/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedElements.h"
#include "SharedDelay.h"
#include "SharedMixer.h"
#include "AudioPlayer.h"
#include "LoopLedSet.h"
#include "PlayersStatistics.h"
#include "Router_16x3.h"
#include "NoclickCrossmix.h"
#include "WavetableManager.h"
#include "config.h"
#include "AudioADSR.h"

class PlayersManager
{
private:
    // puntatori esterni
    AudioPlayer *Player_ptr = nullptr;
    Router_16x3 *Router_L_ptr = nullptr;
    Router_16x3 *Router_R_ptr = nullptr;
    NoclickCrossmix *Noclick_ptr = nullptr;    // Players_Manager.Noclick_ptr = &Noclick[0]
    WavetableManager *Wavetable_ptr = nullptr; // Players_Manager.Wavetable_ptr = &Wavetable[0]
    AudioADSR *ADSR = nullptr;

    // statistiche
    int players_playing = 0;
    int players_using_Flash = 0;
    int players_using_Wavetable = 0;
    uint8_t players_to_restart = 0; // numero di Player che devono ripartire; la ripartenza richiede una doppia lettura di campioni da vecchio e nuovo file ed il calcolo di mix_samples fatto dalla funzione Calculate_and_set_mix_samples

    // Play notes
    bool Player_booked[PLAYERS] = {false};
    bool restart_Player[PLAYERS] = {false}; // questo array serve per contare, ad ogni ciclo, il numero di Player che devono ripartire; la ripartenza richiede una doppia lettura di campioni da vecchio e nuovo file ed il calcolo di mix_samples fatto dalla funzione Calculate_and_set_mix_samples

    static inline uint8_t Sound_Id(int patch_id, int instrument_id)
    {
        return Patch[patch_id].Instrument[instrument_id].sound_id;
    }

    static inline float Calc_pitch(float value)
    {
        return pow(2.0f, value / 192.0f); // 0: no shift
    }

    static inline float Calc_attack(float value)
    {
        return (value / 100.0f);
    }

    static inline float Calc_decay(float value)
    {
        return (value / 25.0f);
    }

    static inline float Calc_sustain(float value)
    {
        return (value / 50.0f);
    }

    static inline float Calc_release(float value)
    {
        return (value / 2.0f);
    }

public:
    PlayersManager(AudioPlayer *P, Router_16x3 *RL, Router_16x3 *RR, NoclickCrossmix *NC, WavetableManager *WT) : Player_ptr(P), Router_L_ptr(RL), Router_R_ptr(RR), Noclick_ptr(NC), Wavetable_ptr(WT) {}
    
    void Set_ADSR_ptr(AudioADSR* ptr); // requires &ADSR[0] from main.cpp

    // chiamate da MidiReader
    void Reset_booked_and_restart_player(void);
    void Reset_players_to_restart(void);

    /*
    Play_note, chiamata da:
    MidiReader a seguito di
    - NoteOn da MIDI
    - evento NoteOn da Midi Loop

    Compiti:
    - individua il Player (id_player) da utilizzare
    - se Player(id_player).isPlaying() incrementa players_to_restart, utilizzata da void Calculate_and_set_mix_samples()
    - chiama le funzioni di AudioPlayer che inizializzano Player(id_player) utilizzando i valori di Preset
    - chiama le funzioni di AudioPlayer che loro volta inizializzano immediatamente il VCF (anche se il Player(id_player) va in restart)
    - chiama la funzione AudioPlayer::Get_ready_to_play
    - aggiorna Router_L_ptr/R per instradamento verso Delay si/no
    - aggiorna PlayersStatistics per l'aggiornamento dei led degli Instrument
    - richiede a main() di aggiornare i led
    */

    void MX_multicast_change_routing(int instrument_id);
    void Play_note(uint8_t instrument_id, uint8_t note_number, float velocity_float, int track);

    int Get_players_to_restart(void);
    void Calculate_and_set_mix_samples(void);
    void Multicast_stop_players_for_NoteOff(int midi_channel, int note_number, int track);
    void Broadcast_pitch_bend(int midi_channel, float value);
    void Multicast_all_notes_off(int midi_channel);
    void Multicast_update_vibrato(int midi_channel, bool vibrato_active);
    void Multicast_stop_players_for_loop_track(int track);

    void Update_players_stistics(void);
    int Get_players_playing(void);
    int Get_players_using_Flash(void);
    int Get_players_using_Wavetable(void);
    void Release_Player_noteOff(uint8_t player, int track = NO_TRACK);

    bool Get_restart_player(int player);
    void Cancel_restart_player(int player);

    float Get_cross_mix_time(int player, int mix_samples);
    int Get_span_for_all_cross_mix(void);

    int Smartfind_oldest_player(uint8_t instrument_id, bool power_on, bool playing);
    int Simplefind_oldest_player(bool power_on);                     // "precedence" instruments are EXCLUDED
    int Simplefind_oldest_player_flash(bool power_on, bool playing); // "precedence" instruments are EXCLUDED

    void Change_from_key(int patch_id, int instrument_id, int from_key_new);
    void Change_to_key(int patch_id, int instrument_id, int to_key_new);
    void Multicast_change_players_notes(int patch_id, int instrument_id);
    bool Get_use_Wavetable(int sound_id);
    int8_t Find_oldest_player(int instrument_id, bool power_on, bool playing);
    bool Verify_if_stop_players(int patch_id, int instrument_id);

    void Release_player(int player);
    void Release_all_players_for_instrument(int instrument_id);
    void Release_all_players_for_instrument_solo(int instrument_id);
    void Release_all_players(void);
    void Release_softly_all_players(int patch_id);
    void Stop_all_players(void); // BROADCAST_stop_all_Players()

    void Release_player(int player, int track);
    void Release_all_players_loop(void);
    void Release_all_players_loop(int track);
    void Multicast_volume_for_MIDI_LOOP_running(int track, float value);

    void Multicast_release_players(int sound_id);
    void Broadcast_volume(void);
    void Multicast_volume_for_instrument_edit(int instrument_id); // chiamata da main e MidiReader
    void Multicast_pitch_for_sound_edit(int instrument_id);
    void Multicast_pan(int instrument_id);

    void Multicast_effects(float resolution, uint8_t downsampling);
    void Broadcast_reset_effect(float resolution, uint8_t downsampling, int effect);

    void Multicast_main_settings_editing(int patch_id, int instrument_id);
    void Multicast_reset_pitch_bend_effects(int instrument_id);
    void Broadcast_restore_pitch_bend_and_effects(int midi_channel, float value);

    void Update_all_Preset(int patch_id, float volume_patch);
    void Update_all_Preset_volume(int patch_id, float volume_patch);
    void Update_Preset(int patch_id, int instrument_id, float volume_patch);
    void Update_Preset_volume(int patch_id, int instrument_id, float volume_patch); // chiamata da main e MidiReader
    void Update_Preset_pan(int patch_id, int instrument_id);
    void Update_Preset_sound_id(int patch_id, int instrument_id);
    void Update_Preset_file(int patch_id, int instrument_id);
    void Update_Preset_midi_channel(int patch_id, int instrument_id);
    void Update_Preset_pitch(int patch_id, int instrument_id);
    void Update_Preset_mode(int patch_id, int instrument_id);
    void Update_Preset_A_B_Wavetable(int patch_id, int instrument_id);
    void Update_Preset_Noclick(int patch_id, int instrument_id);
    void Update_Preset_attack_type(int patch_id, int instrument_id);
    void Update_Preset_attack(int patch_id, int instrument_id);
    void Update_Preset_decay(int patch_id, int instrument_id);
    void Update_Preset_sustain(int patch_id, int instrument_id);
    void Update_Preset_release(int patch_id, int instrument_id);
    void Update_Preset_precedence(int patch_id, int instrument_id);
    void Update_Preset_lock(int patch_id, int instrument_id);

    void Multicast_IF_update_filter_type(int instrument_id);
    void Update_IF_resonance(int patch_id, int instrument_id);
    void Multicast_IF_pivot(int instrument_id);
    void Multicast_IF_frequency_filter(int instrument_id);
    void Multicast_IF_resonance(int instrument_id);
    void Multicast_IF_index(int instrument_id, float value); // chiamata da main e MidiReader
    void Update_Preset_IF(int patch_id, int instrument_id);
    void Update_Preset_IF_resonance(int patch_id, int instrument_id);
    void Update_Preset_IF_filter_type(int patch_id, int instrument_id);
    void Update_Preset_IF_modulation(int patch_id, int instrument_id);
    void Update_Preset_IF_index(int patch_id, int instrument_id);

    void Broadcast_FIFO_stereo(int16_t *_LS_buffer_L, int16_t *_LS_buffer_R); // chiamata da main LIVE_SAMPLER
    void Broadcast_FIFO_mono(int16_t *_LS_buffer_mono);                       // chiamata da main LIVE_SAMPLER
};