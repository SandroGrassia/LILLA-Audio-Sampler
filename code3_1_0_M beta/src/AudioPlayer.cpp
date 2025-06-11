/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "AudioPlayer.h"

void AudioPlayer::begin(void)
{
    idle = true;
    power_on = false;
    state = 0;
}

int16_t AudioPlayer::samples_basket[BASKET_DIM];

void AudioPlayer::Set_effects(float resolution_exp, uint8_t downsampling_in) // 1.0bit <= resolution_exp <= 16.0bit  ;  0 <= downsampling_in <= 6
{
    set_effects_flag = true;
    resolution_flag_wait = resolution_exp < 15.9;
    K_resolution_step_wait = 65536 / pow(2.0, resolution_exp); //  1.0 < K_resolution_step_wait < 32768
    // if(K_resolution_step_wait < 1.9)
    //     K_resolution_step_wait = 1;
    // else if(K_resolution_step_wait > 32700)
    //     K_resolution_step_wait = 32767;
    downsampling_flag_wait = downsampling_in > 0;
    downsampling_wait = downsampling_in;
}

void AudioPlayer::Main_settings(uint8_t mode_in, int A_value_in, int B_value_in, uint16_t delta_Noclick_in, bool use_Wavetable_in, int16_t *p_Noclick_in, int16_t *p_Wavetable_in)
{
    p_Noclick_wait = p_Noclick_in;
    p_Wavetable_wait = p_Wavetable_in;

    A_Flash_sample_wait = A_value_in;
    B_Flash_sample_wait = B_value_in;
    delta_Noclick_wait = delta_Noclick_in;
    use_Wavetable_wait = use_Wavetable_in;
    // PRINT("AudioPlayer.Main_settings - ", "id_player: ", identity);

    // modo LIVE_SAMPLING (lettura campioni da PSRAM)
    if (file_id_wait >= FIRST_LIVE_SAMPLING_FILE)
    {
        mode_player_wait = mode_in;
        use_Wavetable_wait = false;

        // il codice Main deve garantire che:
        // 0<= LS_X_sample <= (LS_buffer_dim - 1)
        // 0<= LS_X_delta <= (LS_buffer_dim - 1)

        if (LS_XY_lock)
        {
            if (mode_player_wait == ONCE_FWD) // 0
            {
                A_Flash_sample_wait = LS_X_sample;
                B_Flash_sample_wait = A_Flash_sample_wait + 2000000000;
                a_first_sample_wait = A_Flash_sample_wait;
            }
            else if (mode_player_wait == ONCE_REV) // 1
            {
                B_Flash_sample_wait = LS_X_sample;
                A_Flash_sample_wait = B_Flash_sample_wait - 2000000000;
                a_first_sample_wait = B_Flash_sample_wait;
            }
            else if (mode_player_wait == LOOP_FWD || mode_player_wait == LOOP_FWD_REV) // 2 loop A-->B / 3 loop A<-->B
            {
                A_Flash_sample_wait = LS_X_sample;
                B_Flash_sample_wait = LS_X_sample + LS_XY_delta;
                a_first_sample_wait = A_Flash_sample_wait;
            }

            // Serial.print(F("LS_XY_lock is TRUE - mode_player_wait: "));
            // Serial.print(mode_player_wait);
            // Serial.print(F("  A_Flash_sample_wait: "));
            // Serial.print(A_Flash_sample_wait);
            // Serial.print(F("  B_Flash_sample_wait: "));
            // Serial.println(B_Flash_sample_wait);
            // Serial.println();
        }

        // LS_X_sample va calcolato da zero utilizzando LS_X_delta
        else
        {
            LS_Q_sample = LiveSampler_ptr->Q_sample;
            if (mode_player_wait == ONCE_FWD)
            {
                A_Flash_sample_wait = LS_Q_sample + LS_X_delta - AUDIO_BLOCK_SAMPLES;
                if (A_Flash_sample_wait > LS_buffer_dim - 1)
                {
                    A_Flash_sample_wait -= LS_buffer_dim;
                }
                B_Flash_sample_wait = A_Flash_sample_wait + 2000000000;
                a_first_sample_wait = A_Flash_sample_wait;
            }
            else if (mode_player_wait == ONCE_REV)
            {
                B_Flash_sample_wait = LS_Q_sample + LS_X_delta - AUDIO_BLOCK_SAMPLES;
                if (B_Flash_sample_wait > LS_buffer_dim - 1)
                {
                    B_Flash_sample_wait -= LS_buffer_dim;
                }
                A_Flash_sample_wait = B_Flash_sample_wait - 2000000000;
                a_first_sample_wait = B_Flash_sample_wait;
            }
            else if (mode_player_wait == LOOP_FWD || mode_player_wait == LOOP_FWD_REV) // loop A-->B / loop A<-->B
            {
                A_Flash_sample_wait = LS_Q_sample + LS_X_delta - AUDIO_BLOCK_SAMPLES;
                if (A_Flash_sample_wait > LS_buffer_dim - 1)
                {
                    A_Flash_sample_wait -= LS_buffer_dim;
                }
                B_Flash_sample_wait = A_Flash_sample_wait + LS_XY_delta;
                a_first_sample_wait = A_Flash_sample_wait;
            }
            pitch_limit_wait = MAX_PITCH_PSRAM;

            /*
            Serial.print("Player identity: ");
            Serial.println(identity);
            Serial.print("LS_Q_sample: ");
            Serial.println(LS_Q_sample);
            Serial.print("A_Flash_sample_wait: ");
            Serial.println(A_Flash_sample_wait);
            Serial.print("LS_Q_sample - A_Flash_sample_wait: ");
            Serial.println(LS_Q_sample - A_Flash_sample_wait);
            Serial.println();
            */
        }
    }

    // modo PERFORMANCE o MIDI_LOOP
    else
    {
        // lettura samples da RAM
        if (use_Wavetable_wait)
        {
            mode_player_wait = (mode_in == LOOP_REV_FWD ? LOOP_FWD_REV : mode_in); // mode_player 4 and 3 are identical if use_Wavetabe_wait
            switch (mode_player_wait)
            {
            case ONCE_FWD: // A-->B

                // Flash:      A-----------------B
                // Wavetable:  0-----------------(B-A)
                Flash_first_RAM_sample_wait = A_Flash_sample_wait; // indirizzo su flash corrispondente a 0 su RAM
                a_first_sample_wait = A_Flash_sample_wait;         // indirizzo flash di startup
                Wavetable_length_wait = B_Flash_sample_wait - A_Flash_sample_wait + 1;
                break;

            case ONCE_REV: // B-->A

                // Flash:      B-----------------C
                // Wavetable:  0-----------------(B-A)

                // C_Flash_sample_wait = Mirror(B_Flash_sample_wait, A_Flash_sample_wait);
                Flash_first_RAM_sample_wait = B_Flash_sample_wait; // indirizzo su flash corrispondente a 0 su RAM
                a_first_sample_wait = B_Flash_sample_wait;         // indirizzo flash di startup
                Wavetable_length_wait = B_Flash_sample_wait - A_Flash_sample_wait + 1;
                break;

            case LOOP_FWD: // loop A-->B

                // Flash + No_click(RAM=!): (A+d)----------------(B-d)(B-d+1)!!!!!!!B
                // Wavetable:                 0----------------------------------(B-A-d)
                Flash_first_RAM_sample_wait = A_Flash_sample_wait + delta_Noclick_wait; // indirizzo su flash corrispondente a 0 su RAM
                a_first_sample_wait = Flash_first_RAM_sample_wait;                      // indirizzo flash di startup
                Wavetable_length_wait = B_Flash_sample_wait - A_Flash_sample_wait - delta_Noclick_wait + 1;
                break;

            case LOOP_FWD_REV: // loop A<-->B

                // Flash:      A-----------------B (B-1)-----------------(A+1)
                // Wavetable:  0-------------------------------------- (2B-2A-1)
                Flash_first_RAM_sample_wait = A_Flash_sample_wait; // indirizzo su flash corrispondente a 0 su RAM
                a_first_sample_wait = Flash_first_RAM_sample_wait; // indirizzo flash di startup
                Wavetable_length_wait = (B_Flash_sample_wait - A_Flash_sample_wait) << 1;
                break;

            case LOOP_REV_FWD: // loop B<-->A non previsto per Wavetable: per completezza copio lo stesso codice del caso LOOP_FWD_REV

                // Flash:      A-----------------B (B-1)-----------------(A+1)
                // Wavetable:  0-------------------------------------- (2B-2A-1)
                Flash_first_RAM_sample_wait = A_Flash_sample_wait; // indirizzo su flash corrispondente a 0 su RAM
                a_first_sample_wait = Flash_first_RAM_sample_wait; // indirizzo flash di startup
                Wavetable_length_wait = (B_Flash_sample_wait - A_Flash_sample_wait) << 1;
                break;

            case LOOP_REV: // loop B-->A

                // Flash + No_click(RAM=!):  A!!!!!!!!(A+d-1)(A+d)-----------------(B-d)
                // virtual address                                                 (B-d)----------------------------------Mirror((B-d), A)
                // Wavetable:                                                       0-------------------------------------(B-d-A)
                Flash_first_RAM_sample_wait = B_Flash_sample_wait - delta_Noclick_wait; // indirizzo su flash corrispondente a 0 su RAM
                a_first_sample_wait = Flash_first_RAM_sample_wait;                      // indirizzo flash di startup
                Wavetable_length_wait = B_Flash_sample_wait - delta_Noclick_wait - A_Flash_sample_wait + 1;
                break;

            default: // case LOOP_REV_FWD (4) non e' previsto per la wavetable
                break;
            }
            pitch_limit_wait = MAX_PITCH_WAVETABLE;
        }

        // lettura samples da FLASH
        else
        {
            switch (mode_in)
            {
            case 0: // A-->B
                a_first_sample_wait = A_Flash_sample_wait;
                mode_player_wait = mode_in;
                break;
            case 1: // B-->A
                C_Flash_sample_wait = Mirror(B_Flash_sample_wait, A_Flash_sample_wait);
                a_first_sample_wait = B_Flash_sample_wait;
                mode_player_wait = mode_in;
                break;
            case 2: // loop A-->B A-->B
                a_first_sample_wait = A_Flash_sample_wait;
                mode_player_wait = mode_in;
                break;
            case 3: // loop A-->B B-->A
                C_Flash_sample_wait = Mirror(B_Flash_sample_wait, A_Flash_sample_wait);
                a_first_sample_wait = A_Flash_sample_wait;
                mode_player_wait = mode_in;
                break;
            case 4: // loop B-->A A-->B
                C_Flash_sample_wait = Mirror(B_Flash_sample_wait, A_Flash_sample_wait);
                a_first_sample_wait = B_Flash_sample_wait;
                mode_player_wait = 3;
                break;
            case 5: // loop B-->A B-->A
                B_Flash_sample_5_wait = B_Flash_sample_wait - delta_Noclick_wait;
                C_Flash_sample_wait = Mirror(B_Flash_sample_5_wait, A_Flash_sample_wait);
                a_first_sample_wait = B_Flash_sample_5_wait;
                mode_player_wait = mode_in;
                break;

            default:
                break;
            }
            pitch_limit_wait = MAX_PITCH_FLASH[optimization];
        }
    }
}

void AudioPlayer::Main_settings_editing(uint8_t mode_in, int A_value_in, int B_value_in, uint16_t delta_Noclick_in, bool use_Wavetable_in, int16_t *p_Noclick_in, int16_t *p_Wavetable_in)
{
    mode_player_E = (mode_in == 4 ? 3 : mode_in); // switching to mode 4 is ininfluent WHILE playing (besides, mode 4 does NOT exist in harvest functions)
    A_Flash_sample_E = A_value_in;
    B_Flash_sample_E = B_value_in;
    delta_Noclick_E = delta_Noclick_in;
    use_Wavetable_E = use_Wavetable_in;
    p_Noclick_E = p_Noclick_in;
    p_Wavetable_E = p_Wavetable_in;

    // lettura samples da PSRAM
    if (file_id_wait >= FIRST_LIVE_SAMPLING_FILE)
    {
        mode_player_E = mode_in;
        pitch_limit_E = MAX_PITCH_PSRAM;
        use_Wavetable_E = false;

        if (LS_XY_lock)
        {
            if (mode_player_E == 0)
            {
                A_Flash_sample_E = LS_X_sample;
                B_Flash_sample_E = A_Flash_sample_E + 2000000000;
            }
            else if (mode_player_E == 1)
            {
                B_Flash_sample_E = LS_X_sample;
                A_Flash_sample_E = B_Flash_sample_E - 2000000000;
            }
            else if (mode_player_E == 2 || mode_player_E == 3)
            {
                A_Flash_sample_E = LS_X_sample;
                B_Flash_sample_E = LS_X_sample + LS_XY_delta;
            }
        }

        else
        {
            if (mode_player_E == 0)
            {
                A_Flash_sample_E = LS_Q_sample + LS_X_delta - AUDIO_BLOCK_SAMPLES;
                if (A_Flash_sample_E > LS_buffer_dim - 1)
                {
                    A_Flash_sample_E -= LS_buffer_dim;
                }
                B_Flash_sample_E = A_Flash_sample_E + 2000000000;
            }
            else if (mode_player_E == 1)
            {
                B_Flash_sample_E = LS_Q_sample + LS_X_delta - AUDIO_BLOCK_SAMPLES;
                if (B_Flash_sample_E > LS_buffer_dim - 1)
                {
                    B_Flash_sample_E -= LS_buffer_dim;
                }
                A_Flash_sample_E = B_Flash_sample_E - 2000000000;
            }
            else if (mode_player_E == 2 || mode_player_E == 3)
            {
                A_Flash_sample_E = LS_Q_sample + LS_X_delta - AUDIO_BLOCK_SAMPLES;
                if (A_Flash_sample_E > LS_buffer_dim - 1)
                {
                    A_Flash_sample_E -= LS_buffer_dim;
                }
                B_Flash_sample_E = A_Flash_sample_E + LS_XY_delta;
            }
        }
        /*
        Serial.print(F("mode_player_E: "));
        Serial.print(mode_player_E);
        Serial.print(F("  A_Flash_sample_E: "));
        Serial.print(A_Flash_sample_E);
        Serial.print(F("  a_first_sample: "));
        Serial.print(a_first_sample);
        Serial.print(F("  B_Flash_sample_E: "));
        Serial.println(B_Flash_sample_E);
        Serial.println();
        */
    }

    // lettura samples da RAM
    if (use_Wavetable_E)
    {
        switch (mode_player_E)
        {
        case 0: // A-->B

            // Flash:      A-----------------B
            // Wavetable:  0-----------------(B-A)
            Flash_first_RAM_sample_E = A_Flash_sample_E; // address on Flash corresponding to address 0 on Wavetable
            Wavetable_length_E = B_Flash_sample_E - A_Flash_sample_E + 1;
            break;

        case 1: // B-->A

            // Flash:      B-----------------C
            // Wavetable:  0-----------------(B-A)

            Flash_first_RAM_sample_E = B_Flash_sample_E; // address on Flash corresponding to address 0 on Wavetable
            Wavetable_length_E = B_Flash_sample_E - A_Flash_sample_E + 1;
            break;

        case 2: // loop A-->B

            // Flash + No_click(RAM=!): (A+d)----------------(B-d)(B-d+1)----B
            // Wavetable:                 0----------------------------------(B-A-d)
            Flash_first_RAM_sample_E = A_Flash_sample_E + delta_Noclick_E; // address on flash corresponding to address 0 on Wavetable
            Wavetable_length_E = B_Flash_sample_E - A_Flash_sample_E - delta_Noclick_E + 1;
            break;

        case 3: // loop A-->B-->A

            // Flash:      A-----------------B (B-1)-----------------(A+1)
            // Wavetable:  0-------------------------------------- (2B-2A-1)
            Flash_first_RAM_sample_E = A_Flash_sample_E; // address on flash corresponding to address 0 on Wavetable
            Wavetable_length_E = (B_Flash_sample_E - A_Flash_sample_E) << 1;
            break;

        case 5: // loop B-->A B-->A

            // Flash + No_click(RAM=!):  A--------(A+d-1)(A+d)-----------------(B-d)
            // virtual address                                                 (B-d)----------------------------------Mirror((B-d), A)
            // Wavetable:                                                       0-------------------------------------(B-A-d)
            Flash_first_RAM_sample_E = B_Flash_sample_E - delta_Noclick_E; // address on flash corresponding to address 0 on Wavetable
            Wavetable_length_E = B_Flash_sample_E - delta_Noclick_E - A_Flash_sample_E + 1;
            break;

        default:
            break;
        }
        pitch_limit_E = MAX_PITCH_WAVETABLE;
    }

    // lettura samples da FLASH
    else
    {
        switch (mode_player_E)
        {
        case 0: // once A-->B
            break;
        case 1: // once B-->A
            C_Flash_sample_E = Mirror(B_Flash_sample_E, A_Flash_sample_E);
            break;
        case 2: // loop A-->B A-->B
            break;
        case 3: // loop A-->B-->A loop B-->A-->B
            C_Flash_sample_E = Mirror(B_Flash_sample_E, A_Flash_sample_E);
            break;
        case 5: // loop B-->A B-->A
            B_Flash_sample_5_E = B_Flash_sample_E - delta_Noclick_E;
            C_Flash_sample_E = Mirror(B_Flash_sample_5_E, A_Flash_sample_E);
            break;
        default:
            break;
        }
        pitch_limit_E = MAX_PITCH_FLASH[optimization];
    }
    main_settings_editing_flag = true;
}

void AudioPlayer::Get_ready_to_play(float pitch_note_in, float velocity_in, int session_in, uint8_t instrument_in, uint8_t id_sound_in, uint8_t note_in)
{
    pitch_note_wait = pitch_note_in;
    velocity_gain_wait = velocity_in;
    session_wait = session_in;
    instrument_wait = instrument_in;
    id_sound_wait = id_sound_in;
    note_wait = note_in;
    time_stamp = millis();

    power_on = true;

    if (idle)
    {
        Start_playing();
    }
    else
    {
        warmup_for_play_again_flag = true;
        state = 1;
    }
    check = 0;
}

void AudioPlayer::Start_playing(void)
{
    AudioStartUsingSPI();
    file_id = file_id_wait;
    recording_flag = false;
    LS_flag = false;

    // .RAW file
    if (file_id < FIRST_RECORDING_FILE)
    {
        rawfile.fast_open(file_id); // rawfile = SerialFlash.open(filename); // open file
    }

    // .rec file from DIRECT_SAMPLING
    else if (file_id < FIRST_LIVE_SAMPLING_FILE)
    {
        recording_flag = true;
        recording = (file_id - FIRST_RECORDING_FILE) / 2;
        stereo_flag = Recording[recording].stereo;
        if (!stereo_flag)
            first_packet = Recording[recording].first_packet; // solo questo file contiene dati
        else
        {
            if ((file_id - FIRST_RECORDING_FILE) % 2 == 0) // file LEFT
            {
                first_packet = Recording[recording].first_packet;
            }
            else // file RIGHT
            {
                first_packet = Recording[recording].first_packet + 1;
            }
        }

        packet_delta = 0;
        rawfile.packet_fast_open(first_packet); // rawfile = SerialFlash.open(name_packet[first_packet]); // open first Packet

        // Serial.print("First packet played is: ");
        // Serial.println(first_packet);
        // Serial.println(name_packet[first_packet]);
    }

    // Mono.liv or Left.liv or Right.liv files from LIVE_SAMPLING
    else
    {
        LS_flag = true;
        if (file_id == FIRST_LIVE_SAMPLING_FILE)
        {
            FIFO = LS_buffer_mono_ptr;
        }
        else if (file_id == FIRST_LIVE_SAMPLING_FILE + 1)
        {
            FIFO = LS_buffer_L_ptr;
        }
        else
        {
            FIFO = LS_buffer_R_ptr;
        }
    }

    if (!rawfile)
    {
        AudioStopUsingSPI();
    }

    // Main_settings synchronization
    Attack_type = attack_type_wait;
    Update_ADSR_parameters();
    ADSR_phase = 0;
    ADSR_point_0 = -K_Attack_step;
    volume_gain = volume_gain_wait;
    pan_int = pan_int_wait;
    pan_gain_L = pan_gain_L_wait;
    pan_gain_R = pan_gain_R_wait;
    p_Noclick = p_Noclick_wait;
    p_Wavetable = p_Wavetable_wait;
    A_Flash_sample = A_Flash_sample_wait;
    B_Flash_sample = B_Flash_sample_wait;
    delta_Noclick = delta_Noclick_wait;
    mode_player = mode_player_wait;
    use_Wavetable = use_Wavetable_wait;
    Flash_first_RAM_sample = Flash_first_RAM_sample_wait;
    a_first_sample = a_first_sample_wait;
    Wavetable_length = Wavetable_length_wait;
    pitch_limit = pitch_limit_wait;
    C_Flash_sample = C_Flash_sample_wait;
    B_Flash_sample_5 = B_Flash_sample_5_wait;
    initial_index_offset = 0;
    samples_counter = 0;
    local_session = session_wait;
    instrument = instrument_wait;
    id_sound = id_sound_wait;
    note = note_wait;
    pitch_tune = pitch_tune_wait;
    pitch_tune_flag = false;
    pitch_bend = pitch_bend_wait;
    pitch_bend_flag = false;
    pitch_note = pitch_note_wait;
    velocity_gain = velocity_gain_wait;
    volume_flag = false;
    sliding = 0;
    track = track_wait;

    Start_VCF();
    idle = false;

    state = 1;

    if (Lilla_state == MIDI_LOOP && track >= 0)
    {
        Players_statistics_ptr->Inc_total_Players_per_track_instrument(track, instrument);
        Set_led_flags(track);
    }

    else if (Lilla_state != MIDI_LOOP)
    {
        Players_statistics_ptr->Inc_total_Players_per_instrument(instrument);
        Set_led_flags(track);
    }
}

void AudioPlayer::Release_note(void) // release note, fires ADSR "release"
{
    K_Release_delta = ADSR_gain; // [gain]
    ADSR_phase = 3;              // Release
    ADSR_point_0 = -K_Release_step;
    time_stamp = millis();
    power_on = false;
    state = 2;

    if (Lilla_state == MIDI_LOOP && track >= 0)
    {
        Players_statistics_ptr->Dec_total_Players_per_track_instrument(track, instrument);
        Set_led_flags(track);
    }

    else if (Lilla_state != MIDI_LOOP)
    {
        Players_statistics_ptr->Dec_total_Players_per_instrument(instrument);
        Set_led_flags(track);
    }
}

void AudioPlayer::Set_led_flags(int track)
{
    if (Lilla_state != MIDI_LOOP)
    {
        led_flag = true;
    }

    else if (Lilla_state == MIDI_LOOP && track >= 0)
    {
        LOOP_led_flag[track] = true;
    }
}

void AudioPlayer::Update_pitch(void)
{
    pitch = constrain(pitch_note * pitch_bend * pitch_tune * pitch_vibrato, MIN_PITCH, pitch_limit);

    // Softness Filter
    pitch_gain_correction = (pitch <= 1.0 ? 1.0 : MAX_PITCH_WAVETABLE / (pitch - 1.0 + MAX_PITCH_WAVETABLE));
}

void AudioPlayer::update(void)
{

    if (identity == 0)
    {
        big_T = 0;
        // execution_clock = 0;
    }

    audio_block_t *block_L, *block_R;
    int16_t I_basket_L_sample, I_basket_H_sample; // indexes of samples in sample_basket
    float F_basket_sample, F_index_delta;
    float rest;
    uint8_t vibrato_array_element;
    float a_first_sample_real;
    float a_first_sample_E = 0;
    float value_float;

    bool snubber_flag = false;
    bool mix_flag = false;
    bool shoot_flag = false;

    block_L = allocate(); // allocate the audio blocks to transmit
    if (block_L == NULL)
    {
        return;
    }

    block_R = allocate(); // allocate the audio blocks to transmit
    if (block_R == NULL)
    {
        return;
    }

    // protezione da Library 22.17
    if (big_T > 2800)
    {
        Serial.println("*PROTECT*");
        idle = true;
        power_on = false;
        state = 0;

        if (Lilla_state == MIDI_LOOP && track >= 0)
        {
            Players_statistics_ptr->Dec_total_Players_per_track_instrument(track, instrument);
            Set_led_flags(track);
        }

        else if (Lilla_state != MIDI_LOOP)
        {
            Players_statistics_ptr->Dec_total_Players_per_instrument(instrument);
            Set_led_flags(track);
        }
    }

    if (idle)
    {
        for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            block_L->data[i] = 0;
            block_R->data[i] = 0;
        }
        transmit(block_L, 0);
        release(block_L);
        transmit(block_R, 1);
        release(block_R);
    }

    else
    {
        if (warmup_for_play_again_flag)
        {
            if (mix_samples > 0)
            {
                // harvest the last set of samples from the actual file
                a_sample = a_first_sample;
                initial_index_offset = a_sample - floor(a_sample);
                b_sample = a_sample + (pitch * (mix_samples - 1));
                (use_Wavetable ? Wavetable_harvest() : Flash_memory_harvest());

                float volume_tmp = volume_gain * velocity_gain * ADSR_gain * pitch_gain_correction;
                for (uint8_t i = 0; i < mix_samples; ++i)
                {
                    F_basket_sample = (i * pitch) + initial_index_offset; // "initial_index_offset" may be modified in harvest function
                    I_basket_L_sample = floor(F_basket_sample);           // index of the lower sample needed for calculation
                    I_basket_H_sample = ceil(F_basket_sample);            // index of the upper sample needed for calculation
                    F_index_delta = F_basket_sample - I_basket_L_sample;
                    raw_first_value_cache[i] = volume_tmp * (samples_basket[I_basket_L_sample] + (F_index_delta * (samples_basket[I_basket_H_sample] - samples_basket[I_basket_L_sample])));
                }
                for (uint8_t i = mix_samples; i < AUDIO_BLOCK_SAMPLES; ++i)
                {
                    raw_first_value_cache[i] = 0;
                }

                Start_playing();
                mix_flag = true;
                shoot_flag = true;
            }
            else
            {
                restart_flag = true;
            }
            main_settings_editing_flag = false;
            warmup_for_play_again_flag = false;
        }

        if (pitch_tune_flag)
        {
            pitch_tune = pitch_tune_wait;
            pitch_tune_flag = false;
        }

        if (pitch_bend_flag)
        {
            pitch_bend = pitch_bend_wait;
            pitch_bend_flag = false;
        }

        if (vibrato_flag)
        {
            vibrato_array_element_float += VIBRATO_STEP;
            vibrato_array_element = ((uint8_t)vibrato_array_element_float) % 32;
            *p_vibrato_array_last_element = vibrato_array_element;
            pitch_vibrato = *(p_vibrato_array + vibrato_array_element);
        }

        Update_pitch();

        if (main_settings_editing_flag)
        {
            // mix_samples = 64;

            if (mode_player_E != mode_player) // calculating a_first_sample_E if mode_player has been changed
            {
                if (mode_player == ONCE_FWD && mode_player_E == ONCE_REV)
                {
                    //   A---------------------a--------B  0: once A-->B
                    //   AE||||||||||||||||||||a'|||||||BE-------aE------------------CE 1: once B-->A
                    a_first_sample_E = Mirror(B_Flash_sample_E, a_first_sample); // a_first_sample_E is virtual
                    // Serial.println("case M1");
                }
                else if (mode_player == ONCE_REV && mode_player_E == ONCE_FWD)
                {
                    //   A----------------------a'-------B-------a---------------------C 1: once B-->A
                    //   AE---------------------aE-------BE  0: once A-->B
                    a_first_sample_E = Mirror(B_Flash_sample, a_first_sample); // a_first_sample_E is real
                    // Serial.println("case M2");
                }
                else if (mode_player == ONCE_REV && mode_player_E == LOOP_FWD)
                {
                    a_first_sample_real = Mirror(B_Flash_sample, a_first_sample);

                    //    A--a'---------------------------B------------------------a----C 1: once B-->A
                    //            (AE+dE)-----------------BE    2: loop A-->B A-->B
                    if (floor(a_first_sample_real) < (A_Flash_sample_E + delta_Noclick_E))
                    {
                        a_first_sample_E = A_Flash_sample_E + delta_Noclick_E;
                        // Serial.println("case M3");
                    }

                    //    A---------------------a'--------B--------a--------------------C 1: once B-->A
                    //            (AE+dE)-------aE--------BE    2: loop A-->B A-->B
                    else
                    {
                        a_first_sample_E = a_first_sample_real;
                        // Serial.println("case M4");
                    }
                    snubber_flag = true; // the landing pool of samples is DIFFERENT because here there is Noclick table
                }
                else if (mode_player == LOOP_FWD && mode_player_E == ONCE_REV)
                {
                    //            (A+d)---a---------------B                                   2: loop A-->B A-->B
                    //    AE||||||||||||||a'||||||||||||||BE--------------aE-------------CE   1: once B-->A
                    a_first_sample_E = Mirror(B_Flash_sample_E, a_first_sample); // a_first_sample_E is virtual
                    snubber_flag = true;                                         // the landing pool of samples is DIFFERENT because here there is NOT Noclick table
                    // Serial.println("case M5");
                }
                else if (mode_player == LOOP_FWD && mode_player_E == LOOP_FWD_REV)
                {
                    //            (A+d)--------a----------B                                  2: loop A-->B A-->B
                    //    AE-------------------aE---------BE-----------------------------C   3: loop A-->B-->A
                    a_first_sample_E = a_first_sample;
                    snubber_flag = true; // the landing pool of samples is DIFFERENT because here there is NOT Noclick table
                    // Serial.println("case M6");
                }
                else if (mode_player == LOOP_FWD_REV && mode_player_E == LOOP_FWD)
                {
                    //    A--------------------------------B----------a------------------C   3: loop A-->B-->A
                    //            (AE+dE)------------------BE                                2: loop A-->B A-->B
                    if (floor(a_first_sample) > B_Flash_sample)
                    {
                        a_first_sample_real = Mirror(B_Flash_sample, a_first_sample);

                        //    A----a'--------------------------B--------------------------a--C   3: loop A-->B-->A
                        //            (AE+dE)------------------BE                                2: loop A-->B A-->B
                        if (floor(a_first_sample_real) < (A_Flash_sample_E + delta_Noclick_E))
                        {
                            a_first_sample_E = A_Flash_sample_E + delta_Noclick_E;
                            // Serial.println("case M7");
                        }

                        //    A----------------------a'--------B--------a-----------------a--C   3: loop A-->B-->A
                        //            (AE+dE)--------aE--------BE                                2: loop A-->B A-->B
                        else
                        {
                            a_first_sample_E = a_first_sample_real;
                            // Serial.println("case M8");
                        }
                        snubber_flag = true; // the landing pool of samples is DIFFERENT because here there is Noclick table
                    }

                    //   A-------------------a------------B-----------------------------C  3: loop A-->B-->A
                    //           (AE+dE)------------------BE    2: loop A-->B A-->B
                    else if (floor(a_first_sample) <= B_Flash_sample)
                    {

                        //   A--a-----------------------------B-----------------------------C  3: loop A-->B-->A
                        //           (AE+dE)------------------BE    2: loop A-->B A-->B
                        if (floor(a_first_sample) < (A_Flash_sample_E + delta_Noclick_E))
                        {
                            a_first_sample_E = A_Flash_sample_E + delta_Noclick_E;
                            // Serial.println("case M9");
                        }

                        //   A--------------------a-----------B-----------------------------C  3: loop A-->B-->A
                        //           (AE+dE)------aE----------BE    2: loop A-->B A-->B
                        else
                        {
                            a_first_sample_E = a_first_sample;
                            // Serial.println("case M10");
                        }
                        snubber_flag = true; // the landing pool of samples is DIFFERENT because here there is Noclick table
                    }
                }

                else if (mode_player == LOOP_FWD_REV && mode_player_E == LOOP_REV)
                {
                    a_first_sample_real = Mirror(B_Flash_sample, a_first_sample);

                    //    A-------a'--------------------------------B-------------------a----------------------C     3: loop A-->B B-->A
                    //    AE||||||aE'||||||||||||||||||(BE-dE)---------------------aE-----------Mirror((BE-dE), AE)  5: loop B-->A B-->A
                    if (floor(a_first_sample_real) <= (B_Flash_sample_E - delta_Noclick_E))
                    {
                        a_first_sample_E = Mirror(B_Flash_sample_E - delta_Noclick_E, a_first_sample_real);
                        // Serial.println("case M13");
                    }

                    //    A------------------------------------a'---B---a--------------------------------------C     3: loop A-->B B-->A
                    //    A||||||||||||||||||||||||||||(BE-dE)----------------------------------Mirror((BE-dE), AE)  5: loop B-->A B-->A
                    else
                    {
                        a_first_sample_E = B_Flash_sample_E - delta_Noclick_E;
                        // Serial.println("case M14");
                    }
                    snubber_flag = true; // the landing pool of samples is DIFFERENT because here there is Noclick table
                }

                else if (mode_player == LOOP_REV && mode_player_E == LOOP_FWD_REV)
                {
                    a_first_sample_real = Mirror(B_Flash_sample - delta_Noclick, a_first_sample);

                    //    A||||||||a'|||||||||||||||||||(B-d)------------------------a--Mirror((B-d), A)  5: loop B-->A B-->A
                    //    AE-------aE'-----------------------BE--------------------------aE-------CE      3: loop A-->B B-->A
                    a_first_sample_E = Mirror(B_Flash_sample_E, a_first_sample_real);
                    snubber_flag = true; // the landing pool of samples is DIFFERENT because here there is NOT Noclick table
                    // Serial.println("case M15");
                }
            }

            else // calculating a_first_sample_E if A_Flash_sample or B_Flash_sample has been changed
            {
                a_first_sample_E = a_first_sample;

                /*
                if(use_Wavetable_E && mode_player_E == 0) // once A-->B
                {
                    //   a-----AE---------------------BE
                    if(floor(a_first_sample) < A_Flash_sample_E)
                    {
                        a_first_sample_E = A_Flash_sample_E;
                        snubber_flag = true;
                    }

                    //         AE---------------------BE-----a
                    else if(floor(a_first_sample) >= B_Flash_sample_E)
                    {
                        a_first_sample_E = B_Flash_sample_E;
                    }
                }
                */

                if (mode_player_E == ONCE_FWD) // once A-->B
                {
                    //   a-----AE---------------------BE
                    if (floor(a_first_sample) < A_Flash_sample_E)
                    {
                        a_first_sample_E = A_Flash_sample_E;
                        snubber_flag = true;
                    }

                    //         AE---------------------BE-----a
                    else if (floor(a_first_sample) >= B_Flash_sample_E)
                    {
                        a_first_sample_E = B_Flash_sample_E;
                    }
                }

                else if (mode_player_E == ONCE_REV) // once B-->A
                {
                    a_first_sample_real = Mirror(B_Flash_sample, a_first_sample); // a'

                    // AE---------a'------------BE
                    if (floor(a_first_sample_real) <= B_Flash_sample_E && floor(a_first_sample_real) > A_Flash_sample_E)
                    {
                        a_first_sample_E = Mirror(B_Flash_sample_E, a_first_sample_real);
                        snubber_flag = true;
                    }

                    // out of range AE-----------------BE
                    else
                    {
                        a_first_sample_E = B_Flash_sample_E;
                    }
                }

                if (mode_player_E == LOOP_FWD) // loop A-->B A-->B
                {
                    if (use_Wavetable_E)
                    {
                        //   a********.(AE+dE)---------------------BE
                        if (floor(a_first_sample) < Flash_first_RAM_sample_E)
                        {
                            a_first_sample_E = Flash_first_RAM_sample_E;
                            snubber_flag = true;
                            // Serial.println("Wavetable! - case a loop A-->B A-->B");
                        }

                        //   .........(AE+dE)--------------------BE......a*******
                        else if (floor(a_first_sample) >= B_Flash_sample_E)
                        {
                            a_first_sample_E = Flash_first_RAM_sample_E;
                            snubber_flag = true;
                            // Serial.println("Wavetable! - case b loop A-->B A-->B");
                        }
                    }

                    else
                    {
                        //   a******...AE.....(AE+dE-1)(AE+dE)----------------(BE-dE)(BE-dE+1)----BE........
                        if (floor(a_first_sample) < A_Flash_sample_E)
                        {
                            a_first_sample_E = A_Flash_sample_E;
                            // Serial.println("NoWavetable - case a loop A-->B A-->B");
                        }

                        //  ........AE.......(AE+dE-1)(AE+dE)----------------(BE-dE)(BE-dE+1)----BE...a********
                        else if (floor(a_first_sample) >= B_Flash_sample_E)
                        {
                            a_first_sample_E = B_Flash_sample_E;
                            // Serial.println("NoWavetable - case b loop A-->B A-->B");
                        }
                        snubber_flag = true;
                    }
                }

                else if (mode_player_E == LOOP_FWD_REV) // loop A-->B-->A
                {

                    // direction is REVERSE
                    // A----------------------B-----a*********
                    if (floor(a_first_sample) >= B_Flash_sample)
                    {
                        a_first_sample_real = Mirror(B_Flash_sample, a_first_sample); // a'

                        // AE----*****a'------------BE
                        if (ceil(a_first_sample_real) <= B_Flash_sample_E && ceil(a_first_sample_real) > A_Flash_sample_E)
                        {
                            a_first_sample_E = Mirror(B_Flash_sample_E, a_first_sample_real);
                            snubber_flag = true;
                            // Serial.println("case a");
                        }

                        // a'****-AE-----------------BE
                        else if (ceil(a_first_sample_real) < A_Flash_sample_E)
                        {
                            a_first_sample_E = A_Flash_sample_E;
                            snubber_flag = true;
                            // Serial.println("case b");
                        }

                        // AE-----------------BE---a'
                        else if (ceil(a_first_sample_real) > B_Flash_sample_E)
                        {
                            a_first_sample_E = B_Flash_sample_E;
                            snubber_flag = true;
                            // Serial.println("case d");
                        }
                    }

                    // direction is FORWARD
                    // A--------------a*****--B
                    else
                    {
                        //  AE--------a*****--BE
                        if (floor(a_first_sample) >= A_Flash_sample_E && floor(a_first_sample) <= B_Flash_sample_E)
                        {
                            a_first_sample_E = a_first_sample;
                            // Serial.println("case e");
                        }
                        //  AE-------------------BE---a
                        if (floor(a_first_sample) > B_Flash_sample_E)
                        {
                            a_first_sample_E = B_Flash_sample_E;
                            snubber_flag = true;
                            // Serial.println("case e");
                        }

                        //  a*****-AE-------------------BE
                        else if (floor(a_first_sample) < A_Flash_sample_E)
                        {
                            if (use_Wavetable_E)
                            {
                                a_first_sample_E = A_Flash_sample_E;
                                snubber_flag = true;
                                // Serial.println("case f");
                            }
                        }
                    }
                }

                else if (mode_player_E == LOOP_REV) // loop B-->A B-->A
                {
                    if (use_Wavetable_E)
                    {
                        a_first_sample_real = Mirror(Flash_first_RAM_sample, a_first_sample); // a'

                        // a' MUST be between A_Flash_sample_E and (B_Flash_sample_E - delta_Noclick_E)
                        // ...........AE--<<<<<<<<a'-------------------(BE-dE)..........
                        if (ceil(a_first_sample_real) > A_Flash_sample_E && ceil(a_first_sample_real) <= Flash_first_RAM_sample_E)
                        {
                            a_first_sample_E = Mirror(Flash_first_RAM_sample_E, a_first_sample_real);
                            snubber_flag = true;
                            // Serial.println("Wavetable - case g loop B-->A B-->A");
                        }

                        // out of range
                        else
                        {
                            a_first_sample_E = Flash_first_RAM_sample_E;
                            snubber_flag = true;
                            // Serial.println("Wavetable - case h loop B-->A B-->A");
                        }
                    }

                    else
                    {
                        a_first_sample_real = Mirror(B_Flash_sample_5, a_first_sample); // a'

                        // a' MUST be between A_Flash_sample_E and B_Flash_sample_5_E
                        // ............AE--<<<<<<<a'----------------------B_5E........
                        if (ceil(a_first_sample_real) > A_Flash_sample_E && ceil(a_first_sample_real) <= B_Flash_sample_5_E)
                        {
                            a_first_sample_E = Mirror(B_Flash_sample_5_E, a_first_sample_real);
                            snubber_flag = true;
                            // Serial.println("NoWavetable - case i loop B-->A B-->A");
                        }

                        // out of range
                        else
                        {
                            a_first_sample_E = B_Flash_sample_5_E;
                            snubber_flag = true;
                            // Serial.println("NoWavetable - case j loop B-->A B-->A");
                        }
                    }
                }
            }

            // read HALF of samples from the old segment and old pitch and old gain
            if (snubber_flag)
            {
                a_sample = a_first_sample; // original a_first_sample
                initial_index_offset = a_sample - floor(a_sample);
                b_sample = a_sample + pitch * (mix_samples - 1); // read half samples
                (use_Wavetable ? Wavetable_harvest() : Flash_memory_harvest());
                float volume_tmp = volume_gain * velocity_gain * ADSR_gain * pitch_gain_correction;

                for (uint8_t i = 0; i < mix_samples; ++i)
                {
                    F_basket_sample = (i * pitch) + initial_index_offset; // "initial_index_offset" may be modified in harvest function
                    I_basket_L_sample = floor(F_basket_sample);           // index of the lower sample needed for calculation
                    I_basket_H_sample = ceil(F_basket_sample);            // index of the upper sample needed for calculation
                    F_index_delta = F_basket_sample - I_basket_L_sample;
                    raw_first_value_cache[i] = volume_tmp * (samples_basket[I_basket_L_sample] + (F_index_delta * (samples_basket[I_basket_H_sample] - samples_basket[I_basket_L_sample])));
                    // Serial.print(i); PRINT("raw_first",": ", raw_first_value_cache[i]);
                }
                for (uint8_t i = mix_samples; i < AUDIO_BLOCK_SAMPLES; ++i)
                {
                    raw_first_value_cache[i] = 0;
                    // Serial.print(i); PRINT("raw_first",": ", raw_first_value_cache[i]);
                }
                snubber_flag = false;
                mix_flag = true;
            }

            pitch_limit = pitch_limit_E;
            Update_pitch();

            mode_player = mode_player_E;
            a_first_sample = a_first_sample_E;
            delta_Noclick = delta_Noclick_E;
            p_Wavetable = p_Wavetable_E;
            p_Noclick = p_Noclick_E;
            use_Wavetable = use_Wavetable_E;
            A_Flash_sample = A_Flash_sample_E;
            B_Flash_sample = B_Flash_sample_E;

            if (use_Wavetable)
            {
                Flash_first_RAM_sample = Flash_first_RAM_sample_E;
                Wavetable_length = Wavetable_length_E;
            }
            else
            {
                C_Flash_sample = C_Flash_sample_E;
                B_Flash_sample_5 = B_Flash_sample_5_E;
            }
            main_settings_editing_flag = false;
        }

        // harvest for samples
        a_sample = a_first_sample;
        initial_index_offset = a_sample - floor(a_sample);
        b_sample = a_sample + (pitch * (AUDIO_BLOCK_SAMPLES - 1));

        /*
        Serial.print(Flash_first_RAM_sample);
        Serial.print(" <= ");
        Serial.print(a_sample);
        Serial.print(" <= ");
        Serial.print(b_sample);
        Serial.print(" <= ");
        Serial.println(Flash_first_RAM_sample + Wavetable_length - 1);
        */

        (use_Wavetable ? Wavetable_harvest() : Flash_memory_harvest());

        if (volume_gain_warmup_flag)
        {
            volume_gain_warmup_flag = false;
            K_volume_gain = (volume_gain_wait - volume_gain) / SAMPLES_VOLUME; // [Gain/sample]
            JV0 = samples_counter + SAMPLES_VOLUME;                            // [sample]
            volume_flag = true;
        }
        if (pan_gain_warmup_flag)
        {
            pan_gain_warmup_flag = false;
            K_pan_gain_L = (pan_gain_L_wait - pan_gain_L) / SAMPLES_VOLUME; // [Gain/sample]
            K_pan_gain_R = (pan_gain_R_wait - pan_gain_R) / SAMPLES_VOLUME; // [Gain/sample]
            JP0 = samples_counter + SAMPLES_VOLUME;                         // [sample]
            pan_flag = true;
        }
        if (set_effects_flag)
        {
            resolution_flag = resolution_flag_wait;
            K_resolution_step = K_resolution_step_wait;
            // Serial.print("Player - K_resolution_step:");
            // Serial.println(K_resolution_step);

            downsampling_flag = downsampling_flag_wait;
            downsampling = downsampling_wait;
            set_effects_flag = false;
        }

        for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            F_basket_sample = (i * pitch) + initial_index_offset; // "initial_index_offset" may be modified in harvest function
            I_basket_L_sample = floor(F_basket_sample);           // index of the lower sample needed for calculation
            I_basket_H_sample = ceil(F_basket_sample);            // index of the upper sample needed for calculation
            F_index_delta = F_basket_sample - I_basket_L_sample;
            if (volume_flag)
            {
                Update_volume_gain();
            }
            if (pan_flag)
            {
                Update_pan_gain();
            }
            Update_ADSR_gain();

            if (!shoot_flag && mix_flag && i < mix_samples)
            {
                value_float = mix_samples;
                cache = (i / value_float) * volume_gain * velocity_gain * ADSR_gain * pitch_gain_correction * (samples_basket[I_basket_L_sample] + (F_index_delta * (samples_basket[I_basket_H_sample] - samples_basket[I_basket_L_sample]))) + (mix_samples - 1 - i) / ((float)mix_samples - 1.0) * raw_first_value_cache[i];
                block[i] = Lilla_saturate16(cache);
            }
            else if (shoot_flag && mix_flag && i < mix_samples)
            {
                value_float = mix_samples;
                cache = volume_gain * velocity_gain * ADSR_gain * pitch_gain_correction * (samples_basket[I_basket_L_sample] + (F_index_delta * (samples_basket[I_basket_H_sample] - samples_basket[I_basket_L_sample]))) + (mix_samples - 1 - i) / (value_float - 1.0f) * raw_first_value_cache[i];
                block[i] = Lilla_saturate16(cache);
            }

            else
            {
                cache = volume_gain * velocity_gain * ADSR_gain * pitch_gain_correction * (samples_basket[I_basket_L_sample] + (F_index_delta * (samples_basket[I_basket_H_sample] - samples_basket[I_basket_L_sample])));
                block[i] = Lilla_saturate16(cache);
            }

            if (restart_flag)
            {
                fast_stop_gain = (AUDIO_BLOCK_SAMPLES - 1 - i) / (AUDIO_BLOCK_SAMPLES - 1);
                block[i] = block[i] * fast_stop_gain;
            }

            if (downsampling_flag)
            {
                rest = (sliding + i) % downsampling;
                if (rest == 0)
                {
                    stored_sample = block[i];
                }
                else
                {
                    block[i] = stored_sample;
                }
            }

            if (resolution_flag)
            {
                block[i] = (block[i] / K_resolution_step) * K_resolution_step;
            }

            samples_counter++;
        }

        if (mix_flag)
        {
            mix_flag = false;
        }

        if (shoot_flag)
        {
            shoot_flag = false;
        }

        if (downsampling_flag)
        {
            sliding = (sliding + 128) % downsampling;
        }

        if (VCF_connect)
        {
            if (VCF_modulated)
            {
                LFO_ptr->Update();
                Send_LFO_to_VCF();
            }
            VCF_ptr->Update();
        }

        for (auto i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            block_L->data[i] = pan_gain_L * block[i];
            block_R->data[i] = pan_gain_R * block[i];
        }
        transmit(block_L, 0);
        release(block_L);
        transmit(block_R, 1);
        release(block_R);

        if (restart_flag)
        {
            restart_flag = false;
            Start_playing();
        }

        if ((ADSR_gain < 0.0001) && (ADSR_phase > 0))
        {
            idle = true;
            power_on = false;
            state = 0;

            rawfile.close();
            AudioStopUsingSPI();
        }

        // execution time
        /*
        if (identity == 0)
        {
            execution_time = execution_clock;
            Serial.print("Player ");
            Serial.print(identity);
            Serial.print(" - mode:");
            Serial.print((use_Wavetable ? "Wavetable" : "Flash"));
            Serial.print(" - pitch:");
            Serial.print(pitch);
            Serial.print(" - REAL execution_time:");
            Serial.print(execution_time);
            Serial.print(" - RECEIVED update_time is:");
            Serial.println(update_time);
            Serial.println();
        }
        */
    }
}

void AudioPlayer::Update_ADSR_parameters(void)
{
    J1 = Attack * AUDIO_SAMPLE_RATE;           // [sample]
    J2 = (Attack + Decay) * AUDIO_SAMPLE_RATE; // [sample]
    DJ3_4 = Release * AUDIO_SAMPLE_RATE;       // [sample]
    K_Attack_step = 10.0f / J1;                // Attack step [1/sample]
    K_Dacay_step = 10.0f / (J2 - J1);          // [1/sample]
    K_Sustain_delta = 1.0f - Sustain;          // [gain]
    K_Release_step = 10.0f / DJ3_4;            // [1/sample]
}

void AudioPlayer::Update_ADSR_gain(void)
{
    uint8_t P;

    if (ADSR_phase == 0) // Attack
    {
        ADSR_point = ADSR_point_0 + K_Attack_step;

        P = floor(ADSR_point);

        if (P < 10)
        {

            ADSR_point_0 = ADSR_point;
            if (Attack_type == 1) // FAST Attack
            {
                ADSR_gain = (exp_table[P] + m_exp_table[P] * (ADSR_point - P));
                return;
            }
            else // SLOW Attack
            {
                ADSR_gain = (sin_table[P] + m_sin_table[P] * (ADSR_point - P));
                return;
            }
        }
        else
        {
            ADSR_phase = 1; // Decay
            if ((P - 10) < K_Attack_step / 2.0)
            {
                ADSR_point = 0.0;
            }
            else
            {
                ADSR_point = K_Dacay_step;
            }
            P = floor(ADSR_point);
            ADSR_point_0 = ADSR_point;
            ADSR_gain = Sustain + K_Sustain_delta * (decay_table[P] + m_decay_table[P] * (ADSR_point - P));
            return;
        }
    }

    else if (ADSR_phase == 1) // Decay
    {
        ADSR_point = ADSR_point_0 + K_Dacay_step;
        P = floor(ADSR_point);
        if (P < 10)
        {
            ADSR_point_0 = ADSR_point;
            ADSR_gain = Sustain + K_Sustain_delta * (decay_table[P] + m_decay_table[P] * (ADSR_point - P));
            return;
        }
        else
        {
            ADSR_phase = 2; // Sustain
            ADSR_point_0 = 0.0;
            ADSR_gain = Sustain;
            return;
        }
    }

    else if (ADSR_phase == 2) // Sustain
    {
        ADSR_gain = Sustain;
        return;
    }

    else if (ADSR_phase == 3) // Release
    {
        ADSR_point = ADSR_point_0 + K_Release_step;
        P = floor(ADSR_point);
        if (P < 10)
        {
            ADSR_point_0 = ADSR_point;
            ADSR_gain = K_Release_delta * (release_table[P] + m_release_table[P] * (ADSR_point - P));
            return;
        }
        else
        {
            ADSR_phase = 4;
            ADSR_gain = 0.0f;
            return;
        }
    }

    else // ADSR_phase == 4
    {
        ADSR_gain = 0.0f;
        return;
    }
}

void AudioPlayer::Update_volume_gain(void)
{
    if (samples_counter <= JV0)
    {
        volume_gain += K_volume_gain;
    }
    else if (samples_counter > JV0)
    {
        volume_flag = false;
    }
}

void AudioPlayer::Update_pan_gain(void)
{
    if (samples_counter <= JP0)
    {
        pan_gain_L += K_pan_gain_L;
        pan_gain_R += K_pan_gain_R;
    }
    else if (samples_counter > JP0)
    {
        pan_flag = false;
    }
}

void AudioPlayer::Flash_memory_harvest(void)
{
    int samples_to_read, samples_to_read_1, samples_to_read_2, samples_to_read_3; // int16_t
    int32_t i;
    int16_t samples_basket_local[BASKET_DIM];
    int16_t samples_basket_local_2[BASKET_DIM];
    float a_Flash_sample;
    float b_Flash_sample, b_Flash_sample_new;
    float b_sample_tmp;
    float a_b_distance;
    int32_t INT_a_Flash_sample;
    int32_t INT_b_Flash_sample;
    uint16_t first_index;
    bool forward;

    switch (mode_player)
    {
    case 0: // A-->B
        a_Flash_sample = a_sample;
        b_Flash_sample = b_sample;
        INT_a_Flash_sample = floor(a_Flash_sample);
        INT_b_Flash_sample = ceil(b_Flash_sample);
        samples_to_read = INT_b_Flash_sample - INT_a_Flash_sample + 1;

        // --A-------a*******b-----B------
        if (INT_b_Flash_sample <= B_Flash_sample)
        {
            // rawfile.seek(INT_a_Flash_sample << 1);
            // rawfile.read(samples_basket, samples_to_read << 1);
            Read_flash(samples_basket, INT_a_Flash_sample, samples_to_read); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            a_first_sample = b_sample + pitch;
            if (ceil(a_first_sample) >= B_Flash_sample && !warmup_for_play_again_flag)
            {
                idle = true;
                power_on = false;
                state = 0;

                rawfile.close();
                AudioStopUsingSPI();
            }
        }

        // ---A------------------a*B*******b-----
        else if (INT_a_Flash_sample <= B_Flash_sample)
        {
            samples_to_read_1 = B_Flash_sample - INT_a_Flash_sample + 1;
            // rawfile.seek(INT_a_Flash_sample << 1);
            // rawfile.read(samples_basket, samples_to_read_1 << 1);
            Read_flash(samples_basket, INT_a_Flash_sample, samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            for (i = samples_to_read_1; i < samples_to_read; ++i)
            {
                samples_basket[i] = 0;
            }

            if (!warmup_for_play_again_flag)
            {
                idle = true;
                power_on = false;
                state = 0;

                rawfile.close();
                AudioStopUsingSPI();
            }
        }

        // -------------A----------------------B---a*******b-----
        else
        {
            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = 0;
            }

            if (!warmup_for_play_again_flag)
            {
                idle = true;
                power_on = false;
                state = 0;

                rawfile.close();
                AudioStopUsingSPI();
            }
        }
        break;

    case 1: // B-->A
        a_b_distance = b_sample - a_sample;
        a_Flash_sample = Mirror(B_Flash_sample, a_sample);
        b_Flash_sample = a_Flash_sample - a_b_distance;

        INT_a_Flash_sample = ceil(a_Flash_sample);  // highest sample
        INT_b_Flash_sample = floor(b_Flash_sample); // lowest sample
        samples_to_read = INT_a_Flash_sample - INT_b_Flash_sample + 1;

        //  ----------A----b'*********a'----B--------------------C
        if (INT_b_Flash_sample >= A_Flash_sample)
        {
            // rawfile.seek(INT_b_Flash_sample <<1);
            // rawfile.read(samples_basket_local, samples_to_read << 1);
            Read_flash(samples_basket_local, INT_b_Flash_sample, samples_to_read); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = samples_basket_local[samples_to_read - 1 - i];
            }

            b_sample_tmp = Mirror(B_Flash_sample, b_Flash_sample);

            a_first_sample = b_sample_tmp + pitch;
            if (ceil(a_first_sample) >= C_Flash_sample)
            {
                idle = true;
                power_on = false;
                state = 0;

                rawfile.close();
                AudioStopUsingSPI();
            }
        }

        //  -----b'****A*****a'-------------B--------------------C
        else
        {
            samples_to_read_1 = INT_a_Flash_sample - A_Flash_sample + 1;
            // rawfile.seek(A_Flash_sample <<1);
            // rawfile.read(samples_basket_local, samples_to_read_1 << 1);
            Read_flash(samples_basket_local, A_Flash_sample, samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            for (i = 0; i < samples_to_read_1; ++i)
            {
                samples_basket[i] = samples_basket_local[samples_to_read_1 - 1 - i];
            }

            for (i = samples_to_read_1; i < samples_to_read; ++i)
            {
                samples_basket[i] = 0;
            }

            if (!warmup_for_play_again_flag)
            {
                idle = true;
                power_on = false;
                state = 0;

                rawfile.close();
                AudioStopUsingSPI();
            }
        }
        break;

    case 2: // loop A-->B

        // .........(A+d)-----------------(B-d)(B-d+1)----B......a*******b......
        // ....a****(A+d)**b--------------(B-d)(B-d+1)----B.....................
        if ((floor(a_sample) > B_Flash_sample) || (floor(a_sample) < (A_Flash_sample + delta_Noclick)))
        {
            a_b_distance = b_sample - a_sample;
            a_sample = A_Flash_sample + delta_Noclick;
            initial_index_offset = 0.0;
            b_sample = a_sample + a_b_distance; // .........(A+d)a*****b---------(B-d)(B-d+1)----B..........
        }
        INT_b_Flash_sample = ceil(b_sample);
        INT_a_Flash_sample = floor(a_sample);
        samples_to_read = INT_b_Flash_sample - INT_a_Flash_sample + 1;

        // .........(A+d)------a*****b----(B-d)(B-d+1)----B.......... --> single read
        if (ceil(b_sample) <= (B_Flash_sample - delta_Noclick))
        {
            // rawfile.seek(INT_a_Flash_sample << 1);
            // rawfile.read(samples_basket, samples_to_read << 1);
            Read_flash(samples_basket, INT_a_Flash_sample, samples_to_read); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            a_first_sample = b_sample + pitch;

            // ......(A+d)-----------------(B-d)(B-d+1)----B....(B+1)..(a_f_s)
            if (floor(a_first_sample) > B_Flash_sample)
            {
                a_first_sample = (A_Flash_sample + delta_Noclick) + (a_first_sample - B_Flash_sample);
            }
        }

        // ..........(A+d)-----------a***(B-d)(B-d+1)**b----B......  --> double read
        else if ((INT_a_Flash_sample <= (B_Flash_sample - delta_Noclick)) && ((INT_b_Flash_sample >= (B_Flash_sample - delta_Noclick + 1)) && (INT_b_Flash_sample <= (B_Flash_sample))))
        {
            // read from flash
            samples_to_read_1 = (B_Flash_sample - delta_Noclick) - INT_a_Flash_sample + 1;
            // rawfile.seek(INT_a_Flash_sample << 1);
            // rawfile.read(samples_basket, samples_to_read_1 << 1);
            Read_flash(samples_basket, INT_a_Flash_sample, samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            // read from RAM and merge
            samples_to_read_2 = INT_b_Flash_sample - (B_Flash_sample - delta_Noclick + 1) + 1;
            for (i = 0; i < samples_to_read_2; ++i)
            {
                samples_basket[samples_to_read_1 + i] = *(p_Noclick + i);
            }

            a_first_sample = b_sample + pitch;
            if (floor(a_first_sample) > B_Flash_sample)
            {
                a_first_sample = (A_Flash_sample + delta_Noclick) + (a_first_sample - B_Flash_sample);
            }
        }

        // ...........(A+d)----------------(B-d)(B-d+1)---a***b---B....  --> single read
        else if ((INT_a_Flash_sample >= (B_Flash_sample - delta_Noclick + 1)) && (INT_b_Flash_sample <= B_Flash_sample))
        {
            // read from RAM
            first_index = floor(a_sample) - (B_Flash_sample - delta_Noclick + 1);
            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = *(p_Noclick + first_index + i);
            }

            a_first_sample = b_sample + pitch;
            if (floor(a_first_sample) > B_Flash_sample)
            {
                a_first_sample = (A_Flash_sample + delta_Noclick) + (a_first_sample - B_Flash_sample);
            }
        }

        // ...........(A+d)***b'----------(B-d)(B-d+1)------a***B****b..
        // ...........(A+d)***b'----------(B-d)(B-d+1)------a***B.......  --> double read
        else if ((INT_a_Flash_sample >= (B_Flash_sample - delta_Noclick + 1)) && (INT_b_Flash_sample > B_Flash_sample))
        {
            // read from RAM
            samples_to_read_1 = B_Flash_sample - INT_a_Flash_sample + 1;
            first_index = floor(a_sample) - (B_Flash_sample - delta_Noclick + 1);
            for (i = 0; i < samples_to_read_1; ++i)
            {
                samples_basket[i] = *(p_Noclick + first_index + i);
            }

            // read again from flash and merge
            samples_to_read_2 = samples_to_read - samples_to_read_1;
            // rawfile.seek((A_Flash_sample + delta_Noclick) << 1);
            // rawfile.read(samples_basket_local, samples_to_read_2 << 1);
            Read_flash(samples_basket_local, (A_Flash_sample + delta_Noclick), samples_to_read_2); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            Append(samples_basket, samples_to_read_1, samples_basket_local, samples_to_read_2);
            // for(i = 0; i < samples_to_read_2; i ++)
            //    samples_basket[samples_to_read_1 + i] = samples_basket_local[i];

            a_first_sample = (A_Flash_sample + delta_Noclick) + (b_sample - B_Flash_sample - 1) + pitch;
        }

        // .............(A+d)***b'--------a**(B-d)(B-d+1)*******B***b.......
        // .............(A+d)***b'--------a**(B-d)(B-d+1)*******B........... --> triple read
        else if ((INT_a_Flash_sample <= (B_Flash_sample - delta_Noclick)) && (INT_b_Flash_sample > B_Flash_sample))
        {
            // read from flash
            samples_to_read_1 = (B_Flash_sample - delta_Noclick) - INT_a_Flash_sample + 1;
            // rawfile.seek(INT_a_Flash_sample << 1);
            // rawfile.read(samples_basket, samples_to_read_1 << 1);
            Read_flash(samples_basket, INT_a_Flash_sample, samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            // read from RAM and merge
            samples_to_read_2 = delta_Noclick;
            for (i = 0; i < samples_to_read_2; ++i)
            {
                samples_basket[samples_to_read_1 + i] = *(p_Noclick + i);
            }

            // read again from flash and merge
            samples_to_read_3 = samples_to_read - samples_to_read_1 - samples_to_read_2;
            // rawfile.seek((A_Flash_sample + delta_Noclick) << 1);
            // rawfile.read(samples_basket_local, samples_to_read_3 << 1);
            Read_flash(samples_basket_local, (A_Flash_sample + delta_Noclick), samples_to_read_3); // Read_flash (int16_t *destination, int seek_in, int samples_in)
            Append(samples_basket, samples_to_read_1 + samples_to_read_2, samples_basket_local, samples_to_read_3);
            // for(i = 0; i < samples_to_read_3; i ++)
            //    samples_basket[samples_to_read_1 + samples_to_read_2 + i] = samples_basket_local[i];
            a_first_sample = (A_Flash_sample + delta_Noclick) + (b_sample - B_Flash_sample - 1) + pitch;
        }
        break;

    case 3: // loop A-->B-->A
        // check direction
        forward = false;
        if (floor(a_sample) < B_Flash_sample) // initial direction: forward (a_sample is REAL address)
        {
            forward = true;
        }

        if (forward) //  ----A--------a>>>>--B-----  (a_sample is REAL address)
        {
            INT_a_Flash_sample = floor(a_sample);
            INT_b_Flash_sample = ceil(b_sample);
            samples_to_read = INT_b_Flash_sample - INT_a_Flash_sample + 1;

            if (b_sample <= B_Flash_sample) //  ----A--------a>>>>>b-B-----  (b_sample is REAL address)
            {
                // read from INT_a_Flash_sample to INT_b_Flash_sample
                // rawfile.seek(INT_a_Flash_sample << 1);
                // rawfile.read(samples_basket, samples_to_read << 1);

                // Serial.print("  samples_to_read: ");
                // Serial.print(samples_to_read);
                // Serial.print("  INT_a_Flash_sample: ");
                // Serial.println(INT_a_Flash_sample);
                Read_flash(samples_basket, INT_a_Flash_sample, samples_to_read); // Read_flash (int16_t *destination, int seek_in, int samples_in)

                a_first_sample = b_sample + pitch;
            }

            else //  ----A---------a>>>B>>b-------------C   (b_sample is NOT_REAL address)
            {
                // read from INT_a_Flash_sample to B_Flash_sample (included)
                samples_to_read_1 = B_Flash_sample - INT_a_Flash_sample + 1;
                // rawfile.seek(INT_a_Flash_sample << 1);
                // rawfile.read(samples_basket, samples_to_read_1 << 1);
                Read_flash(samples_basket, INT_a_Flash_sample, samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

                // read from INT_b_Flash_sample to (B_Flash_sample - 1)
                b_Flash_sample = Mirror(B_Flash_sample, b_sample);
                INT_b_Flash_sample = floor(b_Flash_sample);
                samples_to_read_2 = samples_to_read - samples_to_read_1; // DO NOT read restart sample B_Flash_sample
                // rawfile.seek(INT_b_Flash_sample << 1);
                // rawfile.read(samples_basket_local, samples_to_read_2 << 1);

                // Serial.print("  samples_to_read_2: ");
                // Serial.print(samples_to_read_2);
                // Serial.print("  INT_b_Flash_sample: ");
                // Serial.println(INT_b_Flash_sample);

                Read_flash(samples_basket_local, INT_b_Flash_sample, samples_to_read_2); // Read_flash (int16_t *destination, int seek_in, int samples_in)

                // merge arrays transposing sample_basket_local
                Append_transposed(samples_basket, samples_to_read_1, samples_basket_local, samples_to_read_2);
                a_first_sample = b_sample + pitch;
            }
        }

        else // reverse  ----A---------------B----a>>>>>-----C (a_sample is NOT_REAL address)
        {
            a_Flash_sample = Mirror(B_Flash_sample, a_sample);
            b_Flash_sample = Mirror(B_Flash_sample, b_sample);
            INT_a_Flash_sample = ceil(a_Flash_sample);
            INT_b_Flash_sample = floor(b_Flash_sample);

            // Serial.print("a_Flash_sample: ");
            // Serial.print(a_Flash_sample);
            // Serial.print("  b_Flash_sample: ");
            // Serial.println(b_Flash_sample);

            if (INT_b_Flash_sample >= A_Flash_sample) // -----A---bF<<<<<aF-------B---- (all REAL addresses)
            {
                samples_to_read = INT_a_Flash_sample - INT_b_Flash_sample + 1;
                // rawfile.seek(INT_b_Flash_sample << 1);
                // rawfile.read(samples_basket_local, samples_to_read << 1);
                Read_flash(samples_basket_local, INT_b_Flash_sample, samples_to_read); // Read_flash (int16_t *destination, int seek_in, int samples_in)

                // transpose samples
                Append_transposed(samples_basket, 0, samples_basket_local, samples_to_read);

                b_sample_tmp = Mirror(B_Flash_sample, b_Flash_sample);

                a_first_sample = b_sample + pitch;
                if (a_first_sample > C_Flash_sample)
                {
                    a_first_sample = A_Flash_sample + (a_first_sample - C_Flash_sample);
                }
            }

            else // ----bF<<<A<<aF-----------B---- (all REAL addresses, but b_Flash_sample has to be "mirrored" again)
            {
                // read from A_Flash_sample to INT_a_Flash_sample (both included)
                samples_to_read = INT_a_Flash_sample - A_Flash_sample + 1;

                // Serial.print("samples_to_read: ");
                // Serial.print(samples_to_read);

                // rawfile.seek(A_Flash_sample << 1);
                // rawfile.read(samples_basket_local, samples_to_read << 1);
                Read_flash(samples_basket_local, A_Flash_sample, samples_to_read); // Read_flash (int16_t *destination, int seek_in, int samples_in)

                // swap samples
                Append_transposed(samples_basket, 0, samples_basket_local, samples_to_read);

                // read from (A_Flash_sample + 1) to INT_b_Flash_sample
                b_Flash_sample = Mirror(A_Flash_sample, b_Flash_sample); // ---A----bF*****aF-----------B---- (all REAL addresses)
                INT_b_Flash_sample = ceil(b_Flash_sample);

                samples_to_read_1 = INT_b_Flash_sample - A_Flash_sample; // // DO NOT read restart sample A_Flash_sample
                // rawfile.seek((A_Flash_sample + 1) <<1);
                // rawfile.read(samples_basket_local, samples_to_read_1 << 1);

                // Serial.print(" samples_to_read_1: ");
                // Serial.println(samples_to_read_1);

                Read_flash(samples_basket_local, (A_Flash_sample + 1), samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

                // merge vectors
                Append(samples_basket, samples_to_read, samples_basket_local, samples_to_read_1);
                a_first_sample = b_Flash_sample + pitch;
            }
        }
        break;

    case 5: // loop B-->A
        // a_sample and b_sample are virtual sample
        // B_Flash_sample_5 is (B_value_in - delta_Noclick)
        a_b_distance = b_sample - a_sample;
        a_Flash_sample = Mirror(B_Flash_sample_5, a_sample);
        b_Flash_sample = Mirror(B_Flash_sample_5, b_sample); // a_Flash_sample - a_b_distance;
        INT_a_Flash_sample = ceil(a_Flash_sample);
        INT_b_Flash_sample = floor(b_Flash_sample);
        samples_to_read = INT_a_Flash_sample - INT_b_Flash_sample + 1;

        // .........A------------(A+d-1)(A+d)-------bF<<<<<<<aF--------(B_5)..... --> single read
        if ((INT_b_Flash_sample >= (A_Flash_sample + delta_Noclick)))
        {
            // read from flash and transpose
            // rawfile.seek(INT_b_Flash_sample << 1);
            // rawfile.read(samples_basket_local, samples_to_read << 1);
            Read_flash(samples_basket_local, INT_b_Flash_sample, samples_to_read); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = samples_basket_local[samples_to_read - 1 - i];
            }
            // Append_transposed(samples_basket, 0, samples_basket_local, samples_to_read);

            b_sample_tmp = Mirror(B_Flash_sample_5, b_Flash_sample);
            a_first_sample = b_sample_tmp + pitch;
            if (floor(a_first_sample) > C_Flash_sample)
            {
                a_first_sample = B_Flash_sample_5 + (a_first_sample - C_Flash_sample);
            }
        }

        // .........A------bF<<(A+d-1)(A+d)<<<<<aF----------------------(B_5)..... --> double read
        else if ((INT_a_Flash_sample >= (A_Flash_sample + delta_Noclick)) && (INT_b_Flash_sample >= A_Flash_sample) && (INT_b_Flash_sample <= (A_Flash_sample + delta_Noclick - 1)))
        {
            // from RAM
            samples_to_read_1 = A_Flash_sample + delta_Noclick - INT_b_Flash_sample;
            first_index = INT_b_Flash_sample - A_Flash_sample;
            for (i = 0; i < samples_to_read_1; ++i)
            {
                samples_basket_local[i] = *(p_Noclick + first_index + i);
            }

            // from flash
            samples_to_read_2 = INT_a_Flash_sample - (A_Flash_sample + delta_Noclick) + 1;
            // rawfile.seek((A_Flash_sample + delta_Noclick) << 1);
            // rawfile.read(samples_basket_local_2, samples_to_read_2 << 1);
            Read_flash(samples_basket_local_2, (A_Flash_sample + delta_Noclick), samples_to_read_2); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            // Append
            for (i = 0; i < samples_to_read_2; ++i)
            {
                samples_basket_local[samples_to_read_1 + i] = samples_basket_local_2[i];
            }

            // Copy and transpose
            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = samples_basket_local[samples_to_read - 1 - i];
            }

            b_sample_tmp = Mirror(B_Flash_sample_5, b_Flash_sample);
            a_first_sample = b_sample_tmp + pitch;
            if (floor(a_first_sample) > C_Flash_sample)
            {
                a_first_sample = B_Flash_sample_5 + (a_first_sample - C_Flash_sample);
            }
        }

        // ....bF<<<A<<<<<<<<<<<<(A+d-1)(A+d)<<<<aF-----------------(B_5).......
        // .........A<<<<<<<<<<<<(A+d-1)(A+d)<<<<aF-----------b'F<<<(B_5)....... --> triple read
        else if ((INT_a_Flash_sample >= (A_Flash_sample + delta_Noclick)) && (INT_b_Flash_sample < A_Flash_sample))
        {
            b_Flash_sample_new = B_Flash_sample_5 - (A_Flash_sample - 1 - b_Flash_sample);
            INT_b_Flash_sample = floor(b_Flash_sample_new);

            // from flash - last
            samples_to_read_1 = B_Flash_sample_5 - INT_b_Flash_sample + 1;
            // rawfile.seek(INT_b_Flash_sample << 1);
            // rawfile.read(samples_basket_local, samples_to_read_1 << 1);
            Read_flash(samples_basket_local, INT_b_Flash_sample, samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            // from RAM
            samples_to_read_2 = delta_Noclick;
            first_index = 0;
            // Append directly
            for (i = 0; i < samples_to_read_2; ++i)
            {
                samples_basket_local[i + samples_to_read_1] = *(p_Noclick + first_index + i);
            }

            // from flash - first
            samples_to_read_3 = INT_a_Flash_sample - (A_Flash_sample + delta_Noclick) + 1;
            // rawfile.seek((A_Flash_sample + delta_Noclick) << 1);
            // rawfile.read(samples_basket_local_2, samples_to_read_3 << 1);
            Read_flash(samples_basket_local_2, (A_Flash_sample + delta_Noclick), samples_to_read_3); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            // Append
            for (i = 0; i < samples_to_read_3; ++i)
            {
                samples_basket_local[samples_to_read_1 + samples_to_read_2 + i] = samples_basket_local_2[i];
            }

            // Copy and transpose
            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = samples_basket_local[samples_to_read - 1 - i];
            }

            b_sample_tmp = Mirror(B_Flash_sample_5, b_Flash_sample_new);
            a_first_sample = b_sample_tmp + pitch;
        }

        // ...........A--bF<<<<<aF---(A+d-1)(A+d)--------------------(B_5).....  --> single read
        else if ((INT_a_Flash_sample <= (A_Flash_sample + delta_Noclick - 1)) && (INT_b_Flash_sample >= A_Flash_sample))
        {
            // from RAM
            first_index = INT_b_Flash_sample - A_Flash_sample;
            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket_local[i] = *(p_Noclick + first_index + i);
            }

            // Transpose
            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = samples_basket_local[samples_to_read - 1 - i];
            }

            b_sample_tmp = Mirror(B_Flash_sample_5, b_Flash_sample);
            a_first_sample = b_sample_tmp + pitch;
            if (floor(a_first_sample) > C_Flash_sample)
            {
                a_first_sample = B_Flash_sample_5 + (a_first_sample - C_Flash_sample);
            }
        }

        // ....bF<<<A<<<<<<<<aF-----(A+d-1)(A+d)--------------------(B_5).......
        // ...........A<<<<<<aF-----(A+d-1)(A+d)-------------b'F<<<<(B_5)....... --> double read
        else if ((INT_a_Flash_sample <= (A_Flash_sample + delta_Noclick - 1)) && (INT_b_Flash_sample < A_Flash_sample))
        {
            b_Flash_sample_new = B_Flash_sample_5 - (A_Flash_sample - 1 - b_Flash_sample);
            INT_b_Flash_sample = floor(b_Flash_sample_new);

            // from flash
            samples_to_read_1 = B_Flash_sample_5 - INT_b_Flash_sample + 1;
            // rawfile.seek(INT_b_Flash_sample << 1);
            // rawfile.read(samples_basket_local, samples_to_read_1 << 1);
            Read_flash(samples_basket_local, INT_b_Flash_sample, samples_to_read_1); // Read_flash (int16_t *destination, int seek_in, int samples_in)

            // from RAM
            samples_to_read_2 = INT_a_Flash_sample - A_Flash_sample + 1;
            first_index = 0;
            // Append directly
            for (i = 0; i < samples_to_read_2; ++i)
            {
                samples_basket_local[i + samples_to_read_1] = *(p_Noclick + first_index + i);
            }

            // Transpose
            for (i = 0; i < samples_to_read; ++i)
            {
                samples_basket[i] = samples_basket_local[samples_to_read - 1 - i];
            }

            b_sample_tmp = Mirror(B_Flash_sample_5, b_Flash_sample_new);
            a_first_sample = b_sample_tmp + pitch;
        }
        break;
    }
}

void AudioPlayer::Wavetable_harvest()
{
    uint16_t samples_to_read = 0;
    uint16_t samples_to_read_W;
    float Wavetable_LOW_sample;    // indirizzo float su RAM
    float Wavetable_HIGH_sample;   // indirizzo float su RAM
    int Wavetable_LOW_sample_int;  // indirizzo su RAM
    int Wavetable_HIGH_sample_int; // indirizzo su RAM
    int16_t i = 0;

    Wavetable_LOW_sample = a_sample - Flash_first_RAM_sample;  // address on RAM corresponding to a_sample on Flash
    Wavetable_HIGH_sample = b_sample - Flash_first_RAM_sample; // address on RAM corresponding to b_sample on Flash
    Wavetable_LOW_sample_int = floor(Wavetable_LOW_sample);
    Wavetable_HIGH_sample_int = ceil(Wavetable_HIGH_sample);

    if (mode_player < 2) // A-->B  B-->A
    {
        initial_index_offset = Wavetable_LOW_sample - Wavetable_LOW_sample_int;
        samples_to_read = Wavetable_HIGH_sample_int - Wavetable_LOW_sample_int + 1;

        // 0-----a*******b----(WTL-1)
        if (Wavetable_HIGH_sample_int <= (Wavetable_length - 1))
        {
            while (i < samples_to_read)
            {
                samples_basket[i] = *(p_Wavetable + Wavetable_LOW_sample_int + i);
                i++;
            }
        }

        // 0-----a*******(WTL-1)**b
        else
        {
            samples_to_read_W = Wavetable_length - Wavetable_LOW_sample_int;
            while (i < samples_to_read_W)
            {
                samples_basket[i] = *(p_Wavetable + Wavetable_LOW_sample_int + i);
                i++;
            }
            while (i < samples_to_read)
            {
                samples_basket[i] = 0;
                i++;
            }

            if (!warmup_for_play_again_flag)
            {
                idle = true;
                power_on = false;
                state = 0;

                rawfile.close();
                AudioStopUsingSPI();
            }
        }
        a_first_sample = b_sample + pitch;
        return;
    }

    else // mode_player >= 2 (loops)
    {
        initial_index_offset = Wavetable_LOW_sample - Wavetable_LOW_sample_int;
        samples_to_read = Wavetable_HIGH_sample_int - Wavetable_LOW_sample_int + 1;

        // read from Wavetable_LOW_sample_int to Wavetable_HIGH_sample_int
        int16_t i = 0;
        while (i < samples_to_read)
        {
            samples_basket[i] = *(p_Wavetable + ((Wavetable_LOW_sample_int + i) % Wavetable_length));
            i++;
        }
        a_first_sample = fmod(Wavetable_HIGH_sample + pitch, Wavetable_length) + Flash_first_RAM_sample; // The function fmod() returns the floating-point remainder of x/y.
    }
}

void AudioPlayer::Set_identity(uint8_t value)
{
    identity = value;
}
bool AudioPlayer::isPlaying(void)
{
    return !idle;
}

bool AudioPlayer::isPoweredOn(void)
{
    return power_on;
}

void AudioPlayer::set_file(int file_id_in)
{
    file_id_wait = file_id_in;
}

void AudioPlayer::Set_ADSR(float attack_in, float decay_in, float sustain_in, float release_in, uint8_t attack_type_in) // set ADSR parameters. Times are in seconds. 0 <= sustain_value <= 1.0
{
    Attack = ((attack_in > 0.01) ? attack_in : 0.01);    // [sec]; referred to pitch = 1.0
    Decay = ((decay_in > 0.05) ? decay_in : 0.05);       // [sec]; referred to pitch = 1.0
    Sustain = sustain_in;                                // [number]; referred to pitch = 1.0
    Release = ((release_in > 0.05) ? release_in : 0.05); // [sec]; referred to pitch = 1.0
    attack_type_wait = attack_type_in;
}

void AudioPlayer::Set_volume(float volume_gain_value)
{
    volume_gain_wait = volume_gain_value;
}

void AudioPlayer::Update_volume(float volume_gain_value)
{
    volume_gain_wait = volume_gain_value;
    if (!idle && (volume_gain_value != volume_gain))
    {
        volume_gain_warmup_flag = true;
    }
}

void AudioPlayer::Set_pan(int pan_int_value)
{
    pan_int_wait = pan_int_value;
    pan_gain_L_wait = pan_gain_L_table[pan_int_wait + 16];
    pan_gain_R_wait = pan_gain_R_table[pan_int_wait + 16];
}

void AudioPlayer::Set_pitch(float pitch_tune_in) // 0.5= half speed, 1.0= no change 2.0=double speed
{
    pitch_tune_wait = pitch_tune_in;
    pitch_tune_flag = true;
}

void AudioPlayer::Set_note(float pitch_note_in)
{
    pitch_note = pitch_note_in;
}

void AudioPlayer::Set_pitch_bend(float pitch_bend_in) // 0.5= half speed, 1.0= no change 2.0=double speed
{
    pitch_bend_wait = pitch_bend_in;
    pitch_bend_flag = true;
}

void AudioPlayer::Write_loop_track(int track)
{
    track_wait = track;
}

int AudioPlayer::Read_loop_track(void)
{
    return track;
}

void AudioPlayer::Fast_stop(void)
{
    K_Release_step = 10 / 128.0f; // gain fall to 0 in 10 samples!
    ADSR_point_0 = -K_Release_step;
    K_Release_delta = ADSR_gain;
    ADSR_phase = 3; // Release
    power_on = false;

    if (Lilla_state == MIDI_LOOP && track >= 0)
    {
        Players_statistics_ptr->Dec_total_Players_per_track_instrument(track, instrument);
        Set_led_flags(track);
    }

    else if (Lilla_state != MIDI_LOOP)
    {
        Players_statistics_ptr->Dec_total_Players_per_instrument(instrument);
        Set_led_flags(track);
    }

    state = 2;
}

void AudioPlayer::Update_pan(float pan_int_value)
{
    pan_int_wait = pan_int_value;
    pan_gain_L_wait = pan_gain_L_table[pan_int_wait + 16];
    pan_gain_R_wait = pan_gain_R_table[pan_int_wait + 16];
    if (!idle && (pan_int_wait != pan_int))
    {
        pan_gain_warmup_flag = true;
    }
}

void AudioPlayer::Set_vibrato_pointers(float *p_vibrato_array_in, uint8_t *p_vibrato_array_last_element_in)
{
    p_vibrato_array = p_vibrato_array_in;
    p_vibrato_array_last_element = p_vibrato_array_last_element_in;
}

void AudioPlayer::Set_vibrato_flag(bool value)
{
    if (vibrato_flag)
    {
        if (value)
        {
            return;
        }
        else
        {
            vibrato_flag = false;
            pitch_vibrato = 1.0;
        }
    }
    else
    {
        if (value)
        {
            vibrato_flag = true;
            vibrato_array_element_float = *p_vibrato_array_last_element;
            return;
        }
        else
        {
            return;
        }
    }
}

void AudioPlayer::Set_mix_samples(uint8_t value)
{
    mix_samples = value;
}

float AudioPlayer::Read_pitch(void)
{
    return constrain(pitch_note * pitch_bend * pitch_tune * pitch_vibrato, MIN_PITCH, pitch_limit);
}

void AudioPlayer::Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated)
{
    VCF_connect = use;
    VCF_modulated = modulated;
    VCF_frequency_pivot = pivot;
    // Serial.print("VCF_frequency_pivot:");
    // Serial.println(VCF_frequency_pivot);
    // Serial.print("VCF_modulated:");
    // Serial.println(VCF_modulated);

    if (VCF_connect && VCF_modulated)
    {
        VCF_ptr->LFO_connect = true;
        VCF_ptr->filter_type = type;
        VCF_ptr->q_value = resonance; // Butterworth: 0.7071

        VCF_ptr->_block = block;                   // int16_t*
        VCF_ptr->_frequency = VCF_frequency_array; // float*
    }

    else if (VCF_connect && !VCF_modulated)
    {
        VCF_ptr->LFO_connect = false;
        VCF_ptr->filter_type = type;
        VCF_ptr->q_value = resonance;                // Butterworth: 0.7071
        VCF_ptr->Set_filter(0, VCF_frequency_pivot); // Set_filter(uint32_t stage, float frequency, float q = 0.7071)

        VCF_ptr->_block = block;                   // int16_t*
        VCF_ptr->_frequency = VCF_frequency_array; // float*
    }
}

void AudioPlayer::Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time) // used by Midi_Reader
{
    LFO_modulation = modulation;
    LFO_index = index;
    LFO_index_steps = 0;
    LFO_periodic = periodic;
    if (LFO_periodic == 1)
    {
        LFO_ptr->Set_frequency(frequency_time);
    }
    else
    {
        LFO_seconds = frequency_time;
    }
}

void AudioPlayer::Start_VCF(void)
{
    if (VCF_connect)
    {
        // VFC setup
        Update_pitch();
        VCF_central_frequency = VCF_frequency_pivot * pitch;
        // Serial.print("Player.h - VCF_connect - VCF_central_frequency:");
        // Serial.println(VCF_central_frequency);

        if (VCF_modulated)
        {
            if (LFO_periodic == 1)
            {
                // Serial.println("LFO Setup: PERIODIC");
                LFO_ptr->Set_amplitude(3000); // amplitude <= 32767
            }
            else
            {
                // Serial.println("LFO Setup: APERIODIC");
                LFO_ptr->Setup_aperiodic_wave(LFO_seconds, LFO_index * 3000, LFO_modulation); // LFO_ptr->SETUP_wave(LFO_seconds, LFO_index * 1000.0 * pitch, LFO_modulation); // SETUP_wave(float seconds, float max_value, int waveform) // aperiodic wave - period: seconds  n: waveform
            }
        }
    }
}

void AudioPlayer::Update_VCF_pivot(float new_pivot)
{
    float delta_pivot = new_pivot - VCF_frequency_pivot;
    if (abs(delta_pivot) > 5)
    {
        VCF_pivot_steps = 40;
        VCF_pivot_grain = (delta_pivot) / 40.0; // 20 steps
    }
}

void AudioPlayer::Update_LFO_index(float new_index)
{
    float delta_index = new_index - LFO_index;
    if (abs(delta_index) > 0.01)
    {
        LFO_index_steps = 1000;
        LFO_index_grain = (delta_index) / 1000.0f; // == /LFO_index_steps
    }
}

void AudioPlayer::Update_VCF_resonance(float resonance)
{
    VCF_ptr->q_value = resonance; // Butterworth: 0.7071
}

void AudioPlayer::Send_LFO_to_VCF(void)
{
    const uint8_t ABS_4 = AUDIO_BLOCK_SAMPLES / 4;
    const uint8_t ABS_2 = AUDIO_BLOCK_SAMPLES / 2;
    const uint8_t ABS_3_4 = 3 * AUDIO_BLOCK_SAMPLES / 4;

    if (LFO_index_steps <= 0 && VCF_pivot_steps <= 0)
    {
        VCF_ptr->Set_filter(0, constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[0] / 1000.0f) * LFO_index), 50, 15000));     // Set_filter(uint32_t stage, float frequency)
        VCF_frequency_array[0] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[0] / 1000.0f) * LFO_index), 50, 15000); // ONLY FOR PRINT
        VCF_frequency_array[1] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_4] / 1000.0f) * LFO_index), 50, 15000);
        VCF_frequency_array[2] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_2] / 1000.0f) * LFO_index), 50, 15000);
        VCF_frequency_array[3] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_3_4] / 1000.0f) * LFO_index), 50, 15000);
    }
    else if (LFO_index_steps > 0)
    {
        LFO_index += LFO_index_grain;
        VCF_ptr->Set_filter(0, constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[0] / 1000.0f) * LFO_index), 50, 15000));     // Set_filter(uint32_t stage, float frequency)
        VCF_frequency_array[0] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[0] / 1000.0f) * LFO_index), 50, 15000); // ONLY FOR PRINT
        LFO_index += LFO_index_grain;
        VCF_frequency_array[1] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_4] / 1000.0f) * LFO_index), 50, 15000);
        LFO_index += LFO_index_grain;
        VCF_frequency_array[2] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_2] / 1000.0f) * LFO_index), 50, 15000);
        LFO_index += LFO_index_grain;
        VCF_frequency_array[3] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_3_4] / 1000.0f) * LFO_index), 50, 15000);
        LFO_index_steps -= 4;
        // Serial.print("LFO_index_steps: ");
        // Serial.println(LFO_index_steps);
    }
    else if (VCF_pivot_steps > 0)
    {
        VCF_frequency_pivot += VCF_pivot_grain;
        VCF_central_frequency = (VCF_frequency_pivot * pitch);
        VCF_ptr->Set_filter(0, constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[0] / 1000.0f) * LFO_index), 50, 15000));     // Set_filter(uint32_t stage, float frequency, float q = 0.7071)
        VCF_frequency_array[0] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[0] / 1000.0f) * LFO_index), 50, 15000); // ONLY FOR PRINT
        VCF_frequency_pivot += VCF_pivot_grain;
        VCF_central_frequency = (VCF_frequency_pivot * pitch);
        VCF_frequency_array[1] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_4] / 1000.0f) * LFO_index), 50, 15000);
        VCF_frequency_pivot += VCF_pivot_grain;
        VCF_central_frequency = (VCF_frequency_pivot * pitch);
        VCF_frequency_array[2] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_2] / 1000.0f) * LFO_index), 50, 15000);
        VCF_frequency_pivot += VCF_pivot_grain;
        VCF_central_frequency = (VCF_frequency_pivot * pitch);
        VCF_frequency_array[3] = constrain(VCF_central_frequency * pow(2.0f, (LFO_ptr->block[ABS_3_4] / 1000.0f) * LFO_index), 50, 15000);
        VCF_pivot_steps -= 4;
        // Serial.print("VCF_pivot_steps: ");
        // Serial.println(VCF_pivot_steps);
    }
}

float AudioPlayer::Mirror(float pivot, float value)
{
    return (2.0 * pivot) - value;
}

void AudioPlayer::Append_transposed(int16_t *_target, uint16_t first_index, int16_t *_source, uint16_t N)
{
    _target += first_index;
    _source += N - 1;
    for (uint16_t i = 0; i < N; ++i)
    {
        *_target = *_source;
        _target++;
        _source--;
    }
}

void AudioPlayer::Append(int16_t *_target, uint16_t first_index, int16_t *_source, uint16_t N)
{
    _target += first_index;
    for (uint16_t i = 0; i < N; ++i)
    {
        *_target = *_source;
        _target++;
        _source++;
    }
}

void AudioPlayer::Read_flash(int16_t *destination, int first_sample, int total_samples)
{
    int first_byte = (first_sample) * 2; // PD - 2
    int total_bytes = total_samples * 2; // 8
    byte *destination_byte = (byte *)destination;
    int first_part_samples;
    int second_part_bytes;

    /*
    Nella modalità SAMPLER, si devono leggere i file Packet di dimensione 64KB - PD = PACKET_DIMENSION = 64K

    Packet       |     first Packet 13         |          Packet 14          |          Packet 15          |
    local Byte   | 0 1 2  ............. (PD-1) | 0 1 2 3 4 .. 11 .... (PD-1) | 0 ..... 5........... (PD-1) |
    first/last                        fB  *                                    * * * * lB
    local_fB = PD - 2
    local_lB = 5
    */

    if (recording_flag)
    {
        int needed_packet_delta = (stereo_flag ? 2 : 1) * (first_byte >> 16); // 2*(PD - 2)/PD = 0
        // Serial.print("needed_packet_delta is: ");
        // Serial.println(needed_packet_delta);

        if (needed_packet_delta != packet_delta)
        {
            rawfile.close();
            packet_delta = needed_packet_delta;                    // 0
            rawfile.packet_fast_open(first_packet + packet_delta); // rawfile = SerialFlash.open(name_packet[first_packet + packet_delta]); // 13

            // Serial.print(F("1 - Packet played is: "));
            // Serial.println(name_packet[first_packet + packet_delta]);
        }

        int local_first_byte = first_byte % PACKET_DIM;           // (PD - 2)%PD = (PD - 2)
        int local_last_byte = local_first_byte + total_bytes - 1; // (PD - 2) + 8 - 1 = PD + 5

        if (local_last_byte < PACKET_DIM) // 1 Block is needed
        {
            // timer = 0;
            rawfile.seek(local_first_byte);
            rawfile.read(destination_byte, total_bytes);
            // Serial.println(timer);
        }

        else // 2 Blocks are needed - with T41@600MHz adds 40microseconds
        {
            // Serial.print ("2 Blocks are needed");
            // Serial.print("first_sample: ");
            // Serial.print(first_sample);
            // Serial.print(" total_samples: ");
            // Serial.println(total_samples);

            // timer = 0;
            int first_part = PACKET_DIM - local_first_byte; // PD - (PD - 2) = 2
            int second_part = total_bytes - first_part;     // 8 - 2 = 6

            rawfile.seek(local_first_byte);             // (PD - 2)
            rawfile.read(destination_byte, first_part); // read 2 bytes
            rawfile.close();

            packet_delta = packet_delta + (stereo_flag ? 2 : 1);   // 0 + 2 = 2
            rawfile.packet_fast_open(first_packet + packet_delta); // rawfile = SerialFlash.open(name_packet[first_packet + packet_delta]); // 15
            rawfile.seek(0);
            rawfile.read(destination_byte + first_part, second_part); // read 6 bytes
            // Serial.println("*** Player.h  ****  Flash reading_time is: ");
            // Serial.println(timer);
            // Serial.print(F("2 - Packet played is: "));
            // Serial.println(name_packet[first_packet + packet_delta]);
        }
    }

    else if (LS_flag)
    {
        while (first_sample > LS_buffer_dim - 1)
        {
            first_sample -= LS_buffer_dim;
        }

        while (first_sample < 0)
        {
            first_sample += LS_buffer_dim;
        }

        int last_sample = first_sample + total_samples - 1;

        if (last_sample <= LS_buffer_dim - 1)
        {
            memcpy(destination, (FIFO + first_sample), total_bytes);
        }

        else
        {
            first_part_samples = LS_buffer_dim - first_sample; // lenght in samples
            second_part_bytes = total_bytes - 2 * first_part_samples;
            memcpy(destination, (FIFO + first_sample), 2 * first_part_samples);
            memcpy(destination + first_part_samples, FIFO, second_part_bytes);
        }
    }

    else
    {
        rawfile.seek(first_byte);
        rawfile.read(destination_byte, total_bytes);
    }
}

int AudioPlayer::Read_session_wait(void)
{
    return session_wait;
}

int AudioPlayer::Read_local_session(void)
{
    return local_session;
}

bool AudioPlayer::Read_precedence(void)
{
    return precedence;
}

int AudioPlayer::Read_midi_channel(void)
{
    return midi_channel;
}

int AudioPlayer::Read_instrument(void)
{
    return instrument;
}

int AudioPlayer::Read_id_sound(void)
{
    return id_sound;
}
int AudioPlayer::Read_note(void)
{
    return note;
}

unsigned long AudioPlayer::Read_time_stamp(void)
{
    return time_stamp;
}

int AudioPlayer::Read_use_Wavetable(void)
{
    return use_Wavetable;
}

int AudioPlayer::State(void)
{
    return state;
}

int AudioPlayer::Read_update_time(void)
{
    return update_time;
}
void AudioPlayer::Write_update_time(uint16_t value)
{
    update_time = value;
}
void AudioPlayer::Write_midi_channel(int value)
{
    midi_channel = value;
}

void AudioPlayer::Write_precedence(bool value)
{
    precedence = value;
}

void AudioPlayer::Write_time_stamp(unsigned long value)
{
    time_stamp = value;
}