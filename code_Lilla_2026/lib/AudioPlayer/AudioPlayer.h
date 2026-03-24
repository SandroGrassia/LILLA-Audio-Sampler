/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <SerialFlash.h>
#include <spi_interrupt.h>
#include <Wire.h>
#include <utility/dspinst.h>
#include "config.h"
#include "LillaSerialFlash.h"
#include "SharedElements.h"
#include "AudioVCF.h"
#include "WaveLFO.h"
#include "StereoLiveSampler.h"
#include "PlayersStatistics.h"
#include "Functions.h"
#include "AudioADSR.h"

class AudioPlayer : public AudioStream
{
private:
    static constexpr int BASKET_DIM = 3100;    // la dimensione deve essere non inferiore a AUDIO_BLOCK_SAMPLES * MAX_PITCH_WAVETABLE
    static int16_t samples_basket[BASKET_DIM]; // cache array unico per samples copiati dai Player
    int16_t block[AUDIO_BLOCK_SAMPLES];
    uint8_t mix_samples = 32;
    uint8_t identity;
    enum PlayerStates
    {
        IDLE,   // no output available
        RUNNING, // running, no stop request
        FADING,  //  stop requested, output is falling down
        IDLE_REQUEST // last update, than go to IDLE
    };
    
    int state;
    LillaSerialFlashFile rawfile; // SerialFlashFile rawfile;
    int file_id;
    uint32_t samples_counter; // starting from play()

    volatile bool power_on = false; // play() ONLY can set power_on = true; the stop event (Release_note(), ADSR_gain=0, end of file) coming for first sets power_on=false:
    volatile bool idle = true;

    /*
    power_on and idle

    Get_ready_to_play() sets power_on=true, than Player waits for next update() to start playing (idle=false);
    When Player starts playing, idle is set idle=false;
    The stop event coming for first sets power_on=false:
        - Release_note() has less priority, it only sets power_on=false
        - ADSR_gain=0 immediately sets power_on=false, idle=true
        - same End-Of-File event

    ** Release_note() comes first:
    play     ______!________________________________
    Release_note ____________________%______________

    power_on ______|-----------------%______________
    idle     ------|_______________________|--------

    ADSR_gain _____|***********************|________
    EOF      ____________________________________!__

    state   0000000111111111111111111222222000000000



    ** ADSR_gain=0 comes first:
    play     ______!________________________________
    Release_note _____________________!_____________

    power_on ______|--------------%_________________
    idle     ------|______________|-----------------

    ADSR_gain______|**************%_________________
    EOF      ____________________________________!__

    state   0000000111111111111111100000000000000000



    ** End-Of-File comes first:
    play     ______!________________________________
    Release_note ____________________!______________

    power_on ______|----------%_____________________
    idle     ------|__________|---------------------

    ADSR    _______|*************************|______
    EOF      _________________%_____________________

    state   0000000111111111110000000000000000000000

    */

    float *vibrato_array_ptr;
    float vibrato_array_element_float = 0;
    uint8_t *vibrato_array_last_element_ptr;
    float pitch_vibrato = 1.0;

    // ADSR
    AudioADSR *ADSR = nullptr;

    // MIDI_LOOP
    int track;
    int track_wait;

    // VCF
    bool VCF_connect = false;
    bool VCF_modulated = false;
    float VCF_frequency_pivot = 5000.0;
    float VCF_central_frequency;
    float VCF_frequency_array[4];
    int8_t VCF_pivot_steps = 0;
    float VCF_pivot_grain;
    void Send_LFO_to_VCF(void);
    int LFO_modulation = 0;
    float LFO_index = 1.0;
    uint8_t LFO_periodic = 0;
    float LFO_seconds = 1.0;
    int LFO_index_steps = 0;
    float LFO_index_grain = 0;

    // check performance
    elapsedMicros local_timer;

    // Edit variables
    uint8_t mode_player_E;
    int16_t *Wavetable_E_ptr;
    int16_t *Noclick_E_ptr;
    int A_Flash_sample_E;
    int B_Flash_sample_E;
    int C_Flash_sample_E;
    int B_Flash_sample_shifted_E;
    uint16_t delta_Noclick_E;
    bool use_Wavetable_E;
    int32_t Wavetable_length_E;
    int32_t Flash_first_RAM_sample_E;
    float pitch_limit_E;

    // Operation variables
    bool precedence = false;
    int midi_channel = 0;
    int local_patch = 0;
    int instrument_id = 0;
    int sound_id = 0;
    int note = 0;
    unsigned long time_stamp = 0;
    int update_time;
    
    float pitch = 0.0;
    float pitch_note;
    float velocity_gain; // 0 <= velocity_gain <= 1.0
    float pitch_based_gain_correction;
    float pitch_tune = 1.0;
    float pitch_limit;
    float pitch_bend = 1.0;

    float ADSR_gain;
    float volume_gain = 1.0;
    float K_volume_gain; // [Gain/sample]
    uint32_t JV0;
    int8_t pan_int;
    float pan_gain_L;
    float pan_gain_R;
    float K_pan_gain_L, K_pan_gain_R;
    uint32_t JP0;
    float K_resolution_step = 1;
    uint8_t downsampling;
    int16_t stored_sample;
    uint8_t sliding;
    float raw_first_value_cache[AUDIO_BLOCK_SAMPLES];
    float fast_stop_gain;
    uint8_t mode_player;
    int A_Flash_sample;
    int B_Flash_sample;
    int B_Flash_sample_0;
    int C_Flash_sample;
    int B_Flash_sample_shifted;
    float a_sample;
    float b_sample;
    float a_first_sample;
    float initial_index_offset;
    int16_t *Wavetable_ptr;
    bool use_Wavetable_old;
    int16_t INT_Wavetable_B_sample;
    int16_t INT_Wavetable_C_sample;
    int32_t Wavetable_length;
    int32_t Flash_first_RAM_sample;
    int16_t *Noclick_ptr;
    uint16_t delta_Noclick = 0;

    // wait variables
    int file_id_wait;
    float volume_gain_wait;
    int patch_id_wait;
    int instrument_id_wait;
    int sound_id_wait;
    int note_wait;
    float pitch_note_wait;    // 0.5=half speed 1=original speed 2=double speed
    float velocity_gain_wait; // 0 <= velocity_gain <= 1.0
    float pitch_tune_wait;
    float pitch_bend_wait;
    uint8_t mode_player_wait;
    int pan_int_wait;
    float pan_gain_L_wait;
    float pan_gain_R_wait;
    int A_Flash_sample_wait;
    int B_Flash_sample_wait;
    int C_Flash_sample_wait;
    int B_Flash_sample_shifted_wait;
    float a_first_sample_wait;
    int16_t *Wavetable_wait_ptr;
    int16_t *Noclick_wait_ptr;
    int delta_Noclick_wait;
    int Wavetable_length_wait;
    int Flash_first_RAM_sample_wait;
    float pitch_limit_wait;
    bool use_Wavetable_wait;
    int K_resolution_step_wait;
    int downsampling_wait;

    // flags
    bool warmup_for_play_again_flag = false; // quando si riceve Get_ready_to_play ma il Player è !idle, il Player non puo' partire immediatamente
    bool restart_flag = false;
    bool main_settings_editing_flag = false;
    bool vibrato_flag = false;
    bool pitch_tune_flag = false;
    bool pitch_bend_flag = false;
    bool resolution_flag_wait = false;
    bool downsampling_flag_wait = false;
    bool volume_flag = false;
    bool volume_gain_warmup_flag = false;
    bool pan_flag = false;
    bool pan_gain_warmup_flag = false;
    bool set_effects_flag = false;
    bool resolution_flag = false;
    bool downsampling_flag = false;
    bool use_Wavetable = false;

    // DIRECT_SAMPLIG
    int recording;
    bool recording_flag = false;
    bool stereo_flag = false;
    int packet_delta = 0;
    int first_packet = 0;

    // LIVE_SAMPLING
    bool LS_flag = false;
    int16_t *FIFO;
    int FIFO_dim;

    void Update_pitch(void);
    void Update_volume_gain(void);
    void Update_pan_gain(void);
    void Start_playing(void);

    void Flash_memory_harvest(void);
    void Wavetable_harvest(void);
    void Read_flash(int16_t *destination, int first_sample, int total_samples);

    float Mirror(float pivot, float value);
    void Append_reversed(int16_t *target_ptr, uint16_t first_index, int16_t *source_ptr, uint16_t N);
    void Append(int16_t *target_ptr, uint16_t first_index, int16_t *source_ptr, uint16_t N);
    
    bool myLED;
    void My_LED(bool on);

public:
    AudioPlayer(void) : AudioStream(0, NULL)
    {
        begin();
    }

    void begin(void);
    virtual void update(void);
    int State(void);

    // Settati alla creazione di ciascuna istanza
    void Set_identity(uint8_t value);
    AudioVCF *VCF_ptr = nullptr;
    WaveLFO *LFO_ptr = nullptr;
    StereoLiveSampler *LiveSampler_ptr = nullptr;        // meglio static!
    PlayersStatistics *Players_statistics_ptr = nullptr; // meglio static!

    // Received when Live Sampler buffer is created
    int16_t *LS_buffer_mono_ptr = nullptr;
    int16_t *LS_buffer_L_ptr = nullptr;
    int16_t *LS_buffer_R_ptr = nullptr;

    // Received after ADS moduls
    void Set_ADSR_ptr(AudioADSR* ptr);

    // received before playing
    void Connect_VCF(bool use, int type, float pivot, float resonance, bool modulated);
    void Connect_LFO_TO_VCF(uint8_t modulation, float index, uint8_t periodic, float frequency_time); // used by Midi_Reader
    void Start_VCF(void);
    void Update_VCF_pivot(float new_pivot);
    void Update_LFO_index(float new_index);
    void Update_VCF_resonance(float resonance);

    bool isPlaying(void);
    bool isPoweredOn(void);
    void set_file(int file_id_in);

    void Set_volume(float volume_gain_value);
    void Update_volume(float volume_gain_value);
    void Set_pan(int pan_int_value);
    void Set_effects(float resolution_exp, uint8_t downsampling_in);
    void Set_pitch(float pitch_tune_in);
    void Set_note(float pitch_note_in);
    void Set_pitch_bend(float pitch_bend_in);
    void Write_loop_track(int track);
    void Update_pan(float pan_int_value);
    void Set_vibrato_pointers(float *p_vibrato_array_in, uint8_t *p_vibrato_array_last_element_in);
    void Set_vibrato_flag(bool value);
    void Write_update_time(uint16_t value);
    void Set_mix_samples(uint8_t value);

    /*
    Main_settings
    chiamata da PlayersManager

    Compiti:
    - setta una serie di valori e flag, individuati col suffisso "wait", utilizzati alla successiva partenza/ripartenza del Player, comandata da update()
    */
    void Main_settings(uint8_t mode_in, int A_value_in, int B_value_in, uint16_t delta_Noclick_in, bool RAM_mode_in, int16_t *p_Noclick_in, int16_t *p_Wavetable_in);

    /*
   Get_ready_to_play
   chiamata da PlayersManager

   Compiti:
   - setta una serie di valori e flag, individuati col suffisso "wait", utilizzati alla successiva partenza/ripartenza del Player, comandata da update()
   */
    void Get_ready_to_play(float pitch_note_in, float velocity_in, int patch_in, uint8_t instrument_in, uint8_t sound_id_in, uint8_t note_in); // chiamata da Playermanager per suonare

    /*
    Main_settings_editing
    chiamata da PlayersManager

    Compiti:
    - setta una serie di valori e flag, individuati col suffisso "E", utilizzati al successivo update()
    */
    void Main_settings_editing(uint8_t mode_in, int A_value_in, int B_value_in, uint16_t delta_Noclick_in, bool RAM_mode_in, int16_t *p_Noclick_in, int16_t *p_Wavetable_in);
    void Release_note(void); // release note, fires ADSR "release"
    void Fast_stop(void);

    float Read_pitch(void);
    int Read_loop_track(void);
    int Read_update_time(void);
    bool Read_precedence(void);
    int Read_patch_wait(void);
    int Read_local_patch(void);
    int Read_midi_channel(void);
    int Read_instrument(void);
    int Read_sound_id(void);
    int Read_note(void);
    unsigned long Read_time_stamp(void);
    int Read_use_Wavetable(void);

    void Write_midi_channel(int value);
    void Write_precedence(bool value);
    void Write_time_stamp(unsigned long value);
};