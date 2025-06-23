/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#include <Arduino.h>



// **********************************************************
// **************      VERSIONE FIRMWARE       **************
// **********************************************************
String FIRMWARE_VERSION = "3.1.0_U 23/06/2025";


// **********************************************************
// **************       VERSIONE LILLA         **************
// **********************************************************
/*
    PCB_2022 - hw code: Y22
    - gli encoder 24 e 25 hanno i terminali DT e CLK scambiati rispetto agli altri encoder
    - VCF RESONANCE su encoder 21
    - VCF MOD MODULATION_DEPTH su encoder 22

    PCB_2023 - hw code: Y23
    - tutti gli encoder hanno i terminali DT e CLK corrispondenti al PCB
    - i 4 connettori FFC sono collegati diversamente rispetto al PCB_2022
    - VCF RESONANCE su encoder 22
    - VCF MOD MODULATION_DEPTH su encoder 23

    PCB_2024 LILLA 2024 - hw code: Y24
    - tutti gli encoder hanno i terminali DT e CLK invertiti rispetto al PCB
*/

// #define PCB_2022 0 // case legno
// #define PCB_2023 0 // case alluminio
#define PCB_2024 0 // case alluminio - encoder con CLK e DT invertiti



// **********************************************************
// **************           MEMORY             **************
// **********************************************************

/*
Memory usage:
teensy_size: Memory Usage on Teensy 4.1:
teensy_size:   FLASH: code:342156, data:48492, headers:8708   free for files:7727108
teensy_size:    RAM1: variables:169856, code:273384, padding:21528   free for local variables:59520
teensy_size:    RAM2: variables:34240  free for malloc/new:490048
*/

/*
    Lilla PCB: 2022 / 2023

    Hardware
    - Teensy 4.1 (ARM Cortex-M7; 1MB RAM; 8MB Flash memory; EEPROM: 4284 bytes); clock 600MHz
    - Audio Adaptor Rev.D
    - n.1 W25Q512JVFIM 16-pin SOIC 300-mil (Flash memory 64MB)
    - n.2 APS6404L-3SQR (ESP-PSRAM64H) (PSRAM tot 16MB)
    - Display: ILI9341 240x320 SPI DISPLAY
    - n.7 74HC4067 SOIC (Multiplexer)
    - n.1 modulo Mic Amplifier con AD828A

    RAM1 (veloce): 512KB (16 blocks x 32KB)
    RAM2: 512KB (clock 4 volte più lento)

    Compiler options
    - clock: 600MHz
    - Option "Fastest"

    Notes
    - micro SD card: on Teensy
    - Audio files: 16bit signed PCM MONO, 44.1 kHz

    I/O
    - USB
    - MIDI in
    - MIDI thru
    - LINE in
    - LINE out
    - MONITOR out

     (Direct) Sampler
    - Sampler stores audio files in the 64MB Flash memory chip
    - Sampler expots audio files to the micro SD card

    Live Sampler
    - Live sampler stores audio samples in 2 PSRAM chips

    Midi Loop
    - Midi Looper stores loops in the micro SD card
*/

// *************************************************************
// ****************         LIBRERIE           *****************
// *************************************************************

// Classi standard (ex #include <Audio.h>)
#define AudioNoInterrupts() (NVIC_DISABLE_IRQ(IRQ_SOFTWARE))
#define AudioInterrupts() (NVIC_ENABLE_IRQ(IRQ_SOFTWARE))

#include <control_sgtl5000.h>
#include <filter_biquad.h>
#include <input_i2s.h>
#include <memcpy_audio.h>
#include <mixer.h>
#include <output_i2s.h>
#include <record_queue.h>
#include <spi_interrupt.h>
#include <Wire.h> // comunicazione I2C
#include <SD.h>
#include <SerialFlash.h> // accesso alla Flash memory SPI
#include <utility/dspinst.h>
#include <MIDI.h>

// Classi Display
#include <SPI.h>
#include <Adafruit_GFX.h>     // https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
#include <Adafruit_ILI9341.h> // 1.5.12 version - Hardware-specific library

// Classi Lilla non istanziate
#include "ElementiGrafici.h"
#include "SharedElements.h"
#include "SharedDS.h"
#include "SharedLS.h"
#include "SharedLoop.h"
#include "SharedDelay.h"
#include "SharedVFS.h"
#include "SharedMixer.h"
#include "SharedMM.h"
#include "UserInterface.h"

// Classi Lilla istanziate
#include "AudioPlayer.h"
#include "InfoMaster.h"
#include "NoclickCrossmix.h"
#include "WaveSine.h"
#include "WaveVibrato.h"
#include "WavetableManager.h"
#include "FilterBiquadManager.h"
#include "LillaClock.h"
#include "AudioVCF.h"
#include "WaveLFO.h"
#include "StereoDelay.h"
#include "Router_16x3.h"
#include "Mixer_2x1.h"
#include "AmpliOutMuteIn.h"
#include "AudioPeakDetector.h"
#include "StereoSampler.h"
#include "StereoGain.h"
#include "StereoLiveSampler.h"
#include "AudioFeedback.h"
#include "AudioGain.h"
#include "LoopLedSet.h"
#include "PerformanceLedSet.h"
#include "LoopMetronomo.h"
#include "PlayersManager.h"
#include "PlayersStatistics.h"
#include "EepromManager.h"
#include "PsramManager.h"
#include "DisplayManager.h"
#include "DelayManager.h"

// PWM per uscita MONITOR
#include "output_noiseshaped_pwm.h"

// *************************************************************
// ****************   OGGETTI AUDIOSTREAM      *****************
// *************************************************************

// Attenzione: la funzione update() e' chiamata nell'ordine in cui vengono dichiarati gli oggetti Audiostream
LillaClock Trigger_0; // 1a Lettura midi ed esecuzione comandi
AudioPlayer Player[PLAYERS];
Router_16x3 Router_L;
Router_16x3 Router_R;
LillaClock Trigger_1; // 2a Lettura midi
AudioInputI2S InputDevice;
StereoGain LINE_IN_amplifier;
AudioPeakDetector PeakTracking_L; // Track the signal peak amplitude.
AudioPeakDetector PeakTracking_R; // Track the signal peak amplitude.
AudioFeedback LS_Feedback_L;
AudioFeedback LS_Feedback_R;
StereoLiveSampler LiveSampler;

EepromManager Archive; // NON e' AudioStream ma serve a StereoSampler
StereoSampler DirectSampler(Archive);

AudioGain D_gain_L_feedback;
Mixer_2x1 D_mixer_L_feedback;
StereoDelay Delay_L;

AudioGain D_gain_R_n;
Mixer_2x1 D_mixer_R_in;
StereoDelay Delay_R;

WaveSine Tone_generator;
AudioMixer4 mixer_L;
AudioMixer4 mixer_R;
AudioFilterBiquad biquad_L;
AudioFilterBiquad biquad_R;
AmpliOutMuteIn MAIN_mixer_out_L;
AmpliOutMuteIn MAIN_mixer_out_R;
AmpliOutMuteIn PWM_mixer_out_L;
AmpliOutMuteIn PWM_mixer_out_R;
AudioOutputNoiseShapedPWM PWM_L(3); // PWM_L on pin 3;
AudioOutputNoiseShapedPWM PWM_R(4); // PWM_R on pin 4;
AudioOutputI2S audio_out;

// *************************************************************
// ****************     AUDIOCONNECTION        *****************
// *************************************************************

AudioConnection patchCord1(Player[0], 0, Router_L, 0);
AudioConnection patchCord2(Player[1], 0, Router_L, 1);
AudioConnection patchCord3(Player[2], 0, Router_L, 2);
AudioConnection patchCord4(Player[3], 0, Router_L, 3);
AudioConnection patchCord5(Player[4], 0, Router_L, 4);
AudioConnection patchCord6(Player[5], 0, Router_L, 5);
AudioConnection patchCord7(Player[6], 0, Router_L, 6);
AudioConnection patchCord8(Player[7], 0, Router_L, 7);
AudioConnection patchCord9(Player[8], 0, Router_L, 8);
AudioConnection patchCord10(Player[9], 0, Router_L, 9);
AudioConnection patchCord11(Player[10], 0, Router_L, 10);
AudioConnection patchCord12(Player[11], 0, Router_L, 11);
AudioConnection patchCord13(Player[12], 0, Router_L, 12);
AudioConnection patchCord14(Player[13], 0, Router_L, 13);
AudioConnection patchCord15(Player[14], 0, Router_L, 14);
AudioConnection patchCord16(Player[15], 0, Router_L, 15);

AudioConnection patchCord17(Player[0], 1, Router_R, 0);
AudioConnection patchCord18(Player[1], 1, Router_R, 1);
AudioConnection patchCord19(Player[2], 1, Router_R, 2);
AudioConnection patchCord20(Player[3], 1, Router_R, 3);
AudioConnection patchCord21(Player[4], 1, Router_R, 4);
AudioConnection patchCord22(Player[5], 1, Router_R, 5);
AudioConnection patchCord23(Player[6], 1, Router_R, 6);
AudioConnection patchCord24(Player[7], 1, Router_R, 7);
AudioConnection patchCord25(Player[8], 1, Router_R, 8);
AudioConnection patchCord26(Player[9], 1, Router_R, 9);
AudioConnection patchCord27(Player[10], 1, Router_R, 10);
AudioConnection patchCord28(Player[11], 1, Router_R, 11);
AudioConnection patchCord29(Player[12], 1, Router_R, 12);
AudioConnection patchCord30(Player[13], 1, Router_R, 13);
AudioConnection patchCord31(Player[14], 1, Router_R, 14);
AudioConnection patchCord32(Player[15], 1, Router_R, 15);

AudioConnection patchCord33(Router_L, 0, D_mixer_L_feedback, 0);
AudioConnection patchCord34(D_mixer_L_feedback, 0, Delay_L, 0);
AudioConnection patchCord35(D_mixer_L_feedback, 0, mixer_L, 0);
AudioConnection patchCord36(Delay_L, 0, D_gain_L_feedback, 0);
AudioConnection patchCord37(D_gain_L_feedback, 0, D_mixer_L_feedback, 1);
AudioConnection patchCord38(Router_L, 0, Delay_L, 1); // modulazione del delay dallo stesso segnale

AudioConnection patchCord39(Router_R, 0, D_mixer_R_in, 0);
AudioConnection patchCord40(D_mixer_R_in, 0, Delay_R, 0);
AudioConnection patchCord41(D_mixer_R_in, 0, mixer_R, 0);
AudioConnection patchCord42(Delay_R, 0, D_gain_R_n, 0);
AudioConnection patchCord43(D_gain_R_n, 0, D_mixer_R_in, 1);
AudioConnection patchCord44(Router_R, 0, Delay_R, 1); // modulazione del delay dallo stesso segnale

AudioConnection patchCord45(Router_L, 1, mixer_L, 1);
AudioConnection patchCord46(Tone_generator, 0, mixer_L, 2);
AudioConnection patchCord47(Trigger_0, 0, mixer_L, 3);

AudioConnection patchCord48(Router_R, 1, mixer_R, 1);
AudioConnection patchCord49(Tone_generator, 0, mixer_R, 2);
AudioConnection patchCord50(Trigger_1, 0, mixer_R, 3);

AudioConnection patchCord51(mixer_L, 0, biquad_L, 0);
AudioConnection patchCord52(mixer_R, 0, biquad_R, 0);

AudioConnection patchCord53(biquad_L, 0, MAIN_mixer_out_L, 0);
AudioConnection patchCord54(biquad_R, 0, MAIN_mixer_out_R, 0);

AudioConnection patchCord55(biquad_L, 0, LS_Feedback_L, 1);
AudioConnection patchCord56(biquad_R, 0, LS_Feedback_R, 1);

AudioConnection patchCord57(InputDevice, 0, LINE_IN_amplifier, 0);
AudioConnection patchCord58(InputDevice, 1, LINE_IN_amplifier, 1);

AudioConnection patchCord59(LINE_IN_amplifier, 0, PeakTracking_L, 0);
AudioConnection patchCord60(LINE_IN_amplifier, 1, PeakTracking_R, 0);
AudioConnection patchCord61(LINE_IN_amplifier, 0, MAIN_mixer_out_L, 1);
AudioConnection patchCord62(LINE_IN_amplifier, 1, MAIN_mixer_out_R, 1);

AudioConnection patchCord63(MAIN_mixer_out_L, 0, audio_out, 0);
AudioConnection patchCord64(MAIN_mixer_out_R, 0, audio_out, 1);

AudioConnection patchCord65(Router_L, 2, PWM_mixer_out_L, 0);
AudioConnection patchCord66(Router_R, 2, PWM_mixer_out_R, 0);

AudioConnection patchCord67(LINE_IN_amplifier, 0, PWM_mixer_out_L, 1);
AudioConnection patchCord68(LINE_IN_amplifier, 1, PWM_mixer_out_R, 1);

AudioConnection patchCord69(LINE_IN_amplifier, 0, LS_Feedback_L, 0);
AudioConnection patchCord70(LINE_IN_amplifier, 1, LS_Feedback_R, 0);

AudioConnection patchCord71(LS_Feedback_L, 0, LiveSampler, 0);
AudioConnection patchCord72(LS_Feedback_R, 0, LiveSampler, 1);

AudioConnection patchCord73(LINE_IN_amplifier, 0, DirectSampler, 0);
AudioConnection patchCord74(LINE_IN_amplifier, 1, DirectSampler, 1);

AudioConnection patchCord75(PWM_mixer_out_L, 0, PWM_L, 0);
AudioConnection patchCord76(PWM_mixer_out_R, 0, PWM_R, 0);

AudioControlSGTL5000 Audio_shield;

// *************************************************************
// ***************       NOT AUDIOSTREAM       *****************
// *************************************************************
InfoMaster Info;     // Infos about audio files
WaveVibrato Vibrato; // LFO for midi Vibrato effect
float *Vibrato_array_pointer;
uint8_t *Vibrato_array_last_element;
NoclickCrossmix Noclick[INSTRUMENTS_MAX];    // creates n. INSTRUMENTS_MAX  Noclick objects used to mix "head + tail" snippets taken from Flash memory
WavetableManager Wavetable[INSTRUMENTS_MAX]; // creates n. INSTRUMENTS_MAX Wavetable objects ready to copy sound snippets from Falsh memory to RAM
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

FilterBiquadManager Filter_Biquad_Manager;
AudioVCF VCF[PLAYERS];
WaveLFO LFO_P0[PLAYERS];
WaveLFO LFO_D[2];
PlayersStatistics Players_statistics;
FlashFileRegisterParser File_scanner;
PsramManager PSRAM_Manager;
DisplayManager Display(Info);
LoopLedSet Loop_led_set;
PerformanceLedSet Performance_led_set;
LoopMetronomo LOOP_metronomo(Display);
PlayersManager Players_Manager(&Player[0], &Router_L, &Router_R, &Noclick[0], &Wavetable[0]);
MidiReader Midi_reader(LOOP_metronomo);
DelayManager Delay_manager;


// *************************************************************
// ****************    VARIABLES AND ARRAYS     ****************
// *************************************************************

// Polyphony/Max Pitch
uint8_t optimization_cache;
int first_octave_cache;

// Audio files
uint32_t samples_in_file;

// Downsampling and resolution
int resolution_cache;
int downsampling_cache;
bool resolution_reset;
bool downsampling_reset;

// PERFORMANCE

// Menu
int P_menu;
int P_menu_change;
int P_menu_max;

// Patch
Patch_struct Patch_cache_P; // used to save a Patch BEFORE entering in PERFORMANCE mode
uint8_t patch_old;
bool patch_original;
bool patch_original_0;
uint8_t patches_number; // number of patches_number in use (NOT deleted)

// functions
int8_t Get_Patch_id_free(void);
void Delete_all_Patches_and_Sounds(void);
void Read_all_Patches(void);
void Update_Patches_number(void);
uint8_t Get_first_Patch_id_existing(void);
uint8_t Get_next_Patch_id_existing(void);
uint8_t Get_previous_Patch_id_existing(void);
void Ask_if_change_Patch(void);
void Ask_if_delete_this_Patch(void);
bool Verify_is_Patch_original(const int patch_id);
void Select_performance_menu_elements(void);

// Instrument
uint8_t Instrument_id;
int8_t instrument_on_position[INSTRUMENTS_MAX];
uint8_t last_edited_instrument;

// variables
uint8_t midi_channel_change;
uint8_t from_key_change;
uint8_t to_key_change;
uint8_t patch_change;

// functions
void Update_instruments_leds(void);
void Update_Instruments_positions(void); // posizione di tutti gli Instrument sul display
bool Verify_if_Instrument_original(uint8_t I);
void Macro_Instrument_editing(void);
void Map_one_Instrument_for_all_notes(uint8_t instrument);
void Drop_Instrument(uint8_t instrument);        // drop the instrument from the patch_id ONLY IF instruments > 1
uint8_t Clone_Instrument(uint8_t instrument);    // insert ONE new instrument BELOW instrument
void Update_all_maps_Instrument_for_notes(void); // aggiorna la mappatura tra tutte le coppie midi_channel/note_number e relativi Instrument
void Reset_all_maps_Instrument_for_notes(void);
void Reset_map_Instrument_for_notes(uint8_t instrument);
void Delete_one_map_Instrument_for_notes(uint8_t instrument);

// instrument VCF functions
bool Request_VCF_mode(void);
void Macro_VCF_filter_on_none(void);
void Macro_VCF_modulation_none(void);

// Sound
Sound_struct Sound_cache_P[SOUNDS_MAX]; // used to save all Sound starting a new patch_id
uint8_t Sound_id;
bool sound_original = true;

// functions
bool Verify_is_Sound_original(uint8_t sound_id);
void Copy_all_Sound_to_Sound_cache_P(void);
void Pull_all_Sound_from_Sound_cache_P(void);
uint8_t Get_sounds_free(void);
void Read_all_Sounds(void);
void Save_all_Sounds_changed(void);
int8_t Get_sound_free(void);
bool Request_SOUND_EDIT_mode(void);
void Select_sound_edit_menu_elements(void);
void Macro_Sound_menu(void);
uint32_t Calc_trim_step(uint8_t value);
uint8_t Get_midi_channel_from_Sound(uint8_t sound_id);
void Set_midi_channel_for_Sound(uint8_t sound_id, uint8_t midi_channel);
void Set_Sound_SOLO_OFF(void);

// wavetable functions
void Get_all_Wavetable_pointer(void);
void Fill_all_Wavetable(void);
void Fill_Wavetable(uint8_t instrument);

// noclick functions
uint16_t Calc_Noclick_max(bool use_Wavetable);
void Get_all_Noclick_pointer(void);
void Fill_all_Noclick(void);
void Fill_Noclick(uint8_t instrument);

// SOUND_EDIT
// menu
int So_menu;
int So_menu_change;
int So_menu_max;

// slicing
uint32_t trim_step; // samples per each step while trimming audio file
int slicing_window;
const int MIN_SNIPPET = 100; // minimum dimension (number of samples) of the snippet played

// functions
void Calc_pitch_from_note(void);

// SETTINGS
int8_t SET_menu;

// functions
bool Copy_raw_files_from_SD_to_Flash(void);
float eraseBytesPerSecond(const unsigned char *id);
void Ask_if_IMPORT_EXPORT_setup(void);
void Ask_if_FACTORY_RESET(void);

// CC Control Change
uint8_t CC_Sound_gain_cache[INSTRUMENTS_MAX];
uint8_t CC_lowpass_filter_cache;
int8_t CC_menu;
int CC_number;

// functions
void Save_CC_SETTINGS(void);
void Read_all_CC_Sound_gain(void);

// DELAY
int16_t *DELAY_fifo_L = NULL;
int16_t *DELAY_fifo_R = NULL;
uint8_t delay_instrument_routing; // indica un instrument se <=7; se 8 indica instrument 0 e 1

// DIRECT_SAMPLING
// menu
int DS_menu;
int DS_menu_max;

// variables
const int myInput = AUDIO_INPUT_LINEIN; // AUDIO_INPUT_MIC oppure AUDIO_INPUT_LINEIN;
int DS_export;                          // export mono, export stereo
bool DS_gain_volume;
int DS_state; // 0:waiting  1:pause  2:recording
elapsedMillis DS_recording_time;
elapsedMillis DS_recording_time_update;
int recording_change;

// packets
int DS_packets_free;
int VFS_packets_DS = 0; // Packets dedicated to Direct Sampling
int First_DS_packet = 0;
int Last_DS_packet = 0;

// functions
void DS_setup_DIRECT_SAMPLING_Patch_and_Preset(void); // to be called inside AudioNoInterrupt()
void DS_refresh_DS_page(void);
void DS_ask_if_EXIT_from_DS(void);
void DS_back_to_first_DS_Recording(void);
void DS_convert_file_L(int file_L_RAW, int bytes);
void DS_convert_file_R(int file_R_RAW, int bytes);
void DS_seed_all_Recordings(void);
void DS_update_recordings(void);
void DS_read_all_Recordings(void);
void DS_read_Recording(int value);
float DS_get_Recording_seconds(int value);
int DS_find_Recording_free(void);
int DS_get_next_Recording(int value);
int DS_get_last_Recording(void);
int DS_get_previous_Recording(int value);
bool DS_check_conversion(void);
void DS_define_model(void);
void DS_set_DS_Sampling_Patch(void);
int DS_get_samples_in_Recording(int value);
void P_Recording(int value);

// VFS VIRTUAL FILE SYSTEM
void Make_VFS(void);
int Get_VFS_packets(void);
void Print_VFS_allocation(void);
void Compile_VFS_FAT_table(void);
void Reset_VFS_FAT_table(void);
int Get_first_packet_free(void);
int Get_packets_free(void);
void Erase_all_Packet(void);
void Erase_all_Packet_for_DS(void);
void Erase_Packet(int value);
void Clean_up_VFS(void);
void Defragment_VFS(void);
void Shift_file_VFS(int to_packet, int from_packet, int packets);
void Print_VFS_FAT_table(void);

// STANDARD FILE SYSTEM
int Get_next_raw_file_in_flash(int file);
int Get_previous_raw_file_in_flash(int file);
int Get_samples_in_raw_file(int value);
bool Verify_space_on_flash(int value);
int Get_first_raw_file_available(int start_value);
void Print_flash_file_list(void);
int Get_flash_occupation(void);
int Get_flashchip_size(void);
int Get_flash_size(void);
int Get_raw_files(void);
int Get_raw_files_volume(void);
const char *id2chip(const unsigned char *id);

// LIVE_SAMPLING
// menu
uint8_t choice_LS_menu;
int LS_menu;
int LS_menu_max;

// variables
uint8_t LS_gain;
int LS_id_sound;
int LS_instrument;
int LS_COMB = 64;
int LS_window_step;
const int LS_XY_DELTA_MIN = 4 * AUDIO_BLOCK_SAMPLES; // 5000
int16_t *LS_buffer_mono_ptr = nullptr;
int16_t *LS_buffer_L_ptr = nullptr;
int16_t *LS_buffer_R_ptr = nullptr;
const int LS_REFRESH = 160; // tempo di refresh ms
elapsedMillis LS_wave_refresh_timer;

// functions
void LS_refresh_LS_page(void);
bool LS_ask_if_exit_from_LS(void);
void LS_define_model(void);
int LS_constrain_position(int value);
void LS_lock_X_sample(void);
void LS_update_both_X_Y_samples(void);
void LS_update_Q_sample(void);
void LS_erase_FIFO_array(int16_t *Array, int stereo);
void LS_Setup_buffers(bool stereo, bool first);
void LS_setup_LS_Patch(bool stereo);

// MIDI_LOOP
// variables
uint8_t LOOP_time_order[TRACKS][LOOP_EVENTS] = {0};
elapsedMillis LOOP_clock = 0; // clock fisico
int LOOP_volume_int[TRACKS] = {0};
bool LOOP_run_button_state; // stato pulsante 7 true: run loop abilitati -  false: stop tutti i loop
bool LOOP_track_run_memo[TRACKS] = {false};
int LOOP_stretch_int = 100; // stretch comune ai track, in %
bool LOOP_original;
int LOOP_menu;
int LOOP_menu_change;

// functions
void LOOP_reset_all_data(void);
void LOOP_select_menu_elements(void);
void LOOP_restart_clock(void);
void LOOP_stop_all_midi_tracks(void);
unsigned long LOOP_Clock(void); // clock viruale pari a LOOP_clock/LOOP_stretch
unsigned long LOOP_zero_time(void);
int LOOP_normalized_time(void);
unsigned long LOOP_Clock_time_from_virtual_time(int T_evento);
void LOOP_restart_procedure(int track);
void LOOP_set_time_order(int track);
bool Print_midi_loop_complete_data(int loop_id);
void Compile_midi_loop_file(int loop_id, File &file); // private
void Copy_midi_loop_from_SD_to_RAM_local(File &file);
String Filename_midi_loop(int loop_id);     // private
bool Look_for_midi_loop_in_SD(int loop_id); // notice: does NOT check if SD is present
bool Copy_midi_loop_from_RAM_to_SD(int loop_id);
bool Copy_midi_loop_from_SD_to_RAM(int loop_id);
bool Delete_midi_loop_from_SD(int loop_id);
int Get_first_loop_id_free(void);
int Get_next_loop_id_in_SD(int loop_id);
int Get_previous_loop_id_in_SD(int loop_id);
void LOOP_stop_and_reset_runnig_loop_data(void);

// MIXER
// PWM Monitor
int volume_MONITOR = 0;
void Golive_MIXER(int instrument = -1);

// EEPROM
void Factory_setup_Eeprom(void);

// General purpose
bool changed;
bool confirmation;
int action;
int row;
int col;
int result;
uint32_t big_result;

// Switch
void Switch_to_PERFORMANCE_patch_old(void);
void Jump_to_Patch(uint8_t next_patch);
void Rebuild_patch_old(void);
void Golive_with_PERFORMANCE(int patch_id);
void Switch_from_MIDI_LOOP_to_PERFORMANCE(void);       // si fermano i track
void Switch_from_LIVE_SAMPLING_to_PERFORMANCE(void);   // si ripristina patch_old
void Switch_from_DIRECT_SAMPLING_to_PERFORMANCE(void); // si ripristina patch_old
void Switch_to_MIXER(void);                            // si sta sulla patch_id attuale, ci si posiziona sul primo instrument esistente
void Switch_from_LIVE_SAMPLING_to_DELAY(void);
void Golive_with_LIVE_SAMPLING(void);                    // passaggio e visualizzazione
void Switch_from_PERFORMANCE_to_LIVE_SAMPLING(void);     // si memorizza la patch_id attuale e si passa alla LS_patch
void Switch_from_MIDI_LOOP_to_LIVE_SAMPLING(void);       // si fermano i track e si passa alla LS_patch
void Switch_from_DIRECT_SAMPLING_to_LIVE_SAMPLING(void); // si passa alla LS_patch
void Golive_DIRECT_SAMPLING(void);
void Jump_to_DIRECT_SAMPLING_recording(int &recording);
void Switch_to_DIRECT_SAMPLING(void);                    // si passa alla DS_patch
void Switch_from_MIDI_LOOP_to_DIRECT_SAMPLING(void);     // si fermano i track e si passa alla DS_patch
void Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING(void); // si ferma la registrazione e si passa alla DS_patch
void Golive_MIDI_MONITOR(void);
void Switch_from_MIDI_LOOP_to_MIDI_MONITOR(void);    // si fermano i track
void Golive_with_MIDI_LOOP(bool restart = false);    // passaggio e visualizzazione; restart == false non interrompe i track running
void Switch_from_PERFORMANCE_to_MIDI_LOOP(void);     // si conserva la Patch attuale
void Switch_from_DIRECT_SAMPLING_to_MIDI_LOOP(void); // si ripristina patch_old
void Switch_from_LIVE_SAMPLING_to_MIDI_LOOP(void);   // si ripristina patch_old
void Golive_SETUP(void);
void Switch_from_MIDI_LOOP_to_SETUP(void); // si fermano i track

// Print
void P_Patch(uint8_t patch_id);
void P_Instrument(uint8_t patch_id, uint8_t instrument_id);
void P_Patch_cache_P(void);
void P_Sound(uint8_t sound_id);
void P_Lilla_state(void);
void P_keyboard_state(uint8_t mc, int8_t a, int8_t b);
void P_map_instrument_for_note(uint8_t midi_channel);
template <class T>
void P(String &what, T &value);
void Print_Directory(File dir, int numSpaces);

// Display
// functions
int x_pos(float col);

// Test
bool test_devices = false;
bool test_mux = false;
int state_mux = 0;
bool mux_en;

// Protection
bool exibition = false;

// Startup
void Compile_tables(void);
void Bootstrap_setup(void);

// *************************************************************
// *************************************************************
// ********************      SETUP     *************************
// *************************************************************
// *************************************************************

void setup()
{
    AudioNoInterrupts();

    /*
      AudioMemory allocates memory for all audio connections. The numberBlocks input specifies how much memory to reserve for audio data.
      Each block holds 128 audio samples, or approx 2.9 ms of sound. Usually an initial guess is made for numberBlocks and the actual
      usage is checked with AudioMemoryUsageMax().
    */
    AudioMemory(80);
    Serial.begin(115200);

    // Compila le tavole di costanti
    Compile_tables();

    for (auto i = 0; i < 10; ++i)
    {
        m_exp_table[i] = exp_table[i + 1] - exp_table[i];
        m_sin_table[i] = sin_table[i + 1] - sin_table[i];
        m_decay_table[i] = decay_table[i + 1] - decay_table[i];
        m_release_table[i] = release_table[i + 1] - release_table[i];
    }

    const float value_float = 16.0;
    for (auto i = 0; i <= 32; ++i)
    {
        pan_gain_L_table[i] = sin((value_float - (i - 16.0)) * 0.049087f); // Left channel , 0.049087 = M_PI/64.0
        pan_gain_R_table[i] = sin((value_float + (i - 16.0)) * 0.049087f); // Right channel , 0.049087 = M_PI/64.0
    }

    // Audio Adaptor inizialization
    Audio_shield.enable();
    Audio_shield.volume(1.0);
    Audio_shield.inputSelect(myInput);
    Audio_shield.audioPostProcessorEnable();
    Audio_shield.eqSelect(0); // 0=NONE, 1=PEQ (7 IIR Biquad filters), 2=TONE (tone), 3=GEQ (5 band EQ)

    // Start SPI communication with W25Q512 Flash memory chip
    SerialFlash.begin();
    delay(100);

   // User interfaces
   Setup_Mux_Pins();
   Setup_encoders();
   Setup_pushbuttons();


    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //              Inizializzazione degli oggetti
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    Get_all_Wavetable_pointer();
    Get_all_Noclick_pointer();

    // Get Vibrato pointers
    Vibrato_array_pointer = Vibrato.Get_vibrato_array_pointer();
    Vibrato_array_last_element = Vibrato.Get_vibrato_array_last_element();
    Vibrato.Make_vibrato_table();

    // Setup Player and LFO
    for (auto player = 0; player < PLAYERS; ++player)
    {
        Player[player].Set_identity(player);
        Player[player].Set_vibrato_pointers(Vibrato_array_pointer, Vibrato_array_last_element); // void Set_vibrato_pointers(float *p_vibrato_array_in, uint8_t *p_vibrato_array_last_element_in)
        Player[player].VCF_ptr = &VCF[player];
        Player[player].LFO_ptr = &LFO_P0[player];
        Player[player].LiveSampler_ptr = &LiveSampler;
        Player[player].Players_statistics_ptr = &Players_statistics;
        LFO_P0[player].identity = player;
    }

    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        CC_Sound_gain_cache[instrument_local] = -1;
    }

    // Setup Midi_reader
    Midi_reader.Vibrato_ptr = &Vibrato;
    Midi_reader.Tone_generator_ptr = &Tone_generator;
    Midi_reader.Players_Manager_ptr = &Players_Manager;

    // Setup Execute_Commands
    Filter_Biquad_Manager.biquad_L_ptr = &biquad_L;
    Filter_Biquad_Manager.biquad_R_ptr = &biquad_R;

    // Setup Trigger
    Trigger_0.identity = 0;
    Trigger_1.identity = 1;
    Trigger_0.Midi_reader_ptr = &Midi_reader;
    Trigger_1.Midi_reader_ptr = &Midi_reader;
    Trigger_0.Filter_Biquad_Manager_ptr = &Filter_Biquad_Manager;
    Trigger_1.Filter_Biquad_Manager_ptr = &Filter_Biquad_Manager;
    Trigger_0.Delay_Manager_ptr = &Delay_manager;

    // Setup Delays
    Delay_L.LFO_ptr = &LFO_D[0];
    Delay_R.LFO_ptr = &LFO_D[1];
    LFO_D[0].identity = 88;
    LFO_D[0].identity = 99;

    // Setup Infotest
    Info.LiveSampler_ptr = &LiveSampler;

    // Setup Wavetable-s
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        Wavetable[instrument_local].LiveSampler_ptr = &LiveSampler;
    }

    // Setup Live Sampling Feedback mixers
    LS_Feedback_L.identity = 0;
    LS_Feedback_R.identity = 1;
    LS_Feedback_L.value(0); // nessun feedback
    LS_Feedback_R.value(0); // nessun feedback

    // Setup Display
    Display.Start();

    // DelayManager
    Delay_manager.Delay_L_ptr = &Delay_L;
    Delay_manager.Delay_R_ptr = &Delay_R;
    Delay_manager.LFO_D_ptr[0] = &LFO_D[0];
    Delay_manager.LFO_D_ptr[1] = &LFO_D[1];
    Delay_manager.D_gain_L_feedback_ptr = &D_gain_L_feedback;
    Delay_manager.D_gain_R_feedback_ptr = &D_gain_R_n;
    Delay_manager.Players_Manager_ptr = &Players_Manager;

    // Setup PlayersStatistics
    Players_statistics.Loop_led_set_ptr = &Loop_led_set;
    Players_statistics.Performance_led_set_ptr = &Performance_led_set;


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // ************     MUX, ENCODERS, PUSHBUTTONS      ***********
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   ***************    FUNZIONI SPECIALI   *****************
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Prima partenza LILLA
    if (Read_pushbutton(7))
    {
        // Attenzione richiede 2/3 minuti per la cancellazione dei Packet!
        // Se la procedura si interrompe la EEPROM resta azzarata e Patch[0] o Sound[0] NON saranno configurati correttamente!!

        Display.Factory_reset_wait_popup();
        Factory_setup_Eeprom();
    }

    // Test encoder e pulsanti
    // indica quale encoder e' stato ruotato (+/-1) o pulsante e' stato premuto
    if (Read_pushbutton(0))
    {
        test_devices = true;
    }

    // Test mux
    // indica quale porta di quale Mux e' LOW (sono normalmente tutte HIGH)
    if (Read_pushbutton(1))
    {
        test_mux = true;
    }

    // Protected mode
    // impedisce il salvataggio di nuove Patch
    if (Read_pushbutton(2))
    {
        exibition = true;
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //                          PARTENZA
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    Bootstrap_setup();
    File_scanner.Read_all_file_data(); // FlashFileRegisterParser::Read_all_file_data();

    AudioInterrupts();

    // Loop infinito - test encoders, pushbuttons
    while (test_devices)
    {
        for (auto i = 0; i < ENCODERS; ++i)
        {
            result = Read_encoder_simple(i);
            if (result == 1)
            {
                Serial.print("Encoder ");
                Serial.print(i);
                Serial.println(" +1");
            }
            else if (result == -1)
            {
                Serial.print("Encoder ");
                Serial.print(i);
                Serial.println(" -1");
            }
        }

        for (auto i = 0; i < PUSHBUTTONS; ++i)
        {
            if (Read_pushbutton(i))
            {
                Serial.print("Pushbutton ");
                Serial.println(i);
            }
        }
    }

    // Loop infinito - test mux
    while (test_mux)
    {
        mux_en = false;
        for (auto mux = 0; mux < 7; ++mux)
        {
            for (auto i = 0; i < 16; ++i)
            {
                Write_MUX_address(i);
                delayMicroseconds(PAUSE_MUX);
                if (digitalRead(MUX_pin[mux]) == LOW)
                {
                    mux_en = true;
                    state_mux = 0;
                    Serial.print("MUX ");
                    Serial.print(mux + 1); // on PCB MUX sono numerati da 1 a 7
                    Serial.print("  channel ");
                    Serial.print(i);
                    Serial.println("  is LOW");
                    delay(1);
                }
            }
            if (!mux_en && state_mux == 0)
            {
                Serial.println("tutti i mux liberi");
                state_mux = 1;
            }
        }
    }
}

// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//                             LOOP
// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void loop()
{
#pragma region Area_Comune [rgba(118,110,2,0.1)]

    // Update del/i led presenti (varia in base a LILLA_STATE)
    Update_instruments_leds();

    // PB_number is used to select one from the 8 pushbuttons
    PB_number++;
    if (PB_number == 8)
    {
        PB_number = 0;
    }

    // PANIC
    if (Read_pushbutton(15))
    {
        Delay_data.loop_gain = 0;
        Delay_values.loop_gain = Delay_feedback(Delay_data.loop_gain);

        AudioNoInterrupts();
        D_gain_L_feedback.Set_gain(Delay_values.loop_gain);
        D_gain_R_n.Set_gain(Delay_values.loop_gain);
        Players_Manager.Stop_all_players();
        if (Lilla_state == MIDI_LOOP)
        {
            for (auto local_track = 0; local_track < TRACKS; ++local_track)
            {
                LOOP_track_run[local_track] = false;
            }
        }
        AudioInterrupts();

        if (Lilla_state == MIDI_LOOP)
        {
            Loop_led_set.Request_all_LED_switch_off();
        }

        if (Lilla_state == DELAY_SETTINGS)
        {
            Display.D_read_gain();
        }
    }

    // Resolution
    if (Read_encoder(0, resolution, 68, 1, 1))
    {
        resolution_reset = false;
        AudioNoInterrupts();
        Players_Manager.Multicast_effects(Calc_resolution(resolution), downsampling);
        AudioInterrupts();

        if (Lilla_state == PERFORMANCE || Lilla_state == SOUND_EDIT || Lilla_state == INSTRUMENT_VCF || Lilla_state == DELAY_SETTINGS || Lilla_state == MIDI_LOOP)
        {
            Display.Resolution();
        }
    }

    // Downsampling
    if (Read_encoder_inverse(8, downsampling, 128, 1, 1))
    {
        downsampling_reset = false;
        AudioNoInterrupts();
        Players_Manager.Multicast_effects(Calc_resolution(resolution), downsampling);
        AudioInterrupts();

        if (Lilla_state == PERFORMANCE || Lilla_state == SOUND_EDIT || Lilla_state == INSTRUMENT_VCF || Lilla_state == DELAY_SETTINGS || Lilla_state == MIDI_LOOP)
        {
            Display.Downsampling();
        }
    }

    // Resolution (on/off) pushbutton
    if (Read_pushbutton(0))
    {
        if (!resolution_reset)
        {
            resolution_cache = resolution;
            resolution = 68;
            AudioNoInterrupts();
            Players_Manager.Broadcast_reset_effect(Calc_resolution(resolution), downsampling, 0);
            AudioInterrupts();
        }
        else
        {
            resolution = resolution_cache;
            AudioNoInterrupts();
            Players_Manager.Multicast_effects(Calc_resolution(resolution), downsampling);
            AudioInterrupts();
        }

        if (Lilla_state == PERFORMANCE || Lilla_state == SOUND_EDIT || Lilla_state == INSTRUMENT_VCF || Lilla_state == DELAY_SETTINGS || Lilla_state == MIDI_LOOP)
        {
            Display.Resolution();
        }

        resolution_reset = !resolution_reset;
    }

    // Downsampling (on/off) pushbutton
    if (Read_pushbutton(8))
    {
        if (!downsampling_reset)
        {
            downsampling_cache = downsampling;
            downsampling = 1;
            AudioNoInterrupts();
            Players_Manager.Broadcast_reset_effect(Calc_resolution(resolution), downsampling, 1);
            AudioInterrupts();
        }
        else
        {
            downsampling = downsampling_cache;
            AudioNoInterrupts();
            Players_Manager.Multicast_effects(Calc_resolution(resolution), downsampling);
            AudioInterrupts();
        }
        if (Lilla_state == PERFORMANCE || Lilla_state == SOUND_EDIT || Lilla_state == INSTRUMENT_VCF || Lilla_state == DELAY_SETTINGS || Lilla_state == MIDI_LOOP)
        {
            Display.Downsampling();
        }

        downsampling_reset = !downsampling_reset;
    }

    if (Lilla_state != MIDI_LOOP)
    {
        // Pushbutton tuning tone
        if (Read_pushbutton(7)) // switch ON/OFF the Tuning Tone
        {
            tuning_tone_flag = !tuning_tone_flag;
            if (Lilla_state == PERFORMANCE)
            {
                Display.Instrument_TT(Patch_id);
            }
            if (!tuning_tone_flag)
            {
                Tone_generator.Stop();
            }
        }

        // Tuning tone volume
        if (tuning_tone_flag && Read_encoder(7, tuning_tone_volume, 40, 0, 1))
        {
            if (Lilla_state == PERFORMANCE)
            {
                Display.Volume_TT(Patch_id);
            }
        }
    }

    // Low-pass cutoff frequency
    if (Read_encoder(16, lowpass_target, LPF_MAX, 0, 1))
    {
        lowpass_flag = true;
        lowpass_direction = lowpass_target > lowpass;
        Display.Lowpass_filter();
    }

    // Low-pass cutoff from midi CC
    if (display_lowpass_flag)
    {
        Display.Lowpass_filter();
        display_lowpass_flag = false;
    }

    // Pushbutton Low-pass flat
    if (Read_pushbutton(16))
    {
        Archive.Print_EEPROM_content();
        if (lowpass_target < LPF_MAX)
        {
            lowpass_flag = true;
            lowpass_target = LPF_MAX;
            lowpass_direction = true;
            Display.Lowpass_filter();
        }
    }

    // Keyboard management
    /*
    if(Read_pushbutton(1))
    {
        if(keyboard_1_octave_keys > 12)
            keyboard_1_octave_keys = 12;
        else
            keyboard_1_octave_keys = keyboard_1_octave_max_keys;
        Calc_pitch_from_note();
    }
    */

    // Print AudioProcessorUsage
    if (Read_pushbutton(23))
    {
        Serial.print("AudioProcessorUsage(): ");
        Serial.println(AudioProcessorUsage());
        for (auto player = 0; player < PLAYERS; ++player)
        {
            Serial.print("Player[");
            Serial.print(player);
            Serial.print("].processorUsage(): ");
            Serial.println(Player[player].processorUsage());
        }
    }
#pragma endregion // parte comune

#pragma region Performance [rgba(2, 68, 118, 0.1)]
    // *************************************************************
    // ********************    PERFORMANCE  ************************
    // *************************************************************

    if (Lilla_state == PERFORMANCE)
    {

        // Change volume_patch
        if (Read_encoder(15, volume_patch, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
            Players_Manager.Broadcast_volume();
            AudioInterrupts();
            Display.Patch_volume_value(true);
        }

        // Edit Instrument
        if (instrument_editing_flag)
        {
            // Change midi_channel
            result = Read_encoder_simple(2);
            if (result != 0)
            {
                midi_channel_change = Get_midi_channel_from_Sound(Sound_id);
                // Serial.print("attuale canale: ");
                // Serial.println(midi_channel_change);
                if (result == 1)
                {
                    if (midi_channel_change < 15)
                    {
                        midi_channel_change++;
                    }
                }
                else // -1
                {
                    if (midi_channel_change > 0)
                    {
                        midi_channel_change--;
                    }
                }
                if (midi_channel_change != Get_midi_channel_from_Sound(Sound_id))
                {
                    AudioNoInterrupts();
                    Players_Manager.Multicast_release_players(Sound_id);
                    Reset_map_Instrument_for_notes(Instrument_id);
                    Set_midi_channel_for_Sound(Sound_id, midi_channel_change);
                    Update_map_Instrument_for_notes(Patch[Patch_id].Instrument[Instrument_id].from_note, Patch[Patch_id].Instrument[Instrument_id].to_note, Instrument_id);
                    Players_Manager.Update_Preset_midi_channel(Patch_id, Instrument_id);
                    AudioInterrupts();

                    Macro_Instrument_editing();
                }
            }

            // Change Instrument PAN
            if (Read_encoder(3, Sound[Sound_id].pan, 16, -16, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_pan(Patch_id, Instrument_id);
                Players_Manager.Multicast_pan(Instrument_id);
                AudioInterrupts();

                Macro_Instrument_editing();
            }

            // Set PAN to center
            if (Read_pushbutton(3))
            {
                Sound[Sound_id].pan = 0;

                AudioNoInterrupts();
                Players_Manager.Update_Preset_pan(Patch_id, Instrument_id);
                Players_Manager.Multicast_pan(Instrument_id);
                AudioInterrupts();

                Macro_Instrument_editing();
            }

            // Change Instrument GAIN
            if (Read_encoder(4, Sound[Sound_id].gain, 40, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_volume(Patch_id, Instrument_id, Volume_float[volume_patch]);
                Players_Manager.Multicast_volume_for_instrument_edit(Instrument_id);
                AudioInterrupts();

                Macro_Instrument_editing();
            }

            // Change root_key
            if (Read_encoder(9, Patch[Patch_id].Instrument[Instrument_id].root_key, 127, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Multicast_change_players_notes(Patch_id, Instrument_id);
                AudioInterrupts();

                Macro_Instrument_editing();
            }

            // Change from_key
            result = Read_encoder_simple(10);
            if (result != 0)
            {
                changed = false;
                if (result == 1 && Patch[Patch_id].Instrument[Instrument_id].from_note < Patch[Patch_id].Instrument[Instrument_id].to_note)
                {
                    from_key_change = Patch[Patch_id].Instrument[Instrument_id].from_note + 1;
                    changed = true;
                }
                else if (result == -1 && Patch[Patch_id].Instrument[Instrument_id].from_note > 0)
                {
                    from_key_change = Patch[Patch_id].Instrument[Instrument_id].from_note - 1;
                    changed = true;
                }
                if (changed)
                {
                    AudioNoInterrupts();
                    Players_Manager.Change_from_key(Patch_id, Instrument_id, from_key_change);
                    AudioInterrupts();

                    Macro_Instrument_editing();
                }
            }

            // Lock: enable/disable pitch_bend and effects
            if (Read_pushbutton(10))
            {
                AudioNoInterrupts();
                Patch[Patch_id].Instrument[Instrument_id].lock = !Patch[Patch_id].Instrument[Instrument_id].lock;
                Players_Manager.Update_Preset_lock(Patch_id, Instrument_id);
                if (Patch[Patch_id].Instrument[Instrument_id].lock)
                {
                    Players_Manager.Multicast_reset_pitch_bend_effects(Instrument_id);
                }
                else // lock off, enable effect
                {
                    Players_Manager.Broadcast_restore_pitch_bend_and_effects(Instrument_id, pitch_bend_value[Get_midi_channel(Patch_id, Instrument_id)]);
                }
                AudioInterrupts();

                Macro_Instrument_editing();
            }

            // Change to_key
            result = Read_encoder_simple(11);
            if (result != 0)
            {
                changed = false;
                if (result == 1 && Patch[Patch_id].Instrument[Instrument_id].to_note < 127)
                {
                    to_key_change = Patch[Patch_id].Instrument[Instrument_id].to_note + 1;
                    changed = true;
                }

                else if (result == -1 && Patch[Patch_id].Instrument[Instrument_id].to_note > Patch[Patch_id].Instrument[Instrument_id].from_note)
                {
                    to_key_change = Patch[Patch_id].Instrument[Instrument_id].to_note - 1;
                    changed = true;
                }

                if (changed)
                {
                    AudioNoInterrupts();
                    Players_Manager.Change_to_key(Patch_id, Instrument_id, to_key_change);
                    AudioInterrupts();

                    Macro_Instrument_editing();
                }
            }

            // Change Precedence (precedence)
            if (Read_pushbutton(11))
            {
                AudioNoInterrupts();
                Patch[Patch_id].Instrument[Instrument_id].precedence = !Patch[Patch_id].Instrument[Instrument_id].precedence;
                Players_Manager.Update_Preset_precedence(Patch_id, Instrument_id);
                AudioInterrupts();

                Macro_Instrument_editing();
            }
        }

        // Change Patch (Patch)
        result = Read_encoder_simple(23);
        if (result != 0)
        {
            if (result == +1)
            {
                patch_change = Get_next_Patch_id_existing();
            }
            else
            {
                patch_change = Get_previous_Patch_id_existing();
            }

            if (patch_change != Patch_id)
            {
                if (!Verify_is_Patch_original(Patch_id))
                {
                    Ask_if_change_Patch();
                    if (action == 0) // Exit: remain in this patch_id
                    {
                        Serial.println("Remain on this Patch");
                        Golive_with_PERFORMANCE(Patch_id);
                    }
                    else
                    {
                        if (action == 1) // No: discharge changings and switch patch_id
                        {
                            Patch[Patch_id] = Patch_cache_P;
                            Pull_all_Sound_from_Sound_cache_P();
                        }
                        else if (action == 2) // Yes: save changings and switch patch_id
                        {
                            Save_all_Sounds_changed();
                            Archive.Save_Patch(Patch_id, Patch[Patch_id]);
                            Read_all_Sounds();
                        }

                        Jump_to_Patch(patch_change);

                        // Patch Delay: look for delay_<patch_id> in SD
                        if (Archive.Copy_patch_Delay_data_from_SD_to_Eeprom(Patch_id))
                        {
                            Serial.println(F("Smooth changing of delay values COULD start..."));

                            Delay_data_struct delay_final;
                            Archive.Copy_patch_Delay_data_from_Eeprom_to_Ram(delay_final);

                            AudioNoInterrupts();
                            Delay_manager.New_values(&delay_final); // call using AudioNoInterrupt()
                            AudioInterrupts();
                        }
                        patch_old = Patch_id;
                    }
                }

                else
                {
                    Jump_to_Patch(patch_change);

                    // Patch Delay: look for delay_<patch_id> in SD
                    if (Archive.Copy_patch_Delay_data_from_SD_to_Eeprom(Patch_id))
                    {
                        Serial.println(F("Smooth changing of delay values COULD start..."));

                        Delay_data_struct delay_final;
                        Archive.Copy_patch_Delay_data_from_Eeprom_to_Ram(delay_final);

                        AudioNoInterrupts();
                        Delay_manager.New_values(&delay_final); // call using AudioNoInterrupt()
                        AudioInterrupts();
                    }
                    patch_old = Patch_id;
                }
            }
        }

        // Instrument volume changed from MIDI CC
        if (display_instrument_volume_flag)
        {
            Display.Show_Instrument_description(Patch_id, instrument_volume_changed, false);

            // restore LED
            if (Performance_led_set.Read_LED_activity(Instrument_id) > 0)
            {
                Display.Led_PERFORMANCE_instrument(Instrument_id, true);
            }
            else
            {
                Display.Led_PERFORMANCE_instrument(Instrument_id, false);
            }
            display_instrument_volume_flag = false;
        }

        // change MENU item
        result = Read_encoder_simple(25);
        if (result != 0)
        {
#ifdef PCB_2022
            if (result == -1)
#else
            if (result == +1)
#endif
            {
                if (P_menu < (P_menu_max + Patch[Patch_id].instruments))
                    P_menu_change = P_menu + 1;
            }
            else
            {
                if (P_menu > 0)
                    P_menu_change = P_menu - 1;
            }

            if (P_menu_change != P_menu)
            {
                P_menu = P_menu_change;
                if (P_menu <= P_menu_max)
                {
                    if (instrument_editing_flag)
                    {
                        Display.Patch_volume_color(false, true);
                        Display.Instrument_frame_on_position(0, false);

                        Display.Show_Instrument_description(Patch_id, instrument_on_position[0], false);

                        // restore LED
                        Performance_led_set.Restore_all_LED();

                        instrument_editing_flag = false;
                        Display.Frame_performance_menu(P_menu, false);
                    }
                    else
                        Display.Frame_performance_menu(P_menu, false);
                }
                else
                {
                    if (!instrument_editing_flag)
                    {
                        Display.Patch_volume_color(false, false);
                        Display.Delete_all_frame_performance_menu();
                    }
                    Instrument_id = instrument_on_position[P_menu - P_menu_max - 1];
                    Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;

                    if (instrument_editing_flag)
                    {
                        Display.Instrument_frame_on_position(position_of_Instrument[last_edited_instrument], false);

                        Display.Show_Instrument_description(Patch_id, last_edited_instrument, false);

                        // restore LED
                        Performance_led_set.Restore_all_LED();
                    }

                    Display.Show_Instrument_description(Patch_id, Instrument_id, true);

                    // restore LED
                    Performance_led_set.Restore_all_LED();

                    Display.Instrument_frame_on_position(position_of_Instrument[Instrument_id], true);
                    instrument_editing_flag = true;
                    last_edited_instrument = Instrument_id;

                    Serial.print("Editing instrument: ");
                    Serial.println(Instrument_id);
                }
            }
        }

        // choose MENU item
        if (Read_pushbutton(25))
        {
            if (P_menu <= P_menu_max)
            {
                int8_t new_patch;

                switch (choice_performance_menu)
                {
                case 0: // EXIT (drop Sound changes)
                    AudioNoInterrupts();
                    Patch[Patch_id] = Patch_cache_P;
                    Pull_all_Sound_from_Sound_cache_P();
                    Update_all_maps_Instrument_for_notes();
                    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]); // Update_all_Preset(int patch_id, float volume_patch)
                    Fill_all_Noclick();
                    Fill_all_Wavetable();
                    AudioInterrupts();

                    P_Patch(Patch_id);
                    patch_original = true;
                    P_menu = 0;
                    Update_Instruments_positions();
                    Select_performance_menu_elements();
                    Display.Performance_menu(); // displays the menu and updates "Value_Max_encoder.performance_menu" used by encoder_menu
                    Display.Frame_performance_menu(P_menu, true);
                    Display.All_Instrument(Patch_id);
                    break;

                case 1: // SAVE changes in THIS Patch
                    Save_all_Sounds_changed();
                    Archive.Save_Patch(Patch_id, Patch[Patch_id]);
                    Read_all_Sounds();

                    Patch_cache_P = Patch[Patch_id];
                    Copy_all_Sound_to_Sound_cache_P();
                    Golive_with_PERFORMANCE(Patch_id);
                    break;

                case 2: // CLONE (same SAVE changes in a NEW Patch)

                    AudioNoInterrupts();
                    Players_statistics.Reset_total_Players_per_instrument();

                    // 1: copy all Instrument in the new patch_id, with same id_sounds
                    new_patch = Get_Patch_id_free();
                    if (new_patch >= 0)
                    {
                        Sound_struct Sound_NEW[8];
                        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                        {
                            if (Patch[Patch_id].Instrument[instrument_local].used)
                                Sound_NEW[instrument_local] = Sound[Patch[Patch_id].Instrument[instrument_local].sound_id];
                        }
                        Pull_all_Sound_from_Sound_cache_P(); // 3: restore all original Sound
                        Patch[new_patch] = Patch[Patch_id];
                        Patch[Patch_id] = Patch_cache_P; // 4: ora patch_id è ripristinata, anche i relativi Sound sono stati ripristinati

                        // 5: Per ciascun Instrument utilizzato dalla Patch_NEW creo un nuovo Sound
                        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                        {
                            if (Patch[new_patch].Instrument[instrument_local].used)
                            {
                                int S = Get_sound_free();
                                if (S >= 0)
                                {
                                    Sound[S] = Sound_NEW[instrument_local];
                                    Patch[new_patch].Instrument[instrument_local].sound_id = S;
                                }
                                else
                                    Patch[new_patch].Instrument[instrument_local].used = false;
                            }
                        }

                        Save_all_Sounds_changed();
                        Patch_id = new_patch;
                        Archive.Save_Patch(Patch_id, Patch[Patch_id]);

                        // Save Delay_data in delay_<patch_id>.txt in SD
                        Archive.Copy_patch_Delay_data_from_RAM_to_SD(Patch_id);

                        Read_all_Patches();
                        Read_all_Sounds();
                        Update_Patches_number();
                        Update_all_maps_Instrument_for_notes();
                        Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
                        Fill_all_Noclick();
                        Fill_all_Wavetable();
                    }
                    AudioInterrupts();

                    Patch_cache_P = Patch[Patch_id];
                    Copy_all_Sound_to_Sound_cache_P();
                    P_Patch(Patch_id);
                    Update_Instruments_positions();
                    Golive_with_PERFORMANCE(Patch_id);
                    break;

                case 3: // SAVE changes in a NEW Patch

                    AudioNoInterrupts();
                    Players_statistics.Reset_total_Players_per_instrument();
                    // 0: hunt some "instruments" id_sounds free

                    // 1: copy all Instrument in the new patch_id, with same id_sounds
                    new_patch = Get_Patch_id_free();
                    if (new_patch >= 0)
                    {
                        Sound_struct Sound_NEW[INSTRUMENTS_MAX];
                        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                        {
                            if (Patch[Patch_id].Instrument[instrument_local].used)
                            {
                                Sound_NEW[instrument_local] = Sound[Patch[Patch_id].Instrument[instrument_local].sound_id];
                            }
                        }
                        Pull_all_Sound_from_Sound_cache_P(); // 3: restore all original Sound
                        Patch[new_patch] = Patch[Patch_id];
                        Patch[Patch_id] = Patch_cache_P; // 4: ora patch_id è ripristinata, anche i relativi Sound sono stati ripristinati

                        // 5: Create a new Sound for each Instrument in new_patch
                        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                        {
                            if (Patch[new_patch].Instrument[instrument_local].used)
                            {
                                int id_sound_local = Get_sound_free();
                                if (id_sound_local >= 0)
                                {
                                    Sound[id_sound_local] = Sound_NEW[instrument_local];
                                    Patch[new_patch].Instrument[instrument_local].sound_id = id_sound_local;
                                }
                                else
                                    Patch[new_patch].Instrument[instrument_local].used = false;
                            }
                        }

                        Save_all_Sounds_changed();
                        Patch_id = new_patch;
                        Archive.Save_Patch(Patch_id, Patch[Patch_id]);

                        // Save Delay_data in delay_<patch_id>.txt in SD
                        Archive.Copy_patch_Delay_data_from_RAM_to_SD(Patch_id);

                        Read_all_Patches();
                        Read_all_Sounds();
                        Update_Patches_number();
                        Update_all_maps_Instrument_for_notes();
                        Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
                        Fill_all_Noclick();
                        Fill_all_Wavetable();
                    }
                    AudioInterrupts();

                    Patch_cache_P = Patch[Patch_id];
                    Copy_all_Sound_to_Sound_cache_P();
                    P_Patch(Patch_id);
                    Update_Instruments_positions();

                    Golive_with_PERFORMANCE(Patch_id);
                    break;

                case 4: // DROP Patch and go back to PERFORMANCE "Get_first_Patch_id_existing()"

                    // ask for confirmation
                    Ask_if_delete_this_Patch();
                    if (action == 1)
                    {
                        Lilla_state = PERFORMANCE;

                        AudioNoInterrupts();
                        Players_Manager.Release_all_players();
                        AudioInterrupts();

                        Patch[Patch_id].used = false;
                        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
                        {
                            if (Patch[Patch_id].Instrument[instrument_local].used)
                            {
                                Sound[Patch[Patch_id].Instrument[instrument_local].sound_id].used = false;
                            }
                        }

                        Save_all_Sounds_changed();
                        Archive.Save_Patch(Patch_id, Patch[Patch_id]);

                        // Delete delay_<patch_id>.txt file
                        Archive.Delete_Delay_data_in_SD(Patch_id);

                        Read_all_Patches();
                        Update_Patches_number();
                        Read_all_Sounds();

                        Jump_to_Patch(Get_first_Patch_id_existing());
                    }
                    else
                    {
                        P_menu = 0;
                        Select_performance_menu_elements();
                        Display.Performance_page(false, true);
                        Display.Frame_performance_menu(P_menu, true);
                    }
                    break;

                default:
                    Serial.println("Switch MISSING! 1913");
                    break;
                }
            }

            else
            {
                Lilla_state_0 = PERFORMANCE;
                Lilla_state = SOUND_EDIT;
                P_Sound(Sound_id);
                samples_in_file = Get_samples_in_raw_file(Sound[Sound_id].file);
                Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
                trim_step = Calc_trim_step(trim_speed);
                sound_original = Verify_is_Sound_original(Sound_id);
                So_menu = 0;

                Display.Show_sound(Sound_id, Instrument_id);

                // restore LEDs
                Performance_led_set.Restore_all_LED();

                Display.Show_wave(Instrument_id);

                Select_sound_edit_menu_elements();
                Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menuDisplay.Frame_SOUND_EDIT_menu
                Display.Frame_SOUND_EDIT_menu(So_menu);
            }
        }

        // Pushbuttons
        if (!Read_pushbutton_fast(35))
        {
            if (Read_pushbutton(PB_number + 26) && Patch[Patch_id].Instrument[PB_number].used)
            {
                Lilla_state_0 = PERFORMANCE;
                Lilla_state = SOUND_EDIT;
                Instrument_id = PB_number;
                Serial.print("Editing Sound: ");
                Serial.println(Instrument_id);
                Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
                P_Sound(Sound_id);

                samples_in_file = Get_samples_in_raw_file(Sound[Sound_id].file);
                Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
                trim_step = Calc_trim_step(trim_speed);
                sound_original = Verify_is_Sound_original(Sound_id);
                So_menu = 0;

                Display.Show_sound(Sound_id, Instrument_id);

                // restore LEDs
                Performance_led_set.Restore_all_LED();

                Display.Show_wave(Instrument_id);

                Select_sound_edit_menu_elements();
                Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menu
                Display.Frame_SOUND_EDIT_menu(So_menu);
            }
        }

        else
        {
            // Switch to MIXER
            if (Read_pushbutton(27))
            {
                Lilla_state_0 = PERFORMANCE;
                patch_old = Patch_id;
                Switch_to_MIXER();
            }

            // Switch to DELAY
            else if (Read_pushbutton(28))
            {
                Lilla_state_0 = PERFORMANCE;
                patch_old = Patch_id;
                Lilla_state = DELAY_SETTINGS;
                Display.Delay_page();
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                patch_old = Patch_id;
                Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                patch_old = Patch_id;
                Switch_to_DIRECT_SAMPLING();
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Lilla_state_0 = PERFORMANCE;
                patch_old = Patch_id;
                Golive_MIDI_MONITOR();
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                Switch_from_PERFORMANCE_to_MIDI_LOOP();
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Lilla_state_0 = PERFORMANCE;
                patch_old = Patch_id;
                Golive_SETUP();
            }
        }
    }

#pragma endregion // PERFORMANCE

#pragma region Sound Edit [rgba(2, 108, 118, 0.1)]
    // *************************************************************
    // ********************    SOUND EDIT   ************************
    // *************************************************************

    if (Lilla_state == SOUND_EDIT)
    {
        // Change volume_patch
        if (Read_encoder(15, volume_patch, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
            Players_Manager.Broadcast_volume();
            AudioInterrupts();
        }

        // Change MODE
        if (Read_encoder(1, Sound[Sound_id].mode, 5, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_mode(Patch_id, Instrument_id);
            Fill_Wavetable(Instrument_id); // changing mode, a new Wavetable is needed (using the old NoClick table because A and B are NOT changed);
            Players_Manager.Multicast_main_settings_editing(Patch_id, Instrument_id);
            AudioInterrupts();

            Display.Play_mode(Instrument_id);
            Macro_Sound_menu();
        }

        // Scelta multipla
        result = Read_encoder_simple(2);
        if (result != 0)
        {
            // change FILE
            if (file_midi_ch_flag)
            {
                int S_file_change;
                if (result == 1)
                {
                    S_file_change = Get_next_raw_file_in_flash(Sound[Sound_id].file);
                }
                else
                {
                    S_file_change = Get_previous_raw_file_in_flash(Sound[Sound_id].file);
                }
                if (S_file_change != Sound[Sound_id].file)
                {
                    Sound[Sound_id].file = S_file_change;
                    samples_in_file = Get_samples_in_raw_file(Sound[Sound_id].file);
                    Sound[Sound_id].pitch = 0;
                    Sound[Sound_id].A = 0;
                    Sound[Sound_id].B = (samples_in_file > 0 ? samples_in_file - 1 : 0);
                    if (!slicing_mode)
                        slicing_window = Sound[Sound_id].B - Sound[Sound_id].A + 1;
                    // A_value = Sound[sound_id].A;
                    Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);

                    AudioNoInterrupts();
                    Players_Manager.Update_Preset(Patch_id, Instrument_id, Volume_float[volume_patch]);
                    Fill_Noclick(Instrument_id);   // DEVE essere preceduto da Update_Preset
                    Fill_Wavetable(Instrument_id); // DEVE essere preceduto da Fill_Noclick
                    AudioInterrupts();

                    trim_step = Calc_trim_step(trim_speed);
                    So_menu = 0;
                    Display.File(Instrument_id);
                    Display.Show_wave(Instrument_id);
                    Macro_Sound_menu();
                }
            }
            // change Midi Channel
            else
            {
                midi_channel_change = Get_midi_channel_from_Sound(Sound_id);
                if (result == 1)
                {
                    if (midi_channel_change < 15)
                    {
                        midi_channel_change++;
                    }
                }
                else
                {
                    if (midi_channel_change > 0)
                    {
                        midi_channel_change--;
                    }
                }
                if (midi_channel_change != Get_midi_channel_from_Sound(Sound_id))
                {
                    AudioNoInterrupts();
                    Players_Manager.Multicast_release_players(Sound_id);
                    Reset_map_Instrument_for_notes(Instrument_id);
                    Set_midi_channel_for_Sound(Sound_id, midi_channel_change);
                    Update_map_Instrument_for_notes(Patch[Patch_id].Instrument[Instrument_id].from_note, Patch[Patch_id].Instrument[Instrument_id].to_note, Instrument_id);
                    Players_Manager.Update_Preset_midi_channel(Patch_id, Instrument_id);
                    AudioInterrupts();

                    Display.Midi_channel(Instrument_id);
                    Macro_Sound_menu();
                }
            }
        }

        // toggle file/midi channel
        if (Read_pushbutton(2))
        {
            file_midi_ch_flag = !file_midi_ch_flag;
            Display.File(Instrument_id);
            Display.Midi_channel(Instrument_id);
        }

        // Change PAN
        if (Read_encoder(3, Sound[Sound_id].pan, 16, -16, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_pan(Patch_id, Instrument_id);
            Players_Manager.Multicast_pan(Instrument_id);
            AudioInterrupts();

            Display.Pan(Instrument_id);
            Macro_Sound_menu();
        }

        // Set PAN to center
        if (Read_pushbutton(3))
        {
            Sound[Sound_id].pan = 0;

            AudioNoInterrupts();
            Players_Manager.Update_Preset_pan(Patch_id, Instrument_id);
            Players_Manager.Multicast_pan(Instrument_id);
            AudioInterrupts();

            Display.Pan(Instrument_id);
            Macro_Sound_menu();
        }

        // change GAIN
        if (Read_encoder(4, Sound[Sound_id].gain, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_volume(Patch_id, Instrument_id, Volume_float[volume_patch]);
            Players_Manager.Multicast_volume_for_instrument_edit(Instrument_id);
            AudioInterrupts();

            Display.Gain_sound(Patch_id, Instrument_id);
            Display.Show_wave(Instrument_id);
            Macro_Sound_menu();
        }

        // change Noclick windows
        result = Read_encoder_simple(9);
        if (result != 0)
        {
            changed = false;
            if (result == 1)
            {
                if (Sound[Sound_id].Noclick < Noclick_max)
                {
                    if (Sound[Sound_id].Noclick >= 90)
                    {
                        Sound[Sound_id].Noclick += 10;
                    }
                    else if (Sound[Sound_id].Noclick >= 30)
                    {
                        Sound[Sound_id].Noclick += 4;
                    }
                    else
                    {
                        Sound[Sound_id].Noclick += 2;
                    }
                    changed = true;
                }
            }
            else
            {
                if (Sound[Sound_id].Noclick > 0)
                {
                    if (Sound[Sound_id].Noclick <= 30)
                    {
                        Sound[Sound_id].Noclick -= 2;
                    }
                    else if (Sound[Sound_id].Noclick <= 90)
                    {
                        Sound[Sound_id].Noclick -= 4;
                    }
                    else
                    {
                        Sound[Sound_id].Noclick -= 10;
                    }
                    changed = true;
                }
            }
            if (changed)
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_Noclick(Patch_id, Instrument_id);
                Fill_Noclick(Instrument_id);
                Fill_Wavetable(Instrument_id); // MUST be preceded by "Fill_Noclick(instrument)"
                Players_Manager.Multicast_main_settings_editing(Patch_id, Instrument_id);
                AudioInterrupts();

                sound_original = Verify_is_Sound_original(Sound_id);
                So_menu = 0;

                Display.Noclick(Instrument_id, true);
                Display.Show_wave(Instrument_id);
                Macro_Sound_menu();
            }
        }

        // change A
        result = Read_encoder_simple(10);
        if (result != 0)
        {
            uint32_t So_A_change;
            if (result == 1)
            {
                if (slicing_mode)
                {
                    if ((Sound[Sound_id].B - Sound[Sound_id].A + 1) >= (trim_step + MIN_SNIPPET))
                    {
                        So_A_change = Sound[Sound_id].A + trim_step;
                    }
                    else
                    {
                        So_A_change = Sound[Sound_id].B - MIN_SNIPPET + 1;
                    }
                }

                else
                {
                    if ((Sound[Sound_id].A + slicing_window + trim_step) <= samples_in_file)
                    {
                        So_A_change = Sound[Sound_id].A + trim_step;
                    }
                    else
                    {
                        So_A_change = (samples_in_file - slicing_window);
                    }
                }
            }
            else
            {
                if (Sound[Sound_id].A >= trim_step)
                {
                    So_A_change = Sound[Sound_id].A - trim_step;
                }
                else
                {
                    So_A_change = 0;
                }
            }
            if (So_A_change != Sound[Sound_id].A)
            {
                Sound[Sound_id].A = So_A_change;

                AudioNoInterrupts();
                if (!slicing_mode) // slicing A-Samples
                {
                    Sound[Sound_id].B = Sound[Sound_id].A + slicing_window - 1;
                }
                if (trim_speed == 5)
                    trim_step = Calc_trim_step(5);

                // verify if stop players: it can happend if use_Wavetable switches to "false". Than update Preset[I].A (DO NOT invert the sequence)
                Players_Manager.Verify_if_stop_players(Patch_id, Instrument_id);
                Players_Manager.Update_Preset_A_B_Wavetable(Patch_id, Instrument_id);
                Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
                if (Sound[Sound_id].Noclick > Noclick_max)
                {
                    Sound[Sound_id].Noclick = Noclick_max;
                    Players_Manager.Update_Preset_Noclick(Patch_id, Instrument_id);
                }
                // create new Noclick and Wavetable wavetables, than communicate the new references to the Players
                Fill_Noclick(Instrument_id);
                Fill_Wavetable(Instrument_id); // MUST be preceded by "Fill_Noclick(instrument)"
                Players_Manager.Multicast_main_settings_editing(Patch_id, Instrument_id);
                AudioInterrupts();

                Display.Pitch_voices_max(Instrument_id);
                Display.Show_wave(Instrument_id);
                Macro_Sound_menu();
            }
        }

        // change B
        result = Read_encoder_simple(11);
        if (result != 0)
        {
            uint32_t So_B_change;
            if (result == 1)
            {
                if ((Sound[Sound_id].B + 1 + trim_step) <= samples_in_file)
                {
                    So_B_change = Sound[Sound_id].B + trim_step;
                }
                else
                {
                    So_B_change = samples_in_file - 1;
                }
            }
            else
            {
                if ((Sound[Sound_id].B - Sound[Sound_id].A + 1) >= (MIN_SNIPPET + trim_step))
                {
                    So_B_change = Sound[Sound_id].B - trim_step;
                }
                else
                {
                    So_B_change = Sound[Sound_id].A + MIN_SNIPPET - 1;
                }
            }

            if (So_B_change != Sound[Sound_id].B)
            {
                Sound[Sound_id].B = So_B_change;

                AudioNoInterrupts();
                if (trim_speed == 5)
                {
                    trim_step = Calc_trim_step(5);
                }

                // verify if stop players: it can happend if use_Wavetable switches to "false". Than update Preset[I].A (DO NOT invert the sequence)
                Players_Manager.Verify_if_stop_players(Patch_id, Instrument_id); // MUST be done BEFORE updating Preset
                Players_Manager.Update_Preset_A_B_Wavetable(Patch_id, Instrument_id);
                Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
                if (Sound[Sound_id].Noclick > Noclick_max)
                {
                    Sound[Sound_id].Noclick = Noclick_max;
                    Players_Manager.Update_Preset_Noclick(Patch_id, Instrument_id);
                }

                // create new Noclick and Wavetable wavetables, than communicate the new references to the Players
                Fill_Noclick(Instrument_id);
                Fill_Wavetable(Instrument_id); // MUST be preceded by "Fill_Noclick(instrument)"
                Players_Manager.Multicast_main_settings_editing(Patch_id, Instrument_id);
                AudioInterrupts();

                if (!slicing_mode)
                {
                    slicing_window = Sound[Sound_id].B - Sound[Sound_id].A + 1;
                }

                Display.Pitch_voices_max(Instrument_id);
                Display.Show_wave(Instrument_id);
                Macro_Sound_menu();
            }
        }

        // Change slicing mode: true:FIRST/LAST    false:FIRST/WINDOW
        if (Read_pushbutton(11))
        {
            slicing_mode = !slicing_mode;
            if (!slicing_mode)
            {
                slicing_window = Sound[Sound_id].B - Sound[Sound_id].A + 1;
            }
            Display.Show_wave(Instrument_id);
        }

        // Change trim speed
        if (Read_encoder(12, trim_speed, 5, 0, 1))
        {
            trim_step = Calc_trim_step(trim_speed);
            Display.Trim_step();
        }

        // Set Default trim speed
        if (Read_pushbutton(12))
        {
            trim_speed = 5;
            trim_step = Calc_trim_step(trim_speed);
            Display.Trim_step();
        }

        // Change PITCH
        result = Read_encoder_simple(17);
        if (result != 0)
        {
            changed = false;
            if (result == 1)
            {
                if (Sound[Sound_id].pitch < 96)
                {
                    Sound[Sound_id].pitch++;
                    changed = true;
                }
            }
            else
            {
                if (Sound[Sound_id].pitch > -96)
                {
                    Sound[Sound_id].pitch--;
                    changed = true;
                }
            }
            if (changed)
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_pitch(Patch_id, Instrument_id);
                Players_Manager.Multicast_pitch_for_sound_edit(Instrument_id);
                AudioInterrupts();

                Display.Pitch(Instrument_id);
                Macro_Sound_menu();
            }
        }

        // Set Pitch flat
        if (Read_pushbutton(17))
        {
            if (Sound[Sound_id].pitch != 0)
            {
                Sound[Sound_id].pitch = 0;

                AudioNoInterrupts();
                Players_Manager.Update_Preset_pitch(Patch_id, Instrument_id);
                Players_Manager.Multicast_pitch_for_sound_edit(Instrument_id);
                AudioInterrupts();

                Display.Pitch(Instrument_id);
                Macro_Sound_menu();
            }
        }

        // change Attack
        if (Read_encoder(18, Sound[Sound_id].attack, 255, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_attack(Patch_id, Instrument_id);
            AudioInterrupts();

            Display.Attack(Instrument_id);
            Macro_Sound_menu();
        }

        // toggle Attack curve
        if (Read_pushbutton(18))
        {
            AudioNoInterrupts();
            bitWrite(Sound[Sound_id].data, 0, !bitRead(Sound[Sound_id].data, 0));
            Players_Manager.Update_Preset_attack_type(Patch_id, Instrument_id);
            So_menu = 0;
            AudioInterrupts();

            Display.Attack(Instrument_id);
            Macro_Sound_menu();
        }

        // change Decay
        if (Read_encoder(19, Sound[Sound_id].decay, 255, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_decay(Patch_id, Instrument_id);
            AudioInterrupts();

            Display.Decay(Instrument_id);
            Macro_Sound_menu();
        }

        // change Sustain
        if (Read_encoder(20, Sound[Sound_id].sustain, 46, 4, 5))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_sustain(Patch_id, Instrument_id);
            AudioInterrupts();

            Display.Sustain(Instrument_id);
            Macro_Sound_menu();
        }

        // change Release
        if (Read_encoder(21, Sound[Sound_id].release, 50, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_release(Patch_id, Instrument_id);
            AudioInterrupts();

            Display.Release(Instrument_id);
            Macro_Sound_menu();
        }

        // Solo
        if (Read_pushbutton(21))
        {
            if (!solo_flag)
            {
                solo_flag = true;
                AudioNoInterrupts();
                Players_Manager.Release_all_players_for_instrument_solo(Instrument_id);
                Map_one_Instrument_for_all_notes(Instrument_id);
                AudioInterrupts();
            }
            else
            {
                Set_Sound_SOLO_OFF();
            }
            Display.Show_wave(Instrument_id);
        }

        // change MENU position
#ifdef PCB_2022
        if (Read_encoder_inverse(25, So_menu, So_menu_max, 0, 1))
#else
        if (Read_encoder(25, So_menu, So_menu_max, 0, 1))
#endif
            Display.Frame_SOUND_EDIT_menu(So_menu);

        // choose MENU position
        if ((Lilla_state_0 != MIDI_LOOP) && Read_pushbutton(25))
        {
            switch (choice_sound_menu)
            {
            case 0: // Return - keep changes and exit from Sound EDITING
                Set_Sound_SOLO_OFF();

                Lilla_state = PERFORMANCE;
                patch_original = Verify_is_Patch_original(Patch_id);

                Display.Patch_header(true, true); // Patch_header(bool change_patch, bool change_vol)
                P_menu = P_menu_max + position_of_Instrument[Instrument_id] + 1;
                Select_performance_menu_elements();
                Display.Performance_menu(); // display the menu and update P_menu_max
                Display.Instruments_header();
                Display.All_Instrument(Patch_id);

                Display.Show_Instrument_description(Patch_id, Instrument_id, true);

                // restore LED
                Performance_led_set.Restore_all_LED();

                Display.Instrument_frame_on_position(position_of_Instrument[Instrument_id], true);
                instrument_editing_flag = true;
                last_edited_instrument = Instrument_id;
                break;

            case 1: // Clone this Instrument and go back to PERFORMANCE

                AudioNoInterrupts();
                Instrument_id = Clone_Instrument(Instrument_id); // also increments instruments
                Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
                Update_map_Instrument_for_notes(Patch[Patch_id].Instrument[Instrument_id].from_note, Patch[Patch_id].Instrument[Instrument_id].to_note, Instrument_id);
                Players_Manager.Update_Preset(Patch_id, Instrument_id, Volume_float[volume_patch]);
                Fill_Noclick(Instrument_id);
                Fill_Wavetable(Instrument_id);
                AudioInterrupts();

                Set_Sound_SOLO_OFF();
                Lilla_state = PERFORMANCE;
                Update_Instruments_positions();
                patch_original = Verify_is_Patch_original(Patch_id);
                Display.Patch_header(true, true); // Patch_header(bool change_patch, bool change_vol)
                P_menu = P_menu_max + position_of_Instrument[Instrument_id] + 1;
                Select_performance_menu_elements();
                Display.Performance_menu(); // display the menu and update P_menu_max
                Display.Instruments_header();
                Display.All_Instrument(Patch_id);

                Display.Show_Instrument_description(Patch_id, Instrument_id, true);

                // restore LED
                Performance_led_set.Restore_all_LED();

                Display.Instrument_frame_on_position(position_of_Instrument[Instrument_id], true);
                instrument_editing_flag = true;
                last_edited_instrument = Instrument_id;
                break;

            case 2: // Delete this Instrument and go back to PERFORMANCE
                // PB_state[instrument] = 5; // Patch, PB down

                AudioNoInterrupts();
                Players_Manager.Release_all_players_for_instrument(Instrument_id);
                Delete_one_map_Instrument_for_notes(Instrument_id);
                Drop_Instrument(Instrument_id); // instruments is decremented by 1
                AudioInterrupts();

                Lilla_state = PERFORMANCE;
                Update_Instruments_positions();
                patch_original = Verify_is_Patch_original(Patch_id);
                Instrument_id = instrument_on_position[0];
                Display.Patch_header(true, false); // Patch_header(bool change_patch, bool change_vol)
                Select_performance_menu_elements();
                Display.Performance_menu(); // displays the menu and updates P_menu_max used by encoder_menu
                P_menu = P_menu_max + 1;
                Display.Instruments_header();
                Display.All_Instrument(Patch_id);
                Display.Instrument_frame_on_position(position_of_Instrument[Instrument_id], true);
                instrument_editing_flag = true;
                last_edited_instrument = Instrument_id;
                break;

            default:
                Serial.println("Switch MISSING! 2560");
                break;
            }
        }

        // Pushbuttons
        if (!Read_pushbutton_fast(35))
        {
            if (Read_pushbutton(PB_number + 26))
            {
                if (PB_number == Instrument_id)
                {
                    Lilla_state = INSTRUMENT_VCF;

                    Display.Instrument_VCF_page(Patch_id, Instrument_id);

                    // restore all LED
                    Performance_led_set.Restore_all_LED();
                }
                else if (Patch[Patch_id].Instrument[PB_number].used)
                {
                    AudioNoInterrupts();
                    if (solo_flag)
                    {
                        solo_flag = false;
                        Update_all_maps_Instrument_for_notes();
                    }
                    AudioInterrupts();

                    Instrument_id = PB_number;
                    Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
                    P_Sound(Sound_id);
                    samples_in_file = Get_samples_in_raw_file(Sound[Sound_id].file);
                    Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
                    trim_step = Calc_trim_step(trim_speed);
                    sound_original = Verify_is_Sound_original(Sound_id);
                    So_menu = 0;

                    Display.Show_sound(Sound_id, Instrument_id);
                    if (Lilla_state_0 != MIDI_LOOP)
                    {
                        // restore all LED
                        Performance_led_set.Restore_all_LED();
                    }

                    // to do: display LED for MIDI_LOOP

                    Display.Show_wave(Instrument_id);

                    if (Lilla_state_0 != MIDI_LOOP)
                    {
                        Select_sound_edit_menu_elements();
                    }
                    Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menu
                    Display.Frame_SOUND_EDIT_menu(So_menu);
                }
            }
        }

        else
        {
            // Ritorna a PERFORMANCE
            if (Read_pushbutton(26))
            {
                Set_Sound_SOLO_OFF();
                Golive_with_PERFORMANCE(Patch_id);
            }

            // Switch to MIXER
            else if (Read_pushbutton(27))
            {
                Set_Sound_SOLO_OFF();
                Switch_to_MIXER();
            }

            // Switch to DELAY
            else if (Read_pushbutton(28))
            {
                Set_Sound_SOLO_OFF();
                Lilla_state = DELAY_SETTINGS;
                Display.Delay_page();
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                Set_Sound_SOLO_OFF();
                Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                Set_Sound_SOLO_OFF();
                Switch_to_DIRECT_SAMPLING();
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Set_Sound_SOLO_OFF();
                Golive_MIDI_MONITOR();
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                Set_Sound_SOLO_OFF();
                if (Lilla_state_0 == MIDI_LOOP)
                    Golive_with_MIDI_LOOP(false);
                else
                    Switch_from_PERFORMANCE_to_MIDI_LOOP();
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Set_Sound_SOLO_OFF();
                Golive_SETUP();
            }
        }

        // Switch verso INSTRUMENT_VCF implicito
        if (Request_VCF_mode())
        {
            Lilla_state = INSTRUMENT_VCF;

            Display.Instrument_VCF_page(Patch_id, Instrument_id);

            // restore LED
            Performance_led_set.Restore_all_LED();
        }
    }

#pragma endregion // SOUND_EDIT

#pragma region
    // *************************************************************
    // *****************      INSTRUMENT_VCF     *******************
    // *************************************************************
    if (Lilla_state == INSTRUMENT_VCF)
    {
        if (Lilla_state_0 == LIVE_SAMPLING)
        {
            // change volume_patch
            if (Read_encoder(15, volume_patch, 40, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
                Players_Manager.Broadcast_volume();
                AudioInterrupts();
                Display.Patch_volume_value(true); // true: YELLOW
            }
        }

        else if (Lilla_state_0 == PERFORMANCE || Lilla_state_0 == MIDI_LOOP)
        {
            // Change GAIN
            if (Read_encoder(4, Sound[Sound_id].gain, 40, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_volume(Patch_id, Instrument_id, Volume_float[volume_patch]);
                Players_Manager.Multicast_volume_for_instrument_edit(Instrument_id);
                AudioInterrupts();
                Display.Show_VCF_gain(Sound_id);
            }

            // Change volume_patch
            if (Read_encoder(15, volume_patch, 40, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
                Players_Manager.Broadcast_volume();
                AudioInterrupts();
            }
        }

        // VFC type (Lowpass, Highpass...)
        if (Read_encoder(5, Patch[Patch_id].Instrument[Instrument_id].Filter.type, 3, 0, 1))
        {
            if (Lilla_state_0 == LIVE_SAMPLING)
            {
                Patch[Patch_id].Instrument[1].Filter.type = Patch[Patch_id].Instrument[0].Filter.type;
                AudioNoInterrupts();
                Players_Manager.Update_Preset_IF(Patch_id, 0);
                Players_Manager.Update_Preset_IF(Patch_id, 1);
                Players_Manager.Multicast_IF_update_filter_type(0);
                Players_Manager.Multicast_IF_update_filter_type(1);
                AudioInterrupts();
            }
            else
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);
                Players_Manager.Multicast_IF_update_filter_type(Instrument_id);
                AudioInterrupts();
            }
            Display.Show_VCF_filter_type(Instrument_id);
        }

        // VCF filter ON/none
        if (Read_pushbutton(5))
        {
            Macro_VCF_filter_on_none();
            Display.Show_VCF_filter_type(Instrument_id);
        }

        // VCF modulation (none, trail up, trail down, sinus)
        if (Read_encoder(6, Patch[Patch_id].Instrument[Instrument_id].Filter.modulation, 4, 0, 1))
        {
            if (Lilla_state_0 == LIVE_SAMPLING)
            {
                Patch[Patch_id].Instrument[1].Filter.modulation = Patch[Patch_id].Instrument[0].Filter.modulation;
                AudioNoInterrupts();
                Players_Manager.Update_Preset_IF_modulation(Patch_id, 0);
                Players_Manager.Update_Preset_IF_modulation(Patch_id, 1);
                AudioInterrupts();
            }
            else
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_IF_modulation(Patch_id, Instrument_id);
                AudioInterrupts();
            }
            Display.Show_VCF_lfo_type(Instrument_id);
        }

        // VCF modulation "NONE"
        if (Read_pushbutton(6))
        {
            Macro_VCF_modulation_none();
            Display.Show_VCF_lfo_type(Instrument_id);
        }

        // VCF filter Cutoff frequency
        if (Read_encoder(13, Patch[Patch_id].Instrument[Instrument_id].Filter.pivot, 100, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);
            if (Preset[Instrument_id].Filter.use == 1)
            {
                Players_Manager.Multicast_IF_pivot(Instrument_id);
            }

            if (Lilla_state_0 == LIVE_SAMPLING)
            {
                Patch[Patch_id].Instrument[1].Filter.pivot = Patch[Patch_id].Instrument[0].Filter.pivot;
                Instrument_id = 1;

                Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);
                if (Preset[Instrument_id].Filter.use == 1)
                {
                    Players_Manager.Multicast_IF_pivot(Instrument_id);
                }

                Instrument_id = 0;
            }
            AudioInterrupts();
            Display.Show_VCF_cutoff(Instrument_id);
        }

        // VCF modulation frequency or time
        if (Read_encoder(14, Patch[Patch_id].Instrument[Instrument_id].Filter.frequency_time, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);
            if ((Preset[Instrument_id].Filter.use == 1) && (Preset[Instrument_id].Filter.modulation > 0) && (Preset[Instrument_id].Filter.periodic == 1))
            {
                Players_Manager.Multicast_IF_frequency_filter(Instrument_id);
            }

            if (Lilla_state_0 == LIVE_SAMPLING)
            {
                Patch[Patch_id].Instrument[1].Filter.frequency_time = Patch[Patch_id].Instrument[0].Filter.frequency_time;
                Instrument_id = 1;
                Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);
                if ((Preset[Instrument_id].Filter.use == 1) && (Preset[Instrument_id].Filter.modulation > 0) && (Preset[Instrument_id].Filter.periodic == 1))
                {
                    Players_Manager.Multicast_IF_frequency_filter(Instrument_id);
                }

                Instrument_id = 0;
            }
            AudioInterrupts();
            Display.Show_VCF_lfo_freq_time(Instrument_id);
        }

// VCF resonance
#ifdef PCB_2022
        if (Read_encoder(21, Patch[patch_id].Instrument[instrument].Filter.resonance, 40, 0, 1))
#else
        if (Read_encoder(22, Patch[Patch_id].Instrument[Instrument_id].Filter.resonance, 40, 0, 1))
#endif
        {

            AudioNoInterrupts();
            Players_Manager.Update_IF_resonance(Patch_id, Instrument_id);
            if (Lilla_state_0 == LIVE_SAMPLING)
            {
                Patch[Patch_id].Instrument[1].Filter.resonance = Patch[Patch_id].Instrument[0].Filter.resonance;
                Instrument_id = 1;
                Players_Manager.Update_IF_resonance(Patch_id, Instrument_id);
                Instrument_id = 0;
            }
            AudioInterrupts();
            Display.Show_VCF_resonance(Instrument_id);
        }

        // Solo
        if (Read_pushbutton(21))
        {
            AudioNoInterrupts();
            Players_Manager.Release_all_players_for_instrument_solo(Instrument_id);
            Map_one_Instrument_for_all_notes(Instrument_id);
            AudioInterrupts();
            Display.Show_VCF_solo();
        }

// VCF modulation rate
#ifdef PCB_2022
        if (Read_encoder(22, Patch[patch_id].Instrument[instrument].Filter.index, 20, 0, 1))
#else
        if (Read_encoder(23, Patch[Patch_id].Instrument[Instrument_id].Filter.index, 20, 0, 1))
#endif
        {
            AudioNoInterrupts();
            Players_Manager.Update_Preset_IF_index(Patch_id, Instrument_id);

            if (Lilla_state_0 == LIVE_SAMPLING)
            {
                Patch[Patch_id].Instrument[1].Filter.index = Patch[Patch_id].Instrument[0].Filter.index;
                Instrument_id = 1;
                Players_Manager.Update_Preset_IF_index(Patch_id, Instrument_id);
                Instrument_id = 0;
            }
            AudioInterrupts();
            Display.Show_VCF_lfo_index(Instrument_id);
        }

        // pushbuttons
        if (!Read_pushbutton_fast(35))
        {
            if (Lilla_state_0 == PERFORMANCE)
            {
                if (Read_pushbutton(PB_number + 26))
                {
                    if (PB_number == Instrument_id)
                    {
                        Set_Sound_SOLO_OFF();

                        Lilla_state = PERFORMANCE;
                        patch_original = Verify_is_Patch_original(Patch_id);

                        Select_performance_menu_elements();
                        P_menu = P_menu_max + position_of_Instrument[Instrument_id] + 1;

                        Display.Performance_page(true, true);
                        instrument_editing_flag = true;
                        P_menu = P_menu_max + position_of_Instrument[Instrument_id] + 1;

                        Display.Show_Instrument_description(Patch_id, Instrument_id, true);

                        // restore LED
                        Performance_led_set.Restore_all_LED();

                        Display.Instrument_frame_on_position(position_of_Instrument[Instrument_id], true);
                        instrument_editing_flag = true;
                        last_edited_instrument = Instrument_id;
                    }
                    else if (Patch[Patch_id].Instrument[PB_number].used)
                    {
                        AudioNoInterrupts();
                        if (solo_flag)
                        {
                            solo_flag = false;
                            Update_all_maps_Instrument_for_notes();
                        }
                        AudioInterrupts();

                        Lilla_state = SOUND_EDIT;

                        Instrument_id = PB_number;
                        Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;

                        P_Sound(Sound_id);
                        samples_in_file = Get_samples_in_raw_file(Sound[Sound_id].file);
                        Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
                        trim_step = Calc_trim_step(trim_speed);
                        sound_original = Verify_is_Sound_original(Sound_id);

                        So_menu = 0;

                        Display.Show_sound(Sound_id, Instrument_id);

                        // restore LED
                        Performance_led_set.Restore_all_LED();

                        Display.Show_wave(Instrument_id);

                        Select_sound_edit_menu_elements();
                        Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menu
                        Display.Frame_SOUND_EDIT_menu(So_menu);
                    }
                }
            }

            else if (Lilla_state_0 == LIVE_SAMPLING)
            {
                if (LS_stereo)
                {
                    // VCF left channel
                    if (Read_pushbutton(26))
                    {
                        if (Instrument_id == 0)
                        {
                            LS_instrument = 0;        // Left
                            LS_id_sound = SOUNDS_MAX; // Left
                            Golive_with_LIVE_SAMPLING();
                        }
                        else
                        {
                            Instrument_id = 0;
                            Sound_id = SOUNDS_MAX;

                            Display.Instrument_VCF_page(Patch_id, Instrument_id);

                            // restore LED
                            Performance_led_set.Restore_all_LED();
                        }
                    }
                    // VCF right channel
                    if (Read_pushbutton(27))
                    {
                        if (Instrument_id == 1)
                        {
                            LS_instrument = 1;            // Right
                            LS_id_sound = SOUNDS_MAX + 1; // Right
                            Golive_with_LIVE_SAMPLING();
                        }
                        else
                        {
                            Instrument_id = 1;
                            Sound_id = SOUNDS_MAX + 1;
                            Display.Instrument_VCF_page(Patch_id, Instrument_id);

                            // restore LED
                            Performance_led_set.Restore_all_LED();
                        }
                    }
                }
                else
                {
                    // VCF left channel
                    if (Read_pushbutton(26))
                    {
                        Golive_with_LIVE_SAMPLING();
                    }

                    // VCF right channel
                    else if (Read_pushbutton(27))
                    {
                        Golive_with_LIVE_SAMPLING();
                    }
                }
            }

            else if (Lilla_state_0 == MIDI_LOOP)
            {
                if (Read_pushbutton(PB_number + 26))
                {
                    if (PB_number == Instrument_id)
                    {
                        Set_Sound_SOLO_OFF();

                        // Ritorna a MIDI_LOOP
                        Golive_with_MIDI_LOOP(false);
                    }
                    else if (Patch[Patch_id].Instrument[PB_number].used)
                    {
                        AudioNoInterrupts();
                        if (solo_flag)
                        {
                            solo_flag = false;
                            Update_all_maps_Instrument_for_notes();
                        }
                        AudioInterrupts();

                        Lilla_state = SOUND_EDIT;
                        Instrument_id = PB_number;
                        // PB_state[instrument] = 1; // Sound_edit, PB down
                        Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
                        P_Sound(Sound_id);
                        samples_in_file = Get_samples_in_raw_file(Sound[Sound_id].file);
                        Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
                        trim_step = Calc_trim_step(trim_speed);
                        sound_original = Verify_is_Sound_original(Sound_id);

                        So_menu = 0;

                        Display.Show_sound(Sound_id, Instrument_id);
                        // to do: display LED

                        Display.Show_wave(Instrument_id);
                        Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menu
                        Display.Frame_SOUND_EDIT_menu(So_menu);
                    }
                }
            }
        }

        // Switch
        else
        {
            // Switch to Performance
            if (Read_pushbutton(26))
            {
                if (Lilla_state_0 == PERFORMANCE)
                {
                    Set_Sound_SOLO_OFF();
                    Golive_with_PERFORMANCE(Patch_id);
                }
                else
                {
                    Switch_from_LIVE_SAMPLING_to_PERFORMANCE();
                }
            }

            // Switch to Mixer
            else if (Read_pushbutton(27))
            {
                Switch_to_MIXER();
            }

            // Switch to Delay
            else if (Read_pushbutton(28))
            {
                if (Lilla_state_0 == PERFORMANCE)
                {
                    Set_Sound_SOLO_OFF();
                }
                else // Lilla_state_0 == LIVE_SAMPLING
                {
                    if (Delay_values.instrument_route[0] || Delay_values.instrument_route[1])
                    {
                        Delay_values.instrument_route[0] = true;
                        Delay_values.instrument_route[1] = true;
                    }
                }
                Lilla_state = DELAY_SETTINGS;
                Display.Delay_page();
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                if (Lilla_state_0 == PERFORMANCE)
                {
                    Set_Sound_SOLO_OFF();
                    Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
                }
                else // Lilla_state_0 == LIVE_SAMPLING
                {
                    Golive_with_LIVE_SAMPLING();
                }
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                if (Lilla_state_0 == PERFORMANCE)
                {
                    Set_Sound_SOLO_OFF();
                    Switch_to_DIRECT_SAMPLING();
                }
                else // Lilla_state_0 == LIVE_SAMPLING
                {
                    Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING();
                }
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Golive_MIDI_MONITOR();
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                Set_Sound_SOLO_OFF();
                Switch_from_PERFORMANCE_to_MIDI_LOOP();
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Golive_SETUP();
            }
        }

        // Read menu_pushbutton and exit
        if (Lilla_state_0 == PERFORMANCE)
        {
            if (Read_pushbutton(25))
            {
                Golive_with_PERFORMANCE(Patch_id);
            }
        }
        else if (Lilla_state_0 == LIVE_SAMPLING)
        {
            if (Read_pushbutton(25))
            {
                Golive_with_LIVE_SAMPLING();
            }
        }

        // Switch verso SOUND_EDIT implicito
        if ((Lilla_state_0 == PERFORMANCE || Lilla_state_0 == MIDI_LOOP) && Request_SOUND_EDIT_mode())
        {
            Lilla_state = SOUND_EDIT;
            So_menu = 0;

            Display.Show_sound(Sound_id, Instrument_id);
            if (Lilla_state_0 == PERFORMANCE)
            {

                // restore LED
                Performance_led_set.Restore_all_LED();
            }

            // to do: display LED per Lilla_state_0 == MIDI_LOOP

            Display.Show_wave(Instrument_id);

            if (Lilla_state_0 != MIDI_LOOP)
            {
                Select_sound_edit_menu_elements();
            }
            Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menu
            Display.Frame_SOUND_EDIT_menu(So_menu);
        }

    } // end INSTRUMENT_VCF

#pragma endregion // INSTRUMENT_VCF

#pragma region Mixer [rgba(130, 4, 40, 0.19)]
    // *************************************************************
    // ********************        MIXER      **********************
    // *************************************************************
    if (Lilla_state == MIXER)
    {
        if (Lilla_state_0 == PERFORMANCE || (Lilla_state_0 == DIRECT_SAMPLING && DS_state == 0) || Lilla_state_0 == LIVE_SAMPLING)
        {
            if (Read_encoder(15, volume_patch, 40, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
                Players_Manager.Broadcast_volume();
                AudioInterrupts();
            }
        }

        // Choose source instrument: MX_sources 0 --> 7
        if (!Read_pushbutton_fast(35))
        {
            if (Read_pushbutton(PB_number + 26) && Patch[Patch_id].Instrument[PB_number].used)
            {
                Display.MX_source_values_jump(MX_source, PB_number); // MX_source_values_jump(const uint8_t &old_source, const uint8_t &new_source) - qui si assegna il nuovo valore MX_source
                Instrument_id = MX_source;
                Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
                Serial.print("sound_id: ");
                Serial.println(Sound_id);
            }

            // Choose source LINE IN: MX_source 8
            // Leggi dopo Sound 7 (non ad ogni loop!)
            if (PB_number == 7 && Read_pushbutton(34))
            {
                // MX_source = 8;
                Display.MX_source_values_jump(MX_source, 8); // aggiorna MX_source a 8
                Serial.println("Line IN");
            }
        }

        // Change GAIN
        if (MX_source < INSTRUMENTS_MAX)
        {
            if (Read_encoder(4, Sound[Sound_id].gain, 40, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_volume(Patch_id, Instrument_id, Volume_float[volume_patch]);
                Players_Manager.Multicast_volume_for_instrument_edit(Instrument_id);
                AudioInterrupts();

                Serial.println(Sound[Sound_id].gain);
                Serial.println(MX_source);
                Display.MX_source_values_edit(MX_source);
            }
        }
        else if (MX_source == INSTRUMENTS_MAX)
        {
            if (Read_encoder(4, DS_gain, 40, 1, 1))
            {
                LINE_IN_amplifier.Set_gain(Volume_float[DS_gain]);
                Display.MX_source_values_edit(MX_source);
            }
        }

        // Change PAN
        if (MX_source < INSTRUMENTS_MAX)
        {
            if (Read_encoder(3, Sound[Sound_id].pan, 16, -16, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_Preset_pan(Patch_id, Instrument_id);
                Players_Manager.Multicast_pan(Instrument_id);
                AudioInterrupts();

                Display.MX_source_values_edit(MX_source);
            }
        }

        // Mute/unmute a source
        if (Read_pushbutton(4))
        {
            MX_mute[MX_source] = !MX_mute[MX_source];

            if (MX_source < INSTRUMENTS_MAX)
            {
                Serial.print((MX_mute[Instrument_id] ? "Mute MX_source:" : "umute MX_source:"));
                Serial.println(Instrument_id);

                AudioNoInterrupts();
                Players_Manager.Update_Preset_volume(Patch_id, Instrument_id, Volume_float[volume_patch]);
                Players_Manager.Multicast_volume_for_instrument_edit(Instrument_id);
                AudioInterrupts();
            }
            else if (MX_source == 8)
            {
                if (MX_mute[MX_source])
                {
                    Serial.print((MX_mute[MX_source] ? "Mute MX_source:" : "umute MX_source:"));
                    Serial.println(MX_source);

                    MAIN_mixer_out_L.Mute(1);
                    MAIN_mixer_out_R.Mute(1);
                    PWM_mixer_out_L.Mute(1);
                    PWM_mixer_out_R.Mute(1);
                }
                else
                {
                    Serial.print((MX_mute[MX_source] ? "Mute MX_source:" : "umute MX_source:"));
                    Serial.println(MX_source);

                    MAIN_mixer_out_L.unmute(1);
                    MAIN_mixer_out_R.unmute(1);
                    PWM_mixer_out_L.unmute(1);
                    PWM_mixer_out_R.unmute(1);
                }
            }
            Display.MX_source_values_edit(MX_source);
        }

        // Change volume MAIN
        if (Read_encoder(19, MAIN_volume, 40, 0, 1))
        {
            AudioNoInterrupts();
            MAIN_mixer_out_L.gain(Volume_float[MAIN_volume]);
            MAIN_mixer_out_R.gain(Volume_float[MAIN_volume]);
            AudioInterrupts();

            Display.MX_MAIN_volume();
        }

        // Change volume MONITOR
        if (Read_encoder(20, PWM_volume, 40, 0, 1))
        {
            AudioNoInterrupts();
            PWM_mixer_out_L.gain(Volume_float[PWM_volume]);
            PWM_mixer_out_R.gain(Volume_float[PWM_volume]);
            AudioInterrupts();
            Display.MX_MONITOR_volume();
        }

        // Route/unroute a source to MAIN
        if (Read_pushbutton(19))
        {
            if (MX_routing_source[MX_source] == 0) // era tutto muto --> solo MAIN
            {
                MX_routing_source[MX_source] = 2;
            }
            else if (MX_routing_source[MX_source] == 1) // era solo MONITOR --> MONITOR e MAIN
            {
                MX_routing_source[MX_source] = 3;
            }
            else if (MX_routing_source[MX_source] == 2) // era solo MAIN --> tutto muto
            {
                MX_routing_source[MX_source] = 0;
            }
            else // era 3 (MONITOR e MAIN) --> solo MONITOR
            {
                MX_routing_source[MX_source] = 1;
            }

            if (MX_source < INSTRUMENTS_MAX)
            {
                AudioNoInterrupts();
                Players_Manager.MX_multicast_change_routing(Instrument_id);
                AudioInterrupts();
            }

            else if (MX_source == 8)
            {
                switch (MX_routing_source[MX_source])
                {
                case 0:
                    MAIN_mixer_out_L.Mute(1);
                    MAIN_mixer_out_R.Mute(1);
                    break;

                case 1:
                    MAIN_mixer_out_L.Mute(1);
                    MAIN_mixer_out_R.Mute(1);
                    break;

                case 2:
                    MAIN_mixer_out_L.unmute(1);
                    MAIN_mixer_out_R.unmute(1);
                    break;

                case 3:
                    MAIN_mixer_out_L.unmute(1);
                    MAIN_mixer_out_R.unmute(1);
                    break;

                default:
                    Serial.println("Switch MISSING! 3303");
                    break;
                }
            }
            Display.MX_source_values_edit(MX_source);
        }

        // Route/unroute a source to MONITOR (PWM)
        if (Read_pushbutton(20))
        {
            if (MX_routing_source[MX_source] == 0) // era tutto muto --> solo MONITOR
            {
                MX_routing_source[MX_source] = 1;
            }
            else if (MX_routing_source[MX_source] == 1) // era solo MONITOR --> tutto muto
            {
                MX_routing_source[MX_source] = 0;
            }
            else if (MX_routing_source[MX_source] == 2) // era solo MAIN --> MONITOR e MAIN
            {
                MX_routing_source[MX_source] = 3;
            }
            else // era 3 (MONITOR e MAIN) --> solo MAIN
            {
                MX_routing_source[MX_source] = 2;
            }

            if (MX_source < INSTRUMENTS_MAX)
            {
                AudioNoInterrupts();
                Players_Manager.MX_multicast_change_routing(Instrument_id);
                AudioInterrupts();
            }

            if (MX_source == 8)
            {
                switch (MX_routing_source[MX_source])
                {
                case 0:
                    PWM_mixer_out_L.Mute(1);
                    PWM_mixer_out_R.Mute(1);
                    break;

                case 1:
                    PWM_mixer_out_L.unmute(1);
                    PWM_mixer_out_R.unmute(1);
                    break;

                case 2:
                    PWM_mixer_out_L.Mute(1);
                    PWM_mixer_out_R.Mute(1);
                    break;

                case 3:
                    PWM_mixer_out_L.unmute(1);
                    PWM_mixer_out_R.unmute(1);
                    break;

                default:
                    Serial.println("Switch MISSING! 3354");
                    break;
                }
            }
            Display.MX_source_values_edit(MX_source);
        }

        if (Read_pushbutton_fast(35))
        {
            // Switch to PERFORMANCE
            if (Read_pushbutton(26))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Golive_with_PERFORMANCE(Patch_id);
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_PERFORMANCE();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_PERFORMANCE();
                    break;

                default:
                    Serial.println("Switch MISSING! 3381");
                    break;
                }
            }

            // Switch to DELAY
            else if (Read_pushbutton(28))
            {
                if (Lilla_state_0 != DIRECT_SAMPLING)
                {
                    Lilla_state = DELAY_SETTINGS;
                    Display.Delay_page();
                }
                else
                {
                    Display.Delay_disabled();
                    delay(2000);

                    Display.MX_page();
                    for (auto source = 0; source < 9; ++source)
                    {
                        Display.MX_source_values(source);
                    }
                }
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_LIVE_SAMPLING();
                    break;

                case LIVE_SAMPLING:
                    LS_refresh_LS_page();
                    break;

                default:
                    Serial.println("Switch MISSING! 3425");
                    break;
                }
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_to_DIRECT_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    DS_refresh_DS_page();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING();
                    break;

                default:
                    Serial.println("Switch MISSING! 3448");
                    break;
                }
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Golive_MIDI_MONITOR();
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_from_PERFORMANCE_to_MIDI_LOOP();
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_MIDI_LOOP();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_MIDI_LOOP();
                    break;

                case MIDI_LOOP:
                    Golive_with_MIDI_LOOP(false);
                    break;

                default:
                    Serial.println("Switch MISSING! 5435");
                    break;
                }
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Golive_SETUP();
            }
        }
    }

#pragma endregion // MIXER

#pragma region Delay [rgba(16, 86, 208, 0.25)]
    // *************************************************************
    // ********************   DELAY_SETTINGS  **********************
    // *************************************************************
    if (Lilla_state == DELAY_SETTINGS)
    {
        // Change Patch VOLUME
        if (Read_encoder(15, volume_patch, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
            Players_Manager.Broadcast_volume();
            AudioInterrupts();

            Display.Patch_volume_value(true);
        }

        // Change feddback (gain tap 0)
        if (Read_encoder(12, Delay_data.loop_gain, Delay_data_limits[LOOP_GAIN][1], Delay_data_limits[LOOP_GAIN][0], 1))
        {
            Delay_values.loop_gain = Delay_feedback(Delay_data.loop_gain);
            Serial.println(Delay_values.loop_gain);

            AudioNoInterrupts();
            D_gain_L_feedback.Set_gain(Delay_values.loop_gain);
            D_gain_R_n.Set_gain(Delay_values.loop_gain);
            AudioInterrupts();

            Display.D_read_gain();
        }

        // Change value (delay_central_value)
        if (Read_encoder(13, Delay_data.samples, 99, 0, 1))
        {
            Delay_values.samples = Calc_delay_samples(Delay_data.samples);

            AudioNoInterrupts();
            if (Delay_values.samples_LR >= 0) // Left channel
            {
                Delay_L.Set_delay_central_value(Delay_values.samples + Delay_values.samples_LR);
                Delay_R.Set_delay_central_value(Delay_values.samples);
            }
            else
            {
                Delay_R.Set_delay_central_value(Delay_values.samples - Delay_values.samples_LR);
                Delay_L.Set_delay_central_value(Delay_values.samples);
            }
            AudioInterrupts();

            Display.D_delay();
        }

        // Change value_LR
        if (Read_encoder(14, Delay_data.samples_LR, 10, -10, 1))
        {
            Delay_values.samples_LR = Calc_delay_samples_LR(Delay_data.samples_LR);

            AudioNoInterrupts();
            if (Delay_values.samples_LR >= 0) // Left channel
            {
                Delay_L.Set_delay_central_value(Delay_values.samples + Delay_values.samples_LR);
            }
            else
            {
                Delay_R.Set_delay_central_value(Delay_values.samples - Delay_values.samples_LR);
            }
            AudioInterrupts();

            Display.D_delay_LR();
        }

        // Change delay_modulation_type
        if (Read_encoder(19, Delay_data.modulation_source, 2, 0, 1))
        {
            Delay_values.modulation_source = Delay_data.modulation_source;

            AudioNoInterrupts();
            Delay_L.Set_delay_modulation_source(Delay_values.modulation_source); // Left channel
            Delay_R.Set_delay_modulation_source(Delay_values.modulation_source); // Right channel
            AudioInterrupts();

            Display.D_modulation_type();
        }

        // Change delay_modulation_source = NONE
        if (Read_pushbutton(19))
        {
            Delay_data.modulation_source = 0;
            Delay_values.modulation_source = Delay_data.modulation_source;

            AudioNoInterrupts();
            Delay_L.Set_delay_modulation_source(Delay_values.modulation_source); // Left channel
            Delay_R.Set_delay_modulation_source(Delay_values.modulation_source); // Right channel
            AudioInterrupts();

            Display.D_modulation_type();
        }

        // Change depth (depth)
        if (Read_encoder(20, Delay_data.modulation_depth, 39, 0, 1))
        {
            Delay_values.modulation_depth = Calc_delay_depth(Delay_data.modulation_depth);

            AudioNoInterrupts();
            Delay_L.Set_delay_modulation_gain(Delay_values.modulation_depth);
            Delay_R.Set_delay_modulation_gain(Delay_values.modulation_depth);
            AudioInterrupts();

            Display.D_modulation_depth();
        }

        // Change delay_modulation_frequency
        if (Read_encoder(21, Delay_data.modulation_frequency, 90, 0, 1))
        {
            Delay_values.modulation_frequency = Calc_delay_frequency(Delay_data.modulation_frequency);

            AudioNoInterrupts();
            LFO_D[0].Set_frequency(Delay_values.modulation_frequency);
            LFO_D[1].Set_frequency(Delay_values.modulation_frequency);
            AudioInterrupts();

            Display.D_modulation_frequency();
        }

        // Change modulation phase_LR
        if (Read_encoder(22, Delay_data.modulation_phase_LR, 359, 0, 1))
        {
            Delay_values.modulation_phase_LR = Delay_data.modulation_phase_LR;

            AudioNoInterrupts();
            LFO_D[0].Set_phase(Delay_values.modulation_phase_LR);
            AudioInterrupts();

            Display.D_modulation_phase_LR();
        }

        // Configure Instrument routing
        if (!Read_pushbutton_fast(35))
        {
            if (Read_pushbutton(PB_number + 26))
            {
                if (Lilla_state_0 != LIVE_SAMPLING)
                {
                    Delay_values.instrument_route[PB_number] = !Delay_values.instrument_route[PB_number];
                    bitWrite(Delay_data.instrument_route, PB_number, (Delay_values.instrument_route[PB_number] ? 1 : 0));
                    delay_instrument_routing = PB_number; // indica un instrument se <=7; se 8 indica instrument 0 e 1

                    AudioNoInterrupts();
                    if (delay_instrument_routing <= 7)
                    {
                        Players_Manager.MX_multicast_change_routing(delay_instrument_routing);
                    }
                    else
                    {
                        Players_Manager.MX_multicast_change_routing(0);
                        Players_Manager.MX_multicast_change_routing(1);
                    }
                    AudioInterrupts();

                    Display.D_sounds();
                }
                else
                {
                    Delay_values.instrument_route[0] = !Delay_values.instrument_route[0];
                    Delay_values.instrument_route[1] = !Delay_values.instrument_route[1];
                    bitWrite(Delay_data.instrument_route, 0, (Delay_values.instrument_route[0] ? 1 : 0));
                    bitWrite(Delay_data.instrument_route, 1, (Delay_values.instrument_route[1] ? 1 : 0));

                    delay_instrument_routing = 10; // indica un instrument se <=7; se 10 indica instrument 0 e 1

                    AudioNoInterrupts();
                    if (delay_instrument_routing <= 7)
                    {
                        Players_Manager.MX_multicast_change_routing(delay_instrument_routing);
                    }
                    else
                    {
                        Players_Manager.MX_multicast_change_routing(0);
                        Players_Manager.MX_multicast_change_routing(1);
                    }
                    AudioInterrupts();

                    Display.D_sounds();
                }
            }
        }

        // Switch
        if (Read_pushbutton_fast(35))
        {
            // Switch to PERFORMANCE
            if (Read_pushbutton(26))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:

                    // Salva su EEPROM
                    Archive.Save_Delay_to_Eeprom(Delay_data);

                    if (true)
                    {
                        Serial.println();
                        Serial.println(F("main() - Delay_data in RAM:"));
                        Print_Delay_data(Delay_data);
                        Serial.println(F("... has been saved in EEPROM."));
                    }

                    // Save Delay_data in delay_<patch_id>.txt in SD
                    Archive.Copy_patch_Delay_data_from_RAM_to_SD(Patch_id);

                    Golive_with_PERFORMANCE(Patch_id);
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_PERFORMANCE();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_PERFORMANCE();
                    break;

                case MIDI_LOOP:
                    // Patch delay
                    Archive.Copy_patch_Delay_data_from_RAM_to_SD(Patch_id);

                    Switch_from_MIDI_LOOP_to_PERFORMANCE();
                    break;

                default:
                    Serial.println("Switch MISSING! 3673");
                    break;
                }
            }

            // Switch to MIXER
            else if (Read_pushbutton(27))
            {
                Archive.Save_Delay_to_Eeprom(Delay_data);
                Switch_to_MIXER();
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                Archive.Save_Delay_to_Eeprom(Delay_data);
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_LIVE_SAMPLING();
                    break;

                case LIVE_SAMPLING:
                    LS_refresh_LS_page();
                    break;

                case MIDI_LOOP:
                    // Esci da MIDI_LOOP

                    AudioNoInterrupts();
                    // Ferma i track running
                    for (auto local_track = 0; local_track < TRACKS; ++local_track) // true --> il track va suonato
                    {
                        LOOP_track_run[local_track] = false;
                    }

                    // Ferma i Player dei track
                    Players_Manager.Release_all_players_loop();
                    AudioInterrupts();

                    Loop_led_set.Request_all_LED_switch_off();

                    Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
                    break;

                default:
                    Serial.println("Switch MISSING! 3722");
                    break;
                }
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                Archive.Save_Delay_to_Eeprom(Delay_data);
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_to_DIRECT_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    DS_refresh_DS_page();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING();
                    break;

                case MIDI_LOOP:
                    // Esci da MIDI_LOOP

                    AudioNoInterrupts();
                    // Ferma i track running
                    for (auto local_track = 0; local_track < TRACKS; ++local_track) // true --> il track va suonato
                    {
                        LOOP_track_run[local_track] = false;
                    }

                    // Ferma i Player dei loop
                    Players_Manager.Release_all_players_loop();
                    AudioInterrupts();

                    Loop_led_set.Request_all_LED_switch_off();

                    Switch_to_DIRECT_SAMPLING();
                    break;

                default:
                    Serial.println("Switch MISSING! 3764");
                    break;
                }
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Archive.Save_Delay_to_Eeprom(Delay_data);
                Golive_MIDI_MONITOR();
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                Archive.Save_Delay_to_Eeprom(Delay_data);
                Golive_with_MIDI_LOOP(false);
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Archive.Save_Delay_to_Eeprom(Delay_data);
                Golive_SETUP();
            }
        }
    }

#pragma endregion // DELAY_SETTINGS

#pragma region Live Sampler [rgba(244, 229, 26, 0.19)]
    // *************************************************************
    // ********************   LIVE SAMPLING  ***********************
    // *************************************************************
    if (Lilla_state == LIVE_SAMPLING)
    {

        /*

        Live Sampling (LIVE SAMPLER) consente la registrazione sia Mono che Stereo. Prevede l'uso della Patch PATCHES_MAX.

        Se la registrazione è mono, PATCHES_MAX comprende 1 Instrument e il Sound SOUNDS_MAX:
        - Patch[PATCHES_MAX].Instrument[0].id.sound == PATCHES_MAX

        L'Instrument ha:
        from_note = 0
        to_note = 127
        root_key = 60
        midi_ch = 0 (midi channel 1)

        Il Sound è associato al file Mono.liv:
        Sound[SOUNDS_MAX].file = FIRST_LIVE_SAMPLING_FILE;


        Se la registrazione è stereo, PATCHES_MAX comprende 2 Instrument, i Sound SOUNDS_MAX e (SOUNDS_MAX + 1):
        - Patch[PATCHES_MAX].Instrument[0].id.sound == SOUNDS_MAX --> associato a ch. Left
        - Patch[PATCHES_MAX].Instrument[1].id.sound == SOUNDS_MAX + 1 --> associato a ch. Right

        Entrambi gli Instrument hanno:
        from_note = 0
        to_note = 127
        root_key = 60
        midi_ch = 0 (midi channel 1)


        I due Sound sono associati ai file .liv:
        Sound[SOUNDS_MAX].file = FIRST_LIVE_SAMPLING_FILE + 1 (Left.liv)
        Sound[SOUNDS_MAX + 1].file = FIRST_LIVE_SAMPLING_FILE + 2 (Right.liv)

        Fisicamente, i campioni sono salvati su due array "virtual tape" (int16_t LS_buffer_L e int16_t LS_buffer_R) istanziati dinamicamente
        nei PSRAM chip; entrambi gli array comprendono LS_buffer_dim campioni, con indirizzo da 0 a (LS_buffer_dim -1):
        0......................................................................................(LS_buffer_dim -1)

        L'ultimo campione scritto e' Q_sample; esempio di prima scrittura del buffer:
                                                                         Q_sample
        0>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>..................(LS_buffer_dim -1)

        La porzione di LS_buffer_L/R visualizzata (window) mostra la waveform registrata; con riferimento agli elementi di LS_buffer_L
        i samples visualizzati vanno da LS_window_A_sample a LS_window_B_sample; l'ampiezza della window e' LS_window_width.


        LS_X_sample - LS_Y_sample è l'intervallo di esecuzione:
        - FWD e REV : parte da LS_X_sample
        - Loop FWD e Loop FWD/REV : da LS_X_sample a LS_Y_sample.


        LS_X_sample e' sempre al centro della window; al primo accesso a LIVE SAMPLER è sul sample 0:
        .................................(LS_X_sample).........................................(LS_buffer_dim -1)
                  (LS_window_A_sample)+++++++++|+++++++(LS_window_B_sample)

        LS_Y_sample e' definito come LS_X_sample + LS_XY_delta; LS_Y_sample puo' essere anche esterno alla window:

        .................................(LS_X_sample)..........(LS_Y_sample)..................(LS_buffer_dim -1)
                  (LS_window_A_sample)+++++++++|+++++++(LS_window_B_sample)

        VINCOLI:
        - 0 <= LS_window_A_sample <= (LS_buffer_dim -1)
        - LS_window_A_sample < LS_window_B_sample (LS_buffer_L/R viene comunque letto come un ring tape)
        - 0 <= LS_X_sample <= (LS_buffer_dim -1)
        - LS_Y_sample > LS_X_sample (da verificare)
        - LS_XY_delta spazia da valori negativi a positivi (per consentire qualsiasi posizionamento a LS_Y_sample): -(LS_buffer_dim -1) < LS_XY_delta < (LS_buffer_dim -1)

        Live Sampler prevede che i punti di lettura LS_X_sample/LS_Y_sample possano essere:
        1) fissi su un punto dell'array (se fosse un tape sono solidali al tape, solidali ai campioni registrati): LS_XY_lock == true.
        2) spostarsi lungo l'array (se fosse un tape sono solidali con la testa di registrazione, i campioni sottostanti cambiano con continuita'): LS_XY_lock == false

        In entrambi i casi, con il NoteOn le posizioni di partenza (modi FWD e REV) e di arrivo (modi loop FWD, loop FWD/REV) sono fissate sul buffer. Importante notare
        che nel modo loop il suono può cambiare appena il segmento X-Y viene riscritto.


        Calcolo degli estremi della window:
        LS_window_A_sample = LS_X_sample - (LS_window_width - 1)/2 (scalata poi ad un valore positivo: 0 <= LS_window_A_sample <= (LS_buffer_dim -1))
        LS_window_B_sample = LS_window_A_sample + LS_window_width - 1

        1) Caso LS_XY_lock == true
        Se LS_XY_lock == true, la waveform cresce verso DESTRA (ultimo campione a DESTRA).
        LS_X_sample e' fisso su un elemento (campione) dell'array scelto a piacere:
        0 <= LS_X_sample <= (LS_buffer_dim -1)
        LS_Y_sample = LS_X_sample + LS_XY_delta

        2) LS_XY_lock == false
        Se LS_XY_lock == false, la waveform cresce verso SINISTRA (primo campione a SINISTRA).
        Tutti i valori operativi sono in movimento, LS_X_sample e LS_Y_sample vanno continuamente aggiornati:
        LS_X_sample = LS_Q_sample + LS_X_delta (poi scalato con 0 <= LS_X_sample <= (LS_buffer_dim -1)
        LS_Y_sample = LS_X_sample + LS_XY_delta

        Gli step di avanzamento di:
            LS_window_width
            LS_X_sample
            LS_X_delta
        sono sempre PROPORZIONALI a LS_window_width.<<
        */

        // Change play MODE
        if (Read_encoder(1, LS_mode, 3, 0, 1))
        {
            AudioNoInterrupts();
            Sound[SOUNDS_MAX].mode = LS_mode;
            Players_Manager.Update_Preset_mode(Patch_id, 0);
            Players_Manager.Multicast_main_settings_editing(Patch_id, 0);
            if (LS_stereo)
            {
                Sound[SOUNDS_MAX + 1].mode = LS_mode;
                Players_Manager.Update_Preset_mode(Patch_id, 1);
                Players_Manager.Multicast_main_settings_editing(Patch_id, 1);
            }
            AudioInterrupts();

            Display.Show_LS_mode();
            Display.LS_loop_time();
            if (LS_state != 1)
            {
                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }
                Display.Show_LS_ring_tape_wave(LS_id_sound);
            }
        }

        // Change volume_patch
        if (Read_encoder(15, volume_patch, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
            Players_Manager.Broadcast_volume();
            AudioInterrupts();
            Display.LS_volume();
        }

        // Change window WIDTH
        result = Read_encoder_simple(9);
        if (result != 0)
        {
            Info.LS_restart_antiflicker();
            if (result == 1)
            {
                LS_window_width -= LS_window_width / 8;
            }
            else
            {
                LS_window_width += LS_window_width / 8;
            }

            LS_window_width = constrain(LS_window_width, 20 * AUDIO_BLOCK_SAMPLES, LS_buffer_dim); // constrain(LS_window_width, 200 * AUDIO_BLOCK_SAMPLES, LS_buffer_dim);
            if (LS_state != 1)
            {
                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }
                Display.Show_LS_ring_tape_wave(LS_id_sound);
            }
            LS_X_step = LS_window_width / LS_COMB;
            Display.Show_LS_X_step();
        }

        // Se window_width to "ALL TAPE"
        if (Read_pushbutton(9))
        {
            Info.LS_restart_antiflicker();
            LS_window_width = LS_buffer_dim;
            if (LS_state != 1)
            {
                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }
                Display.Show_LS_ring_tape_wave(LS_id_sound);
            }
            LS_X_step = LS_window_width / LS_COMB;
            Display.Show_LS_X_step();
        }

        // Change LS_X_sample o LS_X_delta
        result = Read_encoder_simple(10);
        if (result != 0)
        {
            // si usa LS_X_sample
            if (LS_XY_lock)
            {
                if (result == 1)
                {
                    LS_X_sample += LS_X_step;
                }
                else
                {
                    LS_X_sample -= LS_X_step;
                }

                LS_X_sample = LS_constrain_position(LS_X_sample);
                Display.LS_X_sample_delta();

                Serial.print(F("LS_X_sample: "));
                Serial.println(LS_X_sample);

                LS_Y_sample = LS_X_sample + LS_XY_delta;
                Serial.print(F("LS_Y_sample: "));
                Serial.println(LS_Y_sample);
            }

            // si usa LS_X_delta
            else
            {
                if (result == 1)
                {
                    LS_X_delta += LS_X_step;
                }
                else
                {
                    LS_X_delta -= LS_X_step;
                }

                LS_X_delta = LS_constrain_position(LS_X_delta);
                Display.LS_X_sample_delta();

                Serial.print(F("LS_X_delta: "));
                Serial.println(LS_X_delta);
            }

            if (LS_mode > 1)
            {
                AudioNoInterrupts();
                Players_Manager.Multicast_main_settings_editing(Patch_id, 0);
                if (LS_stereo)
                {
                    Players_Manager.Multicast_main_settings_editing(Patch_id, 1);
                }
                AudioInterrupts();
            }

            if (LS_state != 1)
            {
                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }
                Display.Show_LS_ring_tape_wave(LS_id_sound);
            }
        }

        // toggle LS_XY_lock/!LS_XY_lock
        if (Read_pushbutton(10))
        {
            if (LS_XY_lock)
            {
                // passando a LS_XY_lock non si deve riassegnare
                AudioNoInterrupts();
                Players_Manager.Stop_all_players();
                AudioInterrupts();

                LS_XY_lock = false;
                LS_X_delta = 0;
                Info.LS_restart_antiflicker();
            }
            else
            {
                // passando a !LS_XY_lock si deve riassegnare LS_X_sample
                LS_lock_X_sample();
                Serial.print(F("LS_X_sample "));
                Serial.println(LS_X_sample);
            }

            Display.LS_X_sample_delta();
            Serial.print(F("LS_XY_lock: "));
            Serial.println(LS_XY_lock);

            if (LS_state != 1)
            {
                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }
                Display.Show_LS_ring_tape_wave(LS_id_sound);
            }
        }

        // Change "Loop Width" (LS_XY_delta)
        result = Read_encoder_simple(11);
        if (result != 0)
        {
            if (result == 1)
            {
                LS_XY_delta += LS_X_step;
            }
            else
            {
                LS_XY_delta -= LS_X_step;
            }

            LS_XY_delta = constrain(LS_XY_delta, LS_XY_DELTA_MIN, LS_buffer_dim - 1);

            AudioNoInterrupts();
            Players_Manager.Multicast_main_settings_editing(Patch_id, 0);
            if (LS_stereo)
            {
                Players_Manager.Multicast_main_settings_editing(Patch_id, 1);
            }
            AudioInterrupts();

            if (LS_XY_lock)
            {
                LS_Y_sample = LS_X_sample + LS_XY_delta;
            }
            Display.LS_loop_time();

            Serial.print(F("LS_XY_delta: "));
            Serial.println(LS_XY_delta);

            if (LS_state != 1)
            {
                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }
                Display.Show_LS_ring_tape_wave(LS_id_sound);
            }
        }

        // Change "Step" (LS_X_step)
        result = Read_encoder_simple(12);
        if (result != 0)
        {
            if (result == 1)
            {
                LS_COMB = LS_COMB / 2;
            }

            else // Aumenta LS_X_step
            {
                LS_COMB = 2 * LS_COMB;
            }

            LS_COMB = constrain(LS_COMB, 8, 1024);
            LS_X_step = LS_window_width / LS_COMB;
            Display.Show_LS_X_step();
        }

        // change Feedback level
        if (Read_encoder(17, LS_feedback, 8, 0, 1))
        {
            Serial.println(LS_fbk_table[LS_feedback]);

            AudioNoInterrupts();
            LS_Feedback_L.value(LS_fbk_table[LS_feedback]);
            LS_Feedback_R.value(LS_fbk_table[LS_feedback]);
            AudioInterrupts();

            Display.Show_LS_feedback();
        }

// change MENU item
#ifdef PCB_2022
        if (Read_encoder_inverse(25, LS_menu, LS_menu_max, 0, 1))
#else
        if (Read_encoder(25, LS_menu, LS_menu_max, 0, 1))
#endif
        {
            Serial.print(F("LS_menu: "));
            Serial.println(LS_menu);
            Display.Frame_LS_menu(LS_menu);
            choice_LS_menu = element_Menu_LS[LS_menu];
        }

        // choose menu item
        if (Read_pushbutton(25))
        {
            switch (choice_LS_menu) // {"Exit"}, {"Open"}, {"Close"}}
            {
            case 0: // Rec
                LS_state = 1;

                LS_define_model();
                Display.Show_LS_menu(); // displays the menu and updates "Value_Max_encoder.LS_menu" used by encoder_menu
                LS_menu = 0;
                Display.Frame_LS_menu(LS_menu);
                choice_LS_menu = element_Menu_LS[LS_menu];
                LiveSampler.Start(LS_stereo);
                LS_wave_refresh_timer = 0;
                delay(10);
                break;

            case 1: // Stop
                LS_state = 2;
                LiveSampler.Stop();

                LS_define_model();
                Display.Show_LS_menu(); // displays the menu and updates "Value_Max_encoder.LS_menu" used by encoder_menu
                LS_menu = 0;
                Display.Frame_LS_menu(LS_menu);
                choice_LS_menu = element_Menu_LS[LS_menu];
                delay(20);

                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }

                Display.Show_LS_ring_tape_wave(LS_id_sound);
                break;

            case 2:                 // Toggle Mono/Stereo
                Midi_reader.Stop(); // NON sostituire con AudioNoInterrupts!

                LS_stereo = !LS_stereo;
                LS_buffer_dim = (LS_stereo ? LS_STEREO_SAMPLES : LS_MONO_SAMPLES);
                LS_window_width = LS_buffer_dim;
                LS_window_step = LS_window_width / 8;
                LS_Setup_buffers(LS_stereo, false); // LS_Setup_buffers(bool stereo, bool first)
                LS_setup_LS_Patch(LS_stereo);
                Update_all_maps_Instrument_for_notes();

                AudioNoInterrupts();
                Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
                AudioInterrupts();

                P_Patch(Patch_id);
                LS_id_sound = SOUNDS_MAX; // mostra sempre il primo Sound
                LS_instrument = 0;
                LS_X_delta = 0;
                LS_X_sample = 0;
                LS_XY_delta = 44100;
                LS_Y_sample = LS_X_sample + LS_XY_delta;
                LS_X_step = LS_window_width / LS_COMB;
                LS_menu = 0;
                LS_refresh_LS_page();

                Midi_reader.Start();
                break;

            case 3: // Erase
                AudioNoInterrupts();
                Players_Manager.Stop_all_players();
                AudioInterrupts();

                if (LS_stereo)
                {
                    LS_erase_FIFO_array(LS_buffer_L_ptr, LS_stereo); // LS_erase_FIFO_array(const int16_t* Array, int stereo)
                    LS_erase_FIFO_array(LS_buffer_R_ptr, LS_stereo); // LS_erase_FIFO_array(const int16_t* Array, int stereo)
                }
                else
                    LS_erase_FIFO_array(LS_buffer_mono_ptr, LS_stereo); // LS_erase_FIFO_array(const int16_t* Array, int stereo)
                Serial.println("Live Samplier buffer(s) erased!");

                LiveSampler.Reset(); // reset Q_sample and P_sample
                LS_state = 0;
                LS_id_sound = SOUNDS_MAX; // mostra sempre il primo Sound
                LS_instrument = 0;
                LS_window_width = LS_buffer_dim; // LS_window_width = 441001;
                LS_window_step = LS_window_width / 8;
                LS_X_sample = 0;
                LS_X_delta = 0;
                LS_XY_delta = 44100;
                LS_Y_sample = LS_X_sample + LS_XY_delta;
                LS_X_step = LS_window_width / LS_COMB;

                LS_menu = 0;
                Display.LS_page();

                // restore all LED
                Performance_led_set.Restore_all_LED();

                LS_define_model();
                Display.Show_LS_menu();
                Display.Frame_LS_menu(LS_menu);
                choice_LS_menu = element_Menu_LS[LS_menu];

                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }

                Display.Show_LS_ring_tape_wave(LS_id_sound);
                break;

            default:
                Serial.println("Switch MISSING! 4250");
                break;
            }
        }

        // Update wave
        if (LS_state == 1) // Open
        {
            if (LS_wave_refresh_timer >= LS_REFRESH) // ms
            {
                LS_wave_refresh_timer = 0;

                if (!LS_XY_lock)
                {
                    LS_update_both_X_Y_samples();
                }
                else // altrimenti e' gia' stato calcolato
                {
                    LS_update_Q_sample();
                }

                Display.Show_LS_ring_tape_wave(LS_id_sound);
            }
        }

        if (!Read_pushbutton_fast(35))
        {
            //  toggle wave Left/Right and LPF
            if (LS_stereo)
            {
                // Display Left wave or LPF
                if (Read_pushbutton(26))
                {
                    if (LS_instrument == 1) // Right
                    {
                        LS_instrument = 0;        // Left
                        LS_id_sound = SOUNDS_MAX; // Left
                        if (LS_state != 1)
                        {
                            if (!LS_XY_lock)
                            {
                                LS_update_both_X_Y_samples();
                            }
                            else // altrimenti e' gia' stato calcolato
                            {
                                LS_update_Q_sample();
                            }

                            Display.Show_LS_ring_tape_wave(LS_id_sound);
                        }
                    }

                    else
                    {
                        Instrument_id = 0;
                        Sound_id = SOUNDS_MAX;
                        Lilla_state_0 = LIVE_SAMPLING;
                        Lilla_state = INSTRUMENT_VCF;

                        Display.Instrument_VCF_page(Patch_id, Instrument_id);

                        // restore all LED
                        Performance_led_set.Restore_all_LED();
                    }
                }

                // Display Right wave or LPF
                if (Read_pushbutton(27))
                {
                    if (LS_instrument == 0) // Left
                    {
                        LS_instrument = 1;            // Right
                        LS_id_sound = SOUNDS_MAX + 1; // Right
                        if (LS_state != 1)
                        {
                            if (!LS_XY_lock)
                            {
                                LS_update_both_X_Y_samples();
                            }
                            else // altrimenti e' gia' stato calcolato
                            {
                                LS_update_Q_sample();
                            }
                            Display.Show_LS_ring_tape_wave(LS_id_sound);
                        }
                    }

                    else
                    {
                        Instrument_id = 1;
                        Sound_id = SOUNDS_MAX + 1;
                        Lilla_state_0 = LIVE_SAMPLING;
                        Lilla_state = INSTRUMENT_VCF;

                        Display.Instrument_VCF_page(Patch_id, Instrument_id);

                        // restore all LED
                        Performance_led_set.Restore_all_LED();
                    }
                }
            }

            else
            {
                if (Read_pushbutton(26))
                {
                    Instrument_id = 0;
                    Sound_id = SOUNDS_MAX;
                    Lilla_state_0 = LIVE_SAMPLING;
                    Lilla_state = INSTRUMENT_VCF;

                    Display.Instrument_VCF_page(Patch_id, Instrument_id);

                    // restore all LED
                    Performance_led_set.Restore_all_LED();
                }
            }
        }

        else
        {
            // Switch to PERFORMANCE
            if (Read_pushbutton(26))
            {
                Switch_from_LIVE_SAMPLING_to_PERFORMANCE();
            }

            // Switch to Mixer
            else if (Read_pushbutton(27))
            {
                Lilla_state_0 = LIVE_SAMPLING;
                Switch_to_MIXER();
            }

            // Switch to DELAY
            else if (Read_pushbutton(28))
            {
                Switch_from_LIVE_SAMPLING_to_DELAY(); // setta anche: Lilla_state_0 = LIVE_SAMPLING;
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING();
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Lilla_state_0 = LIVE_SAMPLING;
                Golive_MIDI_MONITOR();
            }

            // Switch to MIDI_LOOP
            if (Read_pushbutton(32))
            {
                Switch_from_LIVE_SAMPLING_to_MIDI_LOOP();
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Lilla_state_0 = LIVE_SAMPLING;
                Golive_SETUP();
            }
        }
    }

#pragma endregion // LIVE_SAMPLING

#pragma region Direct Sampler [rgba(179, 179, 179, 0.32)]
    // *************************************************************
    // ********************  DIRECT SAMPLING  **********************
    // *************************************************************

    /*

    Direct Sampling (SAMPLER) consente la registrazione sia Mono che Stereo. Prevede l'uso della Patch PATCHES_MAX, dei Sound SOUNDS_MAX e (SOUNDS_MAX + 1) e di 2 Instrument:
    - Patch[PATCHES_MAX].Instrument[0].id.sound == SOUNDS_MAX --> associato a ch. Left oppure Mono
    - Patch[PATCHES_MAX].Instrument[1].id.sound == SOUNDS_MAX + 1 --> associato a ch. Right

    Entrambi gli instrument hanno:
    from_note = 0
    to_note = 127
    root_key = 60
    midi_ch = 0 (midi channel 1)

    Se la registrazione è stereo, i due Sound sono associati a due distinti file .rec consecutivi; se la registrazione è mono i due Sound sono associati allo stsso file .rec.

    */

    if (Lilla_state == DIRECT_SAMPLING)
    {
        // Change volume_patch
        if (DS_state == 0 && Read_encoder(15, volume_patch, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
            Players_Manager.Broadcast_volume();
            AudioInterrupts();

            Display.DS_update_volume();
        }

        // Change gain
        if (Read_encoder(4, DS_gain, 40, 1, 1))
        {
            LINE_IN_amplifier.Set_gain(Volume_float[DS_gain]);
            Display.DS_show_gain();
        }

        // Update bar_displays
        if (DS_state == 0 || DS_state == 1 || DS_state == 2)
        {
            float val;
            if (PeakTracking_L.available())
            {
                val = 20 * log10(PeakTracking_L.read()); // 0 <= PeakTracking_L.read() <= 1.0 ; -inf < val < 0
                Display.DS_bar(0, BAR_ELEMENTS + val);   // Display.DS_bar(0, PeakTracking_L.read() * BAR_ELEMENTS);
            }
            if (PeakTracking_R.available())
            {
                val = 20 * log10(PeakTracking_R.read());
                Display.DS_bar(1, BAR_ELEMENTS + val); // Display.DS_bar(1, PeakTracking_R.read() * BAR_ELEMENTS);
            }
        }

        if (DS_state == 2)
        {
            // Update blinking REC
            if (DS_blink_timer >= 500)
            {
                DS_blink_ON = !DS_blink_ON;
                Display.DS_sampler_txt(DS_blink_ON);
                DS_blink_timer = 0;
            }

            // Update seconds and free-memory
            if (DS_recording_time_update >= 200)
            {
                DS_recording_time_update = 0;
                Display.DS_update_recording_seconds(DS_recording_time);
                Display.DS_available_memory();
            }

            // Stop if SteroSampler has stopped
            if (!DirectSampler.Is_recording())
            {
                DS_state = 0;
                DS_menu = 0;
                // switch OFF Audio Input monitor
                MAIN_mixer_out_L.gain(1, 0.0);
                MAIN_mixer_out_R.gain(1, 0.0);

                if (Recording[recording].packets == 0)
                {
                    // Start from first recording existing
                    recording = DS_get_next_Recording(-1);
                }

                else
                {
                    Recording[recording].consistent = true;
                    // consistent Recording must be saved
                    Archive.Save_DS_Recording(recording);
                    DS_read_Recording(recording); // only to update .bytes and .seconds
                }

                DS_update_recordings();
                // Switch off blinking REC
                DS_blink_ON = false;

                DS_define_model();
                Display.DS_menu();
                Display.DS_frame_menu(DS_menu);
                Display.DS_available_memory();

                Display.DS_line_out(false);
                Display.DS_sampler_frame(true);
                Display.DS_sampler_txt(false);

                Print_VFS_FAT_table();
                P_Recording(recording);

                Jump_to_DIRECT_SAMPLING_recording(recording);
            }
        }

        // Change recording
        if (DS_state == 0)
        {
            result = Read_encoder_simple(23);
            if (result != 0)
            {
                recording_change = recording;
                if (result == +1)
                {
                    recording_change = DS_get_next_Recording(recording);
                }
                else
                {
                    recording_change = DS_get_previous_Recording(recording);
                }

                if (recording_change != recording)
                {
                    AudioNoInterrupts();
                    Players_Manager.Stop_all_players();
                    AudioInterrupts();

                    recording = recording_change;
                    Jump_to_DIRECT_SAMPLING_recording(recording);
                    P_Recording(recording);
                }
            }
        }

// Change menu item
#ifdef PCB_2022
        if (Read_encoder_inverse(25, DS_menu, DS_menu_max, 0, 1))
#else
        if (Read_encoder(25, DS_menu, DS_menu_max, 0, 1))
#endif
        {
            Display.DS_frame_menu(DS_menu);
        }

        // Choose menu item
        if (Read_pushbutton(25))
        {
            int first_packet_L = 0;
            int packets_per_channel = 0;
            int last_packet_L = 0;
            int first_packet_R = 0;

            switch (choice_DS_menu) // {"Delete"}, {"Pause+Rec"}, {"Mono Rec"}, {"Stereo Rec"}, {"Stop"}
            {
            case 0: // Delete
                AudioNoInterrupts();
                Players_Manager.Stop_all_players();
                AudioInterrupts();

                Display.DS_hide_recording();
                Display.DS_advice_delete(true);

                // Delete recording
                Recording[recording].consistent = false;
                Clean_up_VFS();
                Defragment_VFS();
                DS_update_recordings();
                Print_VFS_FAT_table();

                // restart from first recording (if exist)
                recording = DS_get_next_Recording(-1);
                DS_back_to_first_DS_Recording();
                Display.DS_available_memory();
                break;

            case 1: // Pause+Rec (pause before recording, listening Audio Input)
                DS_state = 1;
                DS_menu = 0;

                AudioNoInterrupts();
                Midi_reader.Stop();
                Players_Manager.Stop_all_players();
                AudioInterrupts();

                Display.DS_update_volume(false); // cambia il colore del volume in bianco (fisso)

                recording = DS_find_Recording_free();
                Serial.println(F("*** Pause + Record: listen to Audio Input ***"));
                Serial.print(F("**** Prossimo recording: "));
                Serial.println(recording);

                // hide last recording data
                Display.DS_hide_recording();

                // switch on Line OUT monitor
                MAIN_mixer_out_L.gain(1, 1.0);
                MAIN_mixer_out_R.gain(1, 1.0);

                DS_define_model();
                Display.DS_menu();
                Display.DS_frame_menu(DS_menu);
                Display.DS_line_out(true);
                break;

            case 2: // Mono Rec
                DS_state = 2;
                DS_menu = 0;

                Recording[recording].stereo = false;
                Recording[recording].consistent = false;

                // packet:  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
                // free:    * * * * 1 2 3 4 5 6 7  8  9  10 11 12 13 14 15
                // result:  * * * * L L L L L L L  L  L  L  L  L  L  L  L

                packets_per_channel = Get_packets_free();                 // 15
                first_packet_L = Get_first_packet_free();                 // 4
                last_packet_L = first_packet_L + packets_per_channel - 1; // 4 + 15 - 1 = 18

                Serial.println(F("*** Start MONO Sampling! *** "));
                Serial.print(F("Mono recording from packet: "));
                Serial.print(first_packet_L);
                Serial.print(F("  up to packet: "));
                Serial.println(last_packet_L);

                DS_define_model();
                Display.DS_menu();
                Display.DS_frame_menu(DS_menu);
                Display.DS_Recording_description(recording, false);
                Display.DS_sampler_txt(true);

                DS_blink_timer = 0;
                DS_blink_ON = true;

                DirectSampler.Start(first_packet_L, last_packet_L, recording, Recording[recording].stereo); // bool start(int from_packet, int last_packet, int recording_id_in, bool stereo_in)
                DS_recording_time = 0;
                DS_recording_time_update = 0;
                break;

            case 3: // Stereo Rec
                DS_state = 2;
                DS_menu = 0;

                Recording[recording].stereo = true;
                Recording[recording].consistent = false;

                // packet:  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
                // free:    * * * * 1 2 3 4 5 6 7  8  9  10 11 12 13 14 15
                // result:  * * * * L R L R L R L  R  L  R  L  R  L  R  _

                packets_per_channel = Get_packets_free() / 2;                   // (15/2) = 7
                first_packet_L = Get_first_packet_free();                       // 4
                last_packet_L = first_packet_L + 2 * (packets_per_channel - 1); // 16
                first_packet_R = first_packet_L + 1;

                Serial.println(F("*** Start STEREO Sampling! *** "));
                Serial.print(F("Left recording from packet: "));
                Serial.print(first_packet_L);
                Serial.print(F("  Right recording from packet: "));
                Serial.println(first_packet_R);

                DS_define_model();
                Display.DS_menu();
                Display.DS_frame_menu(DS_menu);
                Display.DS_Recording_description(recording, false);
                Display.DS_sampler_txt(true);

                DS_blink_timer = 0;
                DS_blink_ON = true;

                DirectSampler.Start(first_packet_L, last_packet_L, recording, Recording[recording].stereo); // bool start(int from_packet, int last_packet, int recording_id_in, bool stereo_in)
                DS_recording_time = 0;
                DS_recording_time_update = 0;
                break;

            case 4: // Stop
                Serial.println(F("*** Pause+Recording or Recording STOPPED! *** "));
                if (DS_state == 2)
                {
                    DirectSampler.Book_stop();
                }
                DS_state = 0;
                DS_menu = 0;

                // switch OFF Line OUT monitor
                MAIN_mixer_out_L.gain(1, 0.0);
                MAIN_mixer_out_R.gain(1, 0.0);

                if (Recording[recording].packets == 0)
                {
                    Serial.print(F("Recording: "));
                    Serial.print(recording);
                    Serial.println(F(" cancelled."));
                    recording = DS_get_next_Recording(-1);
                }

                else
                {
                    Recording[recording].consistent = true;
                    // consistent Recording must be saved
                    Archive.Save_DS_Recording(recording);
                    DS_read_Recording(recording); // only to update .bytes and .seconds
                }

                DS_update_recordings();
                // Switch off blinking REC
                DS_blink_ON = false;

                DS_define_model();
                Display.DS_menu();
                Display.DS_frame_menu(DS_menu);
                Display.DS_available_memory();
                Display.DS_line_out(false);
                Display.DS_sampler_frame(true);
                Display.DS_sampler_txt(false);

                // Print_VFS_FAT_table();
                P_Recording(recording);

                Jump_to_DIRECT_SAMPLING_recording(recording);
                Midi_reader.Start();
                break;

            case 5: // CONVERT_REC_TO_RAW
            {
                DS_state = 3;

                AudioNoInterrupts();
                Players_Manager.Stop_all_players();
                AudioInterrupts();

                confirmation = false; // no action
                int file_L_RAW = -1;
                int file_R_RAW = -1;
                int blocks_per_file = ceil(Recording[recording].bytes / 256.0f); // quanti block compongono il file

                if (!Recording[recording].stereo)
                {
                    if ((Get_flash_size() - Get_flash_occupation()) >= Recording[recording].bytes)
                    {
                        DS_export = -1; // no filename available;
                        for (auto i = 0; i < FIRST_RECORDING_FILE; ++i)
                        {
                            if (!SerialFlash.exists(name_file[i]))
                            {
                                file_L_RAW = i;
                                DS_export = 1;
                                break;
                            }
                        }
                    }
                    else
                        DS_export = 0; // no space available
                }

                else
                {
                    if ((Get_flash_size() - Get_flash_occupation()) >= (2 * Recording[recording].bytes))
                    {
                        DS_export = -1; // no filename available;
                        for (auto i = 0; i < FIRST_RECORDING_FILE; ++i)
                        {
                            if (!SerialFlash.exists(name_file[i]))
                            {
                                file_L_RAW = i;
                                DS_export = 1;
                                break;
                            }
                        }
                        if (DS_export == 1)
                        {
                            for (auto i = file_L_RAW + 1; i < FIRST_RECORDING_FILE; ++i)
                            {
                                if (!SerialFlash.exists(name_file[i]))
                                {
                                    file_R_RAW = i;
                                    DS_export = 2;
                                    break;
                                }
                            }
                        }
                    }
                    else if ((Get_flash_size() - Get_flash_occupation()) >= Recording[recording].bytes)
                    {
                        DS_export = -1; // no filename available;
                        for (auto i = 0; i < FIRST_RECORDING_FILE; ++i)
                        {
                            if (!SerialFlash.exists(name_file[i]))
                            {
                                file_L_RAW = i;
                                DS_export = 1;
                                break;
                            }
                        }
                    }
                    else
                        DS_export = 0; // no space available
                }

                Serial.print("DS_export ");
                Serial.println(DS_export);
                Serial.print("file_L_RAW proposto ");
                Serial.println(file_L_RAW);
                Serial.print("file_R_RAW proposto ");
                Serial.println(file_R_RAW);
                Serial.println();

                if (DS_export <= 0)
                {
                    Display.DS_hide_recording();
                    Display.DS_advice_no_conversion(DS_export, true);
                    delay(7000);
                    Display.DS_advice_no_conversion(DS_export, false);

                    DS_state = 0;
                    DS_define_model();
                    Display.DS_menu();
                    Display.DS_frame_menu(DS_menu);

                    Display.DS_Recording_description(recording, true);

                    // restore LED
                    Performance_led_set.Restore_all_LED();

                    break;
                }

                DS_menu = 0;
                DS_define_model();
                Display.DS_menu();
                Display.DS_frame_menu(DS_menu);
                Display.DS_conversion_options(file_L_RAW, file_R_RAW, DS_export);

                // Choose what to do
                while (!confirmation)
                {
// move menu frame
#ifdef PCB_2022
                    if (Read_encoder_inverse(25, DS_menu, DS_menu_max, 0, 1))
#else
                    if (Read_encoder(25, DS_menu, DS_menu_max, 0, 1))
#endif
                    {
                        Display.DS_frame_menu(DS_menu);
                    }

                    // choose the action
                    if (Read_pushbutton(25))
                    {
                        confirmation = true;
                    }
                }

                switch (choice_DS_menu)
                {
                case 6: // Cancel (don't export)
                    Serial.println(F("Don't convert any file"));
                    break;

                case 7:                                                   // Convert Mono (file_L)
                    DS_convert_file_L(file_L_RAW, blocks_per_file * 256); // DS_convert_file_L(int file_L_RAW, int bytes)
                    // occorre rifare lo scan di tutti i file per compilare tutti i metadati del nuovo file, dirindex compreso
                    File_scanner.Read_all_file_data();
                    break;

                case 8:                                                   // Convert file_L
                    DS_convert_file_L(file_L_RAW, blocks_per_file * 256); // DS_convert_file_L(int file_L_RAW, int bytes)
                    // occorre rifare lo scan di tutti i file per compilare tutti i metadati del nuovo file, dirindex compreso
                    File_scanner.Read_all_file_data();
                    break;

                case 9:                                                   // Convert file_R
                    DS_convert_file_R(file_R_RAW, blocks_per_file * 256); // DS_convert_file_R(int file_R_RAW, int bytes)
                    // occorre rifare lo scan di tutti i file per compilare tutti i metadati del nuovo file, dirindex compreso
                    File_scanner.Read_all_file_data();
                    break;

                case 10:                                                  // Convert both file_L and file_R
                    DS_convert_file_L(file_L_RAW, blocks_per_file * 256); // DS_convert_file_L(int file_L_RAW, int bytes)
                    DS_convert_file_R(file_R_RAW, blocks_per_file * 256); // DS_convert_file_R(int file_R_RAW, int bytes)
                    // occorre rifare lo scan di tutti i file per compilare tutti i metadati del nuovo file, dirindex compreso
                    File_scanner.Read_all_file_data();
                    break;

                default:
                    Serial.println(F("Don't convert any file"));
                    break;
                }

                // Delete recording
                if (choice_DS_menu > 6)
                {
                    Recording[recording].consistent = false;
                    Clean_up_VFS();
                    Defragment_VFS();
                    DS_update_recordings();
                    Print_VFS_FAT_table();
                    // restart from first recording (if exist)
                    recording = DS_get_next_Recording(-1);
                    DS_back_to_first_DS_Recording();
                }

                Print_flash_file_list();

                // Return
                DS_state = 0;
                DS_menu = 0;
                Display.DS_page(Instrument_id, recording);
                DS_define_model();
                Display.DS_menu();
                Display.DS_frame_menu(DS_menu);
                // Switch bar_display ON
                PeakTracking_L.reset();
                PeakTracking_R.reset();
                Display.DS_bar(0, 0);
                Display.DS_bar(1, 0);
            }
            break;

            case 11: // EXPORT AS RAW TO SD
            {
                DS_state = 4;
                Sd2Card card;
                SdVolume volume;
                SdFile root;
                double SD_volumesize;
                double SD_occupied;

                confirmation = false; // no action
                const char DS_export_directory[] = "/LILLARAW_EXPORT/";
                String DS_export_M_RAW;
                String DS_export_L_RAW;
                String DS_export_R_RAW;

                // check SD presence
                if (!SD.begin(BUILTIN_SDCARD))
                {
                    Display.Popup("SD CARD MISSING", ILI9341_WHITE, ILI9341_RED);
                    delay(2000);
                    DS_state = 0;
                    DS_menu = 0;
                    Display.DS_page(Instrument_id, recording);
                    DS_define_model();
                    Display.DS_menu(); // display the menu and updates DS_menu_max
                    Display.DS_frame_menu(DS_menu);
                    // Switch bar_display ON
                    PeakTracking_L.reset();
                    PeakTracking_R.reset();
                    Display.DS_bar(0, 0);
                    Display.DS_bar(1, 0);
                    break;
                }

                // verifica se NON formattata
                card.init(SPI_HALF_SPEED, BUILTIN_SDCARD);
                if (!volume.init(card))
                {
                    Display.Popup("SD CARD UNFORMATTED", ILI9341_WHITE, ILI9341_RED);
                    delay(2000);
                    DS_state = 0;
                    DS_menu = 0;
                    Display.DS_page(Instrument_id, recording);
                    DS_define_model();
                    Display.DS_menu(); // display the menu and updates DS_menu_max
                    Display.DS_frame_menu(DS_menu);
                    PeakTracking_L.reset();
                    PeakTracking_R.reset();
                    Display.DS_bar(0, 0);
                    Display.DS_bar(1, 0);
                    break;
                }

                // se formattata si procede
                else
                {
                    // calcola lo spazio disponibile
                    Serial.print("\nCard type: ");
                    switch (card.type())
                    {
                    case SD_CARD_TYPE_SD1:
                        Serial.println("SD1");
                        break;
                    case SD_CARD_TYPE_SD2:
                        Serial.println("SD2");
                        break;
                    case SD_CARD_TYPE_SDHC:
                        Serial.println("SDHC");
                        break;
                    default:
                        Serial.println("Unknown");
                    }

                    SD_volumesize = volume.blocksPerCluster(); // clusters are collections of blocks
                    SD_volumesize *= volume.clusterCount();    // numero di blocchi da 512 byte
                    SD_volumesize = SD_volumesize / 2048;      // MB
                    Serial.print(F("SD Volume size (MB): "));
                    Serial.println(SD_volumesize);

                    File root = SD.open("/");
                    big_result = 0;
                    Print_Directory(root, 0);
                    SD_occupied = big_result / 1048576; // MB

                    Serial.println(F("SD total occupied space (MB): "));
                    Serial.println(SD_occupied);
                    Serial.println(F("SD space free (MB): "));
                    Serial.println(SD_volumesize - SD_occupied);

                    // calcola lo spazio richiesto se MONO
                    if (!Recording[recording].stereo)
                    {
                        // export possibile
                        if ((SD_volumesize - SD_occupied) * 1024 > Recording[recording].bytes / 1024)
                        {
                            DS_export = 1;
                        }
                        // export NON possibile
                        else
                        {
                            DS_export = 0;
                        }
                    }

                    // calcola lo spazio richiesto se STEREO
                    else
                    {
                        // export possibile
                        if ((SD_volumesize - SD_occupied) * 1024 > ((2 * Recording[recording].bytes) / 1024))
                        {
                            DS_export = 2;
                        }
                        // export NON possibile
                        else if ((SD_volumesize - SD_occupied) * 1024 > (Recording[recording].bytes / 1024))
                        {
                            DS_export = 1;
                        }
                        else
                        {
                            DS_export = 0;
                        }
                    }

                    // se non c'e' stazio EXIT
                    if (DS_export == 0)
                    {
                        Display.Popup("SD CARD IS FULL - CANNOT WRITE NEW FILES", ILI9341_WHITE, ILI9341_RED);
                        delay(2000);

                        DS_state = 0;
                        DS_menu = 0;
                        Display.DS_page(Instrument_id, recording);
                        DS_define_model();
                        Display.DS_menu(); // display the menu and updates DS_menu_max
                        Display.DS_frame_menu(DS_menu);
                        PeakTracking_L.reset();
                        PeakTracking_R.reset();
                        Display.DS_bar(0, 0);
                        Display.DS_bar(1, 0);
                        break;
                    }

                    // se c'e' spazio procedi
                    else
                    {
                        confirmation = false;

                        // se NON esiste /LILLA_EXPORT, creala e proponi il nome file
                        if (!SD.exists("/LILLARAW_EXPORT"))
                        {
                            SD.mkdir("/LILLARAW_EXPORT");
                            Serial.println("/LILLARAW_EXPORT directory created on SD");
                            DS_export_M_RAW = DS_export_directory;
                            DS_export_M_RAW += "0M.raw";
                            DS_export_L_RAW += "0L.raw";
                            DS_export_R_RAW += "0R.raw";
                            confirmation = true;
                        }

                        // se esiste /LILLA_EXPORT, definisci il nome file mono / file stereo
                        else
                        {
                            // cerca nome/i libero/i per il/i nuovo/i file .raw su SD
                            for (auto i = 0; i < 100000; ++i)
                            {
                                DS_export_M_RAW = DS_export_directory;
                                DS_export_M_RAW.concat(i);
                                DS_export_M_RAW.concat("M.raw");
                                DS_export_L_RAW = DS_export_directory;
                                DS_export_L_RAW.concat(i);
                                DS_export_L_RAW.concat("L.raw");
                                DS_export_R_RAW = DS_export_directory;
                                DS_export_R_RAW.concat(i);
                                DS_export_R_RAW.concat("R.raw");

                                if (!SD.exists(DS_export_M_RAW.c_str()) && !SD.exists(DS_export_L_RAW.c_str()) && !SD.exists(DS_export_R_RAW.c_str()))
                                {
                                    confirmation = true;
                                    if (!Recording[recording].stereo)
                                    {
                                        Serial.print("it's OK: ");
                                        Serial.println(DS_export_M_RAW);
                                        break;
                                    }

                                    else
                                    {
                                        Serial.print("Both OK: ");
                                        Serial.print(DS_export_L_RAW);
                                        Serial.print("  and: ");
                                        Serial.println(DS_export_R_RAW);
                                        break;
                                    }
                                }
                            }
                        }

                        // l'export NON e' possibile perche' la directory e' affollata
                        if (!confirmation)
                        {
                            //            "0123456789012345678901234567890123456789109876543210";
                            Display.Popup("/LILLARAW_EXPORT IS CROWDED --> DELETE SOME FILES", ILI9341_WHITE, ILI9341_RED);
                            delay(2000);

                            DS_state = 0;
                            DS_menu = 0;
                            Display.DS_page(Instrument_id, recording);
                            DS_define_model();
                            Display.DS_menu(); // display the menu and updates DS_menu_max
                            Display.DS_frame_menu(DS_menu);
                            PeakTracking_L.reset();
                            PeakTracking_R.reset();
                            Display.DS_bar(0, 0);
                            Display.DS_bar(1, 0);
                        }

                        // l'export e' possibile
                        else
                        {
                            byte buffer[256];
                            int packet = 0;
                            int last_blocks = -1;
                            File destination_file;
                            SerialFlashFile source_file;

                            // esporta il file MONO
                            if (!Recording[recording].stereo)
                            {
                                destination_file = SD.open(DS_export_M_RAW.c_str(), FILE_WRITE);

                                // copia dal primo al penultimo packet
                                for (packet = Recording[recording].first_packet; packet < (Recording[recording].first_packet + Recording[recording].packets - 1); ++packet)
                                {
                                    source_file = SerialFlash.open(name_packet[packet]);
                                    for (auto i = 0; i < 256; ++i)
                                    {
                                        source_file.read(buffer, 256);
                                        destination_file.write(buffer, 256);
                                    }
                                }

                                // copia l'ultimo packet
                                packet = Recording[recording].first_packet + Recording[recording].packets - 1;
                                source_file = SerialFlash.open(name_packet[packet]);
                                last_blocks = (Recording[recording].bytes % PACKET_DIM) % 256;
                                for (auto i = 0; i < last_blocks; ++i)
                                {
                                    source_file.read(buffer, 256);
                                    destination_file.write(buffer, 256);
                                }
                                source_file.close();
                                destination_file.close();
                                Serial.println(F("File MONO esportato correttamente su SD"));
                            }

                            // esporta i file STEREO
                            else if (Recording[recording].stereo)
                            {
                                // file_L
                                destination_file = SD.open(DS_export_L_RAW.c_str(), FILE_WRITE);
                                // copia dal primo al penultimo packet
                                for (packet = Recording[recording].first_packet; packet < (Recording[recording].first_packet + 2 * (Recording[recording].packets - 1)); packet += 2)
                                {
                                    source_file = SerialFlash.open(name_packet[packet]);
                                    for (auto i = 0; i < 256; ++i)
                                    {
                                        source_file.read(buffer, 256);
                                        destination_file.write(buffer, 256);
                                    }
                                }
                                // copia l'ultimo packet
                                packet = Recording[recording].first_packet + 2 * (Recording[recording].packets - 1);
                                source_file = SerialFlash.open(name_packet[packet]);
                                last_blocks = (Recording[recording].bytes % PACKET_DIM) % 256;
                                for (auto i = 0; i < last_blocks; ++i)
                                {
                                    source_file.read(buffer, 256);
                                    destination_file.write(buffer, 256);
                                }
                                source_file.close();
                                destination_file.close();
                                Serial.println(F("File STEREO LEFT esportato correttamente su SD"));

                                // file_R
                                destination_file = SD.open(DS_export_R_RAW.c_str(), FILE_WRITE);
                                // copia dal primo al penultimo packet
                                if (Recording[recording].packets > 1)
                                {
                                    for (packet = Recording[recording].first_packet + 1; packet < (Recording[recording].first_packet + 1 + 2 * (Recording[recording].packets - 1)); packet += 2)
                                    {
                                        source_file = SerialFlash.open(name_packet[packet]);
                                        for (auto i = 0; i < 256; ++i)
                                        {
                                            source_file.read(buffer, 256);
                                            destination_file.write(buffer, 256);
                                        }
                                    }
                                }
                                // copia l'ultimo packet
                                packet = Recording[recording].first_packet + 1 + 2 * (Recording[recording].packets - 1);
                                source_file = SerialFlash.open(name_packet[packet]);
                                last_blocks = (Recording[recording].bytes % PACKET_DIM) % 256;
                                for (auto i = 0; i < last_blocks; ++i)
                                {
                                    source_file.read(buffer, 256);
                                    destination_file.write(buffer, 256);
                                }
                                source_file.close();
                                destination_file.close();
                                Serial.println(F("File STEREO RIGHT esportato correttamente su SD"));
                            }

                            // conferma export andato a buon fine
                            if (!Recording[recording].stereo)
                            {
                                Display.Popup("MONO FILE EXPORTED TO SD", ILI9341_BLACK, ILI9341_GREEN);
                            }
                            else
                            {
                                Display.Popup("LEFT AND RIGHT FILES EXPORTED TO SD", ILI9341_BLACK, ILI9341_GREEN);
                            }
                        }
                    }

                    // si ripristina
                    delay(2000);
                    DS_state = 0;
                    DS_menu = 0;
                    Display.DS_page(Instrument_id, recording);
                    DS_define_model();
                    Display.DS_menu(); // display the menu and updates DS_menu_max
                    Display.DS_frame_menu(DS_menu);

                    // Switch bar_display ON
                    PeakTracking_L.reset();
                    PeakTracking_R.reset();
                    Display.DS_bar(0, 0);
                    Display.DS_bar(1, 0);
                }
            } // END case 11 (export to SD)
            break;

            default:
                Serial.println("Switch MISSING! 5239");
                break;
            } // END switch(choice_DS_menu)
        }

        if (Read_pushbutton_fast(35))
        {
            // switch to Performance
            if (Read_pushbutton(26))
            {
                Serial.println("Switch_from_DIRECT_SAMPLING_to_PERFORMANCE()");
                Switch_from_DIRECT_SAMPLING_to_PERFORMANCE();
            }

            // Switch to Mixer
            else if (Read_pushbutton(27))
            {
                Lilla_state_0 = DIRECT_SAMPLING;
                Switch_to_MIXER();
            }

            // Vietato lo switch to DELAY
            else if (Read_pushbutton(28))
            {
                Serial.println("DISPLAY_delay_disabled!");
                Display.Delay_disabled();
                delay(2000);
                DS_refresh_DS_page();
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                Switch_from_DIRECT_SAMPLING_to_LIVE_SAMPLING();
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Lilla_state_0 = DIRECT_SAMPLING;
                Golive_MIDI_MONITOR();
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                Switch_from_DIRECT_SAMPLING_to_MIDI_LOOP();
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Lilla_state_0 = DIRECT_SAMPLING;
                Golive_SETUP();
            }
        }

    } // END if(Lilla_state == DIRECT_SAMPLING)

#pragma endregion // DIRECT SAMPLING

#pragma region
    // *************************************************************
    // ****************        MIDI_MONITOR      *******************
    // *************************************************************
    if (Lilla_state == MIDI_MONITOR)
    {
        // Change Patch VOLUME
        if (!instrument_editing_flag)
        {
            if (Read_encoder(15, volume_patch, 40, 0, 1))
            {
                AudioNoInterrupts();
                Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
                Players_Manager.Broadcast_volume();
                AudioInterrupts();
            }
        }

        // Visualizza incoming MIDI
        if (display_wait)
        {
            switch (midi_message_received)
            {
            case 0: // no message received
                break;
            case 1: // note ON
                Display.Midi_monitor_data(MM_midi_channel, 0, MM_note_number, MM_velocity, -1, -1);
                break;
            case 2: // note OFF
                Display.Midi_monitor_data(MM_midi_channel, 1, MM_note_number, MM_velocity, -1, -1);
                break;
            case 3: // pitch bend
                Display.Midi_monitor_data(MM_midi_channel, 2, -1, -1, (MM_pitch_bend_most << 7) + MM_pitch_bend_least, -1);
                break;
            case 4: // after touch poly
                Display.Midi_monitor_data(MM_midi_channel, 3, MM_least_bits, -1, MM_most_bits, -1);
                break;
            case 5: // control change
                Display.Midi_monitor_data(MM_midi_channel, 4, -1, -1, MM_midi_value, MM_midi_controller);
                break;
            case 6: // program change
                Display.Midi_monitor_data(MM_midi_channel, 5, -1, -1, -1, MM_least_bits);
                break;
            case 7: // After Touch Channel
                Display.Midi_monitor_data(MM_midi_channel, 6, -1, -1, MM_least_bits, -1);
                break;
            case 8: // System Exclusive
                Display.Midi_monitor_data(MM_midi_channel, 7, -1, -1, -1, -1);
                break;
            default:
                Serial.println("Switch MISSING! 5350");
                break;
            }
            display_wait = false;
        }

        // Switch
        if (Read_pushbutton_fast(35))
        {
            // Switch to PERFORMANCE
            if (Read_pushbutton(26))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Golive_with_PERFORMANCE(Patch_id);
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_PERFORMANCE();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_PERFORMANCE();
                    break;

                case MIDI_LOOP:
                    Switch_from_MIDI_LOOP_to_PERFORMANCE();
                    break;

                default:
                    Serial.println("Switch MISSING! 5341");
                    break;
                }
            }

            // Switch to MIXER
            else if (Read_pushbutton(27))
            {
                Switch_to_MIXER();
            }

            // Switch to DELAY
            else if (Read_pushbutton(28))
            {
                Lilla_state = DELAY_SETTINGS;
                Display.Delay_page();
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_LIVE_SAMPLING();
                    break;

                case LIVE_SAMPLING:
                    LS_refresh_LS_page();
                    break;

                case MIDI_LOOP:
                    Switch_from_MIDI_LOOP_to_LIVE_SAMPLING();
                    break;

                default:
                    Serial.println("Switch MISSING! 5381");
                    break;
                };
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_to_DIRECT_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    DS_refresh_DS_page();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING();
                    break;

                case MIDI_LOOP:
                    Switch_from_MIDI_LOOP_to_DIRECT_SAMPLING();
                    break;

                default:
                    Serial.println("Switch MISSING! 5408");
                    break;
                }
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_from_PERFORMANCE_to_MIDI_LOOP();
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_MIDI_LOOP();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_MIDI_LOOP();
                    break;

                case MIDI_LOOP:
                    Golive_with_MIDI_LOOP(false);
                    break;

                default:
                    Serial.println("Switch MISSING! 5435");
                    break;
                }
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Switch_from_MIDI_LOOP_to_SETUP();
            }
        }
    }

#pragma endregion // MIDI_MONITOR

#pragma region[rgba(6, 209, 250, 0.13)]
    // *************************************************************
    // ******************       MIDI_LOOP      *********************
    // *************************************************************

    if (Lilla_state == MIDI_LOOP)
    {
        // Change volume_patch
        if (Read_encoder(15, volume_patch, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
            Players_Manager.Broadcast_volume();
            AudioInterrupts();

            Display.Patch_volume_value(true);
        }

        // Change LOOP_id
        result = Read_encoder_simple(7);
        if (result != 0)
        {
            int new_loop_id;
            if (result == +1)
            {
                new_loop_id = Get_next_loop_id_in_SD(LOOP_id);
            }
            else
            {
                new_loop_id = Get_previous_loop_id_in_SD(LOOP_id);
            }

            if (new_loop_id != LOOP_id)
            {
                // delete runnig loop data and stop metronomo
                LOOP_stop_and_reset_runnig_loop_data(); // LOOP_track_run[track] = false; LOOP_metronomo_run == false; LOOP_metronomo_flag_IN[1] = false;

                // switch LOOP_id
                LOOP_id = new_loop_id;

                // import LOOP_id from SD
                Copy_midi_loop_from_SD_to_RAM(LOOP_id);

                // update LOOP_id on display
                Display.Loop_loop_id();

                // update LOOP_time on display
                Display.Loop_time_stretched();

                // update tracks infos on display
                for (auto local_track = 0; local_track < TRACKS; ++local_track)
                {
                    Display.Loop_track_data(local_track);
                }

                // update menu on display
                LOOP_menu = 0;
                Display.Delete_all_frame_loop_menu();
                LOOP_select_menu_elements();
                Display.Loop_menu();
                Display.Frame_loop_menu(LOOP_menu, true);

                // switch off all tracks LEDs on display
                Loop_led_set.Request_all_LED_switch_off();

                // switch on led_0
                LOOP_metronomo.Led_ON(0);

                // setup metronomo
                LOOP_metronomo.Setup(LOOP_time);

                // restart clock
                LOOP_restart_clock();

                // set first event for each track
                for (auto local_track = 0; local_track < TRACKS; ++local_track)
                {
                    LOOP_play_event[local_track] = 0;
                }

                // effettua l'ordinamento temporale degli eventi
                for (auto local_track = 0; local_track < TRACKS; ++local_track)
                {
                    LOOP_set_time_order(local_track);
                }

                // simula Start/Stop all tracks con tutte le tracks attive
                LOOP_run_button_state = false;

                // memorizza lo stato dei track prima di fermarli
                for (auto local_track = 0; local_track < TRACKS; ++local_track)
                {
                    LOOP_track_run_memo[local_track] = LOOP_events[local_track] > 0;
                    LOOP_track_run[local_track] = false;
                }

                Serial.println("Loop uploaded; data in RAM:");
                Print_midi_loop_complete_data(LOOP_id);
            }
        }

        // Learning
        for (auto track = 0; track < TRACKS; ++track)
        {
            // ricezione richiesta
            if (LOOP_run_button_state && Read_pushbutton(LOOP_UI_B + track))
            {
                LOOP_learning_track = track; // LOOP_learning_track e' il nuovo loop

                // attivita' prioritarie AudioNoInterrupts()
                AudioNoInterrupts();
                if (LOOP_events[LOOP_learning_track] != 0)
                {
                    // se si tratta di MASTER_TRACK (0) si fermano e cancellano tutti i track
                    if (LOOP_learning_track == MASTER_TRACK)
                    {
                        // Interrompi i Player che eseguono note di qualsiasi track
                        Players_Manager.Release_all_players_loop();

                        for (auto local_track = 0; local_track < TRACKS; ++local_track)
                        {
                            // Interrompe la lettura
                            LOOP_track_run[local_track] = false;

                            // Cancella il loop
                            LOOP_events[local_track] = 0;

                            // Resetta slide
                            LOOP_slide[local_track] = 0;

                            // Resetta pitch
                            LOOP_pitch_int[local_track] = 0;
                        }

                        // ferma il metronomo
                        LOOP_metronomo_run = false;
                        // LOOP_metronomo_flag_OUT = false; // da eliminare

                        // Annulla (se ci fosse) l'ultima richiesta di aggiornamento del metronomo proveniente da MidiReader
                        LOOP_metronomo_flag_IN[1] = false;

                        // Nuovo loop
                        LOOP_original = true;
                    }
                    else
                    {
                        if (LOOP_track_run[LOOP_learning_track])
                        {
                            // Interrompi i Player di LOOP_learning_track
                            Players_Manager.Release_all_players_loop(LOOP_learning_track);
                        }
                        // Interrompe la lettura
                        LOOP_track_run[LOOP_learning_track] = false;

                        // Cancella il loop
                        LOOP_events[LOOP_learning_track] = 0; // Loop cancellato

                        // Resetta slide
                        LOOP_slide[LOOP_learning_track] = 0;

                        // Resetta pitch
                        LOOP_pitch_int[LOOP_learning_track] = 0;

                        // loop esistente
                        LOOP_original = false;
                    }
                }
                AudioInterrupts();

                // attivita' sul display - NO AudioNoInterrupts()
                if (LOOP_learning_track == MASTER_TRACK)
                {
                    // Spegni i led del metronomo
                    LOOP_metronomo.Leds_off(); // va eseguito fuori da AudioNoInterrupt()

                    // nomina loop_id
                    LOOP_id = NEW_LOOP;
                    Display.Loop_loop_id();

                    LOOP_time = 0;
                    Display.Loop_time_stretched(); // accanto ai led del metronomo appare il tempo totale 0.0s
                }

                // Se si tratta del track master (0) non ancora esistente, oppure si tratta di un altro track ma con track master esistente
                if (LOOP_learning_track == MASTER_TRACK || (LOOP_learning_track > MASTER_TRACK && LOOP_events[MASTER_TRACK] != 0))
                {
                    // show (or delete) all tracks infos
                    for (auto local_track = 0; local_track < TRACKS; ++local_track)
                    {
                        Display.Loop_track_data(local_track);
                    }

                    // display "n-REC"
                    Display.Loop_rec_advice(LOOP_learning_track, true);

                    // switch off all tracks LEDs
                    Loop_led_set.Request_all_LED_switch_off();

                    // update menu
                    LOOP_menu = 0;
                    Display.Delete_all_frame_loop_menu();
                    LOOP_select_menu_elements();
                    Display.Loop_menu();
                    Display.Frame_loop_menu(LOOP_menu, true);

                    // prepare learning
                    LOOP_learn_clock = 0;
                    LOOP_elements = 0;      // ancora nessun evento
                    LOOP_learn_flag = true; // avvia il learning
                    Serial.println("Learning inizializzato!");

                    // learnig
                    while (LOOP_learn_flag)
                    {
                        // Termina il learning
                        if (Read_pushbutton(LOOP_UI_B + LOOP_learning_track))
                        {
                            // Chiude il learning
                            LOOP_learn_flag = false;

                            Serial.println("Loop correttamente chiuso manualmente!");
                            break;
                        }

                        // Dall'avvio sono passati 20 secondi senza eventi --> cancella il loop
                        else if (LOOP_elements == 0 && LOOP_learn_clock > 20000)
                        {
                            // Chiude il learning
                            LOOP_learn_flag = false;
                            Serial.println("Loop chiuso e cancellato perche' dimenticato aperto!");
                            break;
                        }

                        // richiesto annullamento del loop_learn
                        else if (Read_pushbutton(LOOP_UI_C + LOOP_learning_track))
                        {
                            // Chiude il learning
                            LOOP_learn_flag = false;
                            LOOP_elements = 0;
                            Serial.println("Loop chiuso e cancellato");
                            break;
                        }

                        // se si tratta del track master (0), MidiReader chiede l'accensione del primo led del metronomo quando riceve il primo NoteOn
                        else if (LOOP_learning_track == MASTER_TRACK && LOOP_metronomo_flag_IN[0])
                        {
                            LOOP_metronomo_flag_IN[0] = false;
                            // ask metronomo to switch on first led (metronomo is not runnig)
                            LOOP_metronomo.Led_ON(0);
                        }

                        // aggiornamento continuo dei led
                        Update_instruments_leds();

                        // se NON si tratta del track master (0) c'e' l'aggiornamento continuo del metronomo
                        if (LOOP_metronomo_flag_IN[1])
                        {
                            LOOP_metronomo_flag_IN[1] = false;
                            LOOP_metronomo.Update();
                            LOOP_metronomo.metro_time += LOOP_metronomo.Read_metro_delta_ms();
                        }
                    }

                    // from here LOOP_learn_flag == false
                    Serial.println("Learning concluso!");
                    LOOP_events[LOOP_learning_track] = LOOP_elements; // se LOOP_events[LOOP_learning_track] == 0 significa che il LOOP_learning_track è vuoto e non viene eseguito

                    // new track is valid (contains events)
                    if (LOOP_events[LOOP_learning_track] > 0)
                    {
                        // se e' il loop_master:
                        if (LOOP_learning_track == MASTER_TRACK)
                        {
                            // Setup di LOOP_time (durata di tutti i loop)
                            LOOP_time = LOOP_learn_clock;
                            Serial.print("LOOP_time:");
                            Serial.println(LOOP_time);

                            // restart clock
                            LOOP_restart_clock();

                            // reset stretch
                            LOOP_stretch_int = 100;
                            LOOP_stretch = 1.0;
                        }

                        // aggiungi info di slide
                        LOOP_slide[LOOP_learning_track] = 0; // ms

                        // aggiungi info di pitch
                        LOOP_pitch_int[LOOP_learning_track] = 0; // 0 --> pitch = 1.0

                        // evento di avvio
                        LOOP_play_event[LOOP_learning_track] = 0;

                        // calcolo istante esecuzione evento di avvio (LOOP_play_time) e prossimo switch del metronomo rispetto a LOOP_Clock
                        if (LOOP_learning_track == MASTER_TRACK)
                        {
                            LOOP_play_time[LOOP_learning_track] = 0;

                            // setup metronomo
                            LOOP_metronomo.Setup(LOOP_time);

                            // set next metronomo step
                            LOOP_metronomo.metro_time = 0 + LOOP_metronomo.Read_metro_delta_ms();

                            // metronomo switch-on
                            LOOP_metronomo_run = true;
                        }

                        else
                            LOOP_play_time[LOOP_learning_track] = LOOP_Clock_time_from_virtual_time(LOOP_element[LOOP_learning_track][0].time);

                        // effettua l'ordinamento temporale degli eventi
                        LOOP_set_time_order(LOOP_learning_track);

                        // avvio
                        LOOP_track_run[LOOP_learning_track] = true;

                        // update menu
                        if (LOOP_id == NEW_LOOP)
                        {
                            LOOP_original = true;
                        }

                        else if (LOOP_learning_track == MASTER_TRACK && LOOP_original)
                        {
                            LOOP_original = false;
                        }

                        else if (LOOP_learning_track > MASTER_TRACK)
                        {
                            LOOP_original = false;
                        }

                        LOOP_menu = 0;
                        Display.Delete_all_frame_loop_menu();
                        LOOP_select_menu_elements();
                        Display.Loop_menu();
                        Display.Frame_loop_menu(LOOP_menu, true);

                        // report
                        Serial.println(" **************** ");
                        Serial.print("eventi:");
                        Serial.println(LOOP_events[LOOP_learning_track]);
                        for (auto event = 0; event < LOOP_events[LOOP_learning_track]; ++event)
                        {
                            Serial.print(event);
                            Serial.print(" time:");
                            Serial.print(LOOP_element[LOOP_learning_track][event].time);
                            Serial.print(" midi_channel:");
                            Serial.print(LOOP_element[LOOP_learning_track][event].midi_channel);
                            Serial.print(" note_number:");
                            Serial.print(LOOP_element[LOOP_learning_track][event].note_number);
                            Serial.print(" velocity:");
                            Serial.print(LOOP_element[LOOP_learning_track][event].velocity);
                            Serial.print(" note_on:");
                            Serial.println(LOOP_element[LOOP_learning_track][event].note_on ? "NoteOn" : "NoteOff");
                        }

                        Serial.print("Ordine temporale degli eventi: ");
                        for (auto event = 0; event < LOOP_events[LOOP_learning_track]; ++event)
                        {
                            Serial.print(LOOP_time_order[LOOP_learning_track][event]);
                            Serial.print(" - ");
                        }
                        Serial.println();
                        Serial.print("Si inizia con l'evento:");
                        Serial.println(LOOP_play_event[LOOP_learning_track]);
                        Serial.print("Tra ms:");
                        Serial.println(LOOP_play_time[LOOP_learning_track] - LOOP_Clock());
                        Serial.println("*************");
                    }

                    else if (LOOP_events[MASTER_TRACK] == 0)
                    {
                        // spegni il primo led se acceso
                        LOOP_metronomo.Leds_off();

                        // cancella la richiesta per il primo led se arrivata (non dovrebbe essere possibile)
                        LOOP_metronomo_flag_IN[0] = false;
                    }

                    Display.Loop_track_data(track);

                    // visualizza durata totale
                    if (LOOP_learning_track == MASTER_TRACK)
                    {
                        Display.Loop_time_stretched();
                    }
                }
            }
            // qui sopra le funzionalita' learn

            // Existing track
            if (LOOP_events[track] > 0)
            {
                // Track start-stop
                if (Read_pushbutton(LOOP_UI_C + track))
                {
                    // Stop
                    if (LOOP_track_run[track])
                    {
                        LOOP_track_run[track] = false;

                        // interrompi i Player di track
                        AudioNoInterrupts();
                        Players_Manager.Release_all_players_loop(track);
                        AudioInterrupts();

                        // spegni i led del loop
                        Loop_led_set.Request_track_LED_switch_off(track);
                    }

                    // Start
                    else
                    {
                        // accendi il primo led del metronomo
                        // LOOP_metronomo.Led_ON(0);

                        AudioNoInterrupts();
                        // dopo uno stop a tutti i loop, alla prima ripartenza va azzerato LOOP_clock e va fatto ripartire il metronomo
                        if (!LOOP_metronomo_run)
                        {
                            // se LOOP_run_button_state == false va ripristinato
                            LOOP_run_button_state = true;

                            LOOP_restart_clock();

                            // calcolo prossimo evento metronomo
                            LOOP_metronomo.metro_time = 0 + LOOP_metronomo.Read_metro_delta_ms();

                            // avvia il metronomo
                            LOOP_metronomo_run = true;
                        }
                        LOOP_restart_procedure(track); // Procedura di ripartenza
                        AudioInterrupts();
                    }
                }

                // Slide temporale
                result = Read_encoder_simple(LOOP_UI_A + track);
                if (result != 0)
                {
                    LOOP_original = false;

                    int jump;

                    if (result == 1)
                    {
                        jump = 100;
                    }

                    else
                    {
                        if (LOOP_time >= 100)
                        {
                            jump = LOOP_time - 100;
                        }
                        else
                        {
                            jump = 0;
                        }
                    }

                    Serial.print("Shift ms:");
                    Serial.println(jump);

                    AudioNoInterrupts();
                    for (auto event = 0; event < LOOP_events[track]; ++event)
                    {
                        LOOP_element[track][event].time = (LOOP_element[track][event].time + jump) % LOOP_time;
                    }

                    // Interrompi i Player di track
                    Players_Manager.Release_all_players_loop(track);

                    // Effettua l'ordinamento temporale degli eventi
                    LOOP_set_time_order(LOOP_learning_track);

                    // Procedura di ripartenza
                    LOOP_restart_procedure(track);
                    AudioInterrupts();

                    // Spegni i led del loop
                    Loop_led_set.Request_track_LED_switch_off(track);

                    LOOP_slide[track] = (LOOP_slide[track] + jump) % LOOP_time;
                    Display.Loop_track_data(track);
                }

                // Annulla slide temporale
                if (LOOP_slide[track] > 0)
                    if (Read_pushbutton_fast(LOOP_UI_A + track))
                    {
                        int jump = LOOP_time - LOOP_slide[track];

                        AudioNoInterrupts();
                        for (auto event = 0; event < LOOP_events[track]; ++event)
                        {
                            LOOP_element[track][event].time = (LOOP_element[track][event].time + jump) % LOOP_time;
                        }

                        // Interrompi i Player di loop
                        Players_Manager.Release_all_players_loop(track);

                        // Effettua l'ordinamento temporale degli eventi
                        LOOP_set_time_order(LOOP_learning_track);

                        // Procedura di ripartenza
                        LOOP_restart_procedure(track);
                        AudioInterrupts();

                        // Spegni i led del loop
                        Loop_led_set.Request_track_LED_switch_off(track);

                        LOOP_slide[track] = (LOOP_slide[track] + jump) % LOOP_time;
                        Display.Loop_track_data(track);
                    }

                // Volume
                if (Read_encoder(LOOP_UI_C + track, LOOP_volume_int[track], 40, 0, 1))
                {
                    LOOP_original = false;

                    AudioNoInterrupts();
                    LOOP_volume[track] = LOOP_volume_int[track] / 20.0f;
                    Players_Manager.Multicast_volume_for_MIDI_LOOP_running(track, LOOP_volume[track]);
                    AudioInterrupts();

                    Display.Loop_track_data(track);
                    Serial.print("LOOP_volume: ");
                    Serial.println(LOOP_volume[track]);
                }

                // Pitch
                if (Read_encoder(LOOP_UI_B + track, LOOP_pitch_int[track], 24, -24, 1))
                {
                    LOOP_original = false;

                    Display.Loop_track_data(track);
                    Serial.print("LOOP_pitch_int: ");
                    Serial.println(LOOP_pitch_int[track]);
                }
            }
        }

        // Comandi attivi se esiste MASTER_TRACK, comuni a tutti i track
        if (LOOP_events[MASTER_TRACK] > 0)
        {
            // change menu item
            result = Read_encoder_simple(25);
            if (result != 0)
            {
#ifdef PCB_2022
                if (result == -1)
#else
                if (result == +1)
#endif
                {
                    if (LOOP_menu < Loop_menu_max)
                    {
                        LOOP_menu_change = LOOP_menu + 1;
                    }
                }
                else
                {
                    if (LOOP_menu > 0)
                    {
                        LOOP_menu_change = LOOP_menu - 1;
                    }
                }

                if (LOOP_menu_change != LOOP_menu)
                {
                    LOOP_menu = LOOP_menu_change;
                    Display.Delete_all_frame_loop_menu();
                    Display.Frame_loop_menu(LOOP_menu, true);
                }
            }

            // choose menu item
            if (Read_pushbutton(25))
            {
                int choice_loop_menu = element_Menu_Loop[LOOP_menu];
                switch (choice_loop_menu)
                {

                case 0:                                     // New
                    LOOP_stop_and_reset_runnig_loop_data(); // LOOP_track_run[track] = false; LOOP_metronomo_run == false; LOOP_metronomo_flag_IN[1] = false;
                    LOOP_id = NEW_LOOP;
                    LOOP_original = true;
                    LOOP_run_button_state = true;
                    Golive_with_MIDI_LOOP(true);
                    break;

                case 1: // Save
                    Copy_midi_loop_from_RAM_to_SD(LOOP_id);
                    LOOP_original = true;
                    break;

                case 2: // Save as new
                    result = Get_first_loop_id_free();
                    if (result >= 0)
                    {
                        LOOP_id = result;
                        Copy_midi_loop_from_RAM_to_SD(LOOP_id);

                        Serial.println("Current loop has been saved; data in RAM are:");
                        Print_midi_loop_complete_data(LOOP_id);

                        // Update menu
                        LOOP_original = true;
                        LOOP_menu = 0;
                        Display.Delete_all_frame_loop_menu();
                        LOOP_select_menu_elements();
                        Display.Loop_menu();
                        Display.Frame_loop_menu(LOOP_menu, true);

                        // Update loop_id
                        Display.Loop_loop_id();
                    }
                    break;

                case 3: // Delete
                    Delete_midi_loop_from_SD(LOOP_id);

                    // new
                    LOOP_stop_and_reset_runnig_loop_data(); // LOOP_track_run[track] = false; LOOP_metronomo_run == false; LOOP_metronomo_flag_IN[1] = false;
                    LOOP_id = NEW_LOOP;
                    LOOP_original = true;
                    LOOP_run_button_state = true;
                    Golive_with_MIDI_LOOP(true);
                    break;
                }
            }

            // Change tempo
#ifdef PCB_2022
            if (Read_encoder(24, LOOP_stretch_int, 198, 1, 1))
#else
            if (Read_encoder_inverse(24, LOOP_stretch_int, 198, 1, 1))
#endif
            {
                AudioNoInterrupts();
                // Memorizza il tempo virtuale attuale
                LOOP_clock_memo = LOOP_Clock();

                // Aggiorna LOOP_stretch
                if (LOOP_stretch_int <= 100)
                {
                    LOOP_stretch = LOOP_stretch_int / 100.0;
                }
                else
                {
                    LOOP_stretch = 1.0 / (2.0f - LOOP_stretch_int / 100.0f);
                }

                // Ricalcolo LOOP_clock
                LOOP_clock = LOOP_clock_memo * LOOP_stretch;
                AudioInterrupts();

                Display.Loop_time_stretched();

                Serial.print("LOOP_stretch: ");
                Serial.println(LOOP_stretch);
            }

            // Back to original tempo
            if (Read_pushbutton(24))
            {
                AudioNoInterrupts();
                // Memorizza il tempo virtuale attuale
                LOOP_clock_memo = LOOP_Clock();

                // Aggiorna LOOP_stretch
                LOOP_stretch_int = 100;
                LOOP_stretch = 1.0;

                // Ricalcolo LOOP_clock
                LOOP_clock = LOOP_clock_memo;
                AudioInterrupts();

                Display.Loop_time_stretched();

                Serial.print("LOOP_stretch: ");
                Serial.println(LOOP_stretch);
            }

            // Update metronomo tempo
            if (LOOP_metronomo_flag_IN[1])
            {
                LOOP_metronomo_flag_IN[1] = false;
                LOOP_metronomo.Update();
                LOOP_metronomo.metro_time += LOOP_metronomo.Read_metro_delta_ms();
            }

            // Start/Stop all tracks
            if (Read_pushbutton(7))
            {
                // stop all tracks
                if (LOOP_run_button_state)
                {
                    LOOP_run_button_state = false;

                    AudioNoInterrupts();

                    // memorizza lo stato dei track prima di fermarli
                    for (auto local_track = 0; local_track < TRACKS; ++local_track)
                    {
                        LOOP_track_run_memo[local_track] = LOOP_track_run[local_track];
                        LOOP_track_run[local_track] = false;
                    }

                    // Interrompi i Player di loop
                    Players_Manager.Release_all_players_loop();

                    // Ferma il metronomo
                    LOOP_metronomo_run = false;

                    // Annulla l'ultima richiesta di aggiornamento proveniente da MidiReader
                    LOOP_metronomo_flag_IN[1] = false;

                    AudioInterrupts();

                    // Aggiorna (spegni) tutti i led
                    Loop_led_set.Request_all_LED_switch_off();

                    // Spegni i led del metronomo
                    LOOP_metronomo.Leds_off();

                    // Accendi led_0
                    LOOP_metronomo.Led_ON(0);
                }

                // enable all tracks
                else
                {
                    LOOP_run_button_state = true;

                    AudioNoInterrupts();
                    LOOP_restart_clock();

                    // il led 0 e' gia' acceso, riavvia il metronomo
                    LOOP_metronomo.metro_time = 0 + LOOP_metronomo.Read_metro_delta_ms();
                    LOOP_metronomo_run = true;
                    for (auto local_track = 0; local_track < TRACKS; ++local_track)
                    {
                        if (LOOP_track_run_memo[local_track])
                        {
                            LOOP_restart_procedure(local_track);
                        }
                    }
                    AudioInterrupts();
                }
            }
        }

        // Switch
        if (Read_pushbutton_fast(35))
        {
            // Switch to PERFORMANCE
            if (Read_pushbutton(26))
            {
                Switch_from_MIDI_LOOP_to_PERFORMANCE();
            }

            // Switch to MIXER
            else if (Read_pushbutton(27))
            {
                Lilla_state_0 = MIDI_LOOP;
                Switch_to_MIXER();
            }

            // Switch to DELAY
            else if (Read_pushbutton(28))
            {
                Lilla_state_0 = MIDI_LOOP;
                Lilla_state = DELAY_SETTINGS;
                Display.Delay_page();
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                Switch_from_MIDI_LOOP_to_LIVE_SAMPLING();
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                Switch_from_MIDI_LOOP_to_DIRECT_SAMPLING();
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                Switch_from_MIDI_LOOP_to_MIDI_MONITOR();
            }

            // Switch to SETUP
            else if (Read_pushbutton(33))
            {
                Switch_from_MIDI_LOOP_to_SETUP();
            }
        }

        if (Read_pushbutton(PB_number + 26) && Patch[Patch_id].Instrument[PB_number].used)
        {
            Lilla_state_0 = MIDI_LOOP;
            Lilla_state = SOUND_EDIT;
            Instrument_id = PB_number;
            Serial.print("Editing Sound: ");
            Serial.println(Instrument_id);
            Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
            P_Sound(Sound_id);

            samples_in_file = Get_samples_in_raw_file(Sound[Sound_id].file);
            Noclick_max = Calc_Noclick_max(Preset[Instrument_id].use_Wavetable);
            trim_step = Calc_trim_step(trim_speed);
            sound_original = Verify_is_Sound_original(Sound_id);
            So_menu = 0;

            Display.Show_sound(Sound_id, Instrument_id);
            // to do: display LED

            Display.Show_wave(Instrument_id);

            Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menu
            Display.Frame_SOUND_EDIT_menu(So_menu);
        }
    }
#pragma endregion // MIDI_LOOP

#pragma region[rgba(54, 135, 210, 0.2)]
    // *************************************************************
    // ********************      SETUP      ************************
    // *************************************************************
    if (Lilla_state == SETUP)
    {
        // Change Patch VOLUME
        if (Read_encoder(15, volume_patch, 40, 0, 1))
        {
            AudioNoInterrupts();
            Players_Manager.Update_all_Preset_volume(Patch_id, Volume_float[volume_patch]);
            Players_Manager.Broadcast_volume();
            AudioInterrupts();
        }

        // Set Key Step
#ifdef PCB_2022
        if (SET_menu == 0 && Read_encoder(24, key_step, 3, 0, 1))
#else
        if (SET_menu == 0 && Read_encoder_inverse(24, key_step, 3, 0, 1))
#endif
        {
            Display.Key_step();
            Calc_pitch_from_note();
        }

        // Set Prima ottava
#ifdef PCB_2022
        if (SET_menu == 1 && Read_encoder_inverse(24, first_octave, 0, -2, 1))
#else
        if (SET_menu == 1 && Read_encoder(24, first_octave, 0, -2, 1))
#endif
        {
            Display.First_octave();
        }

        // Optimization
        optimization_cache = optimization;

#ifdef PCB_2022
        if (SET_menu == 2 && Read_encoder(24, optimization_cache, 3, 0, 1))
#else
        if (SET_menu == 2 && Read_encoder_inverse(24, optimization_cache, 3, 0, 1))
#endif
        {
            AudioNoInterrupts();
            Players_Manager.Stop_all_players();
            optimization = optimization_cache;
            AudioInterrupts();

            Archive.Save_optimization(optimization);
            Display.Optimization();
        }

// Change menu item  -  uint8_t SET_menu;
#ifdef PCB_2022
        if (Read_encoder_inverse(25, SET_menu, 7, 0, 1))
#else
        if (Read_encoder(25, SET_menu, 7, 0, 1))
#endif
        {
            Display.Settings_frame(SET_menu);
        }

        // Choose menu item
        if (Read_pushbutton(25))
        {
            switch (SET_menu)
            {
            case 3: // switch to CC Settings
                Lilla_state = CC_SETTINGS;
                display_wait = false;

                for (auto local_instrument = 0; local_instrument < INSTRUMENTS_MAX; ++local_instrument)
                {
                    CC_Sound_gain_cache[local_instrument] = CC_Sound_gain[local_instrument];
                }

                CC_lowpass_filter_cache = CC_lowpass_filter;
                Display.CC_page();
                Display.All_CC_Sound_gain();
                Display.Show_CC_lowpass_filter();
                CC_menu = 0;
                Display.Frame_CC_page_menu(CC_menu);
                break;

                // case 3: // USB access to SD card - funzionalita' MTP
                // break;

            case 4: // import RAW files from SD
                AudioNoInterrupts();
                Players_Manager.Stop_all_players();
                AudioInterrupts();

                if (Copy_raw_files_from_SD_to_Flash())
                {
                    Make_VFS();
                    DS_seed_all_Recordings();
                    Bootstrap_setup();
                }
                else
                {
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                }
                break;

            case 5: // Setup (all EEPROM) import from lilla.txt (in SD)
                Display.Confirm_config_import_popup();
                Display.Confirm_config_import_frame(0);
                Ask_if_IMPORT_EXPORT_setup();
                if (result == 0)
                {
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                    break;
                }

                // check SD presence
                if (!SD.begin(BUILTIN_SDCARD))
                {
                    Display.SD_missing(ILI9341_BLACK);
                    delay(5000);
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                    break;
                }
                if (!SD.exists("/LILLASET/lilla.txt"))
                {
                    Display.Config_import_FILE_error_popup();
                    delay(5000);
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                    break;
                }
                else
                {
                    Display.Config_import_REBOOT_popup();

                    File file = SD.open("/LILLASET/lilla.txt"); // apertura file esistente
                    Archive.Save_setup_file(file);
                    file.close();
                    Serial.println("Lilla setup has been copied from lilla.txt to EEPROM");

                    // eventually imported Recordings MUST be deleted
                    DS_seed_all_Recordings();
                    Bootstrap_setup();
                }
                break;

            case 6: // Setup (all EEPROM content) export to SD card (lillaold.txt)
                Display.Confirm_config_export_popup();
                Display.Confirm_config_import_frame(0);
                Ask_if_IMPORT_EXPORT_setup();
                if (result == 0)
                {
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                    break;
                }

                // check SD presence
                if (!SD.begin(BUILTIN_SDCARD))
                {
                    Display.SD_missing(ILI9341_BLACK);
                    delay(5000);
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                    break;
                }

                // do it
                else
                {
                    if (!SD.exists("/LILLASET"))
                    {
                        SD.mkdir("/LILLASET");
                        Serial.println(F("/LILLASET directory created"));
                    }
                    if (SD.open("/LILLASET/lillaold.txt"))
                    {
                        SD.remove("/LILLASET/lillaold.txt");
                        Serial.println(F("existing lillaold.txt has been deleted"));
                    }

                    File file = SD.open("/LILLASET/lillaold.txt", FILE_WRITE); // creazione del file destinazione
                    Serial.println(F("new lillaold.txt has been created"));
                    if (file)
                    {
                        Serial.println("lillaold.txt exists");
                        Archive.Copy_setup_from_Eeprom_to_SD(file);
                        file.close();
                        Display.Config_export_save_popup();
                        delay(5000);
                    }
                    else
                    {
                        Display.Config_export_SD_error_popup();
                        delay(5000);
                    }
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                }
                break;

            case 7: // Factory reset
                Display.Confirm_factory_reset_popup();
                Display.Confirm_config_import_frame(0);
                Ask_if_FACTORY_RESET();
                if (result == 0)
                {
                    Display.Settings_page();
                    Display.Settings_frame(SET_menu);
                    break;
                }
                Display.Factory_reset_wait_popup();

                delay(3000); // per ripensamenti last minute!
                Factory_setup_Eeprom();
                Bootstrap_setup();
                break;

            default:
                Serial.println("Switch MISSING! 6255");
                break;
            }
        }

        while (Read_pushbutton_fast(35))
        {
            // Switch to PERFORMANCE
            if (Read_pushbutton(26))
            {
                if (first_octave != first_octave_cache)
                    Archive.Save_first_octave(first_octave);

                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Golive_with_PERFORMANCE(Patch_id);
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_PERFORMANCE();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_PERFORMANCE();
                    break;

                default:
                    Serial.println("Switch MISSING! 6283");
                    break;
                }
                break;
            }

            // Switch to MIXER
            else if (Read_pushbutton(27))
            {
                if (first_octave != first_octave_cache)
                {
                    Archive.Save_first_octave(first_octave);
                }
                Switch_to_MIXER();
                break;
            }

            // Switch to DELAY
            else if (Read_pushbutton(28))
            {
                if (first_octave != first_octave_cache)
                {
                    Archive.Save_first_octave(first_octave);
                }
                Lilla_state = DELAY_SETTINGS;
                Display.Delay_page();
                break;
            }

            // Switch to LIVE_SAMPLING
            else if (Read_pushbutton(29))
            {
                if (first_octave != first_octave_cache)
                {
                    Archive.Save_first_octave(first_octave);
                }

                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_LIVE_SAMPLING();
                    break;

                case LIVE_SAMPLING:

                    LS_refresh_LS_page();
                    break;

                default:
                    Serial.println("Switch MISSING! 6330");
                    break;
                }
                break;
            }

            // Switch to DIRECT_SAMPLING
            else if (Read_pushbutton(30))
            {
                if (first_octave != first_octave_cache)
                {
                    Archive.Save_first_octave(first_octave);
                }

                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_to_DIRECT_SAMPLING();
                    break;

                case DIRECT_SAMPLING:
                    DS_refresh_DS_page();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING();
                    break;

                default:
                    Serial.println("Switch MISSING! 6357");
                    break;
                }
                break;
            }

            // Switch to MIDI_MONITOR
            else if (Read_pushbutton(31))
            {
                if (first_octave != first_octave_cache)
                {
                    Archive.Save_first_octave(first_octave);
                }

                Golive_MIDI_MONITOR();
                break;
            }

            // Switch to MIDI_LOOP
            else if (Read_pushbutton(32))
            {
                if (first_octave != first_octave_cache)
                {
                    Archive.Save_first_octave(first_octave);
                }
                switch (Lilla_state_0)
                {
                case PERFORMANCE:
                    Switch_from_PERFORMANCE_to_MIDI_LOOP();
                    break;

                case DIRECT_SAMPLING:
                    Switch_from_DIRECT_SAMPLING_to_MIDI_LOOP();
                    break;

                case LIVE_SAMPLING:
                    Switch_from_LIVE_SAMPLING_to_MIDI_LOOP();
                    break;

                case MIDI_LOOP:
                    Golive_with_MIDI_LOOP(false);
                    break;

                default:
                    Serial.println("Switch MISSING! 6398");
                    break;
                }
                break;
            }
        }
    }

#pragma endregion // SETUP

#pragma region CC Settings [rgba(197, 197, 192, 0.37)]
    // *************************************************************
    // ********************   CC_SETTINGS   ************************
    // *************************************************************
    if (Lilla_state == CC_SETTINGS)
    {
#ifdef PCB_2022
        if (Read_encoder_inverse(25, CC_menu, 9, 0, 1))
#else
        if (Read_encoder(25, CC_menu, 9, 0, 1))
#endif
        {
            Display.Frame_CC_page_menu(CC_menu);
            if (CC_menu > 0 && CC_menu < 9)
            {
                CC_number = CC_Sound_gain[CC_menu - 1];
            }
            else if (CC_menu == 9)
            {
                CC_number = CC_lowpass_filter;
            }
        }

#ifdef PCB_2022
        if (Read_encoder_inverse(24, CC_number, 127, 0, 1))
#else
        if (Read_encoder(24, CC_number, 127, 0, 1))
#endif
        {
            if (CC_menu > 0 && CC_menu < 9)
            {
                CC_Sound_gain[CC_menu - 1] = CC_number;
                Display.Show_CC_Sound_gain(CC_menu - 1);
            }
            else if (CC_menu == 9)
            {
                CC_lowpass_filter = CC_number;
                Display.Show_CC_lowpass_filter();
            }
        }

        // scegli l'item
        if (Read_pushbutton(24))
        {
            if (CC_menu > 0 && CC_menu < 9)
            {
                CC_Sound_gain[CC_menu - 1] = 0;
                Display.Show_CC_Sound_gain(CC_menu - 1);
            }
            else if (CC_menu == 9)
            {
                CC_lowpass_filter = 0;
                Display.Show_CC_lowpass_filter();
            }
        }

        // Autolearning
        if (display_wait)
        {
            if (CC_menu > 0 && CC_menu < 9)
            {
                CC_Sound_gain[CC_menu - 1] = CC_midi_controller;
                Display.Show_CC_Sound_gain(CC_menu - 1);
                CC_number = CC_Sound_gain[CC_menu - 1];
            }
            else if (CC_menu == 9)
            {
                CC_lowpass_filter = CC_midi_controller;
                Display.Show_CC_lowpass_filter();
                CC_number = CC_lowpass_filter;
            }
            display_wait = false;
        }

        // Return to SETUP
        if (Read_pushbutton(25) && CC_menu == 0)
        {
            Save_CC_SETTINGS();
            Golive_SETUP();
        }
    }
}

#pragma endregion // CC_SETTINGS

// **************************************************************************************************************************
// **************************************************************************************************************************
// *************************************************   FUNCTIONS   **********************************************************
// **************************************************************************************************************************
// **************************************************************************************************************************

// ***************************************************************************************************************
// **********************************                   TABLES                  **********************************
// ***************************************************************************************************************

FLASHMEM
void Compile_tables(void)
{
    const float value_float = 16.0;

    for (auto i = 0; i < 10; ++i)
    {
        m_exp_table[i] = exp_table[i + 1] - exp_table[i];
        m_sin_table[i] = sin_table[i + 1] - sin_table[i];
        m_decay_table[i] = decay_table[i + 1] - decay_table[i];
        m_release_table[i] = release_table[i + 1] - release_table[i];
    }
    for (auto i = 0; i <= 32; ++i)
    {
        pan_gain_L_table[i] = sin((value_float - (i - 16)) * 0.049087f); // Left channel , 0.049087 = M_PI/64.0
        pan_gain_R_table[i] = sin((value_float + (i - 16)) * 0.049087f); // Right channel , 0.049087 = M_PI/64.0
    }
}

// ***************************************************************************************************************
// **********************************                 UTILITIES                 **********************************
// ***************************************************************************************************************

void Calc_pitch_from_note(void)
{
    float keys = (key_step + 1) * 12;
    for (auto note = 0; note < 128; ++note)
    {
        pitch_from_note[note] = pow(2.0f, (note - 60.0f) / keys); // array used to translate note number to pitch value
    }

    return;
}

template <class T>
FLASHMEM void P(String &what, T &value)
{
    Serial.print(what);
    Serial.print(": ");
    Serial.println(value);
}

// ***************************************************************************************************************
// **********************************                  DISPLAY                  **********************************
// ***************************************************************************************************************

int x_pos(float col)
{
    return 4 + (6.0 * col);
}

// ***************************************************************************************************************
// **********************************          INSTRUMENT MAPPING               **********************************
// ***************************************************************************************************************

void Reset_map_Instrument_for_notes(uint8_t instrument)
{
    for (auto note = 0; note < 128; ++note)
    {
        bitWrite(map_instrument_for_note[Get_midi_channel(Patch_id, instrument)][note], instrument, 0);
    }
}

void Delete_one_map_Instrument_for_notes(uint8_t instrument)
{
    for (auto note = 0; note < 128; ++note)
    {
        bitWrite(map_instrument_for_note[Get_midi_channel(Patch_id, instrument)][note], instrument, 0);
    }
}

void Update_all_maps_Instrument_for_notes()
{
    Reset_all_maps_Instrument_for_notes();
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        if (Patch[Patch_id].Instrument[instrument_local].used)
        {
            Update_map_Instrument_for_notes(Patch[Patch_id].Instrument[instrument_local].from_note, Patch[Patch_id].Instrument[instrument_local].to_note, instrument_local);
        }
    }
}

void Map_one_Instrument_for_all_notes(uint8_t instrument)
{
    Reset_all_maps_Instrument_for_notes();
    for (auto note = 0; note < 128; ++note)
    {
        bitWrite(map_instrument_for_note[Get_midi_channel(Patch_id, instrument)][note], instrument, 1);
    }
}

void Reset_all_maps_Instrument_for_notes()
{
    for (auto midi_ch = 0; midi_ch < 16; ++midi_ch)
    {
        for (auto note = 0; note < 128; ++note)
        {
            map_instrument_for_note[midi_ch][note] = 0;
        }
    }
}

// ***************************************************************************************************************
// **********************************                 PERFORMANCE               **********************************
// ***************************************************************************************************************

FLASHMEM
void Delete_all_Patches_and_Sounds(void)
{
    for (auto patch_local = 0; patch_local < PATCHES_MAX; ++patch_local)
    {
        Patch[patch_local].used = false;
        Patch[patch_local].instruments = 0; // number of instruments in the patch_id
        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
        {
            Patch[patch_local].Instrument[instrument_local].used = false;
            Patch[patch_local].Instrument[instrument_local].sound_id = 0;
            Patch[patch_local].Instrument[instrument_local].from_note = 0;
            Patch[patch_local].Instrument[instrument_local].to_note = 0;
            Patch[patch_local].Instrument[instrument_local].root_key = 0;
            Patch[patch_local].Instrument[instrument_local].precedence = 0;
            Patch[patch_local].Instrument[instrument_local].lock = 0;

            Patch[patch_local].Instrument[instrument_local].Filter.use = 0;        // yes/no
            Patch[patch_local].Instrument[instrument_local].Filter.type = 0;       // bit4,5:filter_type
            Patch[patch_local].Instrument[instrument_local].Filter.pivot = 0;      // 0 --> 100 filter frequency/note frequency
            Patch[patch_local].Instrument[instrument_local].Filter.resonance = 0;  // 0 --> 40
            Patch[patch_local].Instrument[instrument_local].Filter.modulation = 0; // bit1,2,3:modulation
            Patch[patch_local].Instrument[instrument_local].Filter.index = 0;      // 1 --> 20 modulation_index
            Patch[patch_local].Instrument[instrument_local].Filter.frequency_time = 0;
        }
    }

    for (auto sound = 0; sound < SOUNDS_MAX; ++sound)
    {
        Sound[sound].used = false;
        Sound[sound].file = 0;
        Sound[sound].mode = 0;
        Sound[sound].pitch = 0;
        Sound[sound].A = 0;
        Sound[sound].B = 0;
        Sound[sound].Noclick = 0;
        Sound[sound].pan = 0;
        Sound[sound].data = 2;
        Sound[sound].attack = 0;
        Sound[sound].decay = 0;
        Sound[sound].sustain = 0;
        Sound[sound].release = 0;
        Sound[sound].gain = 0; // 20 means gain = 1.0
    }
}

FLASHMEM
void Read_all_Patches(void)
{
    for (auto patch_local = 0; patch_local < PATCHES_MAX; ++patch_local)
    {
        Archive.Read_Patch(patch_local);
    }
}

FLASHMEM
void Update_Patches_number(void)
{
    patches_number = 0;
    for (auto patch_local = 0; patch_local < PATCHES_MAX; ++patch_local)
    {
        if (Patch[patch_local].used)
        {
            ++patches_number;
        }
    }
}

FLASHMEM
uint8_t Get_first_Patch_id_existing(void)
{
    for (auto patch_local = 0; patch_local < PATCHES_MAX; ++patch_local)
    {
        if (Patch[patch_local].used)
        {
            return patch_local;
        }
    }
    return 0;
}

FLASHMEM
uint8_t Get_next_Patch_id_existing(void)
{
    uint8_t patch_local = Patch_id;
    do
    {
        ++patch_local;
        if (patch_local == PATCHES_MAX)
        {
            return Patch_id;
        }
        else if (Patch[patch_local].used)
        {
            return patch_local;
        }
    } while (1);
}

uint8_t Get_previous_Patch_id_existing(void)
{
    int8_t patch_local = Patch_id;
    do
    {
        --patch_local;
        if (patch_local == -1)
        {
            return Patch_id;
        }
        if (Patch[patch_local].used)
        {
            return patch_local;
        }
    } while (1);
}

void Update_Instruments_positions(void)
{
    uint8_t position = 0;
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        position_of_Instrument[instrument_local] = -1;
        instrument_on_position[instrument_local] = -1;
    }
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        if (Patch[Patch_id].Instrument[instrument_local].used)
        {
            position_of_Instrument[instrument_local] = position;
            instrument_on_position[position] = instrument_local;
            position++;
        }
    }
}

void Golive_with_PERFORMANCE(int patch_id)
{
    Lilla_state = PERFORMANCE;

    patch_original = Verify_is_Patch_original(patch_id);
    P_menu = 0;
    Select_performance_menu_elements();
    Update_Instruments_positions();
    Display.Performance_page(true, true);
    Performance_led_set.Request_all_LED_switch_off();
    Display.Frame_performance_menu(P_menu, true);

    P_Lilla_state();
    P_Patch(patch_id);
}

void Rebuild_patch_old(void)
{
    Players_Manager.Release_softly_all_players(Patch_id);
    Players_statistics.Reset_total_Players_per_instrument();

    Patch_id = patch_old;
    Update_Instruments_positions();
    Update_all_maps_Instrument_for_notes();
    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
    Fill_all_Noclick();
    Fill_all_Wavetable();
}

FLASHMEM
void Ask_if_change_Patch(void)
{
    confirmation = false;
    action = 0;
    Display.Confirm_patch_change_popup();
    Display.Confirm_patch_change_popup_frame(0);
    delay(200);
    while (!confirmation)
    {
#ifdef PCB_2022
        if (Read_encoder_inverse(25, action, 2, 0, 1))
#else
        if (Read_encoder(25, action, 2, 0, 1))
#endif
        {
            Display.Confirm_patch_change_popup_frame(action);
        }

        if (Read_pushbutton(25))
        {
            confirmation = true;
        }

#ifdef PCB_2022
        if (Read_encoder_inverse(24, action, 2, 0, 1))
#else
        if (Read_encoder(24, action, 2, 0, 1))
#endif
        {
            Display.Confirm_patch_change_popup_frame(action);
        }

        if (Read_pushbutton(24))
        {
            confirmation = true;
        }
    }
}

FLASHMEM
void Ask_if_delete_this_Patch(void)
{
    confirmation = false;
    action = 0; // NO
    Display.Confirm_patch_delete_popup();
    Display.Confirm_patch_delete_popup_frame(0);
    delay(200);
    while (!confirmation)
    {
#ifdef PCB_2022
        if (Read_encoder_inverse(25, action, 1, 0, 1))
#else
        if (Read_encoder(25, action, 1, 0, 1))
#endif
        {
            Display.Confirm_patch_delete_popup_frame(action);
        }

        if (Read_pushbutton(25))
        {
            confirmation = true;
        }
    }
}

void Jump_to_Patch(uint8_t next_patch)
{
    AudioNoInterrupts();
    Players_Manager.Release_softly_all_players(Patch_id);
    Players_statistics.Reset_total_Players_per_instrument();

    // switch
    Patch_id = next_patch;
    Update_all_maps_Instrument_for_notes();
    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
    Fill_all_Noclick();
    Fill_all_Wavetable();
    AudioInterrupts();

    Patch_cache_P = Patch[Patch_id];
    Copy_all_Sound_to_Sound_cache_P();

    Golive_with_PERFORMANCE(Patch_id);
}

FLASHMEM
void Macro_Instrument_editing(void)
{
    Display.Show_Instrument_description(Patch_id, Instrument_id, true);

    // restore all LED
    Performance_led_set.Restore_all_LED();

    patch_original_0 = patch_original;
    patch_original = Verify_is_Patch_original(Patch_id);
    if (patch_original != patch_original_0)
    {
        Select_performance_menu_elements();
        Display.Performance_menu(); // display the menu and update "P_menu_max"
        P_menu = P_menu_max + 1 + Instrument_id;
    }
}

FLASHMEM
bool Verify_is_Patch_original(const int patch_id)
{
    bool result = true;
    if (Patch[patch_id].instruments == Patch_cache_P.instruments)
    {
        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
        {
            result = result && Verify_if_Instrument_original(instrument_local);
        }
        Serial.print("VERIFY_is_Patch_original: ");
        Serial.println(result);
        return result;
    }
    else
        return false;
}

FLASHMEM
void Select_performance_menu_elements(void)
{
    // voices that can be displayed
    Menu_P[0] = true; // Exit
    Menu_P[1] = true; // Save
    Menu_P[2] = true; // Clone
    Menu_P[3] = true; // SaveAsNew
    Menu_P[4] = true; // DropPatch

    if (exibition)
    {
        Menu_P[1] = false; // Save
        Menu_P[2] = false; // Clone
        Menu_P[3] = false; // SaveAsNew
        Menu_P[4] = false; // DropPatch
    }

    if (patch_original)
    {
        Menu_P[0] = false; // Exit
        Menu_P[1] = false; // Save
        Menu_P[3] = false; // SaveAsNew
    }

    if (!patch_original)
    {
        Menu_P[2] = false; // Clone
    }

    if (patches_number == PATCHES_MAX)
    {
        Menu_P[2] = false; // Clone
        Menu_P[3] = false; // SaveAsNew
    }

    if (patches_number == 1)
    {
        Menu_P[4] = false;
    }

    if (Get_sounds_free() < Patch[Patch_id].instruments)
    {
        Menu_P[2] = false; // Clone
        Menu_P[3] = false; // SaveAsNew
    }

    P_menu_max = Menu_P[0] + Menu_P[1] + Menu_P[2] + Menu_P[3] + Menu_P[4] - 1;
}

// ***************************************************************************************************************
// **********************************           SOUND, INSTRUMENT              ***********************************
// ***************************************************************************************************************

void Set_midi_channel_for_Sound(uint8_t sound_id, uint8_t midi_channel)
{
    // .data contains midi channel in its bits: 7 6 5 M I D I 0
    Sound[sound_id].data = (midi_channel << 1) + (Sound[sound_id].data & 0b11100000);
}

FLASHMEM
uint8_t Get_midi_channel_from_Sound(uint8_t sound_id)
{
    // .data contains midi channel in its bits: 7 6 5 M I D I 0
    return ((Sound[sound_id].data & 30) >> 1);
}

int8_t Get_Patch_id_free(void)
{
    for (auto local_patch = 0; local_patch < PATCHES_MAX; ++local_patch)
    {
        if (!Patch[local_patch].used)
        {
            return local_patch;
        }
    }
    return -1;
}

int8_t Get_sound_free(void)
{
    for (auto sound_id = 0; sound_id < SOUNDS_MAX; ++sound_id)
    {
        if (!Sound[sound_id].used)
        {
            return sound_id;
            Sound[sound_id].used = true;
        }
    }
    return -1;
}

void Set_Sound_SOLO_OFF(void)
{
    AudioNoInterrupts();
    if (solo_flag)
    {
        solo_flag = false;
        Update_all_maps_Instrument_for_notes();
    }
    AudioInterrupts();
}

FLASHMEM
uint32_t Calc_trim_step(uint8_t value)
{
    value = value % 6;
    switch (value)
    {
    case 0:
        return 1;
        break;
    case 1:
        return 10;
        break;
    case 2:
        return 100;
        break;
    case 3:
        return 1000;
        break;
    case 4:
        return 10000;
        break;
    case 5:
        return (Sound[Sound_id].B - Sound[Sound_id].A) / 16;
        break;

    default:
        return 1;
        break;
    }
}

void Drop_Instrument(uint8_t instrument)
{
    Sound[Patch[Patch_id].Instrument[instrument].sound_id].used = false;
    Patch[Patch_id].Instrument[instrument].used = false;
    Patch[Patch_id].instruments--;
}

uint8_t Clone_Instrument(uint8_t instrument)
{
    int new_instrument;
    for (new_instrument = 0; new_instrument < INSTRUMENTS_MAX; ++new_instrument)
    {
        if (!Patch[Patch_id].Instrument[new_instrument].used)
        {
            break;
        }
    }

    Patch[Patch_id].Instrument[new_instrument] = Patch[Patch_id].Instrument[instrument];
    int S = Get_sound_free();
    if (S >= 0)
    {
        Sound[S] = Sound[Patch[Patch_id].Instrument[instrument].sound_id];
        Sound[S].gain = 0;
        Patch[Patch_id].Instrument[new_instrument].sound_id = S;
        Patch[Patch_id].instruments++;
        // instruments ++;
        return new_instrument;
    }
    else
        return 0;
}

// ***************************************************************************************************************
// **********************************                 LEDS                     ***********************************
// ***************************************************************************************************************

void Update_instruments_leds()
{
    if (Lilla_state == MIDI_LOOP)
    {
        for (auto track = 0; track < TRACKS; ++track)
        {
            for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
            {
                if (Patch[Patch_id].Instrument[instrument_local].used) // check if i is used
                {
                    // check led activity
                    if (Loop_led_set.Read_LED_activity(track, instrument_local) == 2)
                    {
                        Display.Loop_led(track, instrument_local, true);
                        Loop_led_set.Write_LED_activity(track, instrument_local, true);
                    }
                    if (Loop_led_set.Read_LED_activity(track, instrument_local) == -2)
                    {
                        Display.Loop_led(track, instrument_local, false);
                        Loop_led_set.Write_LED_activity(track, instrument_local, false);
                    }
                }
            }
        }
    }

    else if (Lilla_state == PERFORMANCE)
    {
        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
        {
            if (Patch[Patch_id].Instrument[instrument_local].used) // check if i is used
            {
                // // check led activity
                if (Performance_led_set.Read_LED_activity(instrument_local) == 2)
                {
                    Display.Led_PERFORMANCE_instrument(instrument_local, true);
                    Performance_led_set.Write_LED_activity(instrument_local, true);
                }
                if (Performance_led_set.Read_LED_activity(instrument_local) == -2)
                {
                    Display.Led_PERFORMANCE_instrument(instrument_local, false);
                    Performance_led_set.Write_LED_activity(instrument_local, false);
                }
            }
        }

        if (TT_led_flag) // TUNING_TONE
        {
            TT_led_flag = false;
            Display.Led_tuning_tone(Patch_id);
        }
    }

    else if (Lilla_state == SOUND_EDIT)
    {
        // check led activity
        if (Performance_led_set.Read_LED_activity(Instrument_id) == 2)
        {
            Display.Led_SOUND_EDIT_instrument(Instrument_id, true);
            Performance_led_set.Write_LED_activity(Instrument_id, true);
        }
        if (Performance_led_set.Read_LED_activity(Instrument_id) == -2)
        {
            Display.Led_SOUND_EDIT_instrument(Instrument_id, false);
            Performance_led_set.Write_LED_activity(Instrument_id, false);
        }

        if (TT_led_flag) // TUNING_TONE
        {
            TT_led_flag = false;
        }
    }

    else if (Lilla_state == INSTRUMENT_VCF)
    {
        // check led activity
        if (Performance_led_set.Read_LED_activity(Instrument_id) == 2)
        {
            Display.Led_INSTRUMENT_VCF_instrument(Instrument_id, true);
            Performance_led_set.Write_LED_activity(Instrument_id, true);
        }
        if (Performance_led_set.Read_LED_activity(Instrument_id) == -2)
        {
            Display.Led_INSTRUMENT_VCF_instrument(Instrument_id, false);
            Performance_led_set.Write_LED_activity(Instrument_id, false);
        }

        if (TT_led_flag) // TUNING_TONE
        {
            TT_led_flag = false;
        }
    }

    else if (Lilla_state == LIVE_SAMPLING)
    {
        // check led activity

        /*
        if (Performance_led_set.Read_LED_activity(0) == 2 || Performance_led_set.Read_LED_activity(1) == 2)
        {
            Display.Led_LIVE_SAMPLING(true);
            if (Performance_led_set.Read_LED_activity(0) == 2)
            {
                Performance_led_set.Write_LED_activity(0, true);
            }
            if (Performance_led_set.Read_LED_activity(1) == 2)
            {
                Performance_led_set.Write_LED_activity(1, true);
            }
        }

        else if (Performance_led_set.Read_LED_activity(0) == -2 && Performance_led_set.Read_LED_activity(1) == -2)
        {
            Display.Led_LIVE_SAMPLING(false);
            Performance_led_set.Write_LED_activity(0, false);
            Performance_led_set.Write_LED_activity(1, false);
        }
        */

        if (Performance_led_set.Read_LED_activity(0) == 2)
        {
            Display.Led_LIVE_SAMPLING(true);
            Performance_led_set.Write_LED_activity(0, true);
        }

        else if (Performance_led_set.Read_LED_activity(0) == -2)
        {
            Display.Led_LIVE_SAMPLING(false);
            Performance_led_set.Write_LED_activity(0, false);
        }

        if (TT_led_flag) // TUNING_TONE
        {
            TT_led_flag = false;
        }
    }
    else if (Lilla_state == DIRECT_SAMPLING)
    {

        // check led activity
        if (Performance_led_set.Read_LED_activity(0) == 2 || Performance_led_set.Read_LED_activity(1) == 2)
        {
            Display.Led_DIRECT_SAMPLING(true);
            if (Performance_led_set.Read_LED_activity(0) == 2)
            {
                Performance_led_set.Write_LED_activity(0, true);
            }
            if (Performance_led_set.Read_LED_activity(1) == 2)
            {
                Performance_led_set.Write_LED_activity(1, true);
            }
        }

        else if (Performance_led_set.Read_LED_activity(0) == -2 && Performance_led_set.Read_LED_activity(1) == -2)
        {
            Display.Led_DIRECT_SAMPLING(false);
            Performance_led_set.Write_LED_activity(0, false);
            Performance_led_set.Write_LED_activity(1, false);
        }

        if (TT_led_flag) // TUNING_TONE
        {
            TT_led_flag = false;
        }
    }
    else
        return;
}

// ***************************************************************************************************************
// ******************************************       DIRECT_SAMPLING       ****************************************
// ***************************************************************************************************************

void DS_setup_DIRECT_SAMPLING_Patch_and_Preset(void)
{
    DS_set_DS_Sampling_Patch();
    Patch_id = PATCHES_MAX;
    Update_all_maps_Instrument_for_notes();
    Turn_ON_Delay(false); // switch on/off Delay (using Instrument routing)
    recording = DS_get_last_Recording();

    // Set up Sound parameters
    if (recording >= 0)
    {
        Sound[SOUNDS_MAX].file = 2 * recording + FIRST_RECORDING_FILE;
        Sound[SOUNDS_MAX].B = DS_get_samples_in_Recording(recording) - 1;

        Sound[SOUNDS_MAX + 1].file = Sound[SOUNDS_MAX].file + (Recording[recording].stereo ? 1 : 0);
        Sound[SOUNDS_MAX + 1].B = Sound[SOUNDS_MAX].B;
        P_Recording(recording);
    }
    else
    {
        Sound[SOUNDS_MAX].file = 0;
        Sound[SOUNDS_MAX].B = 100000;

        Sound[SOUNDS_MAX + 1].file = 0;
        Sound[SOUNDS_MAX + 1].B = 100000;
    }
    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
}

void Golive_DIRECT_SAMPLING(void)
{
    Lilla_state = DIRECT_SAMPLING;
    DS_state = 0;

    // Switch bar_display ON
    PeakTracking_L.reset();
    PeakTracking_R.reset();

    DS_menu = 0;
    Display.DS_page(Instrument_id, recording);
    Display.DS_line_out(false);

    DS_define_model();
    Display.DS_menu(); // display the menu and updates DS_menu_max
    Display.DS_frame_menu(DS_menu);
    Display.DS_bar(0, 0);
    Display.DS_bar(1, 0);

    P_Patch(Patch_id);
    Serial.println(F("*** DIRECT_SAMPLING ***  Sounds are:"));
    P_Sound(SOUNDS_MAX);
    P_Sound(SOUNDS_MAX + 1);
}

FLASHMEM
void DS_refresh_DS_page(void)
{
    Lilla_state = DIRECT_SAMPLING;
    Display.DS_page(Instrument_id, recording);
    Display.DS_line_out(false);
    DS_define_model();
    Display.DS_menu(); // display the menu and updates DS_menu_max
    Display.DS_frame_menu(DS_menu);
    Display.DS_bar(0, 0);
    Display.DS_bar(1, 0);
}

FLASHMEM
void DS_ask_if_EXIT_from_DS(void)
{
    confirmation = false;
    action = 0; // NO
    Display.DS_confirm_EXIT_from_DS();
    Display.Confirm_patch_delete_popup_frame(0);
    delay(200);
    while (!confirmation)
    {
#ifdef PCB_2022
        if (Read_encoder_inverse(25, action, 1, 0, 1))
#else
        if (Read_encoder(25, action, 1, 0, 1))
#endif
        {
            Display.Confirm_patch_delete_popup_frame(action);
        }

        if (Read_pushbutton(25))
        {
            confirmation = true;
        }
    }
}

void Jump_to_DIRECT_SAMPLING_recording(int &recording)
{
    if (recording >= 0)
    {
        Sound[SOUNDS_MAX].file = 2 * recording + FIRST_RECORDING_FILE;
        Sound[SOUNDS_MAX].B = DS_get_samples_in_Recording(recording) - 1;

        Sound[SOUNDS_MAX + 1].file = Sound[SOUNDS_MAX].file + (Recording[recording].stereo ? 1 : 0);
        Sound[SOUNDS_MAX + 1].B = Sound[SOUNDS_MAX].B;
    }
    else
    {
        Sound[SOUNDS_MAX].file = 0;
        Sound[SOUNDS_MAX].B = 100000;
        Sound[SOUNDS_MAX + 1].file = 0;
        Sound[SOUNDS_MAX + 1].B = 100000;
    }

    AudioNoInterrupts();
    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
    AudioInterrupts();

    P_Sound(SOUNDS_MAX);
    P_Sound(SOUNDS_MAX + 1);

    Display.DS_hide_recording();

    Display.DS_Recording_description(recording, true);

    // restore LEDs
    Performance_led_set.Restore_all_LED();
}

void DS_back_to_first_DS_Recording(void)
{
    if (recording >= 0)
    {
        Sound[SOUNDS_MAX].file = 2 * recording + FIRST_RECORDING_FILE;
        Sound[SOUNDS_MAX].B = DS_get_samples_in_Recording(recording) - 1;

        Sound[SOUNDS_MAX + 1].file = Sound[SOUNDS_MAX].file + (Recording[recording].stereo ? 1 : 0);
        Sound[SOUNDS_MAX + 1].B = Sound[SOUNDS_MAX].B;
    }
    else
    {
        Sound[SOUNDS_MAX].file = 0;
        Sound[SOUNDS_MAX].B = 100000;
        Sound[SOUNDS_MAX + 1].file = 0;
        Sound[SOUNDS_MAX + 1].B = 100000;
    }

    AudioNoInterrupts();
    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
    AudioInterrupts();

    P_Sound(SOUNDS_MAX);
    P_Sound(SOUNDS_MAX + 1);

    DS_menu = 0;
    DS_define_model(); // updates "Value_Max_encoder.DS_menu" used by encoder_menu
    Display.DS_menu();
    Display.DS_frame_menu(0);
    Display.DS_hide_recording();

    Display.DS_Recording_description(recording, true);

    // restore LEDs
    Performance_led_set.Restore_all_LED();
}

FLASHMEM
void DS_convert_file_L(int file_L_RAW, int bytes) // bytes = blocks_per_file * 256
{
    Serial.println("*** Convert file_L ***");

    // create the file on the Flash chip and copy data
    Serial.print(F("Create file: "));
    Serial.print(name_file[file_L_RAW]);
    Serial.print(F(" dimension (bytes): "));
    Serial.println(bytes);

    char buffer[256];
    int packet = 0;
    int last_blocks = -1;

    // creazione file vuoto
    SerialFlash.create(name_file[file_L_RAW], bytes);

    // apertura file
    SerialFlashFile destination_file = SerialFlash.open(name_file[file_L_RAW]);

    // copia file, caso MONO
    if (!Recording[recording].stereo)
    {
        // copia dal primo al penultimo packet
        if (Recording[recording].packets > 1)
        {
            for (packet = Recording[recording].first_packet; packet < (Recording[recording].first_packet + Recording[recording].packets - 1); ++packet)
            {
                SerialFlashFile source_file = SerialFlash.open(name_packet[packet]);
                for (auto i = 0; i < 256; ++i)
                {
                    source_file.read(buffer, 256);
                    destination_file.write(buffer, 256);
                }
            }
        }
        // copia l'ultimo packet
        packet = Recording[recording].first_packet + Recording[recording].packets - 1;
        SerialFlashFile source_file = SerialFlash.open(name_packet[packet]);
        last_blocks = (Recording[recording].bytes % PACKET_DIM) % 256;
        for (auto i = 0; i < last_blocks; ++i)
        {
            source_file.read(buffer, 256);
            destination_file.write(buffer, 256);
        }
    }

    // copia file, caso MONO
    else
    {
        // file_L
        // copia dal primo al penultimo packet
        if (Recording[recording].packets > 1)
        {
            for (packet = Recording[recording].first_packet; packet < (Recording[recording].first_packet + 2 * (Recording[recording].packets - 1)); packet += 2)
            {
                SerialFlashFile source_file = SerialFlash.open(name_packet[packet]);
                for (auto i = 0; i < 256; ++i)
                {
                    source_file.read(buffer, 256);
                    destination_file.write(buffer, 256);
                }
            }
        }
        // copia l'ultimo packet
        packet = Recording[recording].first_packet + 2 * (Recording[recording].packets - 1);
        SerialFlashFile source_file = SerialFlash.open(name_packet[packet]);
        last_blocks = (Recording[recording].bytes % PACKET_DIM) % 256;
        for (auto i = 0; i < last_blocks; ++i)
        {
            source_file.read(buffer, 256);
            destination_file.write(buffer, 256);
        }
    }
}

FLASHMEM
void DS_convert_file_R(int file_R_RAW, int bytes) // bytes = blocks_per_file * 256
{
    Serial.println("*** Convert file_R ***");

    // create the file on the Flash chip and copy data
    Serial.print(F("Create file: "));
    Serial.print(name_file[file_R_RAW]);
    Serial.print(F(" dimension (bytes): "));
    Serial.println(bytes);

    char buffer[256];
    int packet = 0;
    int last_blocks = -1;

    SerialFlash.create(name_file[file_R_RAW], bytes);
    SerialFlashFile destination_file = SerialFlash.open(name_file[file_R_RAW]);

    // copia dal primo al penultimo packet
    if (Recording[recording].packets > 1)
    {
        for (packet = Recording[recording].first_packet + 1; packet < (Recording[recording].first_packet + 1 + 2 * (Recording[recording].packets - 1)); packet += 2)
        {
            SerialFlashFile source_file = SerialFlash.open(name_packet[packet]);
            for (auto i = 0; i < 256; ++i)
            {
                source_file.read(buffer, 256);
                destination_file.write(buffer, 256);
            }
        }
    }
    // copia l'ultimo packet
    packet = Recording[recording].first_packet + 1 + 2 * (Recording[recording].packets - 1);
    SerialFlashFile source_file = SerialFlash.open(name_packet[packet]);
    last_blocks = (Recording[recording].bytes % PACKET_DIM) % 256;
    for (auto i = 0; i < last_blocks; ++i)
    {
        source_file.read(buffer, 256);
        destination_file.write(buffer, 256);
    }
}

void Print_Directory(File dir, int numSpaces)
{
    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            // Serial.println("** no more files **");
            break;
        }
        Serial.print("   ");
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            Print_Directory(entry, numSpaces + 2);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("   ");
            Serial.println(entry.size(), DEC);
            big_result += entry.size();
        }
        entry.close();
    }
}

void DS_seed_all_Recordings(void)
{
    Serial.println(F("*** DS_seed_all_Recordings() ***"));
    for (auto i = 0; i < RECORDINGS; ++i)
    {
        Recording[i].first_packet = 0;
        Recording[i].packets = 0;
        Recording[i].bytes = 0;
        Recording[i].seconds = 0.0; // float
        Recording[i].stereo = 0;
        Recording[i].consistent = true;
        Archive.Save_DS_Recording(i);
        Serial.print(F("Seeded Recording: "));
        Serial.println(i);
    }
    Serial.println(F("*** Finished ***"));
    Serial.println();
}

void DS_update_recordings(void)
{
    recordings = 0;
    for (auto i = 0; i < RECORDINGS; ++i)
        if (Recording[i].packets > 0 && Recording[i].consistent == true)
        {
            recordings++;
        }
    Serial.print(F("recordings are: "));
    Serial.println(recordings);
    Serial.println();
}

void DS_read_all_Recordings(void)
{
    for (auto i = 0; i < RECORDINGS; ++i)
    {
        DS_read_Recording(i);
    }
}

void DS_read_Recording(int recording)
{
    if (recording >= 0 && recording < RECORDINGS)
    {
        Archive.Read_DS_Recording(recording, EEPROM_Recording[recording]);

        Recording[recording].first_packet = EEPROM_Recording[recording].first_packet;
        Recording[recording].packets = EEPROM_Recording[recording].packets;
        Recording[recording].stereo = bitRead(EEPROM_Recording[recording].info, 1);
        Recording[recording].consistent = bitRead(EEPROM_Recording[recording].info, 0);
        Recording[recording].bytes = 2 * DS_get_samples_in_Recording(recording);
        Recording[recording].seconds = DS_get_Recording_seconds(recording);

        Serial.print(F("Read from EEPROM Recording: "));
        Serial.print(recording);
        Serial.print(F(" from location: "));
        Serial.println(Archive.GET_location_of_DS_Recording(recording));
        P_Recording(recording);
    }
    else
        Serial.println(F("***** WARNING! --> DS_read_Recording: 'recording' out of range"));
}

float DS_get_Recording_seconds(int value)
{
    return (DS_get_samples_in_Recording(value) / AUDIO_BLOCK_SAMPLES * 0.0029f); // float
}

int DS_find_Recording_free(void)
{
    for (auto i = 0; i < RECORDINGS; ++i)
        if (Recording[i].packets == 0)
        {
            return i;
        }
    return -1;
}

int DS_get_next_Recording(int value)
{
    if (value == RECORDINGS - 1)
    {
        return value;
    }
    for (auto i = (value + 1); i < RECORDINGS; ++i)
    {
        if (Recording[i].consistent && Recording[i].packets > 0)
        {
            return i;
        }
    }
    return value;
}

int DS_get_last_Recording(void)
{
    for (auto i = RECORDINGS - 1; i >= 0; --i)
    {
        if (Recording[i].consistent && Recording[i].packets > 0)
        {
            return i;
        }
    }
    return -1;
}

int DS_get_previous_Recording(int value)
{
    if (value < 0)
        return -1;

    for (auto i = (value - 1); i >= 0; --i)
    {
        if (Recording[i].consistent && Recording[i].packets > 0)
        {
            return i;
        }
    }
    return value;
}

bool DS_check_conversion(void)
{
    if ((Get_flash_size() - Get_flash_occupation()) >= Recording[recording].bytes)
    {
        for (auto i = 0; i < FIRST_RECORDING_FILE; ++i)
        {
            if (!SerialFlash.exists(name_file[i]))
            {
                return true;
            }
        }
    }
    return false;
}

FLASHMEM
void DS_define_model(void) // {"Exit"}, {"Delete"}, {"Pause+Rec"}, {"Mono Rec"}, {"Stereo Rec"}, {"Stop"}
{
    // voices that can be displayed
    Menu_DS[0] = true; // DELETE
    Menu_DS[1] = true; // PAUSE+REC
    Menu_DS[2] = true; // MONO-REC
    Menu_DS[3] = true; // STEREO-REC
    Menu_DS[4] = true; // STOP
    Menu_DS[5] = true; // CONVERT REC-TO-RAW

    Menu_DS[6] = true;  // CANCEL
    Menu_DS[7] = true;  // CONVERT MONO
    Menu_DS[8] = true;  // CONVERT LEFT
    Menu_DS[9] = true;  // CONVERT RIGHT
    Menu_DS[10] = true; // CONVERT BOTH
    Menu_DS[11] = true; // EXPORT TO SD

    if (DS_state == 1 || DS_state == 2 || DS_state == 3 || recordings == 0)
    {
        Menu_DS[11] = false; // EXPORT TO SD
    }

    if (DS_state != 3)
    {
        Menu_DS[6] = false;  // CANCEL
        Menu_DS[7] = false;  // CONVERT MONO
        Menu_DS[8] = false;  // CONVERT LEFT
        Menu_DS[9] = false;  // CONVERT RIGHT
        Menu_DS[10] = false; // CONVERT BOTH
    }

    if (DS_state == 3 && DS_export > 0)
    {
        Menu_DS[0] = false; // DELETE
        Menu_DS[1] = false; // PAUSE+REC
        Menu_DS[2] = false; // MONO-REC
        Menu_DS[3] = false; // STEREO-REC
        Menu_DS[4] = false; // STOP
        Menu_DS[5] = false; // CONVERT REC-TO-RAW
    }

    if (DS_state == 3 && DS_export == 0)
    {
        Menu_DS[7] = false;  // CONVERT_MONO
        Menu_DS[8] = false;  // CONVERT_LEFT
        Menu_DS[9] = false;  // CONVERT_RIGHT
        Menu_DS[10] = false; // CONVERT_BOTH
    }

    if (DS_state == 3 && DS_export == 1)
    {
        Menu_DS[8] = false;  // CONVERT LEFT
        Menu_DS[9] = false;  // CONVERT RIGHT
        Menu_DS[10] = false; // CONVERT BOTH
    }

    if (DS_state == 3 && DS_export == 2)
    {
        Menu_DS[7] = false; // CONVERT MONO
    }

    if (recordings == 0)
    {
        Menu_DS[0] = false; // CANCEL
    }

    if (Get_packets_free() < 4)
    {
        Menu_DS[1] = false; // PAUSE+REC
    }

    if (DS_state == 0)
    {
        Menu_DS[2] = false; // MONO-REC
        Menu_DS[3] = false; // STEREO-REC
        Menu_DS[4] = false; // STOP
    }

    if (DS_state == 1) // Pause+Rec
    {
        Menu_DS[0] = false; // CANCEL
        Menu_DS[1] = false; // PAUSE+REC
        Menu_DS[5] = false; // CONVERT REC-TO-RAW
    }

    if (DS_state == 2 || DS_state == 3) // Recording
    {
        Menu_DS[0] = false; // DELETE
        Menu_DS[1] = false; // PAUSE+REC
        Menu_DS[2] = false; // MONO-REC
        Menu_DS[3] = false; // STEREO-REC
        Menu_DS[5] = false; // CONVERT REC-TO-RAW
    }

    if (recording == -1)
    {
        Menu_DS[5] = false; // CONVERT REC-TO-RAW
    }

    if ((Get_flash_size() - Get_flash_occupation()) < Recording[recording].bytes) // if recording is stereo, at least one file can be saved
    {
        Menu_DS[5] = false; // CONVERT REC-TO-RAW
    }

    DS_menu_max = -1;
    for (auto i = 0; i < DS_MV; ++i)
    {
        DS_menu_max += Menu_DS[i];
    }
}

FLASHMEM
void DS_set_DS_Sampling_Patch(void)
{
    // set DS Patch and Sounds
    Patch[PATCHES_MAX].used = true;
    Patch[PATCHES_MAX].instruments = 2;

    // Left channel recording/instrument
    Patch[PATCHES_MAX].Instrument[0].used = true;
    Patch[PATCHES_MAX].Instrument[0].sound_id = SOUNDS_MAX;
    Patch[PATCHES_MAX].Instrument[0].root_key = 60;
    Patch[PATCHES_MAX].Instrument[0].from_note = 0;
    Patch[PATCHES_MAX].Instrument[0].to_note = 127;
    Patch[PATCHES_MAX].Instrument[0].precedence = false;
    Patch[PATCHES_MAX].Instrument[0].lock = false;
    Patch[PATCHES_MAX].Instrument[0].Filter.use = false;

    // Right channel recording/instrument
    Patch[PATCHES_MAX].Instrument[1].used = true;
    Patch[PATCHES_MAX].Instrument[1].sound_id = SOUNDS_MAX + 1;
    Patch[PATCHES_MAX].Instrument[1].root_key = 60;
    Patch[PATCHES_MAX].Instrument[1].from_note = 0;
    Patch[PATCHES_MAX].Instrument[1].to_note = 127;
    Patch[PATCHES_MAX].Instrument[1].precedence = false;
    Patch[PATCHES_MAX].Instrument[1].lock = false;
    Patch[PATCHES_MAX].Instrument[1].Filter.use = false;

    Patch[PATCHES_MAX].Instrument[2].used = false;
    Patch[PATCHES_MAX].Instrument[3].used = false;
    Patch[PATCHES_MAX].Instrument[4].used = false;
    Patch[PATCHES_MAX].Instrument[5].used = false;
    Patch[PATCHES_MAX].Instrument[6].used = false;
    Patch[PATCHES_MAX].Instrument[7].used = false;

    // LEFT Sound
    Sound[SOUNDS_MAX].used = true;
    Sound[SOUNDS_MAX].mode = 0;
    // Sound[SOUNDS_MAX].file = must be defined;
    Sound[SOUNDS_MAX].pitch = 0; // -128 + 127
    Sound[SOUNDS_MAX].A = 0;
    // Sound[SOUNDS_MAX].B = must be defined;
    Sound[SOUNDS_MAX].Noclick = 0;
    Sound[SOUNDS_MAX].pan = -16; // full Left
    Sound[SOUNDS_MAX].data = 0;  // bit4-3-2-1: midi_channel bit0: Attack ramp ("0" Slow, "1" Fast)
    Sound[SOUNDS_MAX].attack = 0;
    Sound[SOUNDS_MAX].decay = 50;
    Sound[SOUNDS_MAX].sustain = 50;
    Sound[SOUNDS_MAX].release = 10;
    Sound[SOUNDS_MAX].gain = 28;

    // RIGHT Sound
    Sound[SOUNDS_MAX + 1].used = true;
    Sound[SOUNDS_MAX + 1].mode = 0;
    // Sound[SOUNDS_MAX + 1].file = must be defined;
    Sound[SOUNDS_MAX + 1].pitch = 0; // -128 + 127
    Sound[SOUNDS_MAX + 1].A = 0;
    // Sound[SOUNDS_MAX + 1].B = must be defined;
    Sound[SOUNDS_MAX + 1].Noclick = 0;
    Sound[SOUNDS_MAX + 1].pan = 16; // full Right
    Sound[SOUNDS_MAX + 1].data = 0; // bit4-3-2-1: midi_channel bit0: Attack ramp ("0" Slow, "1" Fast)
    Sound[SOUNDS_MAX + 1].attack = 0;
    Sound[SOUNDS_MAX + 1].decay = 50;
    Sound[SOUNDS_MAX + 1].sustain = 50;
    Sound[SOUNDS_MAX + 1].release = 10;
    Sound[SOUNDS_MAX + 1].gain = 28;
}

int DS_get_samples_in_Recording(int recording)
{
    return Info.DS_recording_samples(recording);
}

FLASHMEM
void P_Recording(int value)
{
    Serial.print("Recording[");
    Serial.print(value);
    Serial.println("]");
    Serial.print(".first_packet = ");
    Serial.println(Recording[value].first_packet);
    Serial.print(".packets = ");
    Serial.print(Recording[value].packets);
    Serial.println(F(" (totale packets if mono, only file_L packets if stereo)"));
    Serial.print(".bytes = ");
    Serial.print(Recording[value].bytes);
    Serial.println(F(" (totale bytes if mono, only file_L bytes if stereo) "));
    Serial.print(".seconds = ");
    Serial.println(Recording[value].seconds, 1); // float
    Serial.print(".stereo = ");
    Serial.println((Recording[value].stereo ? "stereo" : "mono"));
    Serial.print(".consistent = ");
    Serial.println((Recording[value].consistent ? "yes" : "no"));
    Serial.println();
}

// ***************************************************************************************************************
// ******************************************            SWITCH           ****************************************
// ***************************************************************************************************************

void Switch_to_DIRECT_SAMPLING(void)
{
    AudioNoInterrupts();
    Players_Manager.Stop_all_players();
    DS_setup_DIRECT_SAMPLING_Patch_and_Preset();
    AudioInterrupts();
    Golive_DIRECT_SAMPLING();
}

void Switch_from_MIDI_LOOP_to_DIRECT_SAMPLING(void)
{
    AudioNoInterrupts();
    LOOP_stop_all_midi_tracks();
    DS_setup_DIRECT_SAMPLING_Patch_and_Preset();
    AudioInterrupts();
    Golive_DIRECT_SAMPLING();
}

void Switch_from_LIVE_SAMPLING_to_DIRECT_SAMPLING(void)
{
    if (LS_state == 1)
    {
        if (!LS_ask_if_exit_from_LS()) // false: remain
        {
            if (Lilla_state == MIXER)
            {
                Golive_MIXER();
            }

            else if (Lilla_state == LIVE_SAMPLING)
            {
                LS_refresh_LS_page();
            }
            else if (Lilla_state == INSTRUMENT_VCF)
            {
                Display.Instrument_VCF_page(Patch_id, Instrument_id);

                // restore LEDs
                Performance_led_set.Restore_all_LED();
            }
            else if (Lilla_state == SETUP)
            {
                Golive_SETUP();
            }
        }
        else // true: stop and exit
        {
            LS_state = 2;
            LiveSampler.Stop();
            Switch_to_DIRECT_SAMPLING();
        }
    }
    else // true: stop and exit
    {
        Switch_to_DIRECT_SAMPLING();
    }
}

void Golive_with_MIDI_LOOP(bool restart)
{
    Lilla_state = MIDI_LOOP;

    LOOP_menu = 0;
    LOOP_select_menu_elements();

    Display.LOOP_page();
    Display.Frame_loop_menu(LOOP_menu, true);

    // LEDs setup
    if (restart)
    {
        Players_statistics.Reset_total_Players_per_track_instrument();
        Loop_led_set.Request_all_LED_switch_off();
    }

    // Update_instruments_leds();

    LOOP_metronomo.Leds_off(); // spegni i LED del metronomo

    // Se esiste loop_0, accendi il metronomo
    if (LOOP_events[0] != 0)
    {
        if (restart)
        {
            LOOP_restart_clock();
            // Accendi primo led metronomo
            LOOP_metronomo.Led_ON(0);

            /*
            // calcolo prossimo evento metronomo
            LOOP_metronomo.metro_time = 0 + LOOP_metronomo.Read_metro_delta_ms();

            // avvia il metronomo
            LOOP_metronomo_run = true
            */
        }
    }
}

void Switch_from_PERFORMANCE_to_MIDI_LOOP(void)
{
    AudioNoInterrupts();
    Players_Manager.Stop_all_players();
    AudioInterrupts();
    Golive_with_MIDI_LOOP(true);
}

void Switch_from_DIRECT_SAMPLING_to_MIDI_LOOP(void)
{
    switch (DS_state)
    {
    case 0: // no activity
        AudioNoInterrupts();
        Rebuild_patch_old();
        Turn_ON_Delay(true);
        AudioInterrupts();
        Switch_from_PERFORMANCE_to_MIDI_LOOP();
        break;
    case 1: // pause + rec
        // switch OFF Line OUT monitor
        MAIN_mixer_out_L.gain(1, 0.0);
        MAIN_mixer_out_R.gain(1, 0.0);
        // Switch off blinking REC
        DS_blink_ON = false;

        AudioNoInterrupts();
        Rebuild_patch_old();
        Turn_ON_Delay(true);
        AudioInterrupts();
        Switch_from_PERFORMANCE_to_MIDI_LOOP();
        break;
    case 2: // recording
        DS_ask_if_EXIT_from_DS();
        if (action == 0) // remain
        {
            DS_refresh_DS_page();
        }
        else // stop and exit
        {
            DirectSampler.Book_stop();
            // switch OFF Line OUT monitor
            MAIN_mixer_out_L.gain(1, 0.0);
            MAIN_mixer_out_R.gain(1, 0.0);
            if (Recording[recording].packets == 0)
            {
                // Recording cancelled
                recording = DS_get_next_Recording(-1);
            }
            else
            {
                Recording[recording].consistent = true;
                // consistent Recording must be saved
                Archive.Save_DS_Recording(recording);
                DS_read_Recording(recording); // only to update .bytes and .seconds
            }

            DS_update_recordings();
            // Switch off blinking REC
            DS_blink_ON = false;

            AudioNoInterrupts();
            Rebuild_patch_old();
            Turn_ON_Delay(true);
            AudioInterrupts();
            Switch_from_PERFORMANCE_to_MIDI_LOOP();
        }
        break;
    case 3: // convert REC --> RAW
        AudioNoInterrupts();
        Rebuild_patch_old();
        Turn_ON_Delay(true);
        AudioInterrupts();
        Switch_from_PERFORMANCE_to_MIDI_LOOP();
        break;

    default:
        Serial.println("Switch MISSING! 6869");
        break;
    }
}

void Switch_from_LIVE_SAMPLING_to_MIDI_LOOP(void)
{
    if (LS_state == 1)
    {
        if (!LS_ask_if_exit_from_LS()) // false: remain
        {
            if (Lilla_state == LIVE_SAMPLING)
            {
                LS_refresh_LS_page();
            }
            else if (Lilla_state == INSTRUMENT_VCF)
            {
                Display.Instrument_VCF_page(Patch_id, Instrument_id);

                // restore LEDs
                Performance_led_set.Restore_all_LED();
            }
        }
        else // true: stop and exit
        {
            LS_state = 2;
            LiveSampler.Stop();

            AudioNoInterrupts();
            Rebuild_patch_old();
            AudioInterrupts();
            Switch_from_PERFORMANCE_to_MIDI_LOOP();
        }
    }
    else // true: stop and exit
    {
        AudioNoInterrupts();
        Rebuild_patch_old();
        AudioInterrupts();
        Switch_from_PERFORMANCE_to_MIDI_LOOP();
    }
}

void Golive_with_LIVE_SAMPLING(void)
{
    Lilla_state = LIVE_SAMPLING;
    LS_menu = 0;
    Display.LS_page();

    // restore LEDs
    Performance_led_set.Restore_all_LED();

    LS_define_model();
    Display.Show_LS_menu();
    Display.Frame_LS_menu(LS_menu);
    choice_LS_menu = element_Menu_LS[LS_menu];

    if (!LS_XY_lock)
    {
        LS_update_both_X_Y_samples();
    }
    else // altrimenti e' gia' stato calcolato
    {
        LS_update_Q_sample();
    }

    Display.Show_LS_ring_tape_wave(LS_id_sound);

    P_Lilla_state();
    P_Patch(Patch_id);
}

void Switch_from_PERFORMANCE_to_LIVE_SAMPLING(void)
{
    AudioNoInterrupts();
    Players_Manager.Stop_all_players();

    // Setup LIVE_SAMPLING
    LS_gain = 28;
    LINE_IN_amplifier.Set_gain(Volume_float[LS_gain]);

    Patch_id = PATCHES_MAX; // Live Sampler uses PATCHES_MAX
    LS_setup_LS_Patch(LS_stereo);

    Update_all_maps_Instrument_for_notes();
    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
    AudioInterrupts();

    Golive_with_LIVE_SAMPLING();
}

void Switch_from_MIDI_LOOP_to_LIVE_SAMPLING(void)
{
    AudioNoInterrupts();
    LOOP_stop_all_midi_tracks();

    // imposta LIVE_SAMPLING
    LS_gain = 28;
    LINE_IN_amplifier.Set_gain(Volume_float[LS_gain]);

    Patch_id = PATCHES_MAX; // Live Sampler uses PATCHES_MAX
    LS_setup_LS_Patch(LS_stereo);

    Update_all_maps_Instrument_for_notes();
    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
    AudioInterrupts();

    Golive_with_LIVE_SAMPLING();
}

void Switch_from_DIRECT_SAMPLING_to_LIVE_SAMPLING(void)
{
    switch (DS_state)
    {
    case 0: // no activity
        Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
        AudioNoInterrupts();
        Turn_ON_Delay(true);
        AudioInterrupts();
        break;
    case 1: // pause + rec
        // switch OFF Line OUT monitor
        MAIN_mixer_out_L.gain(1, 0.0);
        MAIN_mixer_out_R.gain(1, 0.0);
        // Switch off blinking REC
        DS_blink_ON = false;

        Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
        AudioNoInterrupts();
        Turn_ON_Delay(true);
        AudioInterrupts();
        break;
    case 2: // recording
        DS_ask_if_EXIT_from_DS();
        if (action == 0) // remain
        {
            DS_refresh_DS_page();
        }
        else // stop and exit
        {
            DirectSampler.Book_stop();
            // switch OFF Line OUT monitor
            MAIN_mixer_out_L.gain(1, 0.0);
            MAIN_mixer_out_R.gain(1, 0.0);
            if (Recording[recording].packets == 0)
            {
                // Recording cancelled
                recording = DS_get_next_Recording(-1);
            }
            else
            {
                Recording[recording].consistent = true;
                // consistent Recording must be saved
                Archive.Save_DS_Recording(recording);
                DS_read_Recording(recording); // only to update .bytes and .seconds
            }

            DS_update_recordings();
            // Switch off blinking REC
            DS_blink_ON = false;

            Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
            AudioNoInterrupts();
            Turn_ON_Delay(true);
            AudioInterrupts();
        }
        break;
    case 3: // convert REC --> RAW
        Switch_from_PERFORMANCE_to_LIVE_SAMPLING();
        AudioNoInterrupts();
        Turn_ON_Delay(true);
        AudioInterrupts();
        break;

    default:
        Serial.println("Switch MISSING! 7028");
        break;
    }
}

void Switch_to_PERFORMANCE_patch_old(void)
{
    AudioNoInterrupts();
    Rebuild_patch_old();
    AudioInterrupts();
    Golive_with_PERFORMANCE(Patch_id);
}

void Switch_from_MIDI_LOOP_to_PERFORMANCE(void)
{
    AudioNoInterrupts();
    LOOP_stop_all_midi_tracks();
    AudioInterrupts();

    // fissa lo stato run delle track (se registrate) e fermale; in questo modo al riento in MIDI_LOOP ripartiranno con ALL_START/STOP
    for (auto track = 0; track < TRACKS; ++track)
    {
        LOOP_track_run_memo[track] = LOOP_events[track] > 0;
        LOOP_track_run[track] = false;
    }
    Golive_with_PERFORMANCE(Patch_id);
}

void Switch_from_LIVE_SAMPLING_to_PERFORMANCE(void)
{
    if (LS_state == 1) // Recording
    {
        if (!LS_ask_if_exit_from_LS()) // false: remain
        {
            if (Lilla_state == LIVE_SAMPLING)
            {
                LS_refresh_LS_page();
            }
            else if (Lilla_state == INSTRUMENT_VCF)
            {
                // restore LEDs
                Performance_led_set.Restore_all_LED();
                Display.Instrument_VCF_page(Patch_id, Instrument_id);
            }
            else if (Lilla_state == MIXER)
            {
                Switch_to_MIXER();
            }
            else if (Lilla_state == DELAY_SETTINGS)
            {
                Display.Delay_page();
            }
        }
        else // true: stop and exit
        {
            LS_state = 2;
            LiveSampler.Stop();
            Switch_to_PERFORMANCE_patch_old();

            // Patch Delay: look for delay_<patch_id> in SD
            if (Archive.Copy_patch_Delay_data_from_SD_to_Eeprom(Patch_id))
            {
                Serial.println(F("Smooth changing of delay values COULD start..."));

                Delay_data_struct delay_final;
                Archive.Copy_patch_Delay_data_from_Eeprom_to_Ram(delay_final);

                AudioNoInterrupts();
                Delay_manager.New_values(&delay_final); // call using AudioNoInterrupt()
                AudioInterrupts();
            }
        }
    }
    else
    {
        Switch_to_PERFORMANCE_patch_old();
    }
}

void Switch_from_DIRECT_SAMPLING_to_PERFORMANCE(void)
{
    switch (DS_state)
    {
    case 0:                  // no activity
        Turn_ON_Delay(true); // switch on/off Delay (using Instrument routing)
        Switch_to_PERFORMANCE_patch_old();
        break;
    case 1: // pause + rec
        // switch OFF Line OUT monitor
        MAIN_mixer_out_L.gain(1, 0.0);
        MAIN_mixer_out_R.gain(1, 0.0);

        // Switch off blinking REC
        DS_blink_ON = false;

        Turn_ON_Delay(true); // switch on/off Delay (using Instrument routing)
        Switch_to_PERFORMANCE_patch_old();
        break;
    case 2: // recording
        DS_ask_if_EXIT_from_DS();
        if (action == 0) // remain
        {
            DS_refresh_DS_page();
        }
        else // stop and exit
        {
            DirectSampler.Book_stop();

            // switch OFF Line OUT monitor
            MAIN_mixer_out_L.gain(1, 0.0);
            MAIN_mixer_out_R.gain(1, 0.0);
            if (Recording[recording].packets == 0)
            {
                Serial.print(F("Recording: "));
                Serial.print(recording);
                Serial.println(F(" cancelled."));
                recording = DS_get_next_Recording(-1);
            }
            else
            {
                Recording[recording].consistent = true;

                // consistent Recording must be saved
                Archive.Save_DS_Recording(recording);
                DS_read_Recording(recording); // call for updating .bytes and .seconds
            }

            DS_update_recordings();

            // Switch off blinking REC
            DS_blink_ON = false;

            Turn_ON_Delay(true); // switch on/off Delay (using Instrument routing)
            Switch_to_PERFORMANCE_patch_old();
        }
        break;
    case 3:
        Turn_ON_Delay(true);
        Switch_to_PERFORMANCE_patch_old();
        break;

    default:
        Serial.println("Switch MISSING! 7165");
        break;
    }
}

void Switch_from_MIDI_LOOP_to_MIDI_MONITOR(void)
{
    AudioNoInterrupts();
    LOOP_stop_all_midi_tracks();
    AudioInterrupts();

    Golive_MIDI_MONITOR();
}

void Switch_from_MIDI_LOOP_to_SETUP(void)
{
    AudioNoInterrupts();
    LOOP_stop_all_midi_tracks();
    AudioInterrupts();

    Golive_SETUP();
}

void Switch_from_LIVE_SAMPLING_to_DELAY(void)
{
    Lilla_state_0 = LIVE_SAMPLING;
    if (Delay_values.instrument_route[0] || Delay_values.instrument_route[1])
    {
        Delay_values.instrument_route[0] = true;
        Delay_values.instrument_route[1] = true;
    }
    Lilla_state = DELAY_SETTINGS;
    Display.Delay_page();
}

void Golive_MIDI_MONITOR(void)
{
    AudioNoInterrupts();
    Players_Manager.Stop_all_players();
    AudioInterrupts();

    Lilla_state = MIDI_MONITOR;
    display_wait = false;
    Display.Midi_monitor_page();
}

void Golive_SETUP(void)
{
    Lilla_state = SETUP;
    SET_menu = 0;
    Display.Settings_page();
    Display.Settings_frame(SET_menu);
}

// ***************************************************************************************************************
// **********************************                MIDI_LOOP                  **********************************
// ***************************************************************************************************************
// Setup MIDI_LOOP

void LOOP_reset_all_data(void)
{
    LOOP_time = 0;
    LOOP_stretch_int = 100;
    for (auto track = 0; track < TRACKS; ++track)
    {
        LOOP_events[track] = 0;        // numero di eventi nel track
        LOOP_track_run[track] = false; // true --> il track va suonato
        LOOP_volume[track] = 1.0;
        LOOP_volume_int[track] = 20;
        LOOP_slide[track] = 0;     // slittamento temporale
        LOOP_pitch_int[track] = 0; // slittamento pitch
    }
}

void LOOP_stop_all_midi_tracks(void) // to be called inside AudioNoInterrupt()
{
    // Ferma i track running
    for (auto track = 0; track < TRACKS; ++track)
    {
        LOOP_track_run[track] = false; // true --> il track va suonato
    }

    // Interrompi tutti i Player
    Players_Manager.Stop_all_players();

    // Ferma il metronomo
    LOOP_metronomo_run = false;

    // Annulla l'ultima richiesta di aggiornamento proveniente da MidiReader
    LOOP_metronomo_flag_IN[1] = false;

    // simula Start/Stop all tracks con tutte le tracks attive
    LOOP_run_button_state = false;
}

void LOOP_stop_and_reset_runnig_loop_data(void)
{
    AudioNoInterrupts();

    // Interrompi i Player che eseguono note di qualsiasi track
    Players_Manager.Release_all_players_loop();

    LOOP_reset_all_data();

    // ferma il metronomo
    LOOP_metronomo_run = false;

    // cancel (if exists) last metronomo update request from MidiReader
    LOOP_metronomo_flag_IN[1] = false;

    AudioInterrupts();

    // switch off metronomo leds
    LOOP_metronomo.Leds_off(); // va eseguito fuori da AudioNoInterrupt()
}

FLASHMEM
void LOOP_select_menu_elements(void)
{
    // voices that can be displayed
    Menu_Loop[0] = true; // New
    Menu_Loop[1] = true; // Save
    Menu_Loop[2] = true; // Save as New
    Menu_Loop[3] = true; // Delete

    if (LOOP_id >= 0 && LOOP_events[0] == 0) // loop vuoto
    {
        Menu_Loop[0] = false; // New
        Menu_Loop[1] = false; // Save
        Menu_Loop[2] = false; // Save as New
    }

    if (LOOP_id >= 0 && LOOP_original) // loop su SD e inalterato
    {
        Menu_Loop[1] = false; // Save
        Menu_Loop[2] = false; // Save as New
    }

    if (LOOP_id == -1) // nuovo loop
    {
        Menu_Loop[0] = false; // New
        Menu_Loop[1] = false; // Save
        Menu_Loop[3] = false; // Delete

        if (LOOP_events[0] == 0)  // nuovo loop vuoto
            Menu_Loop[2] = false; // Save as New
    }

    Loop_menu_max = Menu_Loop[0] + Menu_Loop[1] + Menu_Loop[2] + Menu_Loop[3] - 1;
}

void LOOP_restart_clock(void)
{
    LOOP_clock = 0;
    Serial.println("restart LOOP_clock");
}

unsigned long LOOP_Clock(void)
{
    return LOOP_clock / LOOP_stretch;
}

unsigned long LOOP_zero_time(void)
{
    // Ultimo passaggio per lo 0 espresso in tempo virtuale LOOP_Clock
    //                               LOOP_zero_time()    LOOP_Clock()
    // 0-------------------------------------*---------------X----------------------------> LOOP_Clock()
    // 0------------------0------------------0---------------X-0-----------------0--------> LOOP_normalized_time()
    //
    return LOOP_Clock() - LOOP_normalized_time();
}

int LOOP_normalized_time(void)
{
    // Calcola tempo attuale normalizzato
    //                                                   LOOP_Clock()
    // 0-----------------------------------------------------X----------------------------> LOOP_Clock()
    // 0------------------0------------------0---------------X-0-----------------0--------> LOOP_normalized_time()
    //
    return LOOP_Clock() % LOOP_time;
}

// Converti un tempo normalizzato in tempo virtuale LOOP_Clock, a partire dal tempo virtuale attuale
unsigned long LOOP_Clock_time_from_virtual_time(int T_evento)
{
    if (T_evento < LOOP_normalized_time())
    {
        //                     T_evento                 LOOP_time
        //          0-------------*-------------------------|
        //
        //  LOOP_zero_time()      LOOP_normalized_time()
        //          |------------------------X--------------|-------------*-----------------------|--->
        //                                                             return
        return LOOP_zero_time() + LOOP_time + T_evento;
    }

    else
    {
        //                                               T_evento                    L_t
        //          0----------------------------------------*------------------------|
        //
        //  LOOP_zero_time()  LOOP_normalized_time()
        //          |------------------X---------------------*------------------------|----------------------------------------------------------|--->
        //                                                 return
        return LOOP_zero_time() + T_evento;
    }
}

// Define time ordering of events
void LOOP_set_time_order(int track)
{
    if (LOOP_events[track] == 1)
    {
        LOOP_time_order[track][0] = 0;
        return;
    }
    else if (LOOP_events[track] > 1)
    {
        // Trova l'indice associato al primo evento rispetto al tempo normalizzato
        int min_time_index = 0; // indice cercato
        int min_time = LOOP_element[track][0].time;

        for (auto i = 1; i < LOOP_events[track]; ++i)
        {
            if (LOOP_element[track][i].time < min_time)
            {
                min_time_index = i;
                min_time = LOOP_element[track][i].time;
            }
        }

        /*
        Gli eventi sono così ordinati:
        LOOP_time_order[track][0] = evento con time minimo
        LOOP_time_order[track][0] = evento successivo
        */

        for (auto i = 0; i < LOOP_events[track]; ++i)
        {
            LOOP_time_order[track][i] = (min_time_index + i) % LOOP_events[track];
        }
        return;
    }
    return;
}

void LOOP_restart_procedure(int track)
{
    bool found = false;

    // Procedura di ripartenza
    if (LOOP_events[track] == 1)
    {
        LOOP_play_event[track] = 0;
        LOOP_play_time[track] = LOOP_Clock_time_from_virtual_time(LOOP_element[track][0].time);
        LOOP_track_run[track] = true;
    }
    else
    {
        LOOP_clock_memo = LOOP_normalized_time();
        for (auto i = 0; i < LOOP_events[track]; ++i)
        {
            if (LOOP_element[track][LOOP_time_order[track][i]].time >= LOOP_clock_memo)
            {
                LOOP_play_event[track] = LOOP_time_order[track][i];
                LOOP_play_time[track] = LOOP_Clock_time_from_virtual_time(LOOP_element[track][LOOP_time_order[track][i]].time);

                // Ferma la ricerca
                found = true;
                break;
            }
        }
        if (!found)
        {
            LOOP_play_event[track] = 0;
            LOOP_play_time[track] = LOOP_Clock_time_from_virtual_time(LOOP_element[track][0].time);
        }
        LOOP_track_run[track] = true;
    }
}

bool Print_midi_loop_complete_data(int loop_id)
{
    if (false)
    {
        return false;
    }

    Serial.print("****  MIDI_LOOP complete data for loop: ");
    Serial.println(loop_id);
    Serial.println();

    // uint16_t LOOP_time;
    Serial.print("uint16_t LOOP_time: ");
    Serial.println(LOOP_time);

    // byte LOOP_events[TRACKS]
    Serial.println("byte LOOP_events[TRACKS]");
    for (auto i = 0; i < TRACKS; ++i)
    {
        Serial.println(LOOP_events[i]);
    }

    // int LOOP_slide[TRACKS]
    Serial.println("int LOOP_slide[6]");
    for (auto i = 0; i < TRACKS; ++i)
    {
        Serial.println(LOOP_slide[i]);
    }

    // int LOOP_pitch_int[TRACKS]
    Serial.println("int LOOP_pitch_int[6]");
    for (auto i = 0; i < TRACKS; ++i)
    {
        Serial.println(LOOP_pitch_int[i]);
    }

    // float LOOP_stretch
    Serial.print("int LOOP_stretch : ");
    Serial.println(LOOP_stretch_int);

    // LOOP_element[TRACKS][LOOP_EVENTS]
    for (byte track = 0; track < TRACKS; ++track)
    {
        for (auto event = 0; event < LOOP_events[track]; ++event)
        {
            Serial.print("*** LOOP_element[");
            Serial.print(track);
            Serial.print("][");
            Serial.print(event);
            Serial.println("]:");
            Serial.print("time: ");
            Serial.println(LOOP_element[track][event].time);
            Serial.print("midi_channel: ");
            Serial.println(LOOP_element[track][event].midi_channel);
            Serial.print("note_number: ");
            Serial.println(LOOP_element[track][event].note_number);
            Serial.print("velocity: ");
            Serial.println(LOOP_element[track][event].velocity);
            Serial.print("note_on: ");
            Serial.println(LOOP_element[track][event].note_on);
        }
    }

    return true;
}

String Filename_midi_loop(int loop_id)
{
    String filename = String(loop_id);
    return String(filename + ".loop");
}

bool Copy_midi_loop_from_RAM_to_SD(int loop_id)
{
    if (SD.begin(BUILTIN_SDCARD))
    {
        String filename = Filename_midi_loop(loop_id);
        String full_path = String("/LILLALOOP/" + filename);

        if (!SD.exists("/LILLALOOP"))
        {
            SD.mkdir("/LILLALOOP");
            Serial.println(F("Copy_midi_loop_from_SD_to_RAM(int loop_id) - /LILLALOOP directory created"));
        }

        const char *full_path_ = &full_path[0];
        if (SD.exists(full_path_))
        {
            SD.remove(full_path_);

            Serial.print(F("Copy_midi_loop_from_RAM_to_SD - existing "));
            Serial.print(full_path);
            Serial.println(" has been deleted.");
        }

        Serial.print(F("Copy_midi_loop_from_RAM_to_SD - this midi_loop will be saved as: "));
        Serial.println(full_path);

        File file = SD.open(full_path_, FILE_WRITE); // creazione del file vuoto
        Compile_midi_loop_file(loop_id, file);
        file.close();

        return true;
    }
    else
    {
        Serial.println(F("Copy_patch_Delay_data_from_SD_to_Eeprom - SD not present!"));
        return false;
    }
}

void Compile_midi_loop_file(int loop_id, File &file) // private
{
    const byte *data; // = (const byte *)(const void *)&Delay_data;

    // uint16_t LOOP_time
    data = (const byte *)(const void *)&LOOP_time;
    for (auto i = 0; i < 2; ++i)
    {
        file.println(*(data + i));
        Serial.println(*(data + i));
    }

    // byte LOOP_events[TRACKS]
    data = &LOOP_events[0];
    for (auto i = 0; i < TRACKS; ++i)
    {
        file.println(*(data + i));
    }

    // int LOOP_slide[6]
    data = (const byte *)(const void *)&LOOP_slide[0];
    for (auto i = 0; i < 24; ++i)
    {
        file.println(*(data + i));
    }

    // int LOOP_pitch_int[6]
    data = (const byte *)(const void *)&LOOP_pitch_int[0];
    for (auto i = 0; i < 24; ++i)
    {
        file.println(*(data + i));
    }

    // int LOOP_stretch
    data = (const byte *)(const void *)&LOOP_stretch_int;
    for (auto i = 0; i < 4; ++i)
    {
        file.println(*(data + i));
        Serial.println(*(data + i));
    }

    // LOOP_struct LOOP_element[TRACKS][LOOP_EVENTS]
    for (byte track = 0; track < TRACKS; ++track)
    {
        for (auto event = 0; event < LOOP_events[track]; ++event)
        {
            // LOOP_struct LOOP_element[track][LOOP_EVENTS] -> 8 byte
            data = (const byte *)(const void *)&LOOP_element[track][event];
            for (auto i = 0; i < 8; ++i)
            {
                file.println(*(data + i));
            }
        }
    }
}

bool Copy_midi_loop_from_SD_to_RAM(int loop_id) // public
{
    if (SD.begin(BUILTIN_SDCARD))
    {
        String filename = Filename_midi_loop(loop_id);
        String full_path = String("/LILLALOOP/" + filename);
        const char *full_path_ = &full_path[0];

        if (SD.exists(full_path_))
        {
            Serial.print(F("Copy_midi_loop_from_SD_to_RAM - midi loop file "));
            Serial.print(full_path);
            Serial.println(F(" found; now starts data import."));

            File file = SD.open(full_path_);

            Copy_midi_loop_from_SD_to_RAM_local(file);

            LOOP_original = true;
            return true;
        }
        else
        {
            Serial.println(F("Copy_midi_loop_from_SD_to_RAM - midi loop file not found on SD!"));
            return false;
        }
    }
    else
    {
        Serial.println(F("Copy_midi_loop_from_SD_to_RAM - ERROR - SD not present!"));
        return false;
    }
}

void Copy_midi_loop_from_SD_to_RAM_local(File &file)
{
    String string_byte;
    uint8_t value_b[4];

    // uint16_t LOOP_time
    for (auto b = 0; b < 2; ++b)
    {
        string_byte = file.readStringUntil('\n');
        value_b[b] = string_byte.toInt();
    }
    memcpy(&LOOP_time, value_b, 2);
    // LOOP_time = (value_b[1] << 8) | value_b[0];

    // byte LOOP_events[TRACKS]
    for (auto i = 0; i < TRACKS; ++i)
    {
        string_byte = file.readStringUntil('\n'); // restituisce String - es: x_txt = "230" ossia i char "2" "3" "0" "\n"
        LOOP_events[i] = string_byte.toInt();
    }

    // int LOOP_slide[TRACKS]
    for (auto i = 0; i < TRACKS; ++i)
    {
        for (auto b = 0; b < 4; ++b)
        {
            string_byte = file.readStringUntil('\n');
            value_b[b] = string_byte.toInt();
        }
        memcpy(&LOOP_slide[i], value_b, 4);
    }

    // int LOOP_pitch_int[TRACKS]
    for (auto i = 0; i < TRACKS; ++i)
    {
        for (auto b = 0; b < 4; ++b)
        {
            string_byte = file.readStringUntil('\n');
            value_b[b] = string_byte.toInt();
        }
        memcpy(&LOOP_pitch_int[i], value_b, 4);
    }

    // int LOOP_stretch_int
    for (auto b = 0; b < 4; ++b)
    {
        string_byte = file.readStringUntil('\n');
        value_b[b] = string_byte.toInt();
    }
    memcpy(&LOOP_stretch_int, value_b, 4);

    LOOP_stretch = static_cast<float>(LOOP_stretch_int) / 100.0f;

    // LOOP_struct LOOP_element[TRACKS][LOOP_EVENTS] -> 8 bytes
    for (auto track = 0; track < TRACKS; ++track)
    {
        for (auto event = 0; event < LOOP_events[track]; ++event)
        {
            // int time
            for (auto b = 0; b < 4; ++b)
            {
                string_byte = file.readStringUntil('\n');
                value_b[b] = string_byte.toInt();
            }
            memcpy(&LOOP_element[track][event].time, value_b, 4);

            // uint8_t midi_channel
            string_byte = file.readStringUntil('\n');
            LOOP_element[track][event].midi_channel = string_byte.toInt();

            // uint8_t note_number
            string_byte = file.readStringUntil('\n');
            LOOP_element[track][event].note_number = string_byte.toInt();

            // uint8_t velocity
            string_byte = file.readStringUntil('\n');
            LOOP_element[track][event].velocity = string_byte.toInt();

            // bool note_on velocity
            string_byte = file.readStringUntil('\n');
            LOOP_element[track][event].note_on = string_byte.toInt();
        }
    }
}

bool Delete_midi_loop_from_SD(int loop_id)
{
    if (SD.begin(BUILTIN_SDCARD))
    {
        if (!SD.exists("/LILLALOOP"))
        {
            Serial.println(F("Delete_midi_loop_from_SD(int loop_id) - file doesn't exist."));
            return true;
        }

        String filename = Filename_midi_loop(loop_id);
        String full_path = String("/LILLALOOP/" + filename);
        const char *full_path_ = &full_path[0];

        if (SD.exists(full_path_))
        {
            SD.remove(full_path_);
            Serial.print(F("Delete_midi_loop_from_SD(int loop_id) - file removed."));
            return true;
        }
        else
        {
            Serial.println(F("Delete_midi_loop_from_SD(int loop_id) - file doesn't exist."));
            return true;
        }
    }

    Serial.print(F("Delete_midi_loop_from_SD(int loop_id) - ERROR - SD not present!"));
    return false;
}

bool Look_for_midi_loop_in_SD(int loop_id)
{
    if (SD.begin(BUILTIN_SDCARD))
    {
        if (!SD.exists("/LILLALOOP"))
        {
            Serial.println(F("Look_for_midi_loop_in_SD(int loop_id) - file doesn't exist."));
            return false;
        }

        String filename = Filename_midi_loop(loop_id);
        String full_path = String("/LILLALOOP/" + filename);
        const char *full_path_ = &full_path[0];

        if (SD.exists(full_path_))
        {
            return true;
        }
        else
        {
            Serial.println(F("Look_for_midi_loop_in_SD(int loop_id) - file doesn't exist."));
            return false;
        }
    }
    Serial.println(F("Look_for_midi_loop_in_SD(int loop_id) - ERROR - SD not present!"));
    return false;
}

int Get_first_loop_id_free(void)
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println(F("Get_first_loop_id_free(void) - ERROR - SD not present!"));
        return -1;
    }
    else if (!SD.exists("/LILLALOOP"))
    {
        Serial.println(F("Get_first_loop_id_free(void) - no .loop file in SD."));
        return -2;
    }
    for (auto loop_id = 0; loop_id < MIDI_LOOP_FILES; ++loop_id)
    {
        String filename = Filename_midi_loop(loop_id);
        String full_path = String("/LILLALOOP/" + filename);
        const char *full_path_ = &full_path[0];
        if (!SD.exists(full_path_))
        {
            Serial.print(F("Get_first_loop_id_free(void) - loop_id: "));
            Serial.println(loop_id);
            return loop_id;
        }
    }
    Serial.println(F("Get_first_loop_id_free(void) - Please delete one .loop file in SD."));
    return -3;
}

int Get_next_loop_id_in_SD(int loop_id)
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println(F("Get_next_loop_id_in_SD(int loop_id) - ERROR - SD not present!"));
        return -1;
    }
    else if (!SD.exists("/LILLALOOP"))
    {
        Serial.println(F("Get_next_loop_id_in_SD(int loop_id) - no .loop file in SD."));
        return -2;
    }
    for (auto next_loop = loop_id + 1; next_loop < MIDI_LOOP_FILES; ++next_loop)
    {
        String filename = Filename_midi_loop(next_loop);
        String full_path = String("/LILLALOOP/" + filename);
        const char *full_path_ = &full_path[0];
        if (SD.exists(full_path_))
        {
            return next_loop;
        }
    }
    Serial.println(F("Get_next_loop_id_in_SD(int loop_id) - loop_id is the last .loop file in SD."));
    return loop_id;
}

int Get_previous_loop_id_in_SD(int loop_id)
{
    if (loop_id <= 0)
    {
        Serial.println(F("Get_previous_loop_id_in_SD(int loop_id) - no .loop file before this!"));
        return loop_id;
    }
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println(F("Get_previous_loop_id_in_SD(int loop_id) - ERROR - SD not present!"));
        return -1;
    }
    else if (!SD.exists("/LILLALOOP"))
    {
        Serial.println(F("Get_previous_loop_id_in_SD(int loop_id) - no .loop file in SD."));
        return -2;
    }
    for (auto previous_loop = loop_id - 1; previous_loop >= 0; --previous_loop)
    {
        String filename = Filename_midi_loop(previous_loop);
        String full_path = String("/LILLALOOP/" + filename);
        const char *full_path_ = &full_path[0];
        if (SD.exists(full_path_))
        {
            return previous_loop;
        }
    }
    Serial.println(F("Get_previous_loop_id_in_SD(int loop_id) - loop_id is the first .loop file in SD."));
    return loop_id;
}

// ***************************************************************************************************************
// **********************************            VIRTUAL FILE SYSTEM            **********************************
// ***************************************************************************************************************

void Make_VFS(void)
{
    Display.Make_VFS_presentazione();
    // calcola DS_packets Free space, in PACKET_DIM
    VFS_packets_max = (Get_flash_size() - Get_flash_occupation() - FLASH_FREE_SPACE) / PACKET_DIM;
    VFS_packets_max = constrain(VFS_packets_max, 0, VFS_PACKETS_MAX);
    if (VFS_packets_max > 40)
    {
        Display.Make_VFS_assegnazioni();
        VFS_packets = VFS_packets_max / 3.0f; // questa proporzione puo' essere modifitata a piacere
        Display.Show_VFS_packets();
        bool confirmation = false;
        while (!confirmation)
        {
            if (ENC_timer > ENC_STOP)
            {
                result = Read_encoder_simple(24);
                if (result != 0)
                {
                    if (result == +1)
                    {
                        if (VFS_packets <= (VFS_packets_max - 2))
                        {
                            VFS_packets += 2;
                            Display.Show_VFS_packets();
                        }
                    }
                    else
                    {
                        if (VFS_packets >= 2)
                        {
                            VFS_packets -= 2;
                            Display.Show_VFS_packets();
                        }
                    }
                }
            }

            if (Read_pushbutton(25))
            {
                confirmation = true;
            }
        }
        // create VFS
        for (auto i = 0; i < VFS_packets; ++i)
        {
            SerialFlash.createErasable(name_packet[i], PACKET_DIM);
        }

        Serial.print(F("Created Virtual File System  - VFS_packets are "));
        Serial.println(VFS_packets);
        Display.Make_VFS_restart();
        delay(10000);
    }

    else
    {
        Display.Make_VFS_no_spazio_per_sampler();
        delay(10000);
    }
}

int Get_VFS_packets(void)
{
    int value = 0;
    for (auto i = 0; i < VFS_PACKETS_MAX; ++i)
    {
        if (SerialFlash.exists(name_packet[i]))
        {
            ++value;
        }
    }
    return value;
}

FLASHMEM
void Print_VFS_allocation(void)
{
    Serial.println();

    Serial.println(F("*** Print_VFS_allocation() *** "));
    Serial.print(F("VFS_packets: "));
    Serial.println(VFS_packets);

    Serial.print(F("VFS_packets_DS: "));
    Serial.println(VFS_packets_DS);

    Serial.print(F("First_DS_packet: "));
    Serial.print(First_DS_packet);
    Serial.print(F(" Last_DS_packet: "));
    Serial.println(Last_DS_packet);

    Serial.println(F("*** Finished *** "));
    Serial.println();
}

void Compile_VFS_FAT_table(void)
{
    // reset array
    Reset_VFS_FAT_table();

    for (auto i = 0; i < RECORDINGS; ++i)
    {
        int last_packet = Recording[i].first_packet + Recording[i].packets * (Recording[i].stereo ? 2 : 1) - 1;
        if (Recording[i].consistent && Recording[i].packets > 0)
        {
            for (auto j = Recording[i].first_packet; j <= last_packet; ++j)
            {
                VFS_FAT_table[j] = i;
                Serial.print(F("Compile_VFS_FAT_table() --> Packet: "));
                Serial.print(j);
                Serial.print(F(" Recording: "));
                Serial.print(i);
            }
        }
    }
}

void Reset_VFS_FAT_table(void)
{
    Serial.println(F("*** Reset_VFS_FAT_table() *** "));
    for (auto i = First_DS_packet; i < VFS_packets_DS; ++i)
    {
        VFS_FAT_table[i] = -1;
    }
}

int Get_first_packet_free(void)
{
    for (auto i = First_DS_packet; i < VFS_packets_DS; ++i)
    {
        if (VFS_FAT_table[i] == -1)
        {
            Serial.print(F("First packet free is: "));
            Serial.println(i);
            return i;
        }
    }
    return -1;
}

int Get_packets_free(void)
{
    int value = 0;
    for (auto i = First_DS_packet; i < VFS_packets_DS; ++i)
    {
        if (VFS_FAT_table[i] == -1)
        {
            ++value;
        }
    }
    return value;
}

void Erase_all_Packet(void)
{
    Serial.println("*** Erase ALL Packets and VFS_FAT ***");
    for (auto i = 0; i < VFS_PACKETS_MAX; ++i) // for(auto i = 0; i < VFS_packets; ++i)
    {
        if (SerialFlash.exists(name_packet[i]))
        {
            Erase_Packet(i);
        }
    }
    Serial.println(F("*** Finished *** "));
    Serial.println();
}

void Erase_all_Packet_for_DS(void)
{
    Serial.println("*** Erase ALL Packets for Direct Sampling and VFS_FAT ***");
    for (auto i = First_DS_packet; i <= Last_DS_packet; ++i)
    {
        Erase_Packet(i);
    }
    Serial.println(F("*** Finished *** "));
    Serial.println();
}

void Erase_Packet(int value)
{
    Serial.print("Erase Packet: ");
    Serial.println(value);
    SerialFlashFile Packet;
    Packet = SerialFlash.open(name_packet[value]);
    Packet.erase();
    Packet.close();

    if (value <= Last_DS_packet)
    {
        VFS_FAT_table[value] = -1;
    }
}

void Clean_up_VFS(void) // Deletes packets occupied by not-consistent recording, delete recording, save recording
{
    Serial.println("*** Clean_up_VFS ***");

    // erase Packets occupied by inconsistent Recordings
    for (auto i = 0; i < RECORDINGS; ++i)
    {
        if (!Recording[i].consistent)
        {
            Serial.print(F("Found NON consistent Recording: "));
            Serial.print(i);
            Serial.println(F(". Now associated Packets will be erased:"));
            // find all Packets registered "i" and erase
            int last_packet = Recording[i].first_packet + Recording[i].packets * (Recording[i].stereo ? 2 : 1);
            for (auto j = Recording[i].first_packet; j < last_packet; ++j)
            {
                Erase_Packet(j);
            }

            // ricompila il recording come consistent
            Serial.println(F("Now delete Recording... "));
            Recording[i].first_packet = 0;
            Recording[i].packets = 0;
            Recording[i].bytes = 0;
            Recording[i].seconds = 0.0f;
            Recording[i].stereo = 0;
            Recording[i].consistent = true;

            // salva su EEPROM
            Serial.println(F("Now save Recording... "));
            Archive.Save_DS_Recording(i);
        }
    }
    Serial.println(F("*** Finished *** "));
    Serial.println();
}

FLASHMEM
void Defragment_VFS(void) // updates VFS_FAT_table, moves packets, updates recording, save recording, again updates VFS_FAT_table
{
    Serial.println("*** Defragment_VFS  ***");
    Compile_VFS_FAT_table();

    // Packet     1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
    // Recording  a a a a _ _ _ _ _ _  cL cR cL cR cL cR cL cR _  _  _  d  d  d  d  d  _  _

    for (auto i = First_DS_packet; i < VFS_packets_DS; ++i)
    {
        if (VFS_FAT_table[i] == -1) // i == 5
        {
            for (auto j = i; j < VFS_packets_DS; ++j) // j = 5 -->
            {
                if (VFS_FAT_table[j] >= 0) // j == 11
                {
                    Serial.print("Space free found from Packet: ");
                    Serial.print(i);
                    Serial.print(" to Packet: ");
                    Serial.println(j - 1);

                    int to_packet = i;                                                                        // 5
                    int from_packet = j;                                                                      // 11
                    int recording_id = VFS_FAT_table[j];                                                      // c
                    int packets = Recording[recording_id].packets * (Recording[recording_id].stereo ? 2 : 1); // 8

                    Serial.print("Recording: ");
                    P_Recording(recording_id);
                    Serial.println("will be moved down.");

                    Recording[recording_id].consistent = false;
                    Shift_file_VFS(to_packet, from_packet, packets); // MOVE_file_VFS(int to_packet, int from_packet, int packets)

                    // Packet     1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
                    // Recording  a a a a cLcRcLcRcLcR cL cR _  _  _  _  _  _  _  _  _  d  d  d  d  d  _  _

                    // update runtime info and Save
                    Recording[recording_id].first_packet = to_packet;
                    Recording[recording_id].consistent = true;
                    Archive.Save_DS_Recording(recording_id);

                    Serial.print("Now this is Recording: ");
                    P_Recording(recording_id);

                    // update VFS_FAT ad run again
                    Compile_VFS_FAT_table();

                    i = i + packets - 1; // = 5 + 8 - 1  = 12
                    break;
                }
            }
        }
    }
    Serial.println(F("*** Finished *** "));
    Serial.println();
    return;
}

void Shift_file_VFS(int to_packet, int from_packet, int packets)
{
    int16_t basket[128];
    SerialFlashFile Packet_from;
    SerialFlashFile Packet_to;

    for (auto i = 0; i < packets; ++i)
    {
        Serial.print("moving packet: ");
        Serial.println(from_packet + i);
        Packet_from = SerialFlash.open(name_packet[from_packet + i]);
        Packet_to = SerialFlash.open(name_packet[to_packet + i]);
        Packet_to.erase();
        // Packet_from.seek(0); // unnecessary
        // Packet_to.seek(0);  // unnecessary
        for (auto block = 0; block < 256; ++block)
        {
            // Packet_from.seek(block * 256); // included in .read function
            Packet_from.read(basket, 256);
            // Packet_to.seek(block * 256); // included in .write function
            Packet_to.write(basket, 256);
        }
        Packet_from.erase();
        Packet_from.close();
        Packet_to.close();
    }
}

void Print_VFS_FAT_table(void)
{
    Serial.println();
    for (auto i = First_DS_packet; i < VFS_packets_DS; ++i)
    {
        Serial.print("VFS_FAT_table[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(VFS_FAT_table[i]);
    }
    Serial.println();
}

// ***************************************************************************************************************
// **********************************            STANDARD RAW FILES             **********************************
// ***************************************************************************************************************

int Get_next_raw_file_in_flash(int file)
{
    int value = file;
    do
    {
        ++value;
        if (value == FIRST_LIVE_SAMPLING_FILE)
        {
            Serial.println(F("Get_next_raw_file_in_flash() --> reached last file!"));
            return file;
        }
        if (Get_samples_in_raw_file(value) > 0)
        {
            Serial.println(F("Get_next_raw_file_in_flash() --> next file found : "));
            Serial.println(value);
            return value;
        }
    } while (1);
}

int Get_previous_raw_file_in_flash(int file)
{
    int value = file;
    do
    {
        --value;
        if (value == -1)
        {
            Serial.println(F("Get_previous_raw_file_in_flash() --> reached first file!"));
            return file;
        }
        if (Get_samples_in_raw_file(value) > 0)
        {
            Serial.println(F("Get_previous_raw_file_in_flash() --> previous file found: "));
            Serial.println(value);
            return value;
        }
    } while (1);
}

int Get_samples_in_raw_file(int file_id) // value is a file_id
{
    if (file_id < FIRST_RECORDING_FILE)
        return Info.Raw_file_samples(file_id);

    else if (file_id < FIRST_LIVE_SAMPLING_FILE)
    {
        int recording = (file_id - FIRST_RECORDING_FILE) / 2;
        bool file_L_flag = ((file_id - FIRST_RECORDING_FILE) % 2 == 0);
        if (!Recording[recording].stereo && !file_L_flag)
            return 0;
        else
            return Info.DS_recording_samples(recording); // DS_recording_samples(int first_packet, int packets)
    }

    else
        return 0;
}

FLASHMEM
bool Verify_space_on_flash(int value)
{
    return ((Get_flashchip_size() - Get_flash_occupation()) >= value ? true : false);
}

FLASHMEM
int Get_first_raw_file_available(int start_value)
{
    if (start_value >= 0)
    {
        for (auto i = start_value; i < FIRST_RECORDING_FILE; ++i)
        {
            if (!SerialFlash.exists(name_file[i]))
            {
                return i;
            }
        }
    }
    return -1; // nessun file .RAW
}

FLASHMEM
void Print_flash_file_list(void)
{
    uint32_t filesize = 0;
    int occupation = 0;
    Serial.println(F("All Files on SPI Flash chip:")); // Questo puo' esser fatto solo per i "Serial.print" che contengono stringhe di testo COSTANTI e non per i "Serial.print" che contengono variabili.
    SerialFlash.opendir();
    while (1)
    {
        char filename[64];
        // SerialFlash.readdir compila
        // - filename
        // - sizeof(filename)
        // - filesize
        // e restituisce "true" se il file esiste
        if (SerialFlash.readdir(filename, sizeof(filename), filesize))
        {
            occupation += filesize;
            Serial.print(F("filename:"));
            Serial.print(filename);
            Serial.print(F("  filesize:"));
            Serial.print(filesize);
            Serial.print(F(" bytes"));
            Serial.println();
        }
        else
            break; // no more files
    }
    Serial.println(F("---------------"));
    Serial.print(F("Total space occupied (kB): "));
    Serial.println(occupation >> 10);
    Serial.print(F("Total space free (kB): "));
    Serial.println((Get_flash_size() - occupation) >> 10);
    Serial.println(F("---------------"));
}

FLASHMEM
int Get_flash_occupation(void)
{
    uint32_t filesize;
    int occupation = 0;
    SerialFlash.opendir();
    while (1)
    {
        char filename[64];
        if (SerialFlash.readdir(filename, sizeof(filename), filesize))
        {
            occupation += filesize; // ceil((float)filesize/PACKET_DIM) * PACKET_DIM;
        }
        else // no more files
        {
            break;
        }
    }
    return occupation;
}

FLASHMEM
int Get_flashchip_size(void)
{
    unsigned char buf[256];
    unsigned long chipsize, blocksize;
    SerialFlash.readID(buf);
    chipsize = SerialFlash.capacity(buf);

    Serial.println();
    Serial.println(F("Read Chip Identification:"));
    Serial.print(F("  JEDEC ID:     "));
    Serial.print(buf[0], HEX);
    Serial.print(' ');
    Serial.print(buf[1], HEX);
    Serial.print(' ');
    Serial.println(buf[2], HEX);
    Serial.print(F("  Part Number: "));
    Serial.println(id2chip(buf));
    Serial.print(F("  Memory Size:  "));
    Serial.print(chipsize);
    Serial.println(F(" bytes"));

    if (chipsize == 0)
    {
        return false;
    }
    blocksize = SerialFlash.blockSize();

    Serial.print(F("  Block Size:   "));
    Serial.print(blocksize);
    Serial.println(F(" bytes"));
    return chipsize;
}

FLASHMEM
int Get_flash_size(void)
{
    unsigned char buf[256];
    unsigned long chipsize;
    SerialFlash.readID(buf);
    chipsize = SerialFlash.capacity(buf); // bytes
    return chipsize;
}

FLASHMEM
int Get_raw_files(void)
{
    int value = 0;
    for (auto i = 0; i < FIRST_RECORDING_FILE; ++i)
    {
        if (SerialFlash.exists(name_file[i]))
        {
            ++value;
        }
    }
    return value; // nessun file .RAW
}

FLASHMEM
int Get_raw_files_volume(void)
{
    unsigned long value = 0;
    for (auto i = 0; i < FIRST_RECORDING_FILE; ++i)
    {
        if (SerialFlash.exists(name_file[i]))
        {
            SerialFlashFile raw_file = SerialFlash.open(name_file[i]);
            value += raw_file.size();
            raw_file.close();
        }
    }
    return value;
}

FLASHMEM
const char *id2chip(const unsigned char *id)
{
    if (id[0] == 0xEF)
    {
        // Winbond
        if (id[1] == 0x40)
        {
            if (id[2] == 0x14)
            {
                return "W25Q80BV";
            }
            if (id[2] == 0x15)
            {
                return "W25Q16DV";
            }
            if (id[2] == 0x17)
            {
                return "W25Q64FV";
            }
            if (id[2] == 0x18)
            {
                return "W25Q128FV";
            }
            if (id[2] == 0x19)
            {
                return "W25Q256FV";
            }
            if (id[2] == 0x20) // aggiunto io
            {
                return "W25Q512FV";
            }
        }
    }

    if (id[0] == 0x01)
    {
        // Spansion
        if (id[1] == 0x02)
        {
            if (id[2] == 0x16)
            {
                return "S25FL064A";
            }
            if (id[2] == 0x19)
            {
                return "S25FL256S";
            }
            if (id[2] == 0x20)
            {
                return "S25FL512S";
            }
        }
        if (id[1] == 0x20)
        {
            if (id[2] == 0x18)
            {
                return "S25FL127S";
            }
        }
    }

    if (id[0] == 0xC2)
    {
        // Macronix
        if (id[1] == 0x20)
        {
            if (id[2] == 0x18)
            {
                return "MX25L12805D";
            }
        }
    }

    if (id[0] == 0x20)
    {
        // Micron
        if (id[1] == 0xBA)
        {
            if (id[2] == 0x20)
            {
                return "N25Q512A";
            }
            if (id[2] == 0x21)
            {
                return "N25Q00AA";
            }
        }
        if (id[1] == 0xBB)
        {
            if (id[2] == 0x22)
            {
                return "MT25QL02GC";
            }
        }
    }

    if (id[0] == 0xBF)
    {
        // SST
        if (id[1] == 0x25)
        {
            if (id[2] == 0x02)
            {
                return "SST25WF010";
            }
            if (id[2] == 0x03)
            {
                return "SST25WF020";
            }
            if (id[2] == 0x04)
            {
                return "SST25WF040";
            }
            if (id[2] == 0x41)
            {
                return "SST25VF016B";
            }
            if (id[2] == 0x4A)
            {
                return "SST25VF032";
            }
        }
        if (id[1] == 0x25)
        {
            if (id[2] == 0x01)
            {
                return "SST26VF016";
            }
            if (id[2] == 0x02)
            {
                return "SST26VF032";
            }
            if (id[2] == 0x43)
            {
                return "SST26VF064";
            }
        }
    }

    if (id[0] == 0x1F)
    {
        // Adesto
        if (id[1] == 0x89)
        {
            if (id[2] == 0x01)
            {
                return "AT25SF128A";
            }
        }
    }

    return "(unknown chip)";
}

// ***************************************************************************************************************
// **********************************                   NOCLICK                 **********************************
// ***************************************************************************************************************

FLASHMEM
uint16_t Calc_Noclick_max(bool use_Wavetable)
{
    if (use_Wavetable)
    {
        return 42;
    }
    else
    {
        return NOCLICK_DIM;
    }
}

void Get_all_Noclick_pointer(void) // initialization of *Noclick_pointer[] array
{
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        Noclick_pointer[instrument_local] = Noclick[instrument_local].get_pointer();
    }
}

void Fill_all_Noclick(void)
{
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        if (Patch[Patch_id].Instrument[instrument_local].used) // Fill_Noclick(instrument)
        {
            Noclick[instrument_local].Make(Preset[instrument_local].file, Preset[instrument_local].A, Preset[instrument_local].B, Preset[instrument_local].Noclick);
        }
    }
}

void Fill_Noclick(uint8_t instrument)
{
    Noclick[instrument].Make(Preset[instrument].file, Preset[instrument].A, Preset[instrument].B, Preset[instrument].Noclick);
}

// ***************************************************************************************************************
// **********************************                WAVETABLE                  **********************************
// ***************************************************************************************************************
void Get_all_Wavetable_pointer(void) // initialization of *Wavetable_pointer[] array
{
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        Wavetable_pointer[instrument_local] = Wavetable[instrument_local].get_pointer();
    }
}

void Fill_all_Wavetable(void) // DELETE
{
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        if (Patch[Patch_id].Instrument[instrument_local].used)
        {
            Fill_Wavetable(instrument_local);
        }
    }
}

void Fill_Wavetable(uint8_t instrument) // fill Wavetable with a sound
{
    if (Preset[instrument].use_Wavetable)
    {
        Wavetable[instrument].Make(Preset[instrument].file, Preset[instrument].mode, Preset[instrument].A, Preset[instrument].B, Preset[instrument].Noclick, Noclick_pointer[instrument]);
    }
}

// ***************************************************************************************************************
// **********************************               FACTORY SETUP               **********************************
// ***************************************************************************************************************
FLASHMEM
void Factory_setup_Eeprom(void)
{
    // la funzione cancella

    // cancella l'intero contenuto della EEPROM (EEPROM emulation memory all'interno della Flash 8M del T4.1)
    Archive.Reset_EEPROM();

    // cancella gli array descrittivi di Patch e Sound
    Delete_all_Patches_and_Sounds();

    // definisci una Patch[0] al solo scopo di salvarla su EEPROM
    Patch[0].used = true;
    Patch[0].instruments = 1; // number of instruments in the patch_id
    Patch[0].Instrument[0].used = 1;
    Patch[0].Instrument[0].sound_id = 0;
    Patch[0].Instrument[0].root_key = 60;
    Patch[0].Instrument[0].from_note = 0;
    Patch[0].Instrument[0].to_note = 127;
    Patch[0].Instrument[0].precedence = 0;
    Patch[0].Instrument[0].lock = 0;

    Patch[0].Instrument[0].Filter.use = 0;            // yes/no
    Patch[0].Instrument[0].Filter.type = 1;           // lowpass, highpass, bandpass, notch
    Patch[0].Instrument[0].Filter.pivot = 20;         // 0 --> 100 filter frequency/note frequency
    Patch[0].Instrument[0].Filter.resonance = 7;      // 0 --> 40
    Patch[0].Instrument[0].Filter.modulation = 3;     // 0 --> 4 bit 1,2,3:modulation
    Patch[0].Instrument[0].Filter.index = 20;         // 1 --> 20 modulation_index
    Patch[0].Instrument[0].Filter.frequency_time = 5; // 0 --> 20

    Patch_id = 0;
    Archive.Save_Patch(Patch_id, Patch[Patch_id]);
    Serial.println(F("Patch[0] Saved"));

    // inizializza l'array descrittivo delle registrazioni (Direct Sampler) e salva su EEPROM
    DS_seed_all_Recordings();

    // definisci un Sound[0] al solo scopo di salvarlo su EEPROM
    Sound[0].used = true;
    Sound[0].file = 0;
    Sound[0].mode = 0;
    Sound[0].pitch = 0;
    Sound[0].A = 0;
    Sound[0].B = 40000;
    Sound[0].Noclick = 0;
    Sound[0].pan = 0;
    Sound[0].data = 0; // bit4-3-2-1: midi_channel bit0: Attack ramp ("0" Slow, "1" Fast)
    Sound[0].attack = 0;
    Sound[0].decay = 50;
    Sound[0].sustain = 50;
    Sound[0].release = 10;
    Sound[0].gain = 12; // 20 means gain = 1.0
    Serial.println(F("Saving Sound[0]"));
    Sound_id = 0;
    Archive.Save_Sound(Sound_id);

    // salva su EEPROM l'ottava del NoteNumber 0 (prima ottava)
    Archive.Save_first_octave(-2);

    // salva su EEPROM l'opzione 1 per optimization
    Archive.Save_optimization(1); // 0: extension  1: polyphony

    // assegna i parametri per il Delay al solo scopo di salvarli su EEPROM
    Delay_data.samples = 20;                  // value ; 0 --> 99
    Delay_data.samples_LR = 0;                // value L/R ; -10 --> 10
    Delay_data.instrument_route = 0b00000000; // all Instruments are NOT routed to Delay
    Delay_data.modulation_source = 0;         // 0: none 1:LFO(sinus) 2:input_1
    Delay_data.modulation_depth = 30;         // 0 --> 40 modulation depth
    Delay_data.modulation_frequency = 12;     // 0 --> 40 only for waveform
    Delay_data.modulation_phase_LR = 0;       // 0 --> 359 only for waveform
    Delay_data.loop_gain = 5;
    Archive.Save_Delay_to_Eeprom(Delay_data);

    // cancella il contenute dei packet sulla Flash aggiuntiva
    Erase_all_Packet();
}

FLASHMEM
void P_Patch(uint8_t patch_id)
{
    Serial.print("Patch:");
    Serial.print(patch_id);
    if (false)
    {
        Serial.print(" used:");
        Serial.print(Patch[patch_id].used);
        Serial.print(" instruments:");
        Serial.println(Patch[patch_id].instruments);
        for (auto instrument_id_local = 0; instrument_id_local < INSTRUMENTS_MAX; ++instrument_id_local)
        {
            if (!Patch[patch_id].Instrument[instrument_id_local].used)
            {
                Serial.print("* ");
                Serial.println(instrument_id_local);
            }
            else
            {
                Serial.print("* ");
                Serial.print(instrument_id_local);
                P_Instrument(patch_id, instrument_id_local);
                Serial.println();
            }
        }
    }
    Serial.println();
}

FLASHMEM
void P_Instrument(uint8_t patch_id, uint8_t instrument_id)
{
    Serial.print(" sound_id:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].sound_id);
    Serial.print(" file:");
    Serial.print(Sound[Patch[patch_id].Instrument[instrument_id].sound_id].file);
    Serial.print(".raw");
    Serial.print(" root_key:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].root_key);
    Serial.print(" from_note:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].from_note);
    Serial.print(" to_note:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].to_note);
    Serial.print(" precedence:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].precedence);
    Serial.print(" lock:");
    Serial.println(Patch[patch_id].Instrument[instrument_id].lock);

    Serial.print("Filter  use:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].Filter.use);
    Serial.print(" type:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].Filter.type);
    Serial.print(" pivot:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].Filter.pivot);
    Serial.print(" resonance:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].Filter.resonance);
    Serial.print(" modulation:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].Filter.modulation);
    Serial.print(" index:");
    Serial.print(Patch[patch_id].Instrument[instrument_id].Filter.index);
    Serial.print(" frequency_time:");
    Serial.println(Patch[patch_id].Instrument[instrument_id].Filter.frequency_time);
}

FLASHMEM
void P_Patch_cache_P(void)
{
    Serial.print("Patch: Patch_cache_P");
    Serial.print(" used:");
    Serial.print(Patch_cache_P.used);
    Serial.print(" instruments:");
    Serial.println(Patch_cache_P.instruments);
    for (uint8_t I = 0; I < INSTRUMENTS_MAX; ++I)
    {
        if (!Patch_cache_P.Instrument[I].used)
        {
            Serial.print("* ");
            Serial.println(I);
        }
        else
        {
            Serial.print("* ");
            Serial.print(I);
            Serial.print(" sound_id:");
            Serial.print(Patch_cache_P.Instrument[I].sound_id);
            Serial.print(" root_key:");
            Serial.print(Patch_cache_P.Instrument[I].root_key);
            Serial.print(" file:");
            Serial.print(Sound[Patch_cache_P.Instrument[I].sound_id].file);
            Serial.print(".raw");
            Serial.print(" lock:");
            Serial.print((Patch_cache_P.Instrument[I].lock ? "yes" : "no"));
            Serial.print(" precedence:");
            Serial.println((Patch_cache_P.Instrument[I].precedence ? "yes" : "no"));
        }
    }
    Serial.println();
}

FLASHMEM
void P_Sound(uint8_t sound_id)
{
    Serial.println();
    Serial.print("sound_id:");
    Serial.print(sound_id);
    Serial.print(" used:");
    Serial.println(Sound[sound_id].used);
    Serial.println((Sound[sound_id].used ? "yes" : "no"));
    Serial.print(" file_id:");
    Serial.print(Sound[sound_id].file);
    Serial.print(" file name: ");
    Serial.print(name_file[Sound[sound_id].file]);

    Serial.print(" samples:");

    // xxx.raw file (standard files coming from micro SD)
    if (sound_id < SOUNDS_MAX)
    {
        Serial.print(Get_samples_in_raw_file(Sound[sound_id].file));
    }

    // Rxxx.raw file (Recording)
    else if (sound_id >= SOUNDS_MAX)
    {
        int recording = ((Sound[sound_id].file - 260) / 2);
        Serial.print(DS_get_samples_in_Recording(recording));
    }

    Serial.print(" mode:");
    Serial.print(Sound[sound_id].mode);
    Serial.print(" pitch:");
    Serial.print(Sound[sound_id].pitch);
    Serial.print(" A:");
    Serial.print(Sound[sound_id].A);
    Serial.print(" B:");
    Serial.print(Sound[sound_id].B);
    Serial.print(" Noclick:");
    Serial.print(Sound[sound_id].Noclick);
    Serial.print(" Attack:");
    Serial.print(Sound[sound_id].attack);
    Serial.print(" attack_ramp:");
    Serial.print((Sound[sound_id].data & 1) ? "fast" : "slow"); // bit4-3-2-1: midi_channel bit0: Attack ramp ("0" Slow, "1" Fast)
    Serial.print(" Decay:");
    Serial.print(Sound[sound_id].decay);
    Serial.print(" Sustain:");
    Serial.print(Sound[sound_id].sustain);
    Serial.print(" Release:");
    Serial.print(Sound[sound_id].release);
    Serial.print(" volume:");
    Serial.print(Sound[sound_id].gain);
    Serial.print(" MIDI channel:");
    Serial.println(Get_midi_channel_from_Sound(sound_id) + 1);
    Serial.println();
}

FLASHMEM
void P_Lilla_state(void)
{
    Serial.print("Displayed page is: ");
    switch (Lilla_state)
    {
    case 0:
        Serial.println("Performance");
        break;
    case 1:
        Serial.println("Sound edit");
        break;
    case 2:
        Serial.println("Setup");
        break;
    case 3:
        Serial.println("Midi monitor");
        break;
    case 4:
        Serial.println("Control Change Settings");
        break;
    case 5:
        Serial.println("Delay");
        break;
    case 6:
        Serial.println("Instrument Filter");
        break;
    case 7:
        Serial.println("Direct Sampling");
        break;
    case 8:
        Serial.println("Live Sampling");
        break;
    case 9:
        Serial.println("MIDI Loop");
        break;
    default:
        Serial.println("Switch MISSING! 9045");
        break;
    }
}

FLASHMEM
void P_keyboard_state(uint8_t mc, int8_t a, int8_t b)
{
    Serial.println();
    Serial.print(F("********  P_keyboard_state - MIDI CH.: "));
    Serial.print(mc + 1);
    Serial.println("   *********");
    for (uint8_t key = a; key <= b; ++key)
    {
        Serial.print(key_state[mc][key]);
        Serial.print("  ");
    }
    Serial.println();
    for (uint8_t key = a; key <= b; ++key)
    {
        Serial.print(key);
        Serial.print("  ");
    }
    Serial.println();
}

FLASHMEM
void P_map_instrument_for_note(uint8_t midi_channel)
{
    Serial.println();
    Serial.print(F("Map notes/Sound for MIDI channel:"));
    Serial.println(midi_channel + 1);
    for (int8_t i = 7; i >= 0; --i)
    {
        for (uint8_t n = 0; n < 128; ++n)
        {
            Serial.print(bitRead(map_instrument_for_note[midi_channel][n], i));
            Serial.print("  ");
        }
        Serial.println();
    }
    Serial.println();

    for (uint8_t n = 0; n < 128; ++n)
    {
        Serial.print(n);
        Serial.print("  ");
    }
    Serial.println();
    Serial.println();
}



// ***************************************************************************************************************
// **********************************                LIVE_SAMPLING              **********************************
// ***************************************************************************************************************

FLASHMEM
void LS_refresh_LS_page(void)
{
    Lilla_state = LIVE_SAMPLING;
    Display.LS_page();

    // restore LEDs
    Performance_led_set.Restore_all_LED();

    LS_define_model();
    Display.Show_LS_menu();
    Display.Frame_LS_menu(LS_menu);
    choice_LS_menu = element_Menu_LS[LS_menu];
    if (!LS_XY_lock)
    {
        LS_update_both_X_Y_samples();
    }
    else // altrimenti e' gia' stato calcolato
    {
        LS_update_Q_sample(); // Usato da LS_wave_color
    }
    Display.Show_LS_ring_tape_wave(LS_id_sound);
}

FLASHMEM
bool LS_ask_if_exit_from_LS(void)
{
    confirmation = false;
    int action = 0;
    Display.Confirm_EXIT_from_LS();
    Display.Confirm_patch_delete_popup_frame(0);
    delay(200);
    while (!confirmation)
    {
#ifdef PCB_2022
        if (Read_encoder_inverse(25, action, 1, 0, 1))
#else
        if (Read_encoder(25, action, 1, 0, 1))
#endif
        {
            Display.Confirm_patch_delete_popup_frame(action);
        }

        if (Read_pushbutton(25))
        {
            confirmation = true;
        }
    }
    return (action == 0 ? false : true);
}

FLASHMEM
void LS_define_model(void)
{
    // voices that can be displayed
    Menu_LS[0] = true; // Open
    Menu_LS[1] = true; // Close
    Menu_LS[2] = true; // Mono/Stereo
    Menu_LS[3] = true; // Erase FIFO

    if (LS_state == 0) // nessuna registrazione
    {
        Menu_LS[1] = false; // Close
        Menu_LS[3] = false; // Erase FIFO
    }

    if (LS_state == 1) // fase di registrazione
    {
        Menu_LS[0] = false; // Open
        Menu_LS[2] = false; // Mono/Stereo
        Menu_LS[3] = false; // Erase FIFO
    }

    if (LS_state == 2) // registrazione disponibile
    {
        Menu_LS[1] = false; // Close
        Menu_LS[2] = false; // Mono/Stereo
    }

    LS_menu_max = Menu_LS[0] + Menu_LS[1] + Menu_LS[2] + Menu_LS[3] - 1;
}

void LS_lock_X_sample(void)
{
    AudioNoInterrupts();
    LS_Q_sample = LiveSampler.Q_sample;
    LS_X_sample = LS_constrain_position(LS_Q_sample + LS_X_delta);
    LS_Y_sample = LS_X_sample + LS_XY_delta;
    LS_XY_lock = true;
    AudioInterrupts();

    // Serial.print(F("LS_Q_sample: "));
    // Serial.println(LS_Q_sample);
    // Serial.print(F("LS_X_sample: "));
    // Serial.println(LS_X_sample);
}

void LS_update_both_X_Y_samples(void)
{
    AudioNoInterrupts();
    LS_Q_sample = LiveSampler.Q_sample;
    LS_X_sample = LS_constrain_position(LS_Q_sample + LS_X_delta);
    LS_Y_sample = LS_X_sample + LS_XY_delta;
    AudioInterrupts();

    // Serial.print(F("LS_Q_sample: "));
    // Serial.println(LS_Q_sample);
    // Serial.print(F("LS_X_sample: "));
    // Serial.println(LS_X_sample);
}

void LS_update_Q_sample(void)
{
    AudioNoInterrupts();
    LS_Q_sample = LiveSampler.Q_sample;
    AudioInterrupts();
}

FLASHMEM
void LS_Setup_buffers(bool stereo, bool first)
{
    /*
    DELAY
    indirizzo DELAY_fifo_L: 7000000C
    decimale: 1879048204
    indirizzo DELAY_fifo_R: 7006BC28
    decimale: 1879489576

    MONO
    *** LS_Setup_buffers - indirizzo _LS_buffer_mono: 700D7844
    decimale: 1879930948

    STEREO
    *** LS_Setup_buffers - indirizzo _LS_buffer_L tx: 700D7844
    decimale: 1879930948
    indirizzo LS_buffer_R_ptr tx: 7086BA64
    decimale: 1887877732

    */

    if (stereo)
    {
        Serial.println(F("*** LS_Setup_buffers() as Stereo ***"));

        if (!first)
        {
            PSRAM_Manager.Remove_samples_array(LS_buffer_mono_ptr);
        }

        // Inizializza PSRAM
        LS_buffer_L_ptr = PSRAM_Manager.New_samples_array(LS_STEREO_BYTES);
        LS_buffer_R_ptr = PSRAM_Manager.New_samples_array(LS_STEREO_BYTES);

        // Aggiorna oggetti
        LiveSampler.LS_buffer_L_ptr = LS_buffer_L_ptr;
        LiveSampler.LS_buffer_R_ptr = LS_buffer_R_ptr;
        Info.LS_buffer_L_ptr = LS_buffer_L_ptr;
        Info.LS_buffer_R_ptr = LS_buffer_R_ptr;

        Players_Manager.Broadcast_FIFO_stereo(LS_buffer_L_ptr, LS_buffer_R_ptr);

        Serial.print("*** LS_Setup_buffers - indirizzo _LS_buffer_L tx: ");
        Serial.println((unsigned long)LS_buffer_L_ptr, HEX);
        Serial.print("decimale: ");
        Serial.println((unsigned long)LS_buffer_L_ptr);
        Serial.print("indirizzo LS_buffer_R_ptr tx: ");
        Serial.println((unsigned long)LS_buffer_R_ptr, HEX);
        Serial.print("decimale: ");
        Serial.println((unsigned long)LS_buffer_R_ptr);
    }

    else
    {
        Serial.println(F("*** LS_Setup_buffers() as Mono ***"));
        if (!first)
        {
            // Libera la PSRAM
            PSRAM_Manager.Remove_samples_array(LS_buffer_L_ptr);
            PSRAM_Manager.Remove_samples_array(LS_buffer_R_ptr);
        }

        // Inizializza PSRAM
        LS_buffer_mono_ptr = PSRAM_Manager.New_samples_array(LS_MONO_BYTES);

        // Aggiorna oggetti
        LiveSampler.LS_buffer_mono_ptr = LS_buffer_mono_ptr;
        Info.LS_buffer_mono_ptr = LS_buffer_mono_ptr;
        Players_Manager.Broadcast_FIFO_mono(LS_buffer_mono_ptr);

        Serial.print("*** LS_Setup_buffers - indirizzo _LS_buffer_mono: ");
        Serial.println((unsigned long)LS_buffer_mono_ptr, HEX);
        Serial.print("decimale: ");
        Serial.println((unsigned long)LS_buffer_mono_ptr);
    }
}

FLASHMEM
void LS_erase_FIFO_array(int16_t *Array, int stereo)
{
    // richiede 620.4 ms
    // elapsedMicros T = 0;
    // int time;

    if (stereo)
    {
        for (auto i = 0; i < LS_STEREO_SAMPLES; ++i)
        {
            *(Array + i) = 0;
        }
    }
    else
    {
        for (auto i = 0; i < LS_MONO_SAMPLES; ++i)
        {
            *(Array + i) = 0;
        }
    }

    // time = T;
    // Serial.println(F("*** LS_erase_FIFO_array() richiede us: "));
    // Serial.println (time);
}

FLASHMEM
void LS_setup_LS_Patch(bool stereo)
{
    // set Sampler Patch and Sounds
    Patch[PATCHES_MAX].used = true;
    if (stereo)
    {
        // Patch
        Patch[PATCHES_MAX].instruments = 2;

        // Left channel recording
        Patch[PATCHES_MAX].Instrument[0].used = true;
        Patch[PATCHES_MAX].Instrument[0].sound_id = SOUNDS_MAX;
        Patch[PATCHES_MAX].Instrument[0].root_key = 60;
        Patch[PATCHES_MAX].Instrument[0].from_note = 0;
        Patch[PATCHES_MAX].Instrument[0].to_note = 127;
        Patch[PATCHES_MAX].Instrument[0].precedence = false;
        Patch[PATCHES_MAX].Instrument[0].lock = false;

        Patch[PATCHES_MAX].Instrument[0].Filter.use = false;        // yes/no
        Patch[PATCHES_MAX].Instrument[0].Filter.type = 0;           // lowpass, highpass, bandpass, notch
        Patch[PATCHES_MAX].Instrument[0].Filter.pivot = 10;         // 0 --> 30 filter frequency/note frequency
        Patch[PATCHES_MAX].Instrument[0].Filter.resonance = 7;      // 0 --> 40
        Patch[PATCHES_MAX].Instrument[0].Filter.modulation = 3;     // 0 --> 4 bit 1,2,3:modulation
        Patch[PATCHES_MAX].Instrument[0].Filter.index = 10;         // 1 --> 20 modulation_index
        Patch[PATCHES_MAX].Instrument[0].Filter.frequency_time = 5; // 0 --> 20

        // Right channel recording
        Patch[PATCHES_MAX].Instrument[1].used = true;
        Patch[PATCHES_MAX].Instrument[1].sound_id = SOUNDS_MAX + 1;
        Patch[PATCHES_MAX].Instrument[1].root_key = 60;
        Patch[PATCHES_MAX].Instrument[1].from_note = 0;
        Patch[PATCHES_MAX].Instrument[1].to_note = 127;
        Patch[PATCHES_MAX].Instrument[1].precedence = false;
        Patch[PATCHES_MAX].Instrument[1].lock = false;

        Patch[PATCHES_MAX].Instrument[1].Filter.use = false;        // yes/no
        Patch[PATCHES_MAX].Instrument[1].Filter.type = 0;           // lowpass, highpass, bandpass, notch
        Patch[PATCHES_MAX].Instrument[1].Filter.pivot = 10;         // 0 --> 30 filter frequency/note frequency
        Patch[PATCHES_MAX].Instrument[1].Filter.resonance = 7;      // 0 --> 40
        Patch[PATCHES_MAX].Instrument[1].Filter.modulation = 3;     // 0 --> 4 bit 1,2,3:modulation
        Patch[PATCHES_MAX].Instrument[1].Filter.index = 10;         // 1 --> 20 modulation_index
        Patch[PATCHES_MAX].Instrument[1].Filter.frequency_time = 5; // 0 --> 20

        // Other instruments
        Patch[PATCHES_MAX].Instrument[2].used = false;
        Patch[PATCHES_MAX].Instrument[3].used = false;
        Patch[PATCHES_MAX].Instrument[4].used = false;
        Patch[PATCHES_MAX].Instrument[5].used = false;
        Patch[PATCHES_MAX].Instrument[6].used = false;
        Patch[PATCHES_MAX].Instrument[7].used = false;

        // Left Sound
        Sound[SOUNDS_MAX].used = true;
        Sound[SOUNDS_MAX].mode = LS_mode;
        Sound[SOUNDS_MAX].file = FIRST_LIVE_SAMPLING_FILE + 1;
        Sound[SOUNDS_MAX].pitch = 0;                           // -128 + 127
        Sound[SOUNDS_MAX].A = LS_X_sample;                     // non utilizzato;
        Sound[SOUNDS_MAX].B = LS_X_sample + LS_buffer_dim - 1; // non utilizzato;
        Sound[SOUNDS_MAX].Noclick = 0;
        Sound[SOUNDS_MAX].pan = -16; // full Left
        Sound[SOUNDS_MAX].data = 0;  // bit4-3-2-1: midi_channel bit0: Attack ramp ("0" Slow, "1" Fast)
        Sound[SOUNDS_MAX].attack = 0;
        Sound[SOUNDS_MAX].decay = 50;
        Sound[SOUNDS_MAX].sustain = 50;
        Sound[SOUNDS_MAX].release = 10;
        Sound[SOUNDS_MAX].gain = 20;

        // Right Sound
        Sound[SOUNDS_MAX + 1].used = true;
        Sound[SOUNDS_MAX + 1].mode = LS_mode;
        Sound[SOUNDS_MAX + 1].file = FIRST_LIVE_SAMPLING_FILE + 2;
        Sound[SOUNDS_MAX + 1].pitch = 0;                           // -128 + 127
        Sound[SOUNDS_MAX + 1].A = LS_X_sample;                     // non utilizzato;
        Sound[SOUNDS_MAX + 1].B = LS_X_sample + LS_buffer_dim - 1; // non utilizzato;
        Sound[SOUNDS_MAX + 1].Noclick = 0;
        Sound[SOUNDS_MAX + 1].pan = 16; // full Right
        Sound[SOUNDS_MAX + 1].data = 0; // bit4-3-2-1: midi_channel bit0: Attack ramp ("0" Slow, "1" Fast)
        Sound[SOUNDS_MAX + 1].attack = 0;
        Sound[SOUNDS_MAX + 1].decay = 50;
        Sound[SOUNDS_MAX + 1].sustain = 50;
        Sound[SOUNDS_MAX + 1].release = 10;
        Sound[SOUNDS_MAX + 1].gain = 20;
    }

    else // mono
    {
        // Patch
        Patch[PATCHES_MAX].instruments = 1;
        Patch[PATCHES_MAX].Instrument[0].used = true;
        Patch[PATCHES_MAX].Instrument[0].sound_id = SOUNDS_MAX;
        Patch[PATCHES_MAX].Instrument[0].root_key = 60;
        Patch[PATCHES_MAX].Instrument[0].from_note = 0;
        Patch[PATCHES_MAX].Instrument[0].to_note = 127;
        Patch[PATCHES_MAX].Instrument[0].precedence = false;
        Patch[PATCHES_MAX].Instrument[0].lock = false;

        Patch[PATCHES_MAX].Instrument[0].Filter.use = false;        // yes/no
        Patch[PATCHES_MAX].Instrument[0].Filter.type = 0;           // lowpass, highpass, bandpass, notch
        Patch[PATCHES_MAX].Instrument[0].Filter.pivot = 10;         // 0 --> 30 filter frequency/note frequency
        Patch[PATCHES_MAX].Instrument[0].Filter.resonance = 7;      // 0 --> 40
        Patch[PATCHES_MAX].Instrument[0].Filter.modulation = 3;     // 0 --> 4 bit 1,2,3:modulation
        Patch[PATCHES_MAX].Instrument[0].Filter.index = 20;         // 1 --> 20 modulation_index
        Patch[PATCHES_MAX].Instrument[0].Filter.frequency_time = 5; // 0 --> 20

        Patch[PATCHES_MAX].Instrument[1].used = false;
        Patch[PATCHES_MAX].Instrument[2].used = false;
        Patch[PATCHES_MAX].Instrument[3].used = false;
        Patch[PATCHES_MAX].Instrument[4].used = false;
        Patch[PATCHES_MAX].Instrument[5].used = false;
        Patch[PATCHES_MAX].Instrument[6].used = false;
        Patch[PATCHES_MAX].Instrument[7].used = false;

        // Mono Sound
        Sound[SOUNDS_MAX].used = true;
        Sound[SOUNDS_MAX].mode = LS_mode;
        Sound[SOUNDS_MAX].file = FIRST_LIVE_SAMPLING_FILE;
        Sound[SOUNDS_MAX].pitch = 0;                           // -128 + 127
        Sound[SOUNDS_MAX].A = LS_X_sample;                     // non utilizzato;
        Sound[SOUNDS_MAX].B = LS_X_sample + LS_buffer_dim - 1; // non utilizzato;
        Sound[SOUNDS_MAX].Noclick = 0;
        Sound[SOUNDS_MAX].pan = 0;  // full Left
        Sound[SOUNDS_MAX].data = 0; // bit4-3-2-1: midi_channel bit0 (ch.1 in questo caso) : Attack ramp ("0" Slow, "1" Fast)
        Sound[SOUNDS_MAX].attack = 0;
        Sound[SOUNDS_MAX].decay = 50;
        Sound[SOUNDS_MAX].sustain = 50;
        Sound[SOUNDS_MAX].release = 10;
        Sound[SOUNDS_MAX].gain = 28;

        Sound[SOUNDS_MAX + 1].used = false;
    }
}

// ***************************************************************************************************************
// **********************************                   MIXER                   **********************************
// ***************************************************************************************************************

FLASHMEM
void Switch_to_MIXER()
{
    // Se non si sta editando, si parte dal primo Instrument esistente
    if (Lilla_state_0 != PERFORMANCE && Lilla_state_0 != SOUND_EDIT && Lilla_state_0 != INSTRUMENT_VCF && Lilla_state_0 != LIVE_SAMPLING)
    {
        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
        {
            if (Patch[Patch_id].Instrument[instrument_local].used)
            {
                Instrument_id = instrument_local;
                break;
            }
        }
        Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
    }

    else if (Lilla_state == LIVE_SAMPLING)
    {
        Instrument_id = LS_instrument;
        Sound_id = Patch[Patch_id].Instrument[Instrument_id].sound_id;
    }

    Golive_MIXER(Instrument_id);
}

FLASHMEM
void Golive_MIXER(int instrument)
{
    if (instrument < 0)
    {
        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
        {
            if (Patch[Patch_id].Instrument[instrument_local].used)
            {
                instrument = instrument_local;
            }
        }

        if (instrument < 0)
        {
            Serial.println(F("Golive_MIXER - ERROR: no instrument used!"));
            return;
        }
    }

    Lilla_state = MIXER;
    MX_source = instrument;

    Display.MX_page();
    for (auto source = 0; source < 9; ++source)
    {
        Display.MX_source_values(source);
    }
}

FLASHMEM
bool Verify_if_Instrument_original(uint8_t I)
{
    if (!Patch[Patch_id].Instrument[I].used && !Patch_cache_P.Instrument[I].used)
    {
        return true;
    }

    bool result;
    result = ((Patch[Patch_id].Instrument[I].used == Patch_cache_P.Instrument[I].used) &&
              (Patch[Patch_id].Instrument[I].sound_id == Patch_cache_P.Instrument[I].sound_id) &&
              (Patch[Patch_id].Instrument[I].root_key == Patch_cache_P.Instrument[I].root_key) &&
              (Patch[Patch_id].Instrument[I].from_note == Patch_cache_P.Instrument[I].from_note) &&
              (Patch[Patch_id].Instrument[I].to_note == Patch_cache_P.Instrument[I].to_note) &&
              (Patch[Patch_id].Instrument[I].precedence == Patch_cache_P.Instrument[I].precedence) &&
              (Patch[Patch_id].Instrument[I].lock == Patch_cache_P.Instrument[I].lock) &&

              (Patch[Patch_id].Instrument[I].Filter.use == Patch_cache_P.Instrument[I].Filter.use) &&
              (Patch[Patch_id].Instrument[I].Filter.type == Patch_cache_P.Instrument[I].Filter.type) &&
              (Patch[Patch_id].Instrument[I].Filter.pivot == Patch_cache_P.Instrument[I].Filter.pivot) &&
              (Patch[Patch_id].Instrument[I].Filter.resonance == Patch_cache_P.Instrument[I].Filter.resonance) &&
              (Patch[Patch_id].Instrument[I].Filter.modulation == Patch_cache_P.Instrument[I].Filter.modulation) &&
              (Patch[Patch_id].Instrument[I].Filter.index == Patch_cache_P.Instrument[I].Filter.index) &&
              (Patch[Patch_id].Instrument[I].Filter.frequency_time == Patch_cache_P.Instrument[I].Filter.frequency_time) &&
              Verify_is_Sound_original(Patch[Patch_id].Instrument[I].sound_id));

    return result;
}

FLASHMEM
bool Verify_is_Sound_original(uint8_t sound_id)
{
    return (
        (Sound[sound_id].file == Sound_cache_P[sound_id].file) &&
        (Sound[sound_id].mode == Sound_cache_P[sound_id].mode) &&
        (Sound[sound_id].pitch == Sound_cache_P[sound_id].pitch) &&
        (Sound[sound_id].A == Sound_cache_P[sound_id].A) &&
        (Sound[sound_id].B == Sound_cache_P[sound_id].B) &&
        (Sound[sound_id].Noclick == Sound_cache_P[sound_id].Noclick) &&
        (Sound[sound_id].pan == Sound_cache_P[sound_id].pan) &&
        (Sound[sound_id].data == Sound_cache_P[sound_id].data) &&
        (Sound[sound_id].attack == Sound_cache_P[sound_id].attack) &&
        (Sound[sound_id].decay == Sound_cache_P[sound_id].decay) &&
        (Sound[sound_id].sustain == Sound_cache_P[sound_id].sustain) &&
        (Sound[sound_id].release == Sound_cache_P[sound_id].release) &&
        (Sound[sound_id].gain == Sound_cache_P[sound_id].gain));
}

void Copy_all_Sound_to_Sound_cache_P(void)
{
    for (auto id_sound_local = 0; id_sound_local < SOUNDS_MAX; ++id_sound_local)
    {
        Sound_cache_P[id_sound_local] = Sound[id_sound_local];
    }
}

void Pull_all_Sound_from_Sound_cache_P(void)
{
    for (auto id_sound_local = 0; id_sound_local < SOUNDS_MAX; ++id_sound_local)
    {
        Sound[id_sound_local] = Sound_cache_P[id_sound_local];
    }
}

uint8_t Get_sounds_free(void)
{
    auto result = 0;

    for (auto id_sound_local = 0; id_sound_local < SOUNDS_MAX; ++id_sound_local)
    {
        if (!Sound[id_sound_local].used)
        {
            result++;
        }
    }
    return result;
}

void Read_all_Sounds(void)
{
    for (auto id_sound_local = 0; id_sound_local < SOUNDS_MAX; ++id_sound_local)
    {
        Archive.Read_Sound(id_sound_local, Sound[id_sound_local]);
        // P_Sound(id_sound_local);
    }
}

void Save_all_Sounds_changed(void)
{
    for (auto id_sound_local = 0; id_sound_local < SOUNDS_MAX; ++id_sound_local)
    {
        // Sound which have been changed only for .used
        if (Sound[id_sound_local].used != Sound_cache_P[id_sound_local].used)
        {
            Archive.Save_Sound(id_sound_local);
            Serial.println("Save_all_Sounds_changed: attenzione! Sound[sound_id].used e' variato per sound_id: ");
            Serial.println(id_sound_local);
        }

        // Sound used which have been changed
        else if ((Sound[id_sound_local].used == 1) && !Verify_is_Sound_original(id_sound_local)) // save Sound used and changed in phisical properties
        {
            Archive.Save_Sound(id_sound_local);
            Serial.println("Save_all_Sounds_changed: attenzione! Verify_is_Sound_original ha dato esito NEGATIVO che ha richiesto salvataggio su EEPROM per per sound_id: ");
            Serial.println(id_sound_local);
        }
    }
}

// ***************************************************************************************************************
// ****************************                         SETTINGS                        **************************
// ***************************************************************************************************************

FLASHMEM
void Save_CC_SETTINGS(void)
{
    Midi_reader.Stop();

    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        if (CC_Sound_gain[instrument_local] != CC_Sound_gain_cache[instrument_local])
        {
            Archive.Save_CC_Sound_gain(instrument_local, CC_Sound_gain[instrument_local]);
        }
    }

    if (CC_lowpass_filter != CC_lowpass_filter_cache)
    {
        Archive.Save_CC_lowpass_filter(CC_lowpass_filter);
    }

    Players_Manager.Stop_all_players();
    Midi_reader.Start();
}

FLASHMEM
void Ask_if_IMPORT_EXPORT_setup(void)
{
    confirmation = false;
    result = 0;
    while (!confirmation)
    {
#ifdef PCB_2022
        if (Read_encoder_inverse(25, result, 1, 0, 1))
#else
        if (Read_encoder(25, result, 1, 0, 1))
#endif
        {
            Display.Confirm_config_import_frame(result);
        }

        if (Read_pushbutton(25))
        {
            confirmation = true;
        }
    }
}

FLASHMEM
void Ask_if_FACTORY_RESET(void)
{
    confirmation = false;
    result = 0;
    while (!confirmation)
    {
#ifdef PCB_2022
        if (Read_encoder_inverse(25, result, 1, 0, 1))
#else
        if (Read_encoder(25, result, 1, 0, 1))
#endif
        {
            Display.Confirm_config_import_frame(result);
        }

        if (Read_pushbutton(25))
        {
            confirmation = true;
        }
    }
}

// ***************************************************************************************************************
// ****************************                 CONTROL CHANGE ASSIGNENT                **************************
// ***************************************************************************************************************

FLASHMEM
void Read_all_CC_Sound_gain()
{
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        Archive.Read_CC_Sound_gain(instrument_local, CC_Sound_gain[instrument_local]);
    }
}

// ***************************************************************************************************************
// ****************************   COPY RAW FILES FROM SD/LILLARAW TO FLASH MEMORY CHIP  **************************
// ***************************************************************************************************************

FLASHMEM
bool Copy_raw_files_from_SD_to_Flash()
{
    SerialFlashFile file;
    int row;

    Display.Copy_raw_files_SD_to_Flash_chip_titolo();

    // Wait for SD card
    while (!SD.begin(BUILTIN_SDCARD))
    {
        Display.Copy_raw_files_SD_to_Flash_chip_attesa_SD();
        delay(10000);
        return false;
    }
    Display.Delete_row(3);

    // Check if LILLARAW directory exists
    if (!SD.exists("/LILLARAW"))
    {
        Display.Copy_raw_files_SD_to_Flash_chip_directory_assente();
        delay(4000);
        return false;
    }

    // SD card info
    unsigned long SD_raw_volume = 0;
    int SD_raw_files = 0;
    File rootdir = SD.open("/LILLARAW");
    while (1)
    {
        // open a file from the SD card
        File f = rootdir.openNextFile();
        if (!f)
        {
            break;
        }

        SD_raw_volume += f.size();
        SD_raw_files++;
        f.close();
    }
    rootdir.close();

    Display.Copy_raw_files_SD_to_Flash_chip_consistenza_presente(SD_raw_volume, SD_raw_files, Get_raw_files_volume(), Get_raw_files());

    unsigned char id[3];
    SerialFlash.readID(id);
    Serial.println();
    Serial.printf("Flash chip Identity: %02X %02X %02X\n", id[0], id[1], id[2]);

    /*
    Chip      Uniform Sector Erase
              20/21   52    D8/DC
              -----   --    -----
    W25Q64CV      4   32    64
    W25Q128FV     4   32    64
    S25FL127S               64
    N25Q512A      4         64
    N25Q00AA      4         64
    S25FL512S               256
    SST26VF032    4
    AT25SF128A   32         64
    */

    //                      size  sector         busy pgm/erase chip
    // Part                 Mbyte kbyte ID bytes  cmd suspend   erase
    // ----                 ----  ----- --------  --- -------   -----
    // Winbond W25Q64CV     8     64    EF 40 17
    // Winbond W25Q128FV    16    64    EF 40 18  05  single    60 & C7
    // Winbond W25Q256FV    32    64    EF 40 19
    // Winbond W25Q512FV    64    64    EF 40 20
    // Spansion S25FL064A   8 ?         01 02 16
    // Spansion S25FL127S   16    64    01 20 18  05
    // Spansion S25FL128P   16    64    01 20 18
    // Spansion S25FL256S   32    64    01 02 19  05            60 & C7
    // Spansion S25FL512S   64   256    01 02 20
    // Macronix MX25L12805D 16     ?    C2 20 18
    // Macronix MX66L51235F 64          C2 20 1A
    // Numonyx M25P128      16     ?    20 20 18
    // Micron M25P80         1     ?    20 20 14
    // Micron N25Q128A      16    64    20 BA 18
    // Micron N25Q512A      64     ?    20 BA 20  70  single    C4 x2
    // Micron N25Q00AA     128    64    20 BA 21      single    C4 x4
    // Micron MT25QL02GC   256    64    20 BA 22  70            C4 x2
    // SST SST25WF010      1/8     ?    BF 25 02
    // SST SST25WF020      1/4     ?    BF 25 03
    // SST SST25WF040      1/2     ?    BF 25 04
    // SST SST25VF016B       1     ?    BF 25 41
    // SST26VF016            ?          BF 26 01
    // SST26VF032            ?          BF 26 02
    // SST25VF032            4    64    BF 25 4A
    // SST26VF064            8     ?    BF 26 43
    // LE25U40CMC          1/2    64    62 06 13
    // Adesto AT25SF128A    16          1F 89 01

    float erasing_time_ms = Get_flash_size() / eraseBytesPerSecond(id) * 1000;
    uint32_t erasing_time_ms_step = erasing_time_ms / 100;
    Display.Copy_raw_files_SD_to_Flash_chip_ultimo_avviso(erasing_time_ms);

    // Confirmation
    bool confirm = false;
    uint8_t action = 0;
    Display.Import_raw_files_frame(action);
    while (!confirm)
    {
        result = Read_encoder_simple(25);
        if (result != 0)
        {
#ifdef PCB_2022
            if (result == -1)
#else
            if (result == +1)
#endif
            {
                if (action == 0)
                {
                    action = 1;
                    Display.Import_raw_files_frame(action);
                }
            }

            else
            {
                if (action == 1)
                {
                    action = 0;
                    Display.Import_raw_files_frame(action);
                }
            }
        }

        if (Read_pushbutton(25))
        {
            confirm = true;
        }
    }

    if (action == 0)
    {
        return false;
    }

    // Start erasing flash chip
    Display.Copy_raw_files_SD_to_Flash_chip_avvio_copia();

    SerialFlash.eraseAll(); // uint32_t size = Get_flash_size(); // SerialFlash.capacity(id);
    elapsedMillis dotMillis = 0;
    unsigned char barcount = 0;

    Display.Copy_raw_files_SD_to_Flash_chip_percentuale_iniziale();

    while (SerialFlash.ready() == false)
    {
        if (dotMillis > erasing_time_ms_step)
        {
            Display.Cancel_text(x_pos(0) + barcount + 10, BAR_POS_Y, 3);
            dotMillis = 0;
            barcount = barcount + 1;
            if (barcount <= 100)
            {
                Display.Copy_raw_files_SD_to_Flash_chip_avanzamento(barcount);
            }
        }
    }

    // Start copying RAW files from SD to Flash chip
    Display.Copy_raw_files_SD_to_Flash_chip_sfondo_popup();
    rootdir = SD.open("/LILLARAW");
    row = 2;
    while (1)
    {
        // open a file from the SD/LILLARAW
        File f = rootdir.openNextFile();
        if (!f)
        {
            break;
        }

        const char *filename = f.name();
        unsigned long length = f.size();

        row++;
        if (row > 14)
        {
            Display.Copy_raw_files_SD_to_Flash_chip_sfondo_popup();
            row = 3;
        }
        Display.Copy_raw_files_SD_to_Flash_chip_file_da_copiare(row, filename, length);

        // create the (empty) file on the Flash chip, than copy data
        if (SerialFlash.create(filename, length))
        {
            SerialFlashFile ff = SerialFlash.open(filename);
            if (ff)
            {
                // copy data loop
                unsigned long count = 0;
                while (count < length)
                {
                    char buf[256];
                    unsigned int n;
                    n = f.read(buf, 256);
                    ff.write(buf, n);
                    count = count + n;
                }
                ff.close();
            }
            else
            {
                Display.Copy_raw_files_SD_to_Flash_chip_errore_flash_chip();
            }
        }

        else
        {
            Display.Copy_raw_files_SD_to_Flash_chip_errore_flash_full();
        }
        f.close();
    }
    rootdir.close();
    delay(10);

    // Display RAW files list
    Display.Copy_raw_files_SD_to_Flash_chip_copia_completata();
    row = 2;
    SerialFlash.opendir();

    char filename[64];
    uint32_t filesize;
    while (SerialFlash.readdir(filename, sizeof(filename), filesize))
    {
        row++;
        if (row > 14)
        {
            delay(4000);
            Display.Copy_raw_files_SD_to_Flash_chip_sfondo_elenco();
            row = 3;
        }

        Display.Copy_raw_files_SD_to_Flash_chip_file_copiato(row, filename, filesize);
    }
    delay(6000);
    return true;
}

FLASHMEM
float eraseBytesPerSecond(const unsigned char *id)
{
    if (id[0] == 0x20) // Micron
    {
        return 152000.0;
    }
    if (id[0] == 0x01) // Spansion
    {
        return 500000.0;
    }
    if (id[0] == 0xEF) // 419430.0 Winbond
    {
        return 512281.4;
    }
    if (id[0] == 0xC2) // Macronix
    {
        return 279620.0;
    }
    return 320000.0; // guess?
}

// ***************************************************************************************************************
// **********************************                 SOUND_EDIT                 *********************************
// ***************************************************************************************************************

// switch da VFC a SOUND_EDIT implicito
bool Request_SOUND_EDIT_mode(void)
{
#ifdef PCB_2022
    const uint8_t i_m = 12;
    const uint8_t SOUND_encoder[i_m] = {1, 2, 3, 9, 10, 11, 12, 17, 18, 19, 20};
#else
    const uint8_t i_m = 13;
    const uint8_t SOUND_encoder[i_m] = {1, 2, 3, 9, 10, 11, 12, 17, 18, 19, 20, 21};
#endif

    for (auto i = 0; i < i_m; ++i)
    {
        if (Read_encoder_fast(SOUND_encoder[i]))
        {
            return true;
        }
    }
    if (Read_pushbutton(2))
    {
        file_midi_ch_flag = !file_midi_ch_flag;
        return true;
    }
    if (Read_pushbutton(3))
    {
        Sound[Sound_id].pan = 0;

        AudioNoInterrupts();
        Players_Manager.Update_Preset_pan(Patch_id, Instrument_id);
        Players_Manager.Multicast_pan(Instrument_id);
        AudioInterrupts();

        return true;
    }
    if (Read_pushbutton(11))
    {
        slicing_mode = !slicing_mode;
        if (!slicing_mode)
        {
            slicing_window = Sound[Sound_id].B - Sound[Sound_id].A + 1;
        }

        return true;
    }
    if (Read_pushbutton(12))
    {
        trim_speed = 5;
        trim_step = Calc_trim_step(trim_speed);
        return true;
    }
    if (Read_pushbutton(17))
    {
        if (Sound[Sound_id].pitch != 0)
        {
            Sound[Sound_id].pitch = 0;

            AudioNoInterrupts();
            Players_Manager.Update_Preset_pitch(Patch_id, Instrument_id);
            Players_Manager.Multicast_pitch_for_sound_edit(Instrument_id);
            AudioInterrupts();
        }
        return true;
    }
    if (Read_pushbutton(18))
    {
        AudioNoInterrupts();
        bitWrite(Sound[Sound_id].data, 0, !bitRead(Sound[Sound_id].data, 0));
        Players_Manager.Update_Preset_attack_type(Patch_id, Instrument_id);
        AudioInterrupts();

        So_menu = 0;
        return true;
    }
    if (Read_pushbutton(21))
    {
        AudioNoInterrupts();
        Players_Manager.Release_all_players_for_instrument_solo(Instrument_id);
        Map_one_Instrument_for_all_notes(Instrument_id);
        AudioInterrupts();

        return true;
    }
    return false;
}

FLASHMEM
void Select_sound_edit_menu_elements(void)
{
    // voices of instrument_edit_menu that can be displayed
    Menu_So[0] = true; // RETURN
    Menu_So[1] = true; // CLONE
    Menu_So[2] = true; // DELETE

    if (Patch[Patch_id].instruments == INSTRUMENTS_MAX)
    {
        Menu_So[1] = false; // CLONE
    }

    if (Get_sounds_free() < 1)
    {
        Menu_So[1] = false; // CLONE
    }

    if (Patch[Patch_id].instruments == 1)
    {
        Menu_So[2] = false; // DELETE
    }

    So_menu_max = Menu_So[0] + Menu_So[1] + Menu_So[2] - 1;
}

FLASHMEM
void Macro_Sound_menu(void)
{
    int sound_original_0 = sound_original;
    sound_original = Verify_is_Sound_original(Sound_id);
    if (sound_original_0 != sound_original)
    {
        So_menu = 0;

        if (Lilla_state_0 != MIDI_LOOP)
        {
            Select_sound_edit_menu_elements();
        }

        Display.SOUND_EDIT_menu(); // displays the menu and updates "SO_menu_max" used by encoder_menu
        Display.Frame_SOUND_EDIT_menu(0);
    }
}

// ***************************************************************************************************************
// **********************************        INSTRUMENT_VCF FUNCTIONS           **********************************
// ***************************************************************************************************************

// switch da VFC da SOUND_EDIT implicito
bool Request_VCF_mode(void)
{
#ifdef PCB_2022
    const uint8_t i_m = 5;
    const uint8_t VCF_encoder[i_m] = {5, 6, 13, 14, 22};
#else
    const uint8_t i_m = 6;
    const uint8_t VCF_encoder[i_m] = {5, 6, 13, 14, 22, 23};
#endif
    for (auto i = 0; i < 5; ++i)
    {
        if (Read_encoder_fast(VCF_encoder[i]))
        {
            return true;
        }
    }

    if (Read_pushbutton(5))
    {
        Macro_VCF_filter_on_none();
        return true;
    }

    if (Read_pushbutton(6))
    {
        Macro_VCF_modulation_none();
        return true;
    }

    return false;
}

void Macro_VCF_filter_on_none(void)
{
    if (!Patch[Patch_id].Instrument[Instrument_id].Filter.use)
    {
        Patch[Patch_id].Instrument[Instrument_id].Filter.use = true;
    }
    else
    {
        Patch[Patch_id].Instrument[Instrument_id].Filter.use = false;
    }

    AudioNoInterrupts();
    Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);
    Players_Manager.Multicast_IF_update_filter_type(Instrument_id);

    if (Lilla_state_0 == LIVE_SAMPLING)
    {
        Patch[Patch_id].Instrument[1].Filter.use = Patch[Patch_id].Instrument[0].Filter.use;
        Instrument_id = 1;

        Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);
        Players_Manager.Multicast_IF_update_filter_type(Instrument_id);

        Instrument_id = 0;
    }
    AudioInterrupts();
}

void Macro_VCF_modulation_none(void)
{
    Patch[Patch_id].Instrument[Instrument_id].Filter.modulation = 0;

    AudioNoInterrupts();
    Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);

    if (Lilla_state_0 == LIVE_SAMPLING)
    {
        Patch[Patch_id].Instrument[1].Filter.modulation = Patch[Patch_id].Instrument[0].Filter.modulation;
        Instrument_id = 1;

        Players_Manager.Update_Preset_IF(Patch_id, Instrument_id);

        Instrument_id = 0;
    }
    AudioInterrupts();
}

// ***************************************************************************************************************
// **********************************                BOOTSTRAP                  **********************************
// ***************************************************************************************************************

FLASHMEM
void Bootstrap_setup(void)
{
    // Ingressi DirectSampler e LiveSampler non sono MAI utilizzati
    MAIN_mixer_out_L.gain(2, 0.0); // Direct Samp
    MAIN_mixer_out_R.gain(2, 0.0); // Live Sampler

    // LINE_IN silenziata verso la MAIN (Audio Board)
    MAIN_mixer_out_L.gain(1, 0.0);
    MAIN_mixer_out_R.gain(1, 0.0);

    // Note-to-pitch conversion array
    key_step = 0;
    Calc_pitch_from_note();

    // *******************   DIRECT SAMPLING  ************************
    // Flash memory data
    flash_dimension_MB = Get_flash_size() / 1048576;

    // Print Flash chip file list, occupation and available space.
    Print_flash_file_list();
    VFS_packets = Get_VFS_packets();
    VFS_packets_DS = VFS_packets; // Packets dedicated to Direct Sampling; PUO' essere dispari
    First_DS_packet = 0;
    Last_DS_packet = VFS_packets - 1;

    Serial.print(F("At startup there are: "));
    Serial.print(VFS_packets);
    Serial.println(F(" VFS packets (64KB --> 32K 16bit-samples)."));
    Print_VFS_allocation();

    // Valuta lo spazio su flash per nuovi file
    Serial.print(F("Flash occupation (kB): "));
    Serial.println(Get_flash_occupation() / 1024);
    Serial.print(F("Flash available for more .raw files (kB): "));
    Serial.println((Get_flash_size() - Get_flash_occupation() - FLASH_FREE_SPACE) / 1024);
    Serial.println();

    // Print EEPROM
    Serial.println("Print_EEPROM_content()");
    Archive.Print_EEPROM_content();

    // Reset all Recording and erase all DS_VFS_Packets
    if (false)
    {
        Serial.println(F("RESET_all_Recording -> SEED_all_Recording"));
        DS_seed_all_Recordings();

        Serial.println(F("RESET_all_Recording -> Erase_all_Packet"));
        Erase_all_Packet_for_DS();
        Serial.println();
    }

    // Read all Recordings (from EEPROM)
    DS_read_all_Recordings(); // reads from EEPROM and print all Recordings
    Clean_up_VFS();           // se ci sono recording non consistent cancella packets e recording
    Defragment_VFS();         // Prima aggiorna la VFS_FAT_table; in base a questa se ci sono buchi nel VFS li chiude, aggiornando i Recording e salvandoli, infine aggiornando VFS_FAT_table
    DS_update_recordings();
    // Print_VFS_FAT_table();

    // Setup Input Gain
    DS_gain = 20;
    LINE_IN_amplifier.Set_gain(Volume_float[DS_gain]);

    // *******************   CORE ARRAYS  ************************
    Delete_all_Patches_and_Sounds();

    Read_all_Sounds(); // compila tutti i Sound leggendo dalla EEPROM
    Copy_all_Sound_to_Sound_cache_P();
    Read_all_Patches(); // compila tutte le Patch leggendo dalla EEPROM

    Archive.Read_optimization(optimization);
    Archive.Read_first_octave(first_octave);
    Read_all_CC_Sound_gain();
    Archive.Read_CC_lowpass_filter(CC_lowpass_filter);

    Update_Patches_number(); // aggiorna patches_number (numero di patchi disponibili)
    Patch_id = Get_first_Patch_id_existing();

    // *******************  DELAY+LFO  ************************
    // Creazione degli array FIFO
    DELAY_fifo_L = PSRAM_Manager.New_samples_array(DELAY_FIFO_BYTES);
    DELAY_fifo_R = PSRAM_Manager.New_samples_array(DELAY_FIFO_BYTES);

    Serial.print("indirizzo DELAY_fifo_L: ");
    Serial.println((unsigned long)DELAY_fifo_L, HEX);
    Serial.print("decimale: ");
    Serial.println((unsigned long)DELAY_fifo_L);
    Serial.print("indirizzo DELAY_fifo_R: ");
    Serial.println((unsigned long)DELAY_fifo_R, HEX);
    Serial.print("decimale: ");
    Serial.println((unsigned long)DELAY_fifo_R);

    // Inizializzazione dei Delay_L, Delay_R
    Delay_L.DELAY_fifo = DELAY_fifo_L;
    Delay_R.DELAY_fifo = DELAY_fifo_R;

    // Tenta l'import dei dati patch_id delay da SD alla EEPROM
    Archive.Copy_patch_Delay_data_from_SD_to_Eeprom(Patch_id);

    // Imposta i parametri per il Delay
    if (Read_pushbutton(35))
    {
        Delay_data.samples = 20;
        Delay_data.samples_LR = 0;
        Delay_data.instrument_route = 0b11111111;
        Delay_data.modulation_source = 0;
        Delay_data.modulation_depth = 30;
        Delay_data.modulation_frequency = 12;
        Delay_data.modulation_phase_LR = 0;
        Delay_data.loop_gain = 5;

        // Salva i parametri per il Delay su EEPROM
        Archive.Save_Delay_to_Eeprom(Delay_data);
    }

    // Leggi i parametri del Delay da EEPROM
    Archive.Copy_patch_Delay_data_from_Eeprom_to_Ram(Delay_data);

    // Calcola i valori del Delay
    Calc_Delay_values(Delay_data);

    // Trasmetti i valori
    Delay_L.Setup_delay(Delay_values.samples);                           // Right channel 0<= value[sample] <= DELAY_PIPELINE - AUDIO_BLOCK_SAMPLES
    Delay_R.Setup_delay(Delay_values.samples + Delay_values.samples_LR); // channel 0<= value[sample] <= DELAY_PIPELINE - AUDIO_BLOCK_SAMPLES
    Delay_L.Set_delay_modulation_source(Delay_values.modulation_source); // 0:none 1:LFO  2:input_1
    Delay_R.Set_delay_modulation_source(Delay_values.modulation_source); // 0:none 1:LFO  2:input_1
    Delay_L.Set_delay_modulation_gain(Delay_values.modulation_depth);
    Delay_R.Set_delay_modulation_gain(Delay_values.modulation_depth);
    D_gain_L_feedback.Set_gain(Delay_values.loop_gain);
    D_gain_R_n.Set_gain(Delay_values.loop_gain);

    LFO_D[0].Set_amplitude(1000); //  modulation depth is set in Delay(0), not here.
    LFO_D[1].Set_amplitude(1000); //  modulation depth is set in Delay(1), not here.
    LFO_D[0].Set_frequency(Delay_values.modulation_frequency);
    LFO_D[1].Set_frequency(Delay_values.modulation_frequency);
    LFO_D[0].Set_phase(Delay_values.modulation_phase_LR);

    // *******************   LIVE SAMPLING  ************************
    LS_stereo = false;
    LS_buffer_dim = (LS_stereo ? LS_STEREO_SAMPLES : LS_MONO_SAMPLES);

    LiveSampler.Reset();
    LS_state = 0;

    LS_id_sound = SOUNDS_MAX;
    LS_instrument = 0;
    LS_X_delta = 0;
    LS_X_sample = 0;
    LS_menu = 0;
    LS_mode = 0; // playing mode 0:A-->B   1:B-->A   2:loop A-->B   3:loop A-->B-->A   4:loop B-->A-->B   5:loop B-->A B-->A
    LS_window_width = LS_buffer_dim;
    LS_window_step = LS_window_width / 8;
    LS_XY_lock = true; // LS_X_sample blocked on FIFO; LS_X_delta is useless
    LS_XY_delta = 44100;
    LS_Y_sample = LS_X_sample + LS_XY_delta;
    LS_X_step = LS_window_width / LS_COMB;
    LS_feedback = 0;

    LS_Setup_buffers(LS_stereo, true); // LS_Setup_buffers(bool stereo, bool first)

    // ****LPF final filter Output Butterworth filters, 12 db/octave ****
    biquad_L.setLowpass(0, 20000, 0.707);
    biquad_R.setLowpass(0, 20000, 0.707);

    Lilla_state = PERFORMANCE;
    Lilla_state_0 = Lilla_state;

    Players_Manager.Update_all_Preset(Patch_id, Volume_float[volume_patch]);
    Fill_all_Noclick(); // fill Noclick for all Instrument in the Patch
    Fill_all_Wavetable();

    // *******************    MIDI LOOP   ************************
    LOOP_id = -1;
    LOOP_run_button_state = true; // stato pulsante 7 (arresta/riavvia tutti i loop)
    LOOP_reset_all_data();

    // *******************    COVER PAGE    **********************
    Display.Lilla_cover_slow();

    // ****************     PERFORMANCE PAGE   ********************
    patch_old = Patch_id;
    P_Patch(Patch_id);
    Patch_cache_P = Patch[Patch_id];
    Update_all_maps_Instrument_for_notes();
    P_map_instrument_for_note(0); // P_map_instrument_for_note(uint8_t midi_channel)

    Golive_with_PERFORMANCE(Patch_id);

    // *******************    START MIDI   ************************
    Midi_reader.Begin();
    Midi_reader.Start();
    delay(10);

    // *******************  START TRIGGERS  ***********************
    Trigger_0.Start();
    Trigger_1.Start();
    delay(20);
}