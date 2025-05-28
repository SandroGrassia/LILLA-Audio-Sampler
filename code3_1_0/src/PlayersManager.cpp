#include "PlayersManager.h"
#include <algorithm>

#define ID_SOUND Session[session].Instrument[instrument].id_sound

void PlayersManager::MX_multicast_change_routing(int instrument)
{
    for (uint8_t p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying())
        {
            // configure Router_L, Router_R input/output routing_table
            if (Delay_values.instrument_route[instrument])
            {
                _Router_L->routing_table[p] = 0;
                _Router_R->routing_table[p] = 0;
            }
            else
            {
                _Router_L->routing_table[p] = 1;
                _Router_R->routing_table[p] = 1;
            }

            _Router_L->routing_MX[p] = MX_routing_source[instrument];
            _Router_R->routing_MX[p] = MX_routing_source[instrument];
        }
    }
}

void PlayersManager::Play_note(uint8_t instrument, uint8_t note_number, float velocity_float, int track) // after receiving a NoteOn command
{
    // Se e' una nota appartenente ad un track: track >= -1
    // Altrimenti: track = NO_TRACK

    uint8_t p;
    int8_t id_player = -1;
    bool finished = false;
    bool use_Wavetable = Preset[instrument].use_Wavetable; // NON valido se LIVE_SAMPLER

    // Caso NoteOn da tastiera reale (track == NO_TRACK) if a Player is_playing with same session, instrument and note_number, and track, this Player must be taken
    for (p = 0; p < PLAYERS; p++)
    {
        if (((_Player + p)->Read_local_session() == session) && ((_Player + p)->Read_note() == note_number) && ((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying() && (_Player + p)->Read_loop_track() == track) // isPlaying() significa !idle
        {
            id_player = p;
            finished = true;
            // PRINT("Play the same note - ", "Stop the same Player:", id_player);
            break;
        }
    }

    if (!finished && !use_Wavetable) // Flash memory will be used --> LIMITED use of players WITHIN the SAME instrument range
    {
        Update_players_stistics();

        if (players_using_Flash < POLYPHONY_FLASH[optimization]) // a Player can be used
        {
            // 1) if there is a Player !isPlaying, it can be used
            for (p = 0; p < PLAYERS; p++)
            {
                if (!(_Player + p)->isPlaying())
                {
                    id_player = p;
                    finished = true;
                    // PRINT("Play from Flash - case 0", "Found available Player:", id_player);
                    break;
                }
            }

            if (!finished)
            {
                // 2) if there is a Player playing an Instrument of a different Session, this Player can be used
                for (p = 0; p < PLAYERS; p++)
                {
                    if ((_Player + p)->Read_session_wait() != session)
                    {
                        id_player = p;
                        finished = true;
                        PRINT("Play from Flash - case 1", "Found available Player:", id_player);
                        break;
                    }
                }
            }

            if (!finished)
            {
                // 3) look for a Player !power_on and playing of the SAME INSTRUMENT: choose the OLDEST
                id_player = Smartfind_oldest_player(instrument, false, true); // SMARTFIND_oldest_player(uint8_t instrument, bool power_on, bool playing)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from Flash - case 2", "Found available Player:", id_player);
                }
            }

            if (!finished)
            {
                // 4) look for a Player !power_on of ANY INSTRUMENT NOT protected: choose the OLDEST
                id_player = Simplefind_oldest_player(false); // SIMPLEFIND_oldest_player_power_on(bool power_on)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from Flash - case 3", "Found available Player:", id_player);
                }
            }
            if (!finished)
            {
                // 5) look for a Player power_on and playing of the SAME INSTRUMENT: choose the OLDEST
                id_player = Smartfind_oldest_player(instrument, true, true); // SMARTFIND_oldest_player(uint8_t instrument, bool power_on, bool playing)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from Flash - case 4", "Found available Player:", id_player);
                }
            }
            if (!finished && Preset[instrument].precedence)
            {
                // 6) look for a Player power_on of ANY INSTRUMENT NOT protected: choose the OLDEST
                id_player = Simplefind_oldest_player(true); // SIMPLEFIND_oldest_player(bool power_on)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from Flash - case 5", "Found available Player:", id_player);
                }
            }
        }

        // only a Player flash_mode can be reused
        else
        {
            // 5A) there is a Player flash_mode from a different Session and playing
            for (p = 0; p < PLAYERS; p++)
            {
                if (((_Player + p)->Read_session_wait() != session) && !(_Player + p)->Read_use_Wavetable() && (_Player + p)->State() > 0)
                {
                    id_player = p;
                    finished = true;
                    PRINT("Play from Flash - case 6", "Found available Player:", id_player);
                    break;
                }
            }

            if (!finished)
            {
                // 6) there is a Player playing the SAME INSTRUMENT: choose the OLDEST
                id_player = Smartfind_oldest_player(instrument, false, true); // SMARTFIND_oldest_player(uint8_t instrument, bool power_on, bool playing)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from Flash - case 7", "Found available Player:", id_player);
                }
            }

            if (!finished)
            {
                // 7)  there is a Player fading (state = 2) ANY INSTRUMENT NOT protected flash_mode: choose the OLDEST
                id_player = Simplefind_oldest_player_flash(false, true); // SIMPLEFIND_oldest_player_flash(bool power_on, bool playing)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from Flash - case 9", "Found available Player:", id_player);
                }
            }

            if (!finished)
            {
                // 8)  there is a Player playing (state = 1) ANY INSTRUMENT NOT protected flash_mode: choose the OLDEST
                id_player = Simplefind_oldest_player_flash(true, true); // SIMPLEFIND_oldest_player_flash(bool power_on, bool playing)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from Flash - case 10", "Found available Player:", id_player);
                }
            }
        }
    }

    if (!finished && use_Wavetable) // NO LIMIT to use players but playing the SAME instrument
    {
        // 1) look for a Player NOT used
        Update_players_stistics();

        if (players_playing < PLAYERS) // there is at least ONE player that can be taken
        {
            for (p = 0; p < PLAYERS; ++p)
            {
                if (!(_Player + p)->isPlaying()) // isPlaying() significa !idle
                {
                    id_player = p;
                    finished = true;
                    // PRINT("Play from RAM - case 1", "Found available Player:", id_player);
                    break;
                }
            }
        }

        else // all Player are playing
        {
            // 2) look for a Player playing an Instrument of a different Session: this Player can be taken
            for (p = 0; p < PLAYERS; p++)
            {
                if ((_Player + p)->Read_session_wait() != session)
                {
                    id_player = p;
                    finished = true;
                    // PRINT("Play from RAM - case 2", "Found available Player:", id_player);
                }
            }

            if (!finished)
            {
                // 3) look for a Player !power_on (ADSR "Release" phase, or idle) and playing of the SAME INSTRUMENT: choose the OLDEST
                id_player = Smartfind_oldest_player(instrument, false, true); // SMARTFIND_oldest_player(uint8_t instrument, bool power_on, bool playing)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from RAM - case 3", "Found available Player:", id_player);
                }
            }

            if (!finished)
            {
                // 4) look for a Player !power_on of ANY INSTRUMENT NOT protected: choose the OLDEST
                id_player = Simplefind_oldest_player(false); // SIMPLEFIND_oldest_player(bool power_on)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from RAM - case 4", "Found available Player:", id_player);
                }
            }

            if (!finished)
            {
                // 5) look for a player power_on and playing of the SAME INSTRUMENT: choose the OLDEST
                id_player = Smartfind_oldest_player(instrument, true, true); // SMARTFIND_oldest_player(uint8_t instrument, bool power_on, bool playing)
                if (id_player >= 0)
                {
                    finished = true;
                    // PRINT("Play from RAM - case 5", "Found available Player:", id_player);
                }
            }
        }

        if (!finished && Preset[instrument].precedence)
        {
            // 6) look for a Player power_on of ANY INSTRUMENT NOT protected: choose the OLDEST
            id_player = Simplefind_oldest_player(true); // SIMPLEFIND_oldest_player(bool power_on)
            if (id_player >= 0)
            {
                finished = true;
                // PRINT("Play from RAM - case 6", "Found available Player:", id_player);
            }
        }
    }

    if (finished)
    {
        Player_booked[id_player] = true;

        if ((_Player + id_player)->State() > 0) // sta inviando sample, cioè !idle
        {
            restart_Player[id_player] = true;
            players_to_restart++;
        }

        (_Player + id_player)->Write_precedence(Preset[instrument].precedence);
        (_Player + id_player)->Write_midi_channel(Preset[instrument].midi_channel);
        (_Player + id_player)->set_file(Preset[instrument].file);
        (_Player + id_player)->Set_ADSR(Preset[instrument].attack, Preset[instrument].decay, Preset[instrument].sustain, Preset[instrument].release, Preset[instrument].attack_type);
        (_Player + id_player)->Write_loop_track(track);
        if (track >= 0)
            (_Player + id_player)->Set_volume(LOOP_volume[track] * Preset[instrument].volume);
        else
            (_Player + id_player)->Set_volume(Preset[instrument].volume);
        (_Player + id_player)->Set_pan(Preset[instrument].pan);
        (_Player + id_player)->Set_pitch(Preset[instrument].pitch);
        (_Player + id_player)->Main_settings(Preset[instrument].mode, Preset[instrument].A, Preset[instrument].B, Preset[instrument].Noclick, Preset[instrument].use_Wavetable, (_Noclick + instrument)->get_pointer(), (_Wavetable + instrument)->get_pointer());

        if (!Preset[instrument].lock)
        {
            (_Player + id_player)->Set_effects(Calc_resolution(resolution), downsampling);
            (_Player + id_player)->Set_pitch_bend(pitch_bend_value[Preset[instrument].midi_channel]);
        }
        else
        {
            (_Player + id_player)->Set_effects(16.0, 0);
            (_Player + id_player)->Set_pitch_bend(1.0);
        }

        // VCF and its LFO_0
        if (Preset[instrument].Filter.use == 1)
        {
            if (Preset[instrument].Filter.modulation == 4) // LFO wave "sine" modulates VCF and MIDI After touch modulates index
            {
                (_Player + id_player)->Connect_VCF(true, Preset[instrument].Filter.type, Preset[instrument].Filter.pivot, Preset[instrument].Filter.resonance, true); // void Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated)
                (_Player + id_player)->Connect_LFO_TO_VCF(Preset[instrument].Filter.modulation, Preset[instrument].Filter.index * after_touch_channel_value[Preset[instrument].midi_channel], Preset[instrument].Filter.periodic, Preset[instrument].Filter.frequency_time); // Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time)
            }
            else if (Preset[instrument].Filter.modulation > 0) // LFO modulates VCF
            {
                (_Player + id_player)->Connect_VCF(true, Preset[instrument].Filter.type, Preset[instrument].Filter.pivot, Preset[instrument].Filter.resonance, true); // void Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated)
                (_Player + id_player)->Connect_LFO_TO_VCF(Preset[instrument].Filter.modulation, Preset[instrument].Filter.index, Preset[instrument].Filter.periodic, Preset[instrument].Filter.frequency_time); // Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time)
            }
            else // VCF is not modulated
            {
                (_Player + id_player)->Connect_VCF(true, Preset[instrument].Filter.type, Preset[instrument].Filter.pivot, Preset[instrument].Filter.resonance, false); // void Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated)
            }
        }
        else
            (_Player + id_player)->Connect_VCF(false, 0, 20000, 1, false);

        /*
        PLAY note; il Player setta:
        power_on = true
        idle = false
        (se interrogato Player[p].is_playing == true);
        */
        (_Player + id_player)->Get_ready_to_play(pitch_from_note[note_number + 60 - Session[session].Instrument[instrument].root_key], velocity_float, session, instrument, Preset[instrument].id_sound, note_number);

        // calcola update_time e trasmetti il valore al Player
        int update_time;
        if (Lilla_state == LIVE_SAMPLING)
        {
            update_time = 9.067 * (_Player + id_player)->Read_pitch() + 35.3;
        }
        else
        {
            if (Preset[instrument].use_Wavetable)
            {
                update_time = 2.7 * (_Player + id_player)->Read_pitch() + 31.0;
            }
            else
            {
                update_time = 48.56 * (_Player + id_player)->Read_pitch() + 48.31;
            }
        }
        if (Preset[instrument].Filter.use == 1)
            update_time += 15;

        (_Player + id_player)->Write_update_time(update_time);

        // Gestione del Delay
        // configura su Router_L e Router_R input/output le routing_table
        if (Delay_values.instrument_route[instrument])
        {
            _Router_L->routing_table[id_player] = 0;
            _Router_R->routing_table[id_player] = 0;
        }
        else
        {
            _Router_L->routing_table[id_player] = 1;
            _Router_R->routing_table[id_player] = 1;
        }

        _Router_L->routing_MX[id_player] = MX_routing_source[instrument];
        _Router_R->routing_MX[id_player] = MX_routing_source[instrument];

        // display Players activity
        if (false)
        {
            for (uint8_t p = 0; p < PLAYERS; p++)
            {
                if ((_Player + p)->isPoweredOn())
                {
                    Serial.print((_Player + p)->Read_note());
                    Serial.print("/");
                    Serial.print((_Player + p)->Read_instrument());
                }
                else
                    Serial.print("NN");
                Serial.print(" ");
            }
            Serial.println();
        }
    }

    if (!finished)
    {
        PRINT("Play from Flash - ", "NOT found available Player!", 0);
    }
}

void PlayersManager::Release_Player_noteOff(uint8_t player, int track) // after receiving a NoteOff command
{
    (_Player + player)->Release_note();
    (_Player + player)->Write_time_stamp(millis());

    // display Players activity
    if (false)
    {
        for (uint8_t p = 0; p < PLAYERS; p++)
        {
            if ((_Player + p)->isPoweredOn())
            {
                Serial.print((_Player + p)->Read_note());
                Serial.print("/");
                Serial.print((_Player + p)->Read_instrument());
            }
            else
                Serial.print("NN");
            Serial.print(" ");
        }
        Serial.println();
    }
}

void PlayersManager::Reset_booked_and_restart_player(void)
{
    // Player_booked serve ad evitare che gli Instrument che sono attivati da NoteOn non competano sullo stesso Player
    // players_to_restart, se risultera' >0, richiede il calcolo dei vari valori di samples (mix_samples_for_Player[p])
    // che ciascun Player da riavviare (restart_Player[p] == true) dovra' utilizzare
    for (int p = 0; p < PLAYERS; ++p)
    {
        Player_booked[p] = false;
        restart_Player[p] = false;
    }
}

bool PlayersManager::Get_restart_player(int player)
{
    return restart_Player[player];
}

void PlayersManager::Cancel_restart_player(int player)
{
    restart_Player[player] = false;
}

int PlayersManager::Smartfind_oldest_player(uint8_t instrument, bool power_on, bool playing)
{
    unsigned long time_min = 0;
    uint8_t p, q;
    int8_t result = -1;
    for (p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_instrument() == instrument) && ((_Player + p)->isPoweredOn() == power_on) && ((_Player + p)->isPlaying() == playing) && !Player_booked[p])
        {
            time_min = (_Player + p)->Read_time_stamp();
            result = p;
            for (q = 0; q < PLAYERS; ++q) // look for the player playing/going to play for the longest time
            {
                if (((_Player + q)->Read_instrument() == instrument) && ((_Player + q)->isPoweredOn() == power_on) && ((_Player + q)->isPlaying() == playing) && ((_Player + q)->Read_time_stamp() < time_min) && !Player_booked[q])
                {
                    time_min = (_Player + q)->Read_time_stamp();
                    result = q;
                }
            }
            break;
        }
    }
    return result;
}

int PlayersManager::Simplefind_oldest_player(bool power_on) // "precedence" instruments are EXCLUDED
{
    unsigned long time_min = 0;
    uint8_t p, q;
    int8_t result = -1;
    for (p = 0; p < PLAYERS; p++)
    {
        if (!(_Player + p)->Read_precedence() && ((_Player + p)->isPoweredOn() == power_on) && !Player_booked[p])
        {
            time_min = (_Player + p)->Read_time_stamp();
            result = p;
            for (q = 0; q < PLAYERS; q++) // look for the player playing for the longest time
            {
                if (!(_Player + q)->Read_precedence() && ((_Player + q)->isPoweredOn() == power_on) && ((_Player + q)->Read_time_stamp() < time_min) && !Player_booked[q])
                {
                    time_min = (_Player + q)->Read_time_stamp();
                    result = q;
                }
            }
            break;
        }
    }
    return result;
}

int PlayersManager::Simplefind_oldest_player_flash(bool power_on, bool playing) // "precedence" instruments are EXCLUDED
{
    unsigned long time_min = 0;
    uint8_t p, q;
    int8_t result = -1;

    for (p = 0; p < PLAYERS; p++)
    {
        if (!(_Player + p)->Read_use_Wavetable() && !(_Player + p)->Read_precedence() && ((_Player + p)->isPlaying() == playing) && ((_Player + p)->isPoweredOn() == power_on) && !Player_booked[p])
        {
            time_min = (_Player + p)->Read_time_stamp();
            result = p;
            for (q = 0; q < PLAYERS; q++) // look for the player playing for the longest time
            {
                if (!(_Player + q)->Read_use_Wavetable() && !(_Player + q)->Read_precedence() && ((_Player + q)->isPlaying() == playing) && ((_Player + q)->isPoweredOn() == power_on) && ((_Player + q)->Read_time_stamp() < time_min) && !Player_booked[q])
                {
                    time_min = (_Player + q)->Read_time_stamp();
                    result = q;
                }
            }
            break;
        }
    }
    return result;
}

void PlayersManager::Change_from_key(int session, int instrument, int from_key_new)
{
    if (from_key_new > Session[session].Instrument[instrument].from_note)
        for (int p = 0; p < PLAYERS; ++p)
            if ((_Player + p)->Read_note() < from_key_new && (_Player + p)->isPoweredOn() && (_Player + p)->Read_instrument() == instrument)
                Release_player(p);
    Session[session].Instrument[instrument].from_note = from_key_new;
    Update_map_Instrument_for_notes(Session[session].Instrument[instrument].from_note, Session[session].Instrument[instrument].to_note, instrument);
}

void PlayersManager::Change_to_key(int session, int instrument, int to_key_new)
{
    if (to_key_new < Session[session].Instrument[instrument].to_note)
        for (int p = 0; p < PLAYERS; ++p)
            if ((_Player + p)->Read_note() > to_key_new && (_Player + p)->isPoweredOn() && (_Player + p)->Read_instrument() == instrument)
                Release_player(p);
    Session[session].Instrument[instrument].to_note = to_key_new;
    Update_map_Instrument_for_notes(Session[session].Instrument[instrument].from_note, Session[session].Instrument[instrument].to_note, instrument);
}

void PlayersManager::Multicast_change_players_notes(int session, int instrument) // usato quando si cambia la root_key
{
    for (int p = 0; p < PLAYERS; ++p)
        if ((_Player + p)->isPoweredOn() && (_Player + p)->Read_instrument() == instrument)
            (_Player + p)->Set_note(pitch_from_note[(_Player + p)->Read_note() + 60 - Session[session].Instrument[instrument].root_key]);
}

void PlayersManager::Multicast_release_players(int id_sound)
{
    for (int p = 0; p < PLAYERS; ++p)
        if ((_Player + p)->Read_id_sound() == id_sound)
            Release_player(p);
}

void PlayersManager::Broadcast_volume(void)
{
    for (int p = 0; p < PLAYERS; ++p)
        if ((_Player + p)->isPlaying())
            (_Player + p)->Update_volume(Preset[(_Player + p)->Read_instrument()].volume);
}

void PlayersManager::Multicast_volume_for_instrument_edit(int instrument)
{
    for (int p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying())
            (_Player + p)->Update_volume(Preset[instrument].volume);
    }
}

void PlayersManager::Multicast_pitch_for_sound_edit(int instrument)
{
    for (int p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying())
            (_Player + p)->Set_pitch(Preset[instrument].pitch);
    }
}

void PlayersManager::Multicast_pan(int instrument)
{
    for (int p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying())
            (_Player + p)->Update_pan(Preset[instrument].pan);
    }
}

void PlayersManager::Multicast_effects(float resolution, uint8_t downsampling)
{
    for (uint8_t p = 0; p < PLAYERS; ++p)
    {
        if ((_Player + p)->isPlaying() && !Preset[(_Player + p)->Read_instrument()].lock)
            (_Player + p)->Set_effects(resolution, downsampling);
    }
}

void PlayersManager::Broadcast_reset_effect(float resolution, uint8_t downsampling, int effect)
{
    for (uint8_t p = 0; p < PLAYERS; p++)
    {
        if (effect == 0)                                    // reset resolution
            (_Player + p)->Set_effects(16.0, downsampling); // (resolution, downsampling)

        else if (effect == 1) // reset downsampling
            (_Player + p)->Set_effects(Calc_resolution(resolution), 1);

        else
            return;
    }
}

void PlayersManager::Update_all_Preset(int session, float volume_session)
{
    for (int instrument = 0; instrument < INSTRUMENTS_MAX; ++instrument)
        if (Session[session].Instrument[instrument].used)
        {
            // Serial.println(instrument);
            Update_Preset(session, instrument, volume_session);
        }
}

void PlayersManager::Update_all_Preset_volume(int session, float volume_session)
{
    for (int instrument = 0; instrument < INSTRUMENTS_MAX; ++instrument)
        if (Session[session].Instrument[instrument].used)
            Update_Preset_volume(session, instrument, volume_session);
}

void PlayersManager::Update_Preset(int session, int instrument, float volume_session)
{
    Update_Preset_volume(session, instrument, volume_session);
    Preset[instrument].pan = Sound[ID_SOUND].pan;
    Preset[instrument].id_sound = ID_SOUND;
    Preset[instrument].file = Sound[ID_SOUND].file;
    Preset[instrument].midi_channel = Get_midi_channel(session, instrument);
    Preset[instrument].pitch = Calc_pitch(Sound[ID_SOUND].pitch);
    Preset[instrument].mode = Sound[ID_SOUND].mode;
    Preset[instrument].A = Sound[ID_SOUND].A;
    Preset[instrument].B = Sound[ID_SOUND].B;
    Preset[instrument].use_Wavetable = (Preset[instrument].B - Preset[instrument].A + 1) <= BLOCK_MIN;
    Preset[instrument].Noclick = Sound[ID_SOUND].Noclick;
    Preset[instrument].attack_type = bitRead(Sound[ID_SOUND].data, 0);
    Preset[instrument].attack = Calc_attack(Sound[ID_SOUND].attack);
    Preset[instrument].decay = CALC_decay(Sound[ID_SOUND].decay);
    Preset[instrument].sustain = Calc_sustain(Sound[ID_SOUND].sustain);
    Preset[instrument].release = Calc_release(Sound[ID_SOUND].release);
    Preset[instrument].precedence = Session[session].Instrument[instrument].precedence; // Preset[I].precedence = bitRead(Session[session].Instrument[I].info, 0);
    Preset[instrument].lock = Session[session].Instrument[instrument].lock;             // Preset[I].lock = bitRead(Session[session].Instrument[I].info, 1)
    Update_Preset_IF(session, instrument);
}

void PlayersManager::Update_Preset_volume(int session, int instrument, float volume_session)
{
    if (MX_mute[instrument])
        Preset[instrument].volume = 0.0;
    else
        Preset[instrument].volume = volume_session * Volume_float[Sound[ID_SOUND].gain];
}

void PlayersManager::Update_Preset_pan(int session, int instrument)
{
    Preset[instrument].pan = Sound[ID_SOUND].pan;
}

void PlayersManager::Update_Preset_id_sound(int session, int instrument)
{
    Preset[instrument].id_sound = ID_SOUND;
}

void PlayersManager::Update_Preset_file(int session, int instrument)
{
    Preset[instrument].file = Sound[ID_SOUND].file;
}

void PlayersManager::Update_Preset_midi_channel(int session, int instrument)
{
    Preset[instrument].midi_channel = Get_midi_channel(session, instrument);
}

void PlayersManager::Update_Preset_pitch(int session, int instrument)
{
    Preset[instrument].pitch = Calc_pitch(Sound[ID_SOUND].pitch);
}

void PlayersManager::Update_Preset_mode(int session, int instrument)
{
    Preset[instrument].mode = Sound[ID_SOUND].mode;
}

void PlayersManager::Update_Preset_A_B_Wavetable(int session, int instrument)
{
    Preset[instrument].A = Sound[ID_SOUND].A;
    Preset[instrument].B = Sound[ID_SOUND].B;
    Preset[instrument].use_Wavetable = (Preset[instrument].B - Preset[instrument].A + 1) <= BLOCK_MIN;
}

void PlayersManager::Update_Preset_Noclick(int session, int instrument)
{
    Preset[instrument].Noclick = Sound[ID_SOUND].Noclick;
}

void PlayersManager::Update_Preset_attack_type(int session, int instrument)
{
    Preset[instrument].attack_type = bitRead(Sound[ID_SOUND].data, 0);
}

void PlayersManager::Update_Preset_attack(int session, int instrument)
{
    Preset[instrument].attack = Calc_attack(Sound[ID_SOUND].attack);
}

void PlayersManager::Update_Preset_decay(int session, int instrument)
{
    Preset[instrument].decay = CALC_decay(Sound[ID_SOUND].decay);
}

void PlayersManager::Update_Preset_sustain(int session, int instrument)
{
    Preset[instrument].sustain = Calc_sustain(Sound[ID_SOUND].sustain);
}

void PlayersManager::Update_Preset_release(int session, int instrument)
{
    Preset[instrument].release = Calc_release(Sound[ID_SOUND].release);
}

void PlayersManager::Update_Preset_precedence(int session, int instrument)
{
    Preset[instrument].precedence = Session[session].Instrument[instrument].precedence;
}

void PlayersManager::Update_Preset_lock(int session, int instrument)
{
    Preset[instrument].lock = Session[session].Instrument[instrument].lock;
}

void PlayersManager::Multicast_IF_update_filter_type(int instrument)
{
    for (int p = 0; p < PLAYERS; p++)
    {
        if (((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying())
        {
            if (Preset[instrument].Filter.use == 1)
            {
                if (Preset[instrument].Filter.modulation == 4) // LFO wave "sine" modulates VCF and MIDI After touch modulates index
                {
                    (_Player + p)->Connect_VCF(true, Preset[instrument].Filter.type, Preset[instrument].Filter.pivot, Preset[instrument].Filter.resonance, true);                                                                                                        // void Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated)
                    (_Player + p)->Connect_LFO_TO_VCF(Preset[instrument].Filter.modulation, Preset[instrument].Filter.index * after_touch_channel_value[Preset[instrument].midi_channel], Preset[instrument].Filter.periodic, Preset[instrument].Filter.frequency_time); // Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time)
                }
                else if (Preset[instrument].Filter.modulation > 0) // LFO modulates VCF
                {
                    (_Player + p)->Connect_VCF(true, Preset[instrument].Filter.type, Preset[instrument].Filter.pivot, Preset[instrument].Filter.resonance, true);                                           // void Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated)
                    (_Player + p)->Connect_LFO_TO_VCF(Preset[instrument].Filter.modulation, Preset[instrument].Filter.index, Preset[instrument].Filter.periodic, Preset[instrument].Filter.frequency_time); // Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time)
                }
                else // VCF is not modulated
                {
                    (_Player + p)->Connect_VCF(true, Preset[instrument].Filter.type, Preset[instrument].Filter.pivot, Preset[instrument].Filter.resonance, false); // void Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated)
                }
            }
            else
                (_Player + p)->Connect_VCF(false, 0, 20000, 1, false);

            (_Player + p)->Start_VCF();
        }
    }
}

void PlayersManager::Update_IF_resonance(int session, int instrument)
{
    Update_Preset_IF_resonance(session, instrument);
    if (Preset[instrument].Filter.use == 1)
        Multicast_IF_resonance(instrument);
}

void PlayersManager::Multicast_IF_pivot(int instrument)
{
    for (uint8_t player = 0; player < PLAYERS; ++player)
    {
        if (((_Player + player)->Read_instrument() == instrument) && (_Player + player)->isPlaying())
            (_Player + player)->Update_VCF_pivot(Preset[instrument].Filter.pivot);
    }
}

void PlayersManager::Multicast_IF_frequency_filter(int instrument)
{
    for (uint8_t player = 0; player < PLAYERS; ++player)
    {
        if (((_Player + player)->Read_instrument() == instrument) && (_Player + player)->isPlaying())
        {
            if ((Preset[instrument].Filter.modulation != 0) && (Preset[instrument].Filter.modulation != 4))
                (_Player + player)->Connect_LFO_TO_VCF(Preset[instrument].Filter.modulation, Preset[instrument].Filter.index, Preset[instrument].Filter.periodic, Preset[instrument].Filter.frequency_time); // Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time)
            else
                (_Player + player)->Connect_LFO_TO_VCF(Preset[instrument].Filter.modulation, Preset[instrument].Filter.index * after_touch_channel_value[Preset[instrument].midi_channel], Preset[instrument].Filter.periodic, Preset[instrument].Filter.frequency_time); // Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time)
        }
    }
}

void PlayersManager::Multicast_IF_resonance(int instrument)
{
    for (uint8_t player = 0; player < PLAYERS; ++player)
    {
        if (((_Player + player)->Read_instrument() == instrument) && (_Player + player)->isPlaying())
            (_Player + player)->Update_VCF_resonance(Preset[instrument].Filter.resonance);
    }
}

void PlayersManager::Update_Preset_IF(int session, int instrument)
{
    Preset[instrument].Filter.use = Session[session].Instrument[instrument].Filter.use;
    Preset[instrument].Filter.type = Session[session].Instrument[instrument].Filter.type; // 0 -> 3

    float value = Session[session].Instrument[instrument].Filter.pivot / 10.0f;                                     // 0 --> 100  0 --> 10
    Preset[instrument].Filter.pivot = 20.0f * pow(2.0f, value);                                                     //  20 --> 20048
    Preset[instrument].Filter.resonance = (5.0f + Session[session].Instrument[instrument].Filter.resonance) / 5.0f; // 0 --> 40
    Preset[instrument].Filter.index = Session[session].Instrument[instrument].Filter.index / 20.0f;                 // 0 --> 20 : 0 --> 1.0
    Update_Preset_IF_modulation(session, instrument);
}

void PlayersManager::Update_Preset_IF_resonance(int session, int instrument)
{
    Preset[instrument].Filter.resonance = (5.0f + Session[session].Instrument[instrument].Filter.resonance) / 5.0f; // 0 --> 40
}

void PlayersManager::Update_Preset_IF_filter_type(int session, int instrument)
{
    Preset[instrument].Filter.type = Session[session].Instrument[instrument].Filter.type; // 0 -> 3
}

void PlayersManager::Update_Preset_IF_modulation(int session, int instrument)
{
    Preset[instrument].Filter.modulation = Session[session].Instrument[instrument].Filter.modulation; // 0 -> 4
    if (Preset[instrument].Filter.modulation == 3 || Preset[instrument].Filter.modulation == 4)
    {
        Preset[instrument].Filter.periodic = 1;
        Preset[instrument].Filter.frequency_time = Session[session].Instrument[instrument].Filter.frequency_time * Session[session].Instrument[instrument].Filter.frequency_time / 40.0f; // 0 --> 40
    }
    else
    {
        Preset[instrument].Filter.periodic = 0;
        Preset[instrument].Filter.frequency_time = Session[session].Instrument[instrument].Filter.frequency_time / 8.0f; // 0 --> 40
    }
}

void PlayersManager::Update_Preset_IF_index(int session, int instrument)
{
    Update_Preset_IF(session, instrument);
    if (Preset[instrument].Filter.use == 1)
    {
        if (Preset[instrument].Filter.modulation == 4)
            Multicast_IF_index(instrument, Preset[instrument].Filter.index * after_touch_channel_value[Preset[instrument].midi_channel]);
        else if (Preset[instrument].Filter.modulation != 0)
            Multicast_IF_index(instrument, Preset[instrument].Filter.index);
    }
}

void PlayersManager::Multicast_IF_index(int instrument, float value)
{
    for (uint8_t p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying())
        {
            (_Player + p)->Update_LFO_index(value);
        }
    }
}

void PlayersManager::Multicast_main_settings_editing(int session, int instrument)
{
    uint8_t players_to_cross_mix = 0;
    uint8_t mix_samples_for_Player[PLAYERS] = {0};
    bool cross_mix_Player[PLAYERS] = {0};
    for (int p = 0; p < PLAYERS; ++p)
    {
        mix_samples_for_Player[p] = 0;
        cross_mix_Player[p] = false;
    }

    for (int p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_local_session() == session) && ((_Player + p)->Read_instrument() == instrument) && (_Player + p)->isPlaying())
        {
            (_Player + p)->Main_settings_editing(Preset[instrument].mode, Preset[instrument].A, Preset[instrument].B, Preset[instrument].Noclick, Preset[instrument].use_Wavetable, Noclick_pointer[instrument], Wavetable_pointer[instrument]);
            players_to_cross_mix++;
            cross_mix_Player[p] = true;
        }
    }

    if (players_to_cross_mix > 0)
    {
        bool finished = false;
        int mix_micros_span = Get_span_for_all_cross_mix();
        // Serial.print("We have ");
        // Serial.print(mix_micros_span);
        // Serial.println(" micros available for ALL cross_mix.");

        bool old_use_wavetable[PLAYERS];
        float old_pitch_of_Player[PLAYERS];
        float standard_pitch_of_Player[PLAYERS];

        // calculate the standardized "flash-play" pitch: pitch of RAM-playing Players will be normalized so they can be traated as Flash-playing Players
        for (uint8_t p = 0; p < PLAYERS; p++)
        {
            if (cross_mix_Player[p])
            {
                old_use_wavetable[p] = (_Player + p)->Read_use_Wavetable();
                old_pitch_of_Player[p] = (_Player + p)->Read_pitch();

                if (old_use_wavetable[p])
                    standard_pitch_of_Player[p] = old_pitch_of_Player[p] * 0.07113;
                else
                    standard_pitch_of_Player[p] = old_pitch_of_Player[p];
                // Serial.print("Player:");
                // Serial.print(p);
                // Serial.print(" has standard_pitch:");
                // Serial.println(standard_pitch_of_Player[p]);
            }
        }
        // Serial.println();

        // find highest-pitch / highest-time Player
        int max_cross_mix_time;
        uint8_t worst_Player = 0;
        float max_pitch = 0;
        for (uint8_t p = 0; p < PLAYERS; p++)
        {
            if (cross_mix_Player[p])
            {
                if (standard_pitch_of_Player[p] > max_pitch)
                {
                    max_pitch = standard_pitch_of_Player[p];
                    worst_Player = p;
                }
            }
        }
        // Serial.print("Worst Player (with highest pitch) is:");
        // Serial.print(worst_Player);
        // Serial.print(" with pitch:");
        // Serial.println(max_pitch);
        // Serial.println();

        // 1) Try to set mix_samples = 64 for ALL Players
        max_cross_mix_time = Get_cross_mix_time(worst_Player, 64);
        // Serial.print("Try Case 1 - max_cross_mix_time (micros) is:");
        // Serial.println(max_cross_mix_time);
        // Serial.println();

        if (mix_micros_span > (max_cross_mix_time * players_to_cross_mix))
        {
            // Serial.println("Case 1 is verified --> transmit mix_samples = 64 to Players to cross_mix.");
            for (uint8_t p = 0; p < PLAYERS; p++)
            {
                if (cross_mix_Player[p])
                {
                    cross_mix_Player[p] = false;
                    players_to_cross_mix--;
                    mix_samples_for_Player[p] = 32;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);

                    mix_micros_span -= Get_cross_mix_time(p, 32); // Get_cross_mix_time(uint8_t p, uint16_t mix_samples)
                    // Serial.print("mix_sample 64 is given to Player ");
                    // Serial.print(p);
                    // Serial.print("; theorical update time is:");
                    // Serial.println(Get_cross_mix_time(p, 32) + (_Player + p)->update_time);
                }
            }
            finished = true;
        }

        // 2) Try to set mix_samples = 48 for ALL Players
        if (!finished)
        {
            max_cross_mix_time = Get_cross_mix_time(worst_Player, 48);
            // Serial.print("Try Case 2 - max_cross_mix_time (micros) is:");
            // Serial.println(max_cross_mix_time);
            // Serial.println();

            if (mix_micros_span > (max_cross_mix_time * players_to_cross_mix))
            {
                // Serial.println("Case 2 is verified --> transmit mix_samples = 48 to Players to cross_mix.");
                for (uint8_t p = 0; p < PLAYERS; p++)
                {
                    if (cross_mix_Player[p])
                    {
                        cross_mix_Player[p] = false;
                        players_to_cross_mix--;
                        mix_samples_for_Player[p] = 48;
                        (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);

                        mix_micros_span -= Get_cross_mix_time(p, 48); // Get_cross_mix_time(uint8_t p, uint16_t mix_samples)

                        if (false)
                        {
                            Serial.print("mix_sample 48 is given to Player ");
                            Serial.print(p);
                            Serial.print("; theorical update time is:");
                            Serial.println(Get_cross_mix_time(p, 48) + (_Player + p)->Read_update_time());
                        }
                    }
                }
                finished = true;
            }
        }

        // 3) Try to set mix_samples = 32 for ALL Players
        if (!finished)
        {
            max_cross_mix_time = Get_cross_mix_time(worst_Player, 32);
            // Serial.print("Try Case 3 - max_cross_mix_time (micros) is:");
            // Serial.println(max_cross_mix_time);
            // Serial.println();

            if (mix_micros_span > (max_cross_mix_time * players_to_cross_mix))
            {
                // Serial.println("Case 3 is verified --> transmit mix_samples = 32 to Players to cross_mix.");
                for (uint8_t p = 0; p < PLAYERS; p++)
                {
                    if (cross_mix_Player[p])
                    {
                        cross_mix_Player[p] = false;
                        players_to_cross_mix--;
                        mix_samples_for_Player[p] = 32;
                        (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);

                        mix_micros_span -= Get_cross_mix_time(p, 32); // Get_cross_mix_time(uint8_t p, uint16_t mix_samples)

                        if (false)
                        {
                            Serial.print("mix_sample 32 is given to Player ");
                            Serial.print(p);
                            Serial.print("; theorical update time is:");
                            Serial.println(Get_cross_mix_time(p, 32) + (_Player + p)->Read_update_time());
                        }
                    }
                }
                finished = true;
            }
        }

        if (!finished)
        {
            // 4: Try to assign minimum  mix_samples to Players
            for (uint8_t p = 0; p < PLAYERS; p++)
            {
                if (cross_mix_Player[p])
                {
                    if (standard_pitch_of_Player[p] <= 0.7)
                    {
                        cross_mix_Player[p] = false;
                        players_to_cross_mix--;
                        mix_samples_for_Player[p] = 64;
                        (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                        mix_micros_span -= Get_cross_mix_time(p, 64);

                        if (false)
                        {
                            Serial.print("mix_sample 64 is given to Player ");
                            Serial.print(p);
                            Serial.print("; theorical update time is:");
                            Serial.println(Get_cross_mix_time(p, 64) + (_Player + p)->Read_update_time());
                        }
                    }
                    else if (standard_pitch_of_Player[p] <= 0.8)
                    {
                        cross_mix_Player[p] = false;
                        players_to_cross_mix--;
                        mix_samples_for_Player[p] = 48;
                        (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                        mix_micros_span -= Get_cross_mix_time(p, 48);

                        if (false)
                        {
                            Serial.print("mix_sample 48 is given to Player ");
                            Serial.print(p);
                            Serial.print("; theorical update time is:");
                            Serial.println(Get_cross_mix_time(p, 48) + (_Player + p)->Read_update_time());
                        }
                    }
                    else if (standard_pitch_of_Player[p] <= 1.0)
                    {
                        cross_mix_Player[p] = false;
                        players_to_cross_mix--;
                        mix_samples_for_Player[p] = 32;
                        (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                        mix_micros_span -= Get_cross_mix_time(p, 32);

                        if (false)
                        {
                            Serial.print("mix_sample 32 is given to Player ");
                            Serial.print(p);
                            Serial.print("; theorical update time is:");
                            Serial.println(Get_cross_mix_time(p, 32) + (_Player + p)->Read_update_time());
                        }
                    }
                    else if (standard_pitch_of_Player[p] <= 1.2)
                    {
                        cross_mix_Player[p] = false;
                        players_to_cross_mix--;
                        mix_samples_for_Player[p] = 24;
                        (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                        mix_micros_span -= Get_cross_mix_time(p, 24);

                        if (false)
                        {
                            Serial.print("mix_sample 24 is given to Player ");
                            Serial.print(p);
                            Serial.print("; theorical update time is:");
                            Serial.println(Get_cross_mix_time(p, 24) + (_Player + p)->Read_update_time());
                        }
                    }
                    else if (standard_pitch_of_Player[p] <= 1.4)
                    {
                        cross_mix_Player[p] = false;
                        players_to_cross_mix--;
                        mix_samples_for_Player[p] = 16;
                        (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                        mix_micros_span -= Get_cross_mix_time(p, 16);

                        if (false)
                        {
                            Serial.print("mix_sample 16 is given to Player ");
                            Serial.print(p);
                            Serial.print("; theorical update time is:");
                            Serial.println(Get_cross_mix_time(p, 16) + (_Player + p)->Read_update_time());
                        }
                    }
                    if (players_to_cross_mix == 0)
                    {
                        finished = true;
                        break;
                    }
                    if (mix_micros_span <= 10)
                    {
                        // Serial.print("Available time is finished :( Players to examin are:");
                        // Serial.println(players_to_cross_mix);
                        break;
                    }
                }
            }
        }

        if (!finished)
        {
            for (uint8_t p = 0; p < PLAYERS; p++)
            {
                if (cross_mix_Player[p])
                {
                    cross_mix_Player[p] = false;
                    players_to_cross_mix--;
                    mix_samples_for_Player[p] = 5;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                    // Serial.print("Mix_sample 5 is given to Player ");
                    // Serial.println(p);
                }
            }
        }

        // Serial.print("FINISHED - Midi_reader: mix_samples calculation required (micros):");
        // Serial.println(micros() - execution_time);
        // Serial.println();
    }
}

bool PlayersManager::Get_use_Wavetable(int id_sound)
{
    return ((Sound[id_sound].B - Sound[id_sound].A + 1) <= BLOCK_MIN); // BLOCK_MIN is defined in Lilla_player.h
}

int8_t PlayersManager::Find_oldest_player(int instrument, bool power_on, bool playing)
{
    unsigned long time_min = 0;
    uint8_t p, q;
    int8_t result = 0;
    for (p = 0; p < PLAYERS; p++)
    {
        if (((_Player + p)->Read_instrument() == instrument) && ((_Player + p)->isPoweredOn() == power_on) && ((_Player + p)->isPlaying() == playing))
        {
            time_min = (_Player + p)->Read_time_stamp();
            result = p;
            for (q = 0; q < PLAYERS; q++) // look for the player playing for the longest time
            {
                if (((_Player + q)->Read_instrument() == instrument) && ((_Player + q)->isPoweredOn() == power_on) && ((_Player + q)->isPlaying() == playing) && ((_Player + q)->Read_time_stamp() < time_min))
                {
                    time_min = (_Player + q)->Read_time_stamp();
                    result = q;
                }
            }
            return result;
        }
    }
    return -1;
}

void PlayersManager::Release_player(int player, int track) // after receiving a NoteOff command
{
    (_Player + player)->Release_note();
    (_Player + player)->Write_time_stamp(millis());
}

bool PlayersManager::Verify_if_stop_players(int instrument) // when EDITING a SOUND, each time B or A change it's MANDATORY to test if SOME Players MUST be stopped
{
    uint8_t players_critical = 0;
    uint8_t players_to_stop = 0;
    int8_t index = 0;

    if ((POLYPHONY_FLASH[optimization] < PLAYERS) && Preset[instrument].use_Wavetable && !Get_use_Wavetable(ID_SOUND)) // Sound passes from use_Wavetable to !use_Wavetable
    {
        // players_critical  = how many Player ARE GOING to !use_Wavetable (those playing "instrument") + how many Player are ALREDY !use_Wavetable
        for (int player = 0; player < PLAYERS; ++player)
        {
            if ((_Player + player)->isPlaying() && ((_Player + player)->Read_instrument() == instrument || !(_Player + player)->Read_use_Wavetable())) // if player is playing instrument or is playing in Flash mode -> counter increase
                players_critical++;
        }

        if (players_critical > POLYPHONY_FLASH[optimization]) // players in excess MUST be stopped BEFORE UPDATING A and B
        {
            players_to_stop = players_critical - POLYPHONY_FLASH[optimization];

            // 1) look for a Player !power_on and playing "instrument": choose the OLDEST
            while (players_to_stop > 0)
            {
                index = Find_oldest_player(instrument, false, true); // Find_oldest_player(uint8_t instrument, bool power_on, bool playing)
                if (index == -1)
                    break;
                else
                {
                    (_Player + index)->Fast_stop();
                    players_to_stop--;
                }
            }

            // 2) look for a Player power_on playing "instrument": choose the OLDEST
            while (players_to_stop > 0)
            {
                index = Find_oldest_player(instrument, true, true); // Find_oldest_player(uint8_t instrument, bool power_on, bool playing)
                if (index == -1)
                    break;
                else
                {
                    (_Player + index)->Fast_stop();
                    (_Player + index)->Write_time_stamp(millis());
                    players_to_stop--;
                }
            }
            return true; // some Player have been stopped
        }
        return false; // no need to stop any Player
    }
    return false; // no need to stop any Player
}

void PlayersManager::Release_player(int player) // after receiving a NoteOff command
{
    (_Player + player)->Release_note();
    (_Player + player)->Write_time_stamp(millis());
}

void PlayersManager::Release_all_players_for_instrument(int instrument) // after receiving a NoteOff command
{
    for (int player = 0; player < PLAYERS; ++player)
        if ((_Player + player)->isPoweredOn() && (_Player + player)->Read_instrument() == instrument)
            Release_player(player);
}

void PlayersManager::Release_all_players_for_instrument_solo(int instrument)
{
    for (int player = 0; player < PLAYERS; ++player)
    {
        if ((_Player + player)->isPlaying() && ((_Player + player)->Read_instrument() != instrument))
        {

            if ((_Player + player)->isPoweredOn())
            {
                (_Player + player)->Fast_stop();
            }
        }
        // Map_one_Instrument_for_all_notes(instrument); // E' PALESEMENTE UN ERRORE!!!
    }
}

void PlayersManager::Release_all_players(void)
{
    for (int player = 0; player < PLAYERS; ++player)
        Release_player(player);
}

void PlayersManager::Release_softly_all_players(int session)
{
    for (int player = 0; player < PLAYERS; ++player)
        if ((_Player + player)->isPoweredOn() && (_Player + player)->Read_local_session() == session)
            Release_player(player); // avvio del Release stabilito per il Sound
}

void PlayersManager::Stop_all_players(void) // meglio Fast_stop...
{
    for (int player = 0; player < PLAYERS; ++player)
    {
        (_Player + player)->Fast_stop(); // Attiva il Release (ADSR) con tempo di caduta pari a 10 samples
        (_Player + player)->Write_time_stamp(millis());
    }
}

void PlayersManager::Release_all_players_loop(void)
{
    for (int player = 0; player < PLAYERS; ++player)
        if ((_Player + player)->isPoweredOn() && ((_Player + player)->Read_loop_track() >= 0))
            Release_player(player);
}

void PlayersManager::Release_all_players_loop(int track)
{
    for (int player = 0; player < PLAYERS; ++player)
        if ((_Player + player)->isPoweredOn() && ((_Player + player)->Read_loop_track() == track))
            Release_player(player);
}

float PlayersManager::Get_cross_mix_time(int player, int mix_samples) // restituisce il tempo (us) che si impiega a leggere i campioni che costituiscono il mix_samples, dal vecchio file e col vecchio pitch.
{
    if ((_Player + player)->Read_use_Wavetable()) // how Player read OLD FILE
        return (mix_samples * (0.03 * (_Player + player)->Read_pitch() + 0.2));
    else
        return (mix_samples * (0.4 * (_Player + player)->Read_pitch() + 0.2));
}

int PlayersManager::Get_span_for_all_cross_mix(void) // ALERT: can be used if Play_note() HAS BEEN ALREADY SENT!
{
    const int audio_time_reserved = 200; // microsecondi riservati per tutti gli altri oggetti in un ciclo update()
    int value = 0;
    for (int p = 0; p < PLAYERS; ++p)
    {
        value += (_Player + p)->Read_update_time();
    }
    return (2900 - audio_time_reserved - value); // save 200 microseconds for other processes!
}

void PlayersManager::Multicast_reset_pitch_bend_effects(int instrument)
{
    for (int p = 0; p < PLAYERS; ++p)
    {
        if ((_Player + p)->Read_instrument() == instrument)
        {
            (_Player + p)->Set_pitch_bend(1.0);
            (_Player + p)->Set_effects(16.0, 0);
        }
    }
}

void PlayersManager::Broadcast_pitch_bend(int midi_channel, float value)
{
    for (uint8_t p = 0; p < PLAYERS; ++p)
    {
        if (!Preset[(_Player + p)->Read_instrument()].lock && Preset[(_Player + p)->Read_instrument()].midi_channel == midi_channel) // if(!bitRead(Session[session].Instrument[(_Player + p)->instrument].info, 1) && (Get_midi_channel(session, (_Player + p)->instrument) == midi_channel))
            (_Player + p)->Set_pitch_bend(value);
    }
}

void PlayersManager::Broadcast_restore_pitch_bend_and_effects(int midi_channel, float value)
{
    for (int p = 0; p < PLAYERS; ++p)
    {
        if (!Preset[(_Player + p)->Read_instrument()].lock && Preset[(_Player + p)->Read_instrument()].midi_channel == midi_channel) // if(!bitRead(Session[session].Instrument[(_Player + p)->instrument].info, 1))
        {
            (_Player + p)->Set_pitch_bend(value);
            (_Player + p)->Set_effects(Calc_resolution(resolution), downsampling);
        }
    }
}

void PlayersManager::Multicast_volume_for_MIDI_LOOP_running(int track, float value)
{
    for (int p = 0; p < PLAYERS; ++p)
    {
        if (((_Player + p)->Read_loop_track() == track) && (_Player + p)->isPlaying())
        {
            (_Player + p)->Update_volume(value * Preset[(_Player + p)->Read_instrument()].volume);
        }
    }
}

void PlayersManager::Update_players_stistics(void)
{
    players_using_Wavetable = 0;
    players_using_Flash = 0;
    for (int p = 0; p < PLAYERS; ++p)
    {
        if ((_Player + p)->isPlaying())
        {
            if ((_Player + p)->Read_use_Wavetable())
                ++players_using_Wavetable;
            else
                ++players_using_Flash;
        }
    }
    players_playing = players_using_Wavetable + players_using_Flash;
}

int PlayersManager::Get_players_playing(void)
{
    return players_playing;
}

int PlayersManager::Get_players_using_Flash(void)
{
    return players_using_Flash;
}

int PlayersManager::Get_players_using_Wavetable(void)
{
    return players_using_Wavetable;
}

void PlayersManager::Calculate_and_set_mix_samples(void)
{
    uint8_t mix_samples_for_Player[PLAYERS] = {0};

    for (uint8_t p = 0; p < PLAYERS; p++)
        mix_samples_for_Player[p] = 0;

    bool finished = false;

    int mix_micros_span = Get_span_for_all_cross_mix(); // IMP! Get_span_for_all_cross_mix() CAN be called ONLY if Play_note() HAS BEEN ALREADY CALLED!
    // Serial.print("We have ");
    // Serial.print(mix_micros_span);
    // Serial.println(" micros available for ALL cross_mix.");

    bool old_use_wavetable[PLAYERS];
    float old_pitch_of_Player[PLAYERS];
    float standard_pitch_of_Player[PLAYERS];

    // Calculate the standardized "flash-play" pitch: pitch of RAM-playing Players will be normalized so they can be traated as Flash-playing Players
    for (uint8_t p = 0; p < PLAYERS; p++)
    {
        if (Get_restart_player(p))
        {
            old_use_wavetable[p] = (_Player + p)->Read_use_Wavetable();
            old_pitch_of_Player[p] = (_Player + p)->Read_pitch();

            if (old_use_wavetable[p])
                standard_pitch_of_Player[p] = old_pitch_of_Player[p] * 0.07113;
            else
                standard_pitch_of_Player[p] = old_pitch_of_Player[p];

            if (false)
            {
                Serial.print("Player:");
                Serial.print(p);
                Serial.print(" has standard_pitch:");
                Serial.println(standard_pitch_of_Player[p]);
            }
        }
    }
    // Serial.println();

    // find highest-pitch / highest-time Player
    int max_cross_mix_time;

    uint8_t worst_Player = 0;
    float max_pitch = 0;
    for (uint8_t p = 0; p < PLAYERS; p++)
    {
        if (Get_restart_player(p))
        {
            if (standard_pitch_of_Player[p] > max_pitch)
            {
                max_pitch = standard_pitch_of_Player[p];
                worst_Player = p;
            }
        }
    }

    if (false)
    {
        Serial.print("Worst Player (with highest pitch) is:");
        Serial.print(worst_Player);
        Serial.print(" with pitch:");
        Serial.println(max_pitch);
        Serial.println();
    }

    // 1) Try to set mix_samples = 64 for ALL Players
    max_cross_mix_time = Get_cross_mix_time(worst_Player, 64);
    // Serial.print("Try Case 1 - max_cross_mix_time (micros) is:");
    // Serial.println(max_cross_mix_time);
    // Serial.println();

    if (mix_micros_span > (max_cross_mix_time * players_to_restart))
    {
        // Serial.println("Case 1 is verified --> transmit mix_samples = 64 to Players to restart.");
        for (uint8_t p = 0; p < PLAYERS; p++)
        {
            if (Get_restart_player(p))
            {
                Cancel_restart_player(p);
                players_to_restart--;
                mix_samples_for_Player[p] = 32;
                (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);

                mix_micros_span -= Get_cross_mix_time(p, 32);

                if (false)
                {
                    Serial.print("mix_sample 64 is given to Player ");
                    Serial.print(p);
                    Serial.print("; theorical update time is:");
                    Serial.println(Get_cross_mix_time(p, 64) + (_Player + p)->Read_update_time());
                }
            }
        }
        finished = true;
    }

    // 2) Try to set mix_samples = 48 for ALL Players
    if (!finished)
    {
        max_cross_mix_time = Get_cross_mix_time(worst_Player, 48);
        // Serial.print("Try Case 2 - max_cross_mix_time (micros) is:");
        // Serial.println(max_cross_mix_time);
        // Serial.println();

        if (mix_micros_span > (max_cross_mix_time * players_to_restart))
        {
            // Serial.println("Case 2 is verified --> transmit mix_samples = 48 to Players to restart.");
            for (uint8_t p = 0; p < PLAYERS; p++)
            {
                if (Get_restart_player(p))
                {
                    Cancel_restart_player(p);
                    players_to_restart--;
                    mix_samples_for_Player[p] = 48;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);

                    mix_micros_span -= Get_cross_mix_time(p, 48);
                    if (false)
                    {
                        Serial.print("mix_sample 48 is given to Player ");
                        Serial.print(p);
                        Serial.print("; theorical update time is:");
                        Serial.println(Get_cross_mix_time(p, 48) + (_Player + p)->Read_update_time());
                    }
                }
            }
            finished = true;
        }
    }

    // 3) Try to set mix_samples = 32 for ALL Players
    if (!finished)
    {
        max_cross_mix_time = Get_cross_mix_time(worst_Player, 32);
        // Serial.print("Try Case 3 - max_cross_mix_time (micros) is:");
        // Serial.println(max_cross_mix_time);
        // Serial.println();

        if (mix_micros_span > (max_cross_mix_time * players_to_restart))
        {
            // Serial.println("Case 3 is verified --> transmit mix_samples = 32 to Players to restart.");
            for (uint8_t p = 0; p < PLAYERS; p++)
            {
                if (Get_restart_player(p))
                {
                    Cancel_restart_player(p);
                    players_to_restart--;
                    mix_samples_for_Player[p] = 32;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);

                    mix_micros_span -= Get_cross_mix_time(p, 32);

                    if (false)
                    {
                        Serial.print("mix_sample 32 is given to Player ");
                        Serial.print(p);
                        Serial.print("; theorical update time is:");
                        Serial.println(Get_cross_mix_time(p, 32) + (_Player + p)->Read_update_time());
                    }
                }
            }
            finished = true;
        }
    }

    if (!finished)
    {
        // 4: Try to assign minimum  mix_samples to Players
        for (uint8_t p = 0; p < PLAYERS; p++)
        {
            if (Get_restart_player(p))
            {
                if (standard_pitch_of_Player[p] <= 0.7)
                {
                    Cancel_restart_player(p);
                    players_to_restart--;
                    mix_samples_for_Player[p] = 64;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                    mix_micros_span -= Get_cross_mix_time(p, 64);

                    if (false)
                    {
                        Serial.print("mix_sample 64 is given to Player ");
                        Serial.print(p);
                        Serial.print("; theorical update time is:");
                        Serial.println(Get_cross_mix_time(p, 64) + (_Player + p)->Read_update_time());
                    }
                }
                else if (standard_pitch_of_Player[p] <= 0.8)
                {
                    Cancel_restart_player(p);
                    players_to_restart--;
                    mix_samples_for_Player[p] = 48;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                    mix_micros_span -= Get_cross_mix_time(p, 48);

                    if (false)
                    {
                        Serial.print("mix_sample 48 is given to Player ");
                        Serial.print(p);
                        Serial.print("; theorical update time is:");
                        Serial.println(Get_cross_mix_time(p, 48) + (_Player + p)->Read_update_time());
                    }
                }
                else if (standard_pitch_of_Player[p] <= 1.0)
                {
                    Cancel_restart_player(p);
                    players_to_restart--;
                    mix_samples_for_Player[p] = 32;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                    mix_micros_span -= Get_cross_mix_time(p, 32);

                    if (false)
                    {
                        Serial.print("mix_sample 32 is given to Player ");
                        Serial.print(p);
                        Serial.print("; theorical update time is:");
                        Serial.println(Get_cross_mix_time(p, 32) + (_Player + p)->Read_update_time());
                    }
                }
                else if (standard_pitch_of_Player[p] <= 1.2)
                {
                    Cancel_restart_player(p);
                    players_to_restart--;
                    mix_samples_for_Player[p] = 24;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                    mix_micros_span -= Get_cross_mix_time(p, 24);

                    if (false)
                    {
                        Serial.print("mix_sample 24 is given to Player ");
                        Serial.print(p);
                        Serial.print("; theorical update time is:");
                        Serial.println(Get_cross_mix_time(p, 24) + (_Player + p)->Read_update_time());
                    }
                }
                else if (standard_pitch_of_Player[p] <= 1.4)
                {
                    Cancel_restart_player(p);
                    players_to_restart--;
                    mix_samples_for_Player[p] = 16;
                    (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                    mix_micros_span -= Get_cross_mix_time(p, 16);

                    if (false)
                    {
                        Serial.print("mix_sample 16 is given to Player ");
                        Serial.print(p);
                        Serial.print("; theorical update time is:");
                        Serial.println(Get_cross_mix_time(p, 16) + (_Player + p)->Read_update_time());
                    }
                }
                if (players_to_restart == 0)
                {
                    finished = true;
                    break;
                }
                if (mix_micros_span <= 10)
                {
                    // Serial.print("Available time is finished :( Players to examin are:");
                    // Serial.println(players_to_restart);
                    break;
                }
            }
        }
    }

    if (!finished)
    {
        for (uint8_t p = 0; p < PLAYERS; p++)
        {
            if (Get_restart_player(p))
            {
                Cancel_restart_player(p);
                players_to_restart--;
                mix_samples_for_Player[p] = 0;
                (_Player + p)->Set_mix_samples(mix_samples_for_Player[p]);
                // Serial.print("Mix_sample ZERO is given to Player ");
                // Serial.println(p);
            }
        }
    }
}

void PlayersManager::Reset_players_to_restart(void)
{
    players_to_restart = 0;
}

int PlayersManager::Get_players_to_restart(void)
{
    return players_to_restart;
}

void PlayersManager::Multicast_update_vibrato(int midi_channel, bool vibrato_active)
{
    for (int p = 0; p < PLAYERS; ++p)
    {
        if ((_Player + p)->Read_midi_channel() == midi_channel)
            (_Player + p)->Set_vibrato_flag(vibrato_active);
    }
}

void PlayersManager::Multicast_all_notes_off(int midi_channel)
{
    for (int p = 0; p < PLAYERS; ++p)
        if ((_Player + p)->isPoweredOn() && (_Player + p)->Read_midi_channel() == midi_channel)
            Release_Player_noteOff(p);
}

void PlayersManager::Multicast_stop_players_for_loop_track(int track)
{
    for (uint8_t p = 0; p < PLAYERS; ++p)
        if ((_Player + p)->isPoweredOn() && (_Player + p)->Read_loop_track() == track)
        {
            Release_Player_noteOff(p, track);
        }
}

void PlayersManager::Multicast_stop_players_for_NoteOff(int midi_channel, int note_number, int track)
{
    for (uint8_t p = 0; p < PLAYERS; ++p)
        if ((_Player + p)->isPoweredOn() && (_Player + p)->Read_note() == note_number && (_Player + p)->Read_midi_channel() == midi_channel && (_Player + p)->Read_loop_track() == track)
        {
            Release_Player_noteOff(p, track);
        }
}

void PlayersManager::Broadcast_FIFO_stereo(int16_t *_LS_buffer_L, int16_t *_LS_buffer_R)
{
    for (int p = 0; p < PLAYERS; p++)
    {
        (_Player + p)->_LS_buffer_L = _LS_buffer_L;
        (_Player + p)->_LS_buffer_R = _LS_buffer_R;
    }
}

void PlayersManager::Broadcast_FIFO_mono(int16_t *_LS_buffer_mono)
{
    for (int p = 0; p < PLAYERS; p++)
    {
        (_Player + p)->_LS_buffer_mono = _LS_buffer_mono;
    }
}