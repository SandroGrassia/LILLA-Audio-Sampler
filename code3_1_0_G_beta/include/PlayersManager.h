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

class PlayersManager
{
private:
    // puntatori esterni
    AudioPlayer *_Player = nullptr;
    LoopLedSet *_LOOP_led_set = nullptr;
    PlayersStatistics *_Players_statistics = nullptr;
    Router_16x3 *_Router_L = nullptr;
    Router_16x3 *_Router_R = nullptr;
    NoclickCrossmix *_Noclick = nullptr;    // Players_Manager._Noclick = &Noclick[0]
    WavetableManager *_Wavetable = nullptr; // Players_Manager._Wavetable = &Wavetable[0]

    // statistiche
    int players_playing = 0;
    int players_using_Flash = 0;
    int players_using_Wavetable = 0;
    uint8_t players_to_restart = 0; // numero di Player che devono ripartire; la ripartenza richiede una doppia lettura di campioni da vecchio e nuovo file ed il calcolo di mix_samples fatto dalla funzione Calculate_and_set_mix_samples

    // Play notes
    bool Player_booked[PLAYERS] = {false};
    bool restart_Player[PLAYERS] = {false}; // questo array serve per contare, ad ogni ciclo, il numero di Player che devono ripartire; la ripartenza richiede una doppia lettura di campioni da vecchio e nuovo file ed il calcolo di mix_samples fatto dalla funzione Calculate_and_set_mix_samples

public:
    PlayersManager(AudioPlayer *P, LoopLedSet *L, PlayersStatistics *LM, Router_16x3 *RL, Router_16x3 *RR, NoclickCrossmix *NC, WavetableManager *WT) : _Player(P), _LOOP_led_set(L), _Players_statistics(LM), _Router_L(RL), _Router_R(RR), _Noclick(NC), _Wavetable(WT) {}

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
    - aggiorna _Router_L/R per instradamento verso Delay si/no
    - aggiorna PlayersStatistics per l'aggiornamento dei led degli Instrument
    - richiede a main() di aggiornare i led
    */

    void MX_multicast_change_routing(int instrument);
    void Play_note(uint8_t instrument, uint8_t note_number, float velocity_float, int track);
    
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

    int Smartfind_oldest_player(uint8_t instrument, bool power_on, bool playing);
    int Simplefind_oldest_player(bool power_on);                     // "precedence" instruments are EXCLUDED
    int Simplefind_oldest_player_flash(bool power_on, bool playing); // "precedence" instruments are EXCLUDED

    void Change_from_key(int session, int instrument, int from_key_new);
    void Change_to_key(int session, int instrument, int to_key_new);
    void Multicast_change_players_notes(int session, int instrument);
    bool Get_use_Wavetable(int id_sound);
    int8_t Find_oldest_player(int instrument, bool power_on, bool playing);
    bool Verify_if_stop_players(int instrument);

    void Release_player(int player);
    void Release_all_players_for_instrument(int instrument);
    void Release_all_players_for_instrument_solo(int instrument);
    void Release_all_players(void);
    void Release_softly_all_players(int session);
    void Stop_all_players(void); // BROADCAST_stop_all_Players()

    void Release_player(int player, int track);
    void Release_all_players_loop(void);
    void Release_all_players_loop(int track);
    void Multicast_volume_for_MIDI_LOOP_running(int track, float value);

    void Multicast_release_players(int id_sound);
    void Broadcast_volume(void);
    void Multicast_volume_for_instrument_edit(int instrument); // chiamata da main e MidiReader
    void Multicast_pitch_for_sound_edit(int instrument);
    void Multicast_pan(int instrument);

    void Multicast_effects(float resolution, uint8_t downsampling);
    void Broadcast_reset_effect(float resolution, uint8_t downsampling, int effect);

    void Multicast_main_settings_editing(int session, int instrument);
    void Multicast_reset_pitch_bend_effects(int instrument);
    void Broadcast_restore_pitch_bend_and_effects(int midi_channel, float value);

    void Update_all_Preset(int session, float volume_session);
    void Update_all_Preset_volume(int session, float volume_session);
    void Update_Preset(int session, int instrument, float volume_session);
    void Update_Preset_volume(int session, int instrument, float volume_session); // chiamata da main e MidiReader
    void Update_Preset_pan(int session, int instrument);
    void Update_Preset_id_sound(int session, int instrument);
    void Update_Preset_file(int session, int instrument);
    void Update_Preset_midi_channel(int session, int instrument);
    void Update_Preset_pitch(int session, int instrument);
    void Update_Preset_mode(int session, int instrument);
    void Update_Preset_A_B_Wavetable(int session, int instrument);
    void Update_Preset_Noclick(int session, int instrument);
    void Update_Preset_attack_type(int session, int instrument);
    void Update_Preset_attack(int session, int instrument);
    void Update_Preset_decay(int session, int instrument);
    void Update_Preset_sustain(int session, int instrument);
    void Update_Preset_release(int session, int instrument);
    void Update_Preset_precedence(int session, int instrument);
    void Update_Preset_lock(int session, int instrument);

    void Multicast_IF_update_filter_type(int instrument);
    void Update_IF_resonance(int session, int instrument);
    void Multicast_IF_pivot(int instrument);
    void Multicast_IF_frequency_filter(int instrument);
    void Multicast_IF_resonance(int instrument);
    void Multicast_IF_index(int instrument, float value); // chiamata da main e MidiReader
    void Update_Preset_IF(int session, int instrument);
    void Update_Preset_IF_resonance(int session, int instrument);
    void Update_Preset_IF_filter_type(int session, int instrument);
    void Update_Preset_IF_modulation(int session, int instrument);
    void Update_Preset_IF_index(int session, int instrument);

    void Broadcast_FIFO_stereo(int16_t *_LS_buffer_L, int16_t *_LS_buffer_R); // chiamata da main LIVE_SAMPLER
    void Broadcast_FIFO_mono(int16_t *_LS_buffer_mono); // chiamata da main LIVE_SAMPLER
};