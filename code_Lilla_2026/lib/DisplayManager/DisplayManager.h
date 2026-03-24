/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>     // https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
#include <Adafruit_ILI9341.h> // 1.5.12 version - Hardware-specific library
#include "DisplayPrimitives.h"

#include <AudioStream.h>      // solo per definizione AUDIO_SAMPLE_RATE
#include "SharedDS.h"
#include "SharedLS.h"
#include "SharedLoop.h"
#include "SharedDelay.h"
#include "SharedMixer.h"
#include "InfoMaster.h"


class DisplayManager
{
private:
    // riferimenti esterni
    InfoMaster &Info;

    void Note(const int note_number);
    int col;
    int row;
    void Cancel_text_reset_cursor(const int X, const int Y, int N);

    // Logo
    void Logo(const float light);
    void Cover_text(const float light);
    uint16_t Calc_color(uint16_t color_peak, float light);
    static constexpr int Logo_position_DX = 80; // pixel
    static constexpr int Logo_position_DY = 50; // pixel

    // Avvisi
    //                         "0123456789012345678901234..7890123456789109876543210";
    const char ADV_VFS_0[50] = "    NOT ENOUGH MEMORY LEFT FOR DIRECT-SAMPLING!";
    const char ADV_VFS_1[50] = "     IF DIRECT-SAMPLING IS NEEDED PLEASE REPEAT";
    const char ADV_VFS_2[50] = "         IMPORT WITH MAX 63MB OF RAW FILES";
    const char ADV_VFS_3[50] = "     RAW FILES IMPORT AND MEMORY CONFIGURATION";
    const char ADV_VFS_4[50] = "     COMPLETED. LILLA RESTARTS IN FEW SECONDS.";

    // Popup
    int L_POPUP;     // Larghezza
    int H_POPUP;     // Altezza
    int X_POPUP;     // X posizione su display
    int Y_POPUP;     // Y posizione su display
    int Y_POPUP_TXT; // prima riga testo
    int X_POPUP_OPT; // riga opzioni
    int Y_POPUP_OPT; // riga opzioni

    // PERFORMANCE
    static constexpr float Frame_heigh = 15;
    static constexpr float P_column_PATCH = 30;
    static constexpr float P_column_Patch_id = 36;
    static constexpr float P_column_VOLUME = 41;
    static constexpr float P_column_Volume_value = 47.5;
    static constexpr float P_column_Instrument_frame = 3;
    static constexpr float P_chars_width_Instrument_frame = 48;
    
    static constexpr float P_row_Instrument_title = 5;
    static constexpr float P_column_SOUND_title = 0.5;
    static constexpr float P_column_LOCK_title = 7;
    static constexpr float P_column_P_title = 12.5;
    static constexpr float P_column_MIDI_title = 15;
    static constexpr float P_column_ROOT_K_title = 20.5;
    static constexpr float P_column_FROM_K_title = 28;
    static constexpr float P_column_TO_K_title = 36.5;
    static constexpr float P_column_PAN_title = 43;
    static constexpr float P_column_GAIN_title = 47.5;

    static constexpr float P_pixel_x_LED = 8; // posizione led pagina Performance
    static constexpr float P_column_Sound = 3;
    static constexpr int P_chars_Instrument_element[8] = {1, 1, 2, 4, 4, 4, 2, 4}; // Lock, Precedence,....., Gain
    static constexpr int P_column_Instrument_element[8] = {8.5, 12.5, 16, 21.5, 28.5, 36.5, 43, 47.5}; // Lock, Precedence,....., Gain

    int P_menu_frame_on_element_0 = 0;
    int P_Instrument_pixels_y(int position);

    int Sound_Id(int patch_id, int instrument_id);
    

    // rappresentazione Instrument VCF page
    static constexpr float Instrument_INDENT_X0 = 0.5; // indentatura dell'header nella Performance (in caratteri) a sinistra
    static constexpr float Instrument_SPACE_X = 1.5;   // spaziatura (in caratteri) tra due titole dell'header nella Performance
    static constexpr int Instrument_VCF_TXT_X = 30;    // (caratteri)
    static constexpr int Instrument_VCF_TXT_Y = 5;     // (caratteri)

    // MIXER
    static constexpr int MX_X0 = 8; // (caratteri)
    static constexpr int MX_Y0 = 5;

    // MIDI_LOOP
    static constexpr int Loop_HEAD_R = 5;     // posizione "LOOP"
    static constexpr int Loop_HEAD_C = 3;     // posizione "LOOP"
    static constexpr int Loop_LOOPS_X = 11;   // centro prima track
    static constexpr int Loop_LOOP_TIME = 20; // posizione di LOOP_time
    int8_t Loop_menu_position_0 = 0;
    int8_t Loop_X_position_menu_0 = 0;
    int8_t Loop_dimension_voice_menu_0 = 0;

    // DIRECT_SAMPLING
    static constexpr int DS_VUMETER_BAR_X = 210; // coordinate angolo in basso a SX
    static constexpr int DS_VUMETER_BAR_Y = 175; // coordinate angolo in basso a SX
    static constexpr int DS_VUMETER_BAR_DISTANCE = 6;
    static constexpr int DS_VUMETER_BAR_DX = 12; // distanza tra le mediane delle DS_VUMETER_BAR_DISTANCE
    static constexpr int DS_START_Y = DS_VUMETER_BAR_Y + 13;
    static constexpr int DS_START_X = DS_VUMETER_BAR_X - 7;
    static constexpr int DS_MV = 12;
    int DS_frame_menu_position_0 = 0;
    int DS_VU_meter_value_old[2] = {0, 0};

    // SETUP
    // Control Change
    static constexpr int Control_change_X = 13; // posizione (in caratteri)
    // Copia file RAW da scheda SD
    static constexpr int FileCopy_BAR_POS_Y = 225; // display_coordinate_y(15)

    // DELAY
    static constexpr int Delay_ROW_BASE = 6;

public:
    DisplayManager(InfoMaster &Info) : Info(Info) {}

    void Lilla_cover_slow(void);
    void Lilla_cover_saturate(void);

    // Funzioni comuni
    void Show_effects(void);
    void Resolution(void);
    void Downsampling(void);
    void Lowpass_filter(void);

    // Gestione LED
    void Led_PERFORMANCE_instrument(int instrument_id, bool on);
    void Led_SOUND_EDIT_instrument(int instrument_id, bool on);
    void Led_INSTRUMENT_VCF_instrument(int instrument_id, bool on);
    void Led_DIRECT_SAMPLING(bool on);
    void Led_tuning_tone(int patch_id);

    // PERFORMANCE
    void P_swow_pointer_frame(P_field_description_struct value, bool show);
    void P_Performance_page(bool change_patch, bool change_vol);
    void P_Show_Performance(void);
    void P_Show_Patch_number(bool change_patch);
    void P_Patch_volume(bool change_vol); // shows VOLUME <value>
    void P_Patch_volume_value(bool change_vol);
    void P_Patch_header(bool change_patch, bool change_vol);
    void Patch_volume_color(bool change_patch, bool change_vol);

    void P_Performance_menu(void);
    void P_Frame_performance_menu(int position, bool fresh);
    void P_Delete_all_frame_performance_menu(void);
    void P_Confirm_patch_change_popup(void);
    void P_Confirm_patch_change_popup_frame(int value);
    void P_Confirm_frame(int X, int Y, int chars, bool print);
    void P_Confirm_patch_delete_popup(void);
    void P_Confirm_patch_delete_popup_frame(int value);
    // Instrument
    void P_show_Instruments_header(void);
    void P_show_all_Instruments(int patch_id);
    void P_show_Instrument_description(int patch_id, int instrument_id, bool editing);
    void P_show_Sound(int instrument_id, bool editing);
    void P_show_Lock(int patch_id, int instrument_id, bool editing);
    void P_show_Precedence(int patch_id, int instrument_id, bool editing);
    void P_show_Midi(int patch_id, int instrument_id, bool editing);
    void P_show_RootKey(int patch_id, int instrument_id, bool editing);
    void P_show_FromKey(int patch_id, int instrument_id, bool editing);
    void P_show_ToKey(int patch_id, int instrument_id, bool editing);
    void P_show_Pan(int patch_id, int instrument_id, bool editing);
    void P_show_Gain(int patch_id, int instrument_id, bool editing);

    void P_Delete_Instrument(int position);
    void P_show_delete_Instrument_frame(float line, bool show);
    // Tuning tone
    void P_show_TuningTone_instrument(int patch_id);
    void P_show_gain_TT(int patch_id);

    // SOUND_EDIT
    void SOUND_EDIT_menu(void);
    void Frame_SOUND_EDIT_menu(int position);
    void Delete_all_frame_SOUND_EDIT_menu(void);
    void Show_sound(int patch_id, int instrument_id);
    void Show_wave(int instrument_id);
    void File(int instrument_id);
    void Midi_channel(int instrument_id);
    void Pitch(int instrument_id);
    void Gain_sound(int patch_id, int instrument_id);
    void Pan(int instrument_id);
    void Attack(int instrument_id);
    void Decay(int instrument_id);
    void Sustain(int instrument_id);
    void Release(int instrument_id);
    void Play_mode(int instrument_id);
    void Noclick(int instrument_id, bool value);
    void Trim_step(void);
    void Pitch_voices_max(int instrument_id);

    // INSTRUMENT_VCF
    void Instrument_VCF_page(int patch_id, int instrument_id);
    void Show_VCF_gain(int sound_id);
    void Show_VCF_solo(void);
    void Show_VCF_filter_type(int instrument_id);
    void Show_VCF_cutoff(int instrument_id);
    void Show_VCF_resonance(int instrument_id);
    void Show_VCF_lfo_type(int instrument_id);
    void Show_VCF_lfo_freq_time(int instrument_id);
    void Show_VCF_lfo_index(int instrument_id);
    void Show_VCF_pan(int sound_id);

    // DELAY
    void Delay_page(void);
    void D_sounds(void);
    void D_delay(void);
    void D_read_gain(void); // feedback
    void D_delay_LR(void);
    void D_modulation_type(void);
    void D_modulation_frequency(void);
    void D_modulation_depth(void); // index
    void D_modulation_phase_LR(void);
    void Delay_disabled(void);

    // DIRECT SAMPLING
    void DS_confirm_EXIT_from_DS(void);
    void DS_page(int recording);
    void DS_sampler_IO(void);
    void DS_bar(int channel, int value);
    uint16_t DS_calc_bar_color(float value);
    void DS_line_out(bool visible);
    void DS_sampler_frame(bool visible);
    void DS_sampler_txt(bool color);
    void DS_available_memory(void);
    void DS_raw_available_memory(void);
    void DS_hide_recording(void);
    void DS_advice_delete(bool value);
    void DS_advice_no_conversion(int DS_export, bool value);
    void DS_conversion_options(int file_L_RAW, int file_R_RAW, int DS_export);
    void DS_export_options(int file_L_RAW, int file_R_RAW, int DS_export);
    void DS_Recording_description(int recording, bool led);
    void DS_recording_seconds(void);
    void DS_update_recording_seconds(float value);
    void DS_volume(void);
    void DS_update_volume(bool adj = true);
    void DS_show_gain(void);
    void DS_menu(void);
    void DS_frame_menu(int position);

    // SETUP
    void Settings_page(void);
    void Key_step(void);
    void First_octave(void);
    void Optimization(void);
    void Settings_frame(int8_t value);
    void CC_page(void);
    void All_CC_Sound_gain(void);
    void Show_CC_Sound_gain(int value);
    void Show_CC_lowpass_filter(void);
    void Frame_CC_page_menu(int value);
    // configura VFS
    void Show_VFS_packets(void);
    void Make_VFS_presentation(void);
    void Make_VFS_assignments(void);
    void Make_VFS_restart(void);
    void Make_VFS_not_enough_memory_for_sampler(void);
    // copia RAW files da SD
    void Import_raw_files_frame(uint8_t value);
    void Confirm_config_import_popup(void);
    void SD_missing(uint16_t color);
    void Config_import_FILE_error_popup(void);
    void Config_import_REBOOT_popup(void);
    void Confirm_config_import_frame(uint8_t value);
    void Confirm_config_export_popup(void);
    void Config_export_SD_error_popup(void);
    void Config_export_save_popup(void);
    void Confirm_factory_reset_popup(void);
    void Factory_reset_wait_popup(void);
    void Encoder_pushbutton_test_board(void);
    void Encoder_pushbutton_test_result(const int device, const int element, const int value);
    void Config_reset_popup(void);
    void Copy_raw_files_SD_to_Flash_chip_titolo(void);           // importa i file RAW dalla scheda SD
    void Copy_raw_files_SD_to_Flash_chip_waiting_for_SD(void);   // attesa 10sec scheda SD
    void Copy_raw_files_SD_to_Flash_chip_lillaraw_missing(void); // manca /LILLARAW
    void Copy_raw_files_SD_to_Flash_chip_files_report(unsigned long SD_raw_volume, int SD_raw_files, int flash_raw_volume, int flash_raw_files);
    void Copy_raw_files_SD_to_Flash_chip_last_warning(float erasing_time_ms);
    void Copy_raw_files_SD_to_Flash_chip_job_start(void); // inizia la cancellazione (erasing) della Flash memory
    void Copy_raw_files_SD_to_Flash_chip_initial_percentage(void);
    void Copy_raw_files_SD_to_Flash_chip_progress(unsigned char barcount);
    void Copy_raw_files_SD_to_Flash_chip_popup_landscape(void); // black panel
    void Copy_raw_files_SD_to_Flash_chip_list_landscape(void);
    void Copy_raw_files_SD_to_Flash_chip_files_to_copy(int row, const char *filename, unsigned long length);
    void Copy_raw_files_SD_to_Flash_chip_flash_error(void);      // Flash memory error!
    void Copy_raw_files_SD_to_Flash_chip_flash_full_error(void); // Flash memory full!
    void Copy_raw_files_SD_to_Flash_chip_job_done(void);
    void Copy_raw_files_SD_to_Flash_chip_file_copied(int row, const char *filename, uint32_t filesize);

    // MIDI_LOOP
    static constexpr int Loop_LED_Y = 151; // Y-PIXEL primo led
    static constexpr int Loop_LED_X = 70;  // X-PIXEL primo led
    static constexpr int Loop_LED_DY = 11; // spaziatura Y
    void LOOP_page(void);
    void Loop_loop_id(void);
    void Loop_midi_loop_title(void);
    void Loop_track_data(int track);
    void Loop_time_stretched(void);
    void Loop_REC_advice(int track, bool on);
    void Loop_led(int track, int instrument_id, bool on);    // displays track/instrument_id LED
    void Loop_led_metronomo(int Xled, int Yled, bool ONled); // displays metronomo LED
    void Loop_menu(void);
    void Frame_loop_menu(int position, bool fresh);
    void Delete_all_frame_loop_menu(void);

    // MIDI_MONITOR
    void Midi_monitor_page(void);
    void Midi_monitor_frame(void);
    void Midi_monitor_data(uint8_t incoming_midi_channel, uint8_t incoming_midi_message, int8_t incoming_note_number, int8_t incoming_velocity, int32_t incoming_midi_value, int8_t incoming_number);

    // MIXER
    void MX_page(void);
    void MX_source_values(int source);
    void MX_source_values_write(int source);
    void MX_source_values_edit(int source);
    void MX_source_values_jump(int old_source, int new_source);
};