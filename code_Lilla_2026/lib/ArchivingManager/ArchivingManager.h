/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedDelay.h"
#include "SharedVFS.h"
#include <EEPROM.h>
#include <SD.h>
#include <FS.h>
#include "config.h"
#include "GlobalFRAM.h"

class ArchivingManager
{
private:
    #define AUDIO_BOARD_SDCARD 10

    // Locations of settings data in emulated EEPROM memory (dimension T4.1: 4284 bytes)
    static constexpr int EEPROM_BYTES = 4284;          // Teensy 4.1 EEPROM total bytes
    static constexpr int LOCATION_PATCH = 0;         // location in EEPROM of Patch[0]   --> 24 patches_number x 90 bytes = 2160 bytes
    static constexpr int LOCATION_RECORDING = 2160;    // location in EEPROM of Recording[0] --> 30 recordings x 4 bytes = 120 bytes
    static constexpr int LOCATION_SOUND = 2280;        // location in EEPROM of Sound[0]     --> 85 sounds x 22 bytes = 2200 bytes
    static constexpr int LOCATION_OPTIMIZATION = 4235; // extension and voices in Flash mode: 0 --> x3, 8 voices   1 --> x1.5, 12 voices
    static constexpr int LOCATION_FIRST_OCTAVE = 4236; // 1 byte   int8_t -2 --> 0
    static constexpr int LOCATION_DELAY = 4237;        // 17 bytes
    static constexpr int LOCATION_CC_SETTINGS = 4254;  // 31 bytes

    template <class T>
    int Eeprom_writeAnything(const size_t, const T &value);
    template <class T>
    int Eeprom_readAnything(const size_t, T &value); // il valore di ritorno e' value (che viene modificato)

    void Eeprom_write_uint8_t(size_t address, const uint8_t &value);
    void Eeprom_write_int8_t(size_t address, const int8_t &value);
    void Eeprom_read_uint8_t(size_t address, uint8_t &destination);
    void Eeprom_read_int8_t(size_t address, int8_t &destination);
    uint16_t Get_location_of_Sound(const uint8_t &sound_id);
    uint16_t GET_location_of_Patch(const uint8_t &patch_id);

    String Filename_patch_Delay(int patch_id);
    String Filename_Patch(int patch_id);
    String Filename_Sound(int patch_id, int instrument_id);

    void Copy_Delay_data_from_RAM_to_SD(File &file);
    void Copy_Delay_data_from_Eeprom_to_SD(File &file);
    void Copy_Delay_data_from_SD_to_Eeprom(File &file);
    void Print_Delay_data_reading_from_Eeprom(void);

    void Copy_Delay_data_from_SD_to_RAM(File &file);

    void Copy_Patch_from_RAM_to_SD(int patch_id, File &file);
    void Copy_Patch_from_SD_to_RAM(int patch_id, File &file);
    void Copy_Sound_from_RAM_to_SD(int patch_id, int instrument_id, File &file);
    void Copy_Sound_from_SD_to_RAM(int patch_id, int instrument_id, int sound_id, File &file);

    struct EEPROM_Instrument_filter_data_struct // 5 bytes
    {
        uint8_t data;           // bit0:use  bit1,2,3:modulation  bit4,5:filter_type
        uint8_t pivot;          // 0 --> 30 filter frequency/note frequency
        uint8_t resonance;      // 0 --> 40
        uint8_t index;          // 1 --> 20
        uint8_t frequency_time; // 0 --> 20
    };

    struct EEPROM_Instrument_struct // 11 bytes
    {
        bool used;
        uint8_t sound_id;
        uint8_t root_key;
        uint8_t from_note;
        uint8_t to_note;
        uint8_t info;                                // bit0: precedence, bit1: lock
        EEPROM_Instrument_filter_data_struct Filter; // 5 bytes
    };

    struct EEPROM_Patch_struct // 90 bytes
    {
        bool used; // 0:deleted  1:active
        uint8_t instruments;
        EEPROM_Instrument_struct Instrument[8]; // 8X11= 88 bytes
    } __attribute__((__packed__));              // https://cs50.stackexchange.com/questions/22297/i-am-getting-an-unexpected-sizeof-error
    EEPROM_Patch_struct EEPROM_Patch;
    static constexpr int SIZE_OF_EEPROM_PATCH = sizeof(EEPROM_Patch);

    bool Is_SD_inserted(void);

public:
    ArchivingManager(void) {}

    void Save_CC_lowpass_filter(int CC_lowpass_filter);
    void Read_CC_lowpass_filter(uint8_t &CC_lowpass_filter);
    void Save_optimization(uint8_t optimization);
    void Read_optimization(uint8_t &optimization);
    void Save_CC_Sound_gain(uint8_t instrument_id, uint8_t CC_Sg_instrument);
    void Read_CC_Sound_gain(uint8_t instrument_id, uint8_t &CC_Sg_instrument);
    void Copy_patch_Delay_data_from_Eeprom_to_Ram(Delay_data_struct &Delay_data);
    void Save_Delay_to_Eeprom(Delay_data_struct Delay_data);
    void Read_first_octave(int8_t &first_octave);
    void Save_first_octave(int8_t first_octave);
    void Save_Sound(int sound_id);
    void Read_Sound(int sound_id);
    void Save_Patch(int patch_id);
    void Read_Patch(int patch_id);
    void Save_DS_Recording(int recording);
    void Save_DS_Recording(const int &recording, const EEPROM_VFS_Recording &EEPROM_Rec_recording);
    void Read_DS_Recording(const int &recording, EEPROM_VFS_Recording &EEPROM_Rec_recording);
    void Save_setup_file(File &file);              // File e' l'oggetto file incluso in FS.h
    void Copy_setup_from_Eeprom_to_SD(File &file); // File e' l'oggetto file incluso in FS.h
    void Reset_EEPROM(void);
    void Print_EEPROM_content(void);
    
    void Copy_patch_Delay_data_from_Eeprom_to_SD(int patch_id);
    void Copy_patch_Delay_data_from_RAM_to_SD(int patch_id);
    bool Copy_patch_Delay_data_from_SD_to_RAM(int patch_id);
    bool Copy_patch_Delay_data_from_SD_to_Eeprom(int patch_id);
    void Print_patch_Delay_file_reading_from_SD(int patch_id);
    bool Delete_patch_Delay_data_in_SD(int patch_id);

    bool Copy_Patch_from_RAM_to_SD(int patch_id);
    bool Copy_Patch_from_SD_to_RAM(int patch_id);

    bool Copy_Sound_from_RAM_to_SD(int patch_id, int instrument_id);
    bool Copy_Sound_from_SD_to_RAM(int patch_id, int instrument_id, int sound_id);
    
    int GET_location_of_DS_Recording(const int &recording);
    bool Test_Fram(uint8_t writevalue);
};