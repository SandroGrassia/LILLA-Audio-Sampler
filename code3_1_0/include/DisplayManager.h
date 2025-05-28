/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>     // https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
#include <Adafruit_ILI9341.h> // 1.5.12 version - Hardware-specific library
#include <AudioStream.h>      // solo per definizione AUDIO_SAMPLE_RATE
#include "SharedElements.h"
#include "ElementiGrafici.h"
#include "PlayersStatistics.h"
#include "InfoMaster.h"

// parametri collegamento SPI Display
#define Sclk 27  // SCK
#define Mosi 26  // SDA
#define Cs 38    // CS
#define Dc 29    // A0
#define Rst 30   // RESET
#define Miso 255 // non connesso

class DisplayManager
{
private:
    // 16-bit ('565') color settings http://www.barth-dev.de/online/rgb565-color-picker/ and https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
    Adafruit_ILI9341 tft = Adafruit_ILI9341(Cs, Dc, Mosi, Sclk, Rst); // https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.h
    GFXcanvas16 canvas = GFXcanvas16(WAVE_WIDTH, WAVE_HEIGHT);        // https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.cpp ; GFXcanvas16 creates an array of w*h*2 bytes in memory

    // riferimenti esterni
    InfoMaster &Info;
    PlayersStatistics &Players_statistics;

    void Note(const uint8_t note_number);
    int col;
    int row;
    int y_pos(const float row);
    int x_pos(const float col);
    void Cancel_text_reset_cursor(const int X, const int Y, int N);

    // Logo
    void Logo(const float light);
    void Cover_text(const float light);
    uint16_t Calc_color(uint16_t color_peak, float light);
    static constexpr int Logo_position_DX = 80; // pixel
    static constexpr int Logo_position_DY = 50; // pixel

    // Avvisi
    //                                  "0123456789012345678901234..7890123456789109876543210";
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
    uint8_t Id_sound(const uint8_t session, const uint8_t instrument);
    static constexpr int X_LED = 8; // posizione led pagina Performance (caratteri)
    int Instrument_Y_POSITION(int position);
    // rappresentazione Instrument VCF page
    static constexpr float Instrument_INDENT_X0 = 0.5; // indentatura dell'header nella Performance (in caratteri) a sinistra
    static constexpr float Instrument_SPACE_X = 1.5;   // spaziatura (in caratteri) tra due titole dell'header nella Performance
    static constexpr int Instrument_VCF_TXT_X = 30;    // (caratteri)
    static constexpr int Instrument_VCF_TXT_Y = 5;     // (caratteri)
    // canvas Y-positions on display
    static constexpr int WAVE_X0 = 5;
    static constexpr int WAVE_MAX = 122;
    static constexpr int WAVE_HEIGHT = 97; // deve essere DISPARI
    // static constexpr int WAVE_WIDTH = 310 definito in SharedElements
    static constexpr int T_LOW = WAVE_HEIGHT - 12;
    // wave Y-positions on canvas
    static constexpr int CANVAS_WAVE_MIN = WAVE_HEIGHT - 1;
    static constexpr int CANVAS_WAVE_0 = CANVAS_WAVE_MIN / 2;

    // MIXER
    static constexpr int MX_X0 = 8; // (caratteri)

    // MIDI_LOOP
    static constexpr int Loop_HEAD_R = 5;     // posizione "LOOP"
    static constexpr int Loop_HEAD_C = 3;     // posizione "LOOP"
    static constexpr int Loop_LOOPS_X = 11;   // centro prima track
    static constexpr int Loop_LOOP_TIME = 20; // posizione di LOOP_time

    // DIRECT_SAMPLING
    static constexpr int DS_VUMETER_BAR_X = 210; // coordinate angolo in basso a SX
    static constexpr int DS_VUMETER_BAR_Y = 175; // coordinate angolo in basso a SX
    static constexpr int DS_VUMETER_BAR_DISTANCE = 6;
    static constexpr int DS_VUMETER_BAR_DX = 12; // distanza tra le mediane delle DS_VUMETER_BAR_DISTANCE
    static constexpr int DS_START_Y = DS_VUMETER_BAR_Y + 13;
    static constexpr int DS_START_X = DS_VUMETER_BAR_X - 7;
    static constexpr int DS_MV = 12;

    // SETUP
    // Control Change
    static constexpr int Control_change_X = 13; // posizione (in caratteri)
    // Copia file RAW da scheda SD
    static constexpr int FileCopy_BAR_POS_Y = 225; // y_pos(15)

    // DELAY
    static constexpr int Delay_ROW_BASE = 6;

    // LIVE_SAMPLING
    float LS_K_wave_color;
    int LS_window_A_sample;
    int LS_window_B_sample;
    static constexpr int LS_MV = 4;
    static constexpr uint16_t LS_WAVE_COLOR = 0xE08A;
    static constexpr uint16_t LS_WAVE_ZERO_COLOR = 0x7BCF;
    static constexpr uint16_t LS_WAVE_BOARD = 0xFE40;     // 0xA514
    static constexpr uint16_t LS_X_COLOR = ILI9341_GREEN; // Live Sampling linee verticali di esecuzione
    static constexpr uint16_t LS_Y_COLOR = ILI9341_WHITE; // Live Sampling linee verticali di esecuzione

public:
    DisplayManager(InfoMaster &Info, PlayersStatistics &Players_statistics) : Info(Info), Players_statistics(Players_statistics) {}

    void Start(void);
    void Lilla_cover_slow(void);

    // Funzioni comuni
    void Frame(const float col, const float row, const int chars, const bool print);
    void Popup(const String text, const uint16_t text_color, const uint16_t filler_color);
    void Board(const float col, const float row, const int chars);
    void Cancel_text(const int X, const int Y, int N);
    void Unity(String A);
    void Delete_row(const float row);
    void Show_effects(void);
    void Resolution(void);
    void Downsampling(void);
    void Lowpass_filter(void);

    // Gestione LED
    void Led_instrument_PERFORMANCE(const uint8_t instrument);
    void Led_instrument_SOUND_EDIT(const uint8_t instrument);
    void Led_instrument_INSTRUMENT_VCF(const uint8_t instrument);
    void Led_instrument_DIRECT_SAMPLING(void);
    void Led_instrument_LIVE_SAMPLING(void);
    void Led_tuning_tone(const int session);

    // PERFORMANCE
    void Performance_page(const bool change_session, const bool change_vol);
    void Show_Performance(void);
    void Session_volume_value(const bool change_vol); // chi la usa??
    void Show_Session_number(const bool change_session);
    void Session_volume(const bool change_vol);
    void Session_header(const bool change_session, const bool change_vol);
    void Session_volume_color(const bool change_session, const bool change_vol);
    void Performance_menu(void);
    void Frame_performance_menu(const uint8_t position, const bool fresh);
    void Delete_all_frame_performance_menu(void);
    void Confirm_session_change_popup(void);
    void Confirm_session_change_popup_frame(const uint8_t value);
    void Confirm_frame(int X, int Y, uint8_t chars, const bool print);
    void Confirm_save_changes_page(void);
    void Confirm_session_delete_popup(void);
    void Confirm_session_delete_popup_frame(const uint8_t value);
    // Instrument
    void Instruments_header(void);
    void All_Instrument(const uint8_t session);
    void Show_Instrument_description(const uint8_t session, const uint8_t instrument, const bool editing);
    void Delete_Instrument(const int position);
    void Instrument_frame_on_position(const float position, const bool print);
    // Tuning tone
    void Instrument_TT(const uint8_t session);
    void Volume_TT(const uint8_t session);

    // SOUND_EDIT
    void SOUND_EDIT_menu(void);
    void Frame_SOUND_EDIT_menu(const uint8_t position);
    void Delete_all_frame_SOUND_EDIT_menu(void);
    void Show_sound(const uint8_t session, const uint8_t instrument);
    void Show_wave(const uint8_t instrument);
    void File(const uint8_t instrument);
    void Midi_channel(const uint8_t instrument);
    void Pitch(const uint8_t instrument);
    void Gain_sound(const uint8_t session, const uint8_t instrument);
    void Pan(const uint8_t instrument);
    void Attack(const uint8_t instrument);
    void Decay(const uint8_t instrument);
    void Sustain(const uint8_t instrument);
    void Release(const uint8_t instrument);
    void Play_mode(const uint8_t instrument);
    void Noclick(const uint8_t instrument, const bool value);
    void Trim_step(void);
    void Pitch_voices_max(const uint8_t instrument);

    // INSTRUMENT_VCF
    void Instrument_VCF_page(const uint8_t session, const uint8_t instrument);
    void Show_VCF_gain(const uint8_t id_sound);
    void Show_VCF_solo(void);
    void Show_VCF_filter_type(const uint8_t instrument);
    void Show_VCF_cutoff(const uint8_t instrument);
    void Show_VCF_resonance(const uint8_t instrument);
    void Show_VCF_lfo_type(const uint8_t instrument);
    void Show_VCF_lfo_freq_time(const uint8_t instrument);
    void Show_VCF_lfo_index(const uint8_t instrument);
    void Show_VCF_pan(const uint8_t id_sound);

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
    void DS_page(const uint8_t instrument, const int recording);
    void DS_sampler_IO(void);
    void DS_bar(const uint8_t channel, int value);
    uint16_t DS_calc_bar_color(const float value);
    void DS_line_out(const bool visible);
    void DS_sampler_frame(const bool visible);
    void DS_sampler_txt(bool color);
    void DS_available_memory(void);
    void DS_raw_available_memory(void);
    void DS_hide_recording(void);
    void DS_advice_delete(const bool value);
    void DS_advice_no_conversion(const int DS_export, const bool value);
    void DS_conversion_options(const int file_L_RAW, const int file_R_RAW, const int DS_export);
    void DS_export_options(const int file_L_RAW, const int file_R_RAW, const int DS_export);
    void DS_Recording_description(const int recording, const bool led);
    void DS_recording_seconds(void);
    void DS_update_recording_seconds(const float value);
    void DS_volume(void);
    void DS_update_volume(bool adj = true);
    void DS_show_gain(void);
    void DS_menu(void);
    void DS_frame_menu(const uint8_t position);

    // SETUP
    void Settings_page(void);
    void Key_step(void);
    void First_octave(void);
    void Optimization(void);
    void Settings_frame(const int8_t value);
    void CC_page(void);
    void All_CC_Sound_gain(void);
    void Show_CC_Sound_gain(int value);
    void Show_CC_lowpass_filter(void);
    void Frame_CC_page_menu(const int value);
    // configura VFS
    void Show_VFS_packets(void);
    void Make_VFS_presentazione(void);
    void Make_VFS_assegnazioni(void);
    void Make_VFS_restart(void);
    void Make_VFS_no_spazio_per_sampler(void);
    // copia RAW files da SD
    void Import_raw_files_frame(const uint8_t value);
    void Confirm_config_import_popup(void);
    void SD_missing(uint16_t color);
    void Config_import_FILE_error_popup(void);
    void Config_import_REBOOT_popup(void);
    void Confirm_config_import_frame(const uint8_t value);
    void Confirm_config_export_popup(void);
    void Config_export_SD_error_popup(void);
    void Config_export_save_popup(void);
    void Confirm_config_reset_popup(void);
    void Config_reset_popup(void);
    void Copy_raw_files_SD_to_Flash_chip_titolo(void); // importa i file RAW dalla scheda SD
    void Copy_raw_files_SD_to_Flash_chip_attesa_SD(void); // attesa 10sec scheda SD
    void Copy_raw_files_SD_to_Flash_chip_directory_assente(void); // manca /LILLARAW
    void Copy_raw_files_SD_to_Flash_chip_consistenza_presente(const unsigned long SD_raw_volume, const int SD_raw_files, const int flash_raw_volume, const int flash_raw_files);
    void Copy_raw_files_SD_to_Flash_chip_ultimo_avviso(const float erasing_time_ms);
    void Copy_raw_files_SD_to_Flash_chip_avvio_copia(void); // inizia la cancellazione (erasing) della Flash memory
    void Copy_raw_files_SD_to_Flash_chip_percentuale_iniziale(void);
    void Copy_raw_files_SD_to_Flash_chip_avanzamento(const unsigned char barcount);
    void Copy_raw_files_SD_to_Flash_chip_sfondo_popup(void); // black panel
    void Copy_raw_files_SD_to_Flash_chip_sfondo_elenco(void);
    void Copy_raw_files_SD_to_Flash_chip_file_da_copiare(const int row, const char *filename, const unsigned long length);
    void Copy_raw_files_SD_to_Flash_chip_errore_flash_chip(void); // Flash memory error!
    void Copy_raw_files_SD_to_Flash_chip_errore_flash_full(void); // Flash memory full!
    void Copy_raw_files_SD_to_Flash_chip_copia_completata(void);
    void Copy_raw_files_SD_to_Flash_chip_file_copiato(const int row, const char *filename, const uint32_t filesize);

    // MIDI_LOOP
    void LOOP_page(void);
    void Loop_loop_id(void);
    void Loop_midi_loop_title(void);
    void Loop_track_data(const int track);
    void Loop_time_stretched(void);
    void Loop_rec_advice(const int track, const bool on);
    void Loop_led(const int Xled, const int Yled, const bool ONled);
    void Loop_led_metronomo(const int Xled, const int Yled, const bool ONled);
    void Loop_menu(void);
    void Frame_loop_menu(const uint8_t position, const bool fresh);
    void Delete_all_frame_loop_menu(void);

    // LIVE_SAMPLING
    void Confirm_EXIT_from_LS(void);
    void LS_page(void);
    void LS_Ring_Tape(void);
    void Show_LS_feedback(void);
    void Show_LS_X_step(void);
    void LS_dim(void);
    void LS_volume(void);
    void Show_LS_mode(void);
    void LS_loop_time(void);
    void LS_X_sample_delta(void);
    void Show_LS_menu(void);
    void Frame_LS_menu(const uint8_t position);
    void Delete_all_frame_LS_menu(void);
    void Show_LS_ring_tape_wave(const int id_sound);
    void LS_draw_XY_lines(void);
    uint16_t Get_LS_wave_color(const int point);
    void Update_LS_rec(void);

    // MIDI_MONITOR
    void Midi_monitor_page(void);
    void Midi_monitor_frame(void);
    void Midi_monitor_data(const uint8_t incoming_midi_channel, uint8_t incoming_midi_message, int8_t incoming_note_number, int8_t incoming_velocity, int32_t incoming_midi_value, int8_t incoming_number);

    // MIXER
    void MX_page(void);
    void MX_MAIN_volume(void);
    void MX_MONITOR_volume(void);
    void MX_source_values(const uint8_t source);
    void MX_source_values_write(const uint8_t source);
    void MX_source_values_edit(const uint8_t source);
    void MX_source_values_jump(const uint8_t old_source, const uint8_t new_source);
};