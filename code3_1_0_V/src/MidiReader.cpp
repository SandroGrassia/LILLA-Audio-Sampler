/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 * Credits: François Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)
 *
 */

#include <MidiReader.h>

void MidiReader::Begin(void)
{
    MIDI.begin(MIDI_CHANNEL_OMNI);
}

void MidiReader::Start(void)
{
    Reset_keys_state();
    midi_message_received = 0; // no message
    midi_stop_flag = false;
}

void MidiReader::Update(void)
{
    // monitor Players execution time
    /*
    monitor_lap ++;
    if((monitor_lap%300)=0)
    {
        monitor_timer = 0;
        monitor_number = 0;
        for(uint8_t p = 0; p < PLAYERS; ++p)
            if((_Player + p)->isPlaying())
            {
                PRINT("Midi_Reader - ", " Player: ", p);
                      PRINT("Midi_Reader - ", " monitor_execution: ", (_Player + p)->monitor_execution);
            }
        Serial.println();

        if((_Player + p)->isPlaying())
        {
            monitor_timer += (_Player + p)->monitor_execution;
            monitor_number ++;
        }
        PRINT("Midi_Reader - ", " Players running: ", monitor_number);
        PRINT("Midi_Reader - ", " All Players run in microseconds: ", monitor_timer);
        PRINT("Midi_Reader - ", "AudioMemoryUsageMax() is:", AudioMemoryUsageMax());
    }
    */

    uint8_t velocity;
    float velocity_float;
    uint8_t midi_channel; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
    uint8_t note_number;
    int controller;
    uint8_t midi_value;

    if (Lilla_state == CC_SETTINGS && !display_wait)
    {
        if (MIDI.read())
            if (MIDI.getType() == midi::ControlChange)
            {
                CC_midi_controller = MIDI.getData1();
                display_wait = true;
            }
    }

    if (LOOP_learn_flag && (Lilla_state == MIDI_LOOP))
    {
        if (LOOP_learning_track > 0 && LOOP_elements > 0 && LOOP_learn_clock >= LOOP_time)
        {
            LOOP_learn_flag = false;
            Serial.println("Loop chiuso per raggiunta durata massima (LOOP_time)!");
        }
    }

    if (Lilla_state != CC_SETTINGS && !midi_stop_flag)
    {
        if (MIDI.read())
        {
            switch (MIDI.getType())
            {
            case midi::NoteOn:
            {
                midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                note_number = MIDI.getData1();
                velocity = MIDI.getData2();

                // Loop learning
                if ((Lilla_state == MIDI_LOOP) & LOOP_learn_flag)
                {
                    // Solo in caso di NoteOn
                    if (LOOP_elements == 0)
                    {
                        LOOP_elements = 1;
                        LOOP_last_event = 0;

                        if (LOOP_learning_track == 0)
                        {
                            // Chiede a Main l'accensione del primo led del metronomo
                            LOOP_metronomo_flag_IN[0] = true;
                        }

                        // Si azzera il contatempo
                        LOOP_learn_clock = 0;
                    }

                    // Aggiorna il conteggio degli elementi
                    else
                    {
                        LOOP_elements++;
                        LOOP_last_event++;
                    }

                    // Memorizza il tempo
                    if (LOOP_learning_track == 0)
                    {
                        LOOP_clock_memo = LOOP_learn_clock;
                    }
                    else
                    {
                        LOOP_clock_memo = LOOP_Clock() % LOOP_time;
                    }

                    LOOP_element[LOOP_learning_track][LOOP_last_event].time = LOOP_clock_memo;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].midi_channel = midi_channel;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].note_number = note_number;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].velocity = velocity;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].note_on = true;

                    // Ultimo evento accettabile
                    if (LOOP_elements == (LOOP_EVENTS - 1))
                    {
                        LOOP_learn_flag = false;
                        Serial.println("Loop chiuso per raggiunto limite eventi!");
                    }
                }

                if (tuning_tone_flag)
                {
                    tuning_tone_last_note = note_number;
                    Tone_generator_ptr->Frequency(pitch_from_note[note_number] * 261.63);
                    Tone_generator_ptr->Amplitude(Volume_float[tuning_tone_volume]);
                    Tone_generator_ptr->Start();
                    TT_playing = true;
                    TT_led_flag = true;
                }

                velocity_float = velocity / 127.0f;
                key_state[midi_channel][note_number] = true; // real key

                // Player_booked serve ad evitare che gli Instrument (quindi anche piu' di uno) che sono attivati da NoteOn non competano sullo stesso Player
                // players_to_restart, se risultera' >0, richiede il calcolo dei vari valori di samples (mix_samples_for_Player[p])
                // che ciascun Player da riavviare (restart_Player[p] == true) dovra' utilizzare
                Players_Manager_ptr->Reset_booked_and_restart_player();
                Players_Manager_ptr->Reset_players_to_restart();

                for (auto instrument = 0; instrument < INSTRUMENTS_MAX; ++instrument)
                {
                    if ((Patch[Patch_id].Instrument[instrument].used) && bitRead(map_instrument_for_note[midi_channel][note_number], instrument))
                    {
                        Players_Manager_ptr->Play_note(instrument, note_number, velocity_float, NO_TRACK);
                    }
                }

                // Tramite Play_note e i Player individuati, sono stati determinati i valori:
                // - players_to_restart
                // - restart_Player[] = true/false, per ciascun Player da avviare o riavviare
                // - update_time della nuova esecuzione, per ciascun Player da avviare o riavviare

                // Ora si calcola e si trasmette mix_samples (numero di samples del cross mix) a ciascun Player da riavviare.
                if (Players_Manager_ptr->Get_players_to_restart() > 0)
                {
                    Players_Manager_ptr->Calculate_and_set_mix_samples();
                }

                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 1;      // note ON
                    MM_midi_channel = midi_channel; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                    MM_note_number = note_number;
                    MM_velocity = velocity;
                    display_wait = true;
                }
            }
            break;

            case midi::NoteOff:
            {
                midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                note_number = MIDI.getData1();
                velocity = MIDI.getData2();

                // Loop learning
                if ((Lilla_state == MIDI_LOOP) && LOOP_learn_flag && LOOP_elements > 0)
                {
                    // Aggiorna il conteggio degli elementi
                    LOOP_elements++;
                    LOOP_last_event++;

                    // Memorizza il tempo
                    if (LOOP_learning_track == 0)
                    {
                        LOOP_clock_memo = LOOP_learn_clock;
                    }
                    else
                    {
                        LOOP_clock_memo = LOOP_Clock() % LOOP_time;
                    }

                    LOOP_element[LOOP_learning_track][LOOP_last_event].time = LOOP_clock_memo;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].midi_channel = midi_channel;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].note_number = note_number;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].velocity = velocity;
                    LOOP_element[LOOP_learning_track][LOOP_last_event].note_on = false;

                    // Ultimo evento accettabile
                    if (LOOP_elements == LOOP_EVENTS - 1)
                    {
                        LOOP_learn_flag = false;
                        // Serial.println("Loop chiuso per raggiunto limite eventi!");
                    }
                }

                if (tuning_tone_flag && (tuning_tone_last_note == note_number))
                {
                    Tone_generator_ptr->Stop();
                    TT_playing = false;
                    TT_led_flag = true;
                }

                key_state[midi_channel][note_number] = false; // real key
                Players_Manager_ptr->Multicast_stop_players_for_NoteOff(midi_channel, note_number, -1);

                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 2;      // note OFF
                    MM_midi_channel = midi_channel; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                    MM_note_number = note_number;
                    MM_velocity = velocity;
                    display_wait = true;
                    break;
                }
            }
            break;

            case midi::PitchBend:
            {
                // PRINT("PitchBend", "", (MIDI.getData2() << 7) + MIDI.getData1());
                midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                MM_pitch_bend_least = MIDI.getData1();
                MM_pitch_bend_most = MIDI.getData2();

                pitch_bend_value[midi_channel] = ((MM_pitch_bend_most << 7) + MM_pitch_bend_least) / 16384.0f + 0.5f;
                Players_Manager_ptr->Broadcast_pitch_bend(midi_channel, pitch_bend_value[midi_channel]);

                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 3;      // pitch bend
                    MM_midi_channel = midi_channel; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                    display_wait = true;
                    break;
                }
            }
            break;

            case midi::AfterTouchChannel: // Status = 1101nnnn Pressure value = 0vvvvvvv
            {
                midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16);
                midi_value = MIDI.getData1();
                after_touch_channel_value[midi_channel] = midi_value / 127.0f;

                for(auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                    if ((Preset[instrument_local].midi_channel == midi_channel) && (Preset[instrument_local].Filter.use == 1) && (Preset[instrument_local].Filter.modulation == 4))
                    {
                        Players_Manager_ptr->Multicast_IF_index(instrument_local, Preset[instrument_local].Filter.index * after_touch_channel_value[midi_channel]);
                    }
                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 7;      // After Touch Channel
                    MM_midi_channel = midi_channel; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                    MM_least_bits = midi_value;
                    display_wait = true;
                    break;
                }
            }
            break;

            case midi::ControlChange: // Status = 1011nnnn  Controller = 0ccccccc (da 0 a 120)  Value = 0vvvvvvv
            {
                // PRINT("ControlChange", "MIDI.getData1()", MIDI.getData1());
                // PRINT("ControlChange", "MIDI.getData2()", MIDI.getData2());
                midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                controller = MIDI.getData1();
                midi_value = MIDI.getData2();

                if (controller == 123) // All sound off
                {
                    // PRINT("All Note off", " data 1:", 123);
                    Players_Manager_ptr->Multicast_all_notes_off(midi_channel);
                }

                else if (controller == 126) // All notes off
                {
                    // PRINT("All Note off", " data 1:", 126);
                    Players_Manager_ptr->Multicast_all_notes_off(midi_channel);
                }

                else if (controller == 1) // Modulation
                {
                    // PRINT("Mod", "", midi_value);
                    if (midi_value > 0)
                    {
                        Vibrato_ptr->Update_vibrato_array(midi_value);
                        if (!vibrato_flag)
                        {
                            vibrato_flag = true;
                            Players_Manager_ptr->Multicast_update_vibrato(midi_channel, vibrato_flag);
                        }
                    }
                    else
                    {
                        vibrato_flag = false;
                        Players_Manager_ptr->Multicast_update_vibrato(midi_channel, vibrato_flag);
                    }
                }

                else if ((controller == CC_lowpass_filter) && (controller > 0)) // change the lowpass filter cut frequency
                {
                    lowpass_target = midi_value * 0.31; // <= 39
                    if (abs(lowpass_target - lowpass) > 0)
                    {
                        lowpass_flag = true;
                        lowpass_direction = lowpass_target > lowpass;
                        display_lowpass_flag = true;
                    }
                }

                else
                {
                    for(auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                    {
                        if ((controller == CC_Sound_gain[instrument_local]) && (controller > 0))
                        {
                            if (Patch[Patch_id].Instrument[instrument_local].used && (Get_midi_channel(Patch_id, instrument_local) == midi_channel))
                            {
                                Sound[Patch[Patch_id].Instrument[instrument_local].sound_id].gain = (float)midi_value * 0.315; // 127 --> 40
                                Players_Manager_ptr->Update_Preset_volume(Patch_id, instrument_local, Volume_float[volume_patch]);
                                Players_Manager_ptr->Multicast_volume_for_instrument_edit(instrument_local);
                                if (Lilla_state == PERFORMANCE)
                                {
                                    instrument_volume_changed = instrument_local;
                                    display_instrument_volume_flag = true;
                                }
                            }
                        }
                    }
                }

                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 5;      // control change
                    MM_midi_channel = midi_channel; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                    MM_midi_controller = controller;
                    MM_midi_value = midi_value;
                    display_wait = true;
                    break;
                }
            }
            break;

            case midi::AfterTouchPoly:
                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 4;               // after touch poly
                    MM_midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16)
                    MM_least_bits = MIDI.getData1();
                    MM_most_bits = MIDI.getData2();
                    display_wait = true;
                }
                break;

            case midi::ProgramChange:
                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 6;
                    MM_midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16);
                    MM_least_bits = MIDI.getData1();
                    display_wait = true;
                }
                break;

            case midi::SystemExclusive:
                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    midi_message_received = 8;               // System Exclusive
                    MM_midi_channel = MIDI.getChannel() - 1; // from 0 (MIDI CH. 1) to 15 (MIDI CH.16);
                    display_wait = true;
                }
                break;

            default:
                if (Lilla_state == MIDI_MONITOR && !display_wait)
                {
                    display_wait = true;
                }
                break;
            }
        }
    }

    // Eseguito se ci sono track running
    if (Lilla_state == MIDI_LOOP || (Lilla_state_0 == MIDI_LOOP && (Lilla_state == DELAY_SETTINGS || Lilla_state == SOUND_EDIT || Lilla_state == INSTRUMENT_VCF || Lilla_state == INSTRUMENT_VCF || Lilla_state == MIXER)))
    {
        unsigned long LOOP_Clock_frozen = LOOP_Clock(); // congela LOOP_Clock()

        // update metronomo
        if (LOOP_metronomo_run && (LOOP_Clock_frozen >= LOOP_metronomo.metro_time))
        {
            if (Lilla_state == MIDI_LOOP) // chiede a track() di aggiornare il metronomo
            {
                LOOP_metronomo_flag_IN[1] = true;
            }
            else
            {
                LOOP_metronomo.Update(false); // comanda lo switch del solo beat
                LOOP_metronomo.metro_time += LOOP_metronomo.Read_metro_delta_ms();
            }
        }

        // check notes to play/stop
        for(auto track = 0; track < TRACKS; ++track)
        {
            if (LOOP_track_run[track] && (LOOP_Clock_frozen >= LOOP_play_time[track]))
            {
                int event = LOOP_play_event[track];

                // Prima del primo evento, tutti i Player di track devono aver gia' ricevuto NoteOff, altrimenti gli vengono inviati
                if (event == 0)
                {
                    Players_Manager_ptr->Multicast_stop_players_for_loop_track(track);
                }
                midi_channel = LOOP_element[track][event].midi_channel;
                note_number = constrain(LOOP_element[track][event].note_number + LOOP_pitch_int[track], 0, 127);
                velocity = LOOP_element[track][event].velocity;
                velocity_float = velocity / 127.0f;

                if (LOOP_element[track][event].note_on)
                {
                    if (tuning_tone_flag)
                    {
                        tuning_tone_last_note = note_number;
                        Tone_generator_ptr->Frequency(pitch_from_note[note_number] * 261.63);
                        Tone_generator_ptr->Amplitude(Volume_float[tuning_tone_volume]);
                        Tone_generator_ptr->Start();
                        TT_playing = true;
                        TT_led_flag = true;
                    }

                    // Player_booked serve ad evitare che gli Instrument che sono attivati da NoteOn non competano sullo stesso Player
                    // players_to_restart, se risultera' >0, richiede il calcolo dei vari valori di samples (mix_samples_for_Player[p])
                    // che ciascun Player da riavviare (restart_Player[p] == true) dovra' utilizzare
                    Players_Manager_ptr->Reset_booked_and_restart_player();

                    // players_to_restart, se risultera' >0, richiede il calcolo dei vari valori di samples (mix_samples_for_Player[p])
                    Players_Manager_ptr->Reset_players_to_restart();

                    // Inoltra note_on
                    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                    {
                        if ((Patch[Patch_id].Instrument[instrument_local].used) && bitRead(map_instrument_for_note[midi_channel][note_number], instrument_local))
                        {
                            Players_Manager_ptr->Play_note(instrument_local, note_number, velocity_float, track);
                        }
                    }

                    // Tramite PLAY_note e i Player individuati, sono stati determinati i valori:
                    // - players_to_restart
                    // - restart_Player[p] = true/false, per ciascun Player da avviare o riavviare
                    // - update_time della nuova esecuzione, per ciascun Player da avviare o riavviare

                    // Ora si calcola e si trasmette mix_samples (numero di samples del cross mix) a ciascun Player da riavviare.
                    if (Players_Manager_ptr->Get_players_to_restart() > 0)
                    {
                        Players_Manager_ptr->Calculate_and_set_mix_samples();
                    }
                }

                else if (!LOOP_element[track][event].note_on)
                {
                    if (tuning_tone_flag && (tuning_tone_last_note == note_number))
                    {
                        Tone_generator_ptr->Stop();
                        TT_playing = false;
                        TT_led_flag = true;
                    }

                    // for(auto i = 0; i < 127; ++i)
                    // {
                    //  Serial.print(key_state[0][i]);
                    //  Serial.print("-");
                    //  }
                    // Serial.println();

                    Players_Manager_ptr->Multicast_stop_players_for_NoteOff(midi_channel, note_number, track);
                }

                // Prossimo evento
                LOOP_play_event[track] = (LOOP_play_event[track] + 1) % LOOP_events[track];
                LOOP_play_time[track] = LOOP_Clock_time_from_virtual_time(LOOP_element[track][LOOP_play_event[track]].time);
                // Serial.print("Prossimo evento tra ms virtuali:");
                // Serial.println(LOOP_play_time[track] - LOOP_Clock_frozen);

                /*
                Serial.print("Si prosegue con l'evento:");
                Serial.print(LOOP_play_event[track]);
                Serial.print(" del track:");
                Serial.print(track);
                Serial.print(" tra ms virtuali:");
                Serial.println(LOOP_play_time[track] - LOOP_Clock());
                Serial.print("*************");
                */
            }
        }
    }
}