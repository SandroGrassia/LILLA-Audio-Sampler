/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "EepromManager.h"

void EepromManager::Save_CC_lowpass_filter(uint8_t CC_lowpass_filter)
{
    Eeprom_writeAnything(LOCATION_CC_SETTINGS + 8, CC_lowpass_filter);
}

void EepromManager::Read_CC_lowpass_filter(uint8_t &CC_lowpass_filter)
{
    Eeprom_readAnything(LOCATION_CC_SETTINGS + 8, CC_lowpass_filter); // Eeprom_readAnything(LOCATION_CC_SETTINGS + 8, CC_lowpass_filter);
}

void EepromManager::Save_optimization(uint8_t optimization)
{
    Eeprom_writeAnything(LOCATION_OPTIMIZATION, optimization);
}

void EepromManager::Read_optimization(uint8_t &optimization)
{
    Eeprom_readAnything(LOCATION_OPTIMIZATION, optimization);
}

void EepromManager::Save_CC_Sound_gain(uint8_t instrument, uint8_t CC_Sg_instrument)
{
    Eeprom_writeAnything(LOCATION_CC_SETTINGS + instrument, CC_Sg_instrument);
}

void EepromManager::Read_CC_Sound_gain(uint8_t instrument, uint8_t &CC_Sg_instrument)
{
    Eeprom_readAnything(LOCATION_CC_SETTINGS + instrument, CC_Sg_instrument);
}

void EepromManager::Copy_patch_Delay_data_from_Eeprom_to_Ram(Delay_data_struct &delay_data)
{
    Eeprom_readAnything(LOCATION_DELAY, delay_data);
}

void EepromManager::Save_Delay_to_Eeprom(Delay_data_struct Delay_data)
{
    Eeprom_writeAnything(LOCATION_DELAY, Delay_data);
}

void EepromManager::Read_first_octave(int8_t &first_octave)
{
    Eeprom_readAnything(LOCATION_FIRST_OCTAVE, first_octave);
}

void EepromManager::Save_first_octave(int8_t first_octave)
{
    Eeprom_writeAnything(LOCATION_FIRST_OCTAVE, first_octave);
}

void EepromManager::Save_Sound(uint8_t sound_id)
{
    Eeprom_writeAnything(Get_location_of_Sound(sound_id), Sound[sound_id]);
}

void EepromManager::Read_Sound(uint8_t sound_id, Sound_struct &Sound_id_sound)
{
    Eeprom_readAnything(Get_location_of_Sound(sound_id), Sound_id_sound);
}

void EepromManager::Save_Patch(uint8_t patch_id, Patch_struct Patch_patch)
{
    EEPROM_Patch.used = Patch_patch.used;
    EEPROM_Patch.instruments = Patch_patch.instruments;
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        EEPROM_Patch.Instrument[instrument_local].used = Patch_patch.Instrument[instrument_local].used;
        EEPROM_Patch.Instrument[instrument_local].sound_id = Patch_patch.Instrument[instrument_local].sound_id;
        EEPROM_Patch.Instrument[instrument_local].root_key = Patch_patch.Instrument[instrument_local].root_key;

        EEPROM_Patch.Instrument[instrument_local].from_note = Patch_patch.Instrument[instrument_local].from_note;
        EEPROM_Patch.Instrument[instrument_local].to_note = Patch_patch.Instrument[instrument_local].to_note;
        EEPROM_Patch.Instrument[instrument_local].info = (Patch_patch.Instrument[instrument_local].precedence == true ? 0b1 : 0b0) + (Patch_patch.Instrument[instrument_local].lock == true ? 0b10 : 0b00); // bit0: precedence, bit1: lock

        EEPROM_Patch.Instrument[instrument_local].Filter.data = Patch_patch.Instrument[instrument_local].Filter.use + (Patch_patch.Instrument[instrument_local].Filter.modulation << 1) + (Patch_patch.Instrument[instrument_local].Filter.type << 4); // bit0: use  bit1,2,3: modulation  bit4,5: type
        EEPROM_Patch.Instrument[instrument_local].Filter.pivot = Patch_patch.Instrument[instrument_local].Filter.pivot;                                                                                                                                        // 0 --> 100 filter frequency/note frequency
        EEPROM_Patch.Instrument[instrument_local].Filter.resonance = Patch_patch.Instrument[instrument_local].Filter.resonance;                                                                                                                                // 0 --> 40
        EEPROM_Patch.Instrument[instrument_local].Filter.index = Patch_patch.Instrument[instrument_local].Filter.index;                                                                                                                                        // 1 --> 20 modulation_index
        EEPROM_Patch.Instrument[instrument_local].Filter.frequency_time = Patch_patch.Instrument[instrument_local].Filter.frequency_time;                                                                                                                      // 0 --> 20
    }
    Eeprom_writeAnything(GET_location_of_Patch(patch_id), EEPROM_Patch);
}

void EepromManager::Read_Patch(uint8_t patch_id)
{
    Eeprom_readAnything(GET_location_of_Patch(patch_id), EEPROM_Patch);

    Patch[patch_id].used = EEPROM_Patch.used;
    Patch[patch_id].instruments = EEPROM_Patch.instruments;
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        Patch[patch_id].Instrument[instrument_local].used = EEPROM_Patch.Instrument[instrument_local].used;
        Patch[patch_id].Instrument[instrument_local].sound_id = EEPROM_Patch.Instrument[instrument_local].sound_id;

        // Serial.println("Patch[patch_id].Instrument[instrument].sound_id"); Serial.println(EEPROM_Patch.Instrument[instrument].sound_id);

        Patch[patch_id].Instrument[instrument_local].root_key = EEPROM_Patch.Instrument[instrument_local].root_key;

        Patch[patch_id].Instrument[instrument_local].from_note = EEPROM_Patch.Instrument[instrument_local].from_note;
        Patch[patch_id].Instrument[instrument_local].to_note = EEPROM_Patch.Instrument[instrument_local].to_note;
        Patch[patch_id].Instrument[instrument_local].precedence = bitRead(EEPROM_Patch.Instrument[instrument_local].info, 0);
        Patch[patch_id].Instrument[instrument_local].lock = bitRead(EEPROM_Patch.Instrument[instrument_local].info, 1);

        Patch[patch_id].Instrument[instrument_local].Filter.use = bitRead(EEPROM_Patch.Instrument[instrument_local].Filter.data, 0);                                                                                                                                                            // yes/no
        Patch[patch_id].Instrument[instrument_local].Filter.type = bitRead(EEPROM_Patch.Instrument[instrument_local].Filter.data, 4) + 2 * bitRead(EEPROM_Patch.Instrument[instrument_local].Filter.data, 5);                                                                                 // bit4,5:filter_type
        Patch[patch_id].Instrument[instrument_local].Filter.pivot = EEPROM_Patch.Instrument[instrument_local].Filter.pivot;                                                                                                                                                                     // 0 --> 100 filter frequency/note frequency
        Patch[patch_id].Instrument[instrument_local].Filter.resonance = EEPROM_Patch.Instrument[instrument_local].Filter.resonance;                                                                                                                                                             // 0 --> 40
        Patch[patch_id].Instrument[instrument_local].Filter.modulation = bitRead(EEPROM_Patch.Instrument[instrument_local].Filter.data, 1) + 2 * bitRead(EEPROM_Patch.Instrument[instrument_local].Filter.data, 2) + 4 * bitRead(EEPROM_Patch.Instrument[instrument_local].Filter.data, 3); // bit1,2,3:modulation
        Patch[patch_id].Instrument[instrument_local].Filter.index = EEPROM_Patch.Instrument[instrument_local].Filter.index;                                                                                                                                                                     // 1 --> 20 modulation_index
        Patch[patch_id].Instrument[instrument_local].Filter.frequency_time = EEPROM_Patch.Instrument[instrument_local].Filter.frequency_time;                                                                                                                                                   // 0 --> 20
    }
}

void EepromManager::Save_DS_Recording(int recording)
{
    Serial.println(F("*** Save_DS_Recording(int recording) ***"));
    if (recording >= 0 && recording < RECORDINGS)
    {
        EEPROM_Recording[recording].first_packet = Recording[recording].first_packet;
        EEPROM_Recording[recording].packets = Recording[recording].packets;
        bitWrite(EEPROM_Recording[recording].info, 1, (Recording[recording].stereo ? 1 : 0));
        bitWrite(EEPROM_Recording[recording].info, 0, (Recording[recording].consistent ? 1 : 0));
        Save_DS_Recording(recording, EEPROM_Recording[recording]);
    }
    else
        Serial.println(F("***** WARNING! --> Save_DS_Recording: recording out of range"));
}

void EepromManager::Save_DS_Recording(const int &recording, const EEPROM_VFS_Recording &EEPROM_Rec_recording)
{
    Serial.print("EepromManager - GET_location_of_DS_Recording(recording): ");
    Serial.println(GET_location_of_DS_Recording(recording));

    Eeprom_writeAnything(GET_location_of_DS_Recording(recording), EEPROM_Rec_recording);
}

void EepromManager::Read_DS_Recording(const int &recording, EEPROM_VFS_Recording &EEPROM_Rec_recording)
{
    Eeprom_readAnything(GET_location_of_DS_Recording(recording), EEPROM_Rec_recording);
}

void EepromManager::Save_setup_file(File &file)
{
    uint8_t x;
    uint16_t location = 0;
    String x_txt;
    while (file.available())
    {
        // leggi il byte x
        x_txt = file.readStringUntil('\n'); // restituisce String - es: x_txt = "230" ossia i char "2" "3" "0" "\n"
        x = x_txt.toInt();                  // conversione da String a uint8_t es: x = 230

        // trascrivi x
        EEPROM.write(location, x);
        delayMicroseconds(100); // important not to block the process
        location++;
    }
}

void EepromManager::Copy_setup_from_Eeprom_to_SD(File &file)
{
    uint8_t x;
    for (auto location = 0; location < EEPROM_BYTES; ++location)
    {
        // leggi il byte x
        x = EEPROM.read(location); // restituisce uint8_t es: x = 230
        delayMicroseconds(100);    // important not to block the process

        // trascrivi x
        file.println(String(x)); // es: String(x) = "230" ossia si salvano i tre digit con a capo 2 3 0 \n
    }
}

void EepromManager::Reset_EEPROM(void)
{
    for (auto location = 0; location < EEPROM_BYTES; ++location)
    {
        EEPROM.write(location, 0);
    }
}

void EepromManager::Print_EEPROM_content(void)
{
    int location;

    for (location = 0; location < LOCATION_RECORDING; ++location)
    {
        if (location % 90 == 0)
        {
            Serial.println();
            Serial.print("Patch ");
            Serial.println(location / 90);
        }

        Serial.print(F("location "));
        Serial.print(location);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(location));
        delay(1);
    }
    Serial.println();

    for (location = LOCATION_RECORDING; location < LOCATION_SOUND; ++location)
    {
        if ((location - LOCATION_RECORDING) % SIZE_OF_EEPROM_RECORDING == 0)
        {
            Serial.println();
        }

        Serial.print(F("Recording - location "));
        Serial.print(location);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(location));
    }
    Serial.println();

    for (location = LOCATION_SOUND; location < LOCATION_OPTIMIZATION; ++location)
    {
        if ((location - LOCATION_SOUND) % SIZE_OF_SOUND == 0)
        {
            Serial.println();
            Serial.print("Sound ");
            Serial.println((location - LOCATION_SOUND) / SIZE_OF_SOUND);
        }

        Serial.print(F("location "));
        Serial.print(location);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(location));
    }
    Serial.println();

    Serial.print(F("Optimization - location "));
    Serial.print(LOCATION_OPTIMIZATION);
    Serial.print(" value: ");
    Serial.println(EEPROM.read(LOCATION_OPTIMIZATION));
    Serial.println();

    Serial.print(F("First Octave - location "));
    Serial.print(LOCATION_FIRST_OCTAVE);
    Serial.print(" value: ");
    Serial.println(EEPROM.read(LOCATION_FIRST_OCTAVE));
    Serial.println();

    for (location = LOCATION_DELAY; location < LOCATION_CC_SETTINGS; ++location)
    {
        Serial.print(F("Delay - location "));
        Serial.print(location);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(location));
    }
    Serial.println();

    for (location = LOCATION_CC_SETTINGS; location < EEPROM_BYTES; ++location)
    {
        Serial.print(F("CC Settings - location "));
        Serial.print(location);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(location));
    }
    Serial.println();
}

int EepromManager::GET_location_of_DS_Recording(const int &recording)
{
    return (LOCATION_RECORDING + (recording * SIZE_OF_EEPROM_RECORDING));
}

uint16_t EepromManager::GET_location_of_Patch(const uint8_t &patch_id)
{
    return LOCATION_PATCH + (patch_id * size_of_EEPROM_Patch);
}

uint16_t EepromManager::Get_location_of_Sound(const uint8_t &sound_id)
{
    // Serial.print("GET_location_of_sound: ");
    // Serial.println(LOCATION_SOUND + (sound_id * SIZE_OF_SOUND));
    return (LOCATION_SOUND + (sound_id * SIZE_OF_SOUND));
}

template <class T>
int EepromManager::Eeprom_writeAnything(int address, const T &source)
{
    if (address >= 0)
    {
        const byte *p = (const byte *)(const void *)&source;
        unsigned int i;
        for (i = 0; i < sizeof(source); ++i)
        {
            EEPROM.write(address++, *p++);
        }

        return i;
    }
    else
    {
        Serial.println(F("Eeprom_writeAnything error!!"));
        return -1;
    }
}

template <class T>
int EepromManager::Eeprom_readAnything(int address, T &destination)
{
    if (address >= 0)
    {
        byte *p = (byte *)(void *)&destination;
        unsigned int i;
        // Serial.print("Read - sizeof(destination): "); Serial.println(sizeof(destination));
        for (i = 0; i < sizeof(destination); ++i)
        {
            *p++ = EEPROM.read(address++);
        }
        return i;
    }
    else
    {
        Serial.println(F("Eeprom_writeAnything error!!"));
        return -1;
    }
}

// Associazione patch_id - delay
String EepromManager::Filename_patch_delay(int patch_id)
{
    String filename = String(patch_id);
    return String(filename + ".delay");
}

void EepromManager::Copy_Delay_data_from_RAM_to_SD(File &file) // private
{
    const byte *data = (const byte *)(const void *)&Delay_data;
    for (auto i = 0; i < DELAY_DATA_DIM; ++i)
    {
        file.println(*(data + i));
    }
}

void EepromManager::Copy_Delay_data_from_Eeprom_to_SD(File &file) // private
{
    uint8_t x;
    for (auto location = LOCATION_DELAY; location < LOCATION_DELAY + DELAY_DATA_DIM; ++location)
    {
        x = EEPROM.read(location);
        delayMicroseconds(100); // important not to block the process
        file.println(String(x));
    }
    if (false)
    {
        Serial.println(F("EepromManager::Copy_Delay_data_from_Eeprom_to_SD(File &file) - Read EEPROM data:"));
        Serial.println(file.name());
        Print_Delay_data_on_EEPROM();
    }
}

void EepromManager::Copy_Delay_data_from_SD_to_Eeprom(File &file) // private
{
    uint8_t x;
    uint16_t location = LOCATION_DELAY;
    String x_txt;

    while (file.available())
    {
        x_txt = file.readStringUntil('\n'); // restituisce String - es: x_txt = "230" ossia i char "2" "3" "0" "\n"
        x = x_txt.toInt();                  // toInt() conversione da String a long es: x = 230
        EEPROM.write(location, x);
        delayMicroseconds(100); // important not to block the process
        location++;
    }
    if (true)
    {
        Serial.println(F("EepromManager::Copy_Delay_data_from_SD_to_Eeprom(File &file)- Read EEPROM data:"));
        Print_Delay_data_on_EEPROM();
    }
}

FLASHMEM
void EepromManager::Print_Delay_data_on_EEPROM()
{
    uint8_t data_LSB;
    uint8_t data_MSB;
    int16_t result_int;
    uint16_t result_uint;
    auto location = 0;

    Serial.println();
    Serial.println(F("EepromManager::Print_Delay_data_on_EEPROM()"));

    Serial.print("uint16_t samples: ");
    data_LSB = EEPROM.read(LOCATION_DELAY + location++);
    data_MSB = EEPROM.read(LOCATION_DELAY + location++);
    result_uint = data_MSB << 8 | data_LSB;
    Serial.println(result_uint);

    Serial.print("int16_t samples_LR: ");
    data_LSB = EEPROM.read(LOCATION_DELAY + location++);
    data_MSB = EEPROM.read(LOCATION_DELAY + location++);
    result_int = data_MSB << 8 | data_LSB;
    Serial.println(result_int);

    Serial.print("instrument_route: ");
    Serial.println(EEPROM.read(LOCATION_DELAY + location++));

    Serial.print("modulation: ");
    Serial.println(EEPROM.read(LOCATION_DELAY + location++));

    Serial.print("depth: ");
    Serial.println(EEPROM.read(LOCATION_DELAY + location++));

    Serial.print("frequency: ");
    Serial.println(EEPROM.read(LOCATION_DELAY + location++));

    Serial.print("uint16_t phase_LR: ");
    data_LSB = EEPROM.read(LOCATION_DELAY + location++);
    data_MSB = EEPROM.read(LOCATION_DELAY + location++);
    result_uint = data_MSB << 8 | data_LSB;
    Serial.println(result_uint);

    Serial.print("uint16_t loop_gain: ");
    data_LSB = EEPROM.read(LOCATION_DELAY + location++);
    data_MSB = EEPROM.read(LOCATION_DELAY + location++);
    result_uint = data_MSB << 8 | data_LSB;
    Serial.println(result_uint);
    Serial.println();
}

void EepromManager::Delete_Delay_data_in_SD(int patch_id)
{
    String filename = Filename_patch_delay(patch_id);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *full_path_ = &full_path[0];
        if (SD.exists(full_path_))
        {
            SD.remove(full_path_);

            Serial.print(F("EepromManager::Delete_Delay_data_in_SD - existing "));
            Serial.print(full_path);
            Serial.println(" has been deleted.");
        }
    }
    else
    {
        Serial.println(F("EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}

void EepromManager::Copy_patch_Delay_data_from_RAM_to_SD(int patch_id) // public
{
    String filename = Filename_patch_delay(patch_id);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *_full_path = &full_path[0];
        if (SD.exists(_full_path))
        {
            SD.remove(_full_path);

            Serial.print(F("EepromManager::Copy_patch_Delay_data_from_Eeprom_to_SD - existing "));
            Serial.print(full_path);
            Serial.println(" has been deleted.");
        }

        Serial.print(F("EepromManager::Copy_patch_Delay_data_from_Eeprom_to_SD - Delay_data will be saved in "));
        Serial.print(full_path);
        Serial.println(" in SD.");

        File file = SD.open(_full_path, FILE_WRITE); // creazione del file vuoto
        Copy_Delay_data_from_RAM_to_SD(file);
    }
    else
    {
        Serial.println(F("EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}

void EepromManager::Copy_patch_Delay_data_from_Eeprom_to_SD(int patch_id) // public
{
    String filename = Filename_patch_delay(patch_id);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *_full_path = &full_path[0];
        if (SD.exists(_full_path))
        {
            SD.remove(_full_path);

            Serial.print(F("EepromManager::Copy_patch_Delay_data_from_Eeprom_to_SD - existing "));
            Serial.print(full_path);
            Serial.println(" has been deleted.");
        }

        File file = SD.open(_full_path, FILE_WRITE); // creazione del file
        Copy_Delay_data_from_Eeprom_to_SD(file);

        Serial.print(F("EepromManager::Copy_patch_Delay_data_from_Eeprom_to_SD - delay data have been saved in "));
        Serial.print(full_path);
        Serial.println(" on SD.");
    }
    else
    {
        Serial.println(F("EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}
bool EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom(int patch_id) // public
{
    String filename = Filename_patch_delay(patch_id);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *_full_path = &full_path[0];
        if (SD.exists(_full_path))
        {
            Serial.print(F("EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom - delay data file "));
            Serial.print(full_path);
            Serial.println(F(" found; now starts data import."));

            File file = SD.open(_full_path);
            Copy_Delay_data_from_SD_to_Eeprom(file);
            return true;
        }
        else
        {
            Serial.println(F("EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom - delay data not found on SD!"));
            return false;
        }
    }
    else
    {
        Serial.println(F("EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom - SD not present!"));
        return false;
    }
}
void EepromManager::Print_Delay_file_SD(int patch_id) // public
{
    String filename = Filename_patch_delay(patch_id);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *_full_path = &full_path[0];
        if (SD.exists(_full_path))
        {
            Serial.print("EepromManager::Print_Delay_file_SD - Reading SD Delay file ");
            Serial.println(full_path);

            File file = SD.open(_full_path);
            uint8_t x;
            String x_txt;
            int i = 0;

            while (file.available())
            {
                x_txt = file.readStringUntil('\n'); // restituisce String - es: x_txt = "230" ossia i char "2" "3" "0" "\n"
                x = x_txt.toInt();                  // conversione da String a uint8_t es: x = 230
                delayMicroseconds(100);             // important not to block the process
                Serial.print("byte ");
                Serial.print(i);
                Serial.print(" value: ");
                Serial.println(x);
                ++i;
            }
        }
    }
    else
    {
        Serial.println(F("EepromManager::Copy_patch_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}