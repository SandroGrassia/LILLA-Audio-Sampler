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

void EepromManager::Copy_session_delay_data_from_Eeprom_to_Ram(Delay_data_struct &delay_data)
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

void EepromManager::Save_Sound(uint8_t id_sound)
{
    Eeprom_writeAnything(Get_location_of_Sound(id_sound), Sound[id_sound]);
}

void EepromManager::Read_Sound(uint8_t id_sound, Sound_struct &Sound_id_sound)
{
    Eeprom_readAnything(Get_location_of_Sound(id_sound), Sound_id_sound);
}

void EepromManager::Save_Session(uint8_t session, Session_struct Session_session)
{
    EEPROM_Session.used = Session_session.used;
    EEPROM_Session.instruments = Session_session.instruments;
    for (int instrument = 0; instrument < INSTRUMENTS_MAX; instrument++)
    {
        EEPROM_Session.Instrument[instrument].used = Session_session.Instrument[instrument].used;
        EEPROM_Session.Instrument[instrument].id_sound = Session_session.Instrument[instrument].id_sound;
        EEPROM_Session.Instrument[instrument].root_key = Session_session.Instrument[instrument].root_key;

        EEPROM_Session.Instrument[instrument].from_note = Session_session.Instrument[instrument].from_note;
        EEPROM_Session.Instrument[instrument].to_note = Session_session.Instrument[instrument].to_note;
        EEPROM_Session.Instrument[instrument].info = (Session_session.Instrument[instrument].precedence == true ? 0b1 : 0b0) + (Session_session.Instrument[instrument].lock == true ? 0b10 : 0b00); // bit0: precedence, bit1: lock

        EEPROM_Session.Instrument[instrument].Filter.data = Session_session.Instrument[instrument].Filter.use + (Session_session.Instrument[instrument].Filter.modulation << 1) + (Session_session.Instrument[instrument].Filter.type << 4); // bit0: use  bit1,2,3: modulation  bit4,5: type
        EEPROM_Session.Instrument[instrument].Filter.pivot = Session_session.Instrument[instrument].Filter.pivot;                                                                                                                            // 0 --> 100 filter frequency/note frequency
        EEPROM_Session.Instrument[instrument].Filter.resonance = Session_session.Instrument[instrument].Filter.resonance;                                                                                                                    // 0 --> 40
        EEPROM_Session.Instrument[instrument].Filter.index = Session_session.Instrument[instrument].Filter.index;                                                                                                                            // 1 --> 20 modulation_index
        EEPROM_Session.Instrument[instrument].Filter.frequency_time = Session_session.Instrument[instrument].Filter.frequency_time;                                                                                                          // 0 --> 20
    }
    Eeprom_writeAnything(GET_location_of_Session(session), EEPROM_Session);
}

void EepromManager::Read_Session(uint8_t session)
{
    Eeprom_readAnything(GET_location_of_Session(session), EEPROM_Session);

    Session[session].used = EEPROM_Session.used;
    Session[session].instruments = EEPROM_Session.instruments;
    for (int instrument = 0; instrument < INSTRUMENTS_MAX; instrument++)
    {
        Session[session].Instrument[instrument].used = EEPROM_Session.Instrument[instrument].used;
        Session[session].Instrument[instrument].id_sound = EEPROM_Session.Instrument[instrument].id_sound;

        // Serial.println("Session[session].Instrument[instrument].id_sound"); Serial.println(EEPROM_Session.Instrument[instrument].id_sound);

        Session[session].Instrument[instrument].root_key = EEPROM_Session.Instrument[instrument].root_key;

        Session[session].Instrument[instrument].from_note = EEPROM_Session.Instrument[instrument].from_note;
        Session[session].Instrument[instrument].to_note = EEPROM_Session.Instrument[instrument].to_note;
        Session[session].Instrument[instrument].precedence = bitRead(EEPROM_Session.Instrument[instrument].info, 0);
        Session[session].Instrument[instrument].lock = bitRead(EEPROM_Session.Instrument[instrument].info, 1);

        Session[session].Instrument[instrument].Filter.use = bitRead(EEPROM_Session.Instrument[instrument].Filter.data, 0);                                                                                                                                                // yes/no
        Session[session].Instrument[instrument].Filter.type = bitRead(EEPROM_Session.Instrument[instrument].Filter.data, 4) + 2 * bitRead(EEPROM_Session.Instrument[instrument].Filter.data, 5);                                                                           // bit4,5:filter_type
        Session[session].Instrument[instrument].Filter.pivot = EEPROM_Session.Instrument[instrument].Filter.pivot;                                                                                                                                                         // 0 --> 100 filter frequency/note frequency
        Session[session].Instrument[instrument].Filter.resonance = EEPROM_Session.Instrument[instrument].Filter.resonance;                                                                                                                                                 // 0 --> 40
        Session[session].Instrument[instrument].Filter.modulation = bitRead(EEPROM_Session.Instrument[instrument].Filter.data, 1) + 2 * bitRead(EEPROM_Session.Instrument[instrument].Filter.data, 2) + 4 * bitRead(EEPROM_Session.Instrument[instrument].Filter.data, 3); // bit1,2,3:modulation
        Session[session].Instrument[instrument].Filter.index = EEPROM_Session.Instrument[instrument].Filter.index;                                                                                                                                                         // 1 --> 20 modulation_index
        Session[session].Instrument[instrument].Filter.frequency_time = EEPROM_Session.Instrument[instrument].Filter.frequency_time;                                                                                                                                       // 0 --> 20
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
        x_txt = file.readStringUntil('\n'); // restituisce String - es: x_txt = "230" ossia i char "2" "3" "0" "\n"
        x = x_txt.toInt();                  // conversione da String a uint8_t es: x = 230
        EEPROM.write(location, x);
        delayMicroseconds(100); // important not to block the process
        location++;
    }

    if (false)
    {
        int i = 0;
        Serial.println(F("EepromManager::Save_setup_file(File &file) - Read EEPROM locations:"));
        Serial.print("(2 bytes) samples: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) samples_LR: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("instrument_route: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("modulation: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("depth: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("frequency: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) phase_LR: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) loop_gain: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println();
    }
}

void EepromManager::Copy_setup_from_Eeprom_to_SD(File &file)
{
    uint8_t x;
    for (uint16_t location = 0; location < EEPROM_BYTES; ++location)
    {
        x = EEPROM.read(location); // restituisce uint8_t es: x = 230
        delayMicroseconds(100);    // important not to block the process
        file.println(String(x));   // es: String(x) = "230" ossia si salvano i tre digit con a capo 2 3 0 \n
    }

    if (false)
    {
        int i = 0;
        Serial.println(F("EepromManager::Copy_setup_from_Eeprom_to_SD(File &file) - Read EEPROM locations:"));
        Serial.print("(2 bytes) samples: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) samples_LR: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("instrument_route: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("modulation: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("depth: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("frequency: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) phase_LR: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) loop_gain: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println();
    }
}

void EepromManager::Reset_EEPROM(void)
{
    for (uint16_t location = 0; location < EEPROM_BYTES; ++location)
        EEPROM.write(location, 0);
}

void EepromManager::Print_EEPROM_content(void)
{
    for (int i = 0; i < LOCATION_RECORDING; i++)
    {
        if (i % 90 == 0)
        {
            Serial.println();
            Serial.print("Session ");
            Serial.println(i / 90);
        }

        Serial.print(F("location "));
        Serial.print(i);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(i));
        delay(1);
    }
    Serial.println();

    for (int i = LOCATION_RECORDING; i < LOCATION_SOUND; i++)
    {
        if ((i - LOCATION_RECORDING) % SIZE_OF_EEPROM_RECORDING == 0)
            Serial.println();

        Serial.print(F("Recording - location "));
        Serial.print(i);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(i));
    }
    Serial.println();

    for (int i = LOCATION_SOUND; i < LOCATION_OPTIMIZATION; i++)
    {
        if ((i - LOCATION_SOUND) % SIZE_OF_SOUND == 0)
        {
            Serial.println();
            Serial.print("Sound ");
            Serial.println((i - LOCATION_SOUND) / SIZE_OF_SOUND);
        }

        Serial.print(F("location "));
        Serial.print(i);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(i));
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

    for (int i = LOCATION_DELAY; i < LOCATION_CC_SETTINGS; i++)
    {
        Serial.print(F("Delay - location "));
        Serial.print(i);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(i));
    }
    Serial.println();

    for (int i = LOCATION_CC_SETTINGS; i < EEPROM_BYTES; i++)
    {
        Serial.print(F("CC Settings - location "));
        Serial.print(i);
        Serial.print(" value: ");
        Serial.println(EEPROM.read(i));
    }
    Serial.println();
}

int EepromManager::GET_location_of_DS_Recording(const int &recording)
{
    return (LOCATION_RECORDING + (recording * SIZE_OF_EEPROM_RECORDING));
}

uint16_t EepromManager::GET_location_of_Session(const uint8_t &session)
{
    return LOCATION_SESSION + (session * size_of_EEPROM_Session);
}

uint16_t EepromManager::Get_location_of_Sound(const uint8_t &id_sound)
{
    // Serial.print("GET_location_of_sound: ");
    // Serial.println(LOCATION_SOUND + (id_sound * SIZE_OF_SOUND));
    return (LOCATION_SOUND + (id_sound * SIZE_OF_SOUND));
}

template <class T>
int EepromManager::Eeprom_writeAnything(int address, const T &source)
{
    if (address >= 0)
    {
        const byte *p = (const byte *)(const void *)&source;
        unsigned int i;
        for (i = 0; i < sizeof(source); i++)
            EEPROM.write(address++, *p++);
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
        for (i = 0; i < sizeof(destination); i++)
            *p++ = EEPROM.read(address++);
        return i;
    }
    else
    {
        Serial.println(F("Eeprom_writeAnything error!!"));
        return -1;
    }
}

// Associazione session - delay
String EepromManager::Filename_session_delay(int session)
{
    String filename = String(session);
    return String(filename + ".delay");
}

void EepromManager::Copy_Delay_data_from_RAM_to_SD(File &file) // private
{
    const byte *data = (const byte *)(const void *)&Delay_data;
    for (int i = 0; i < DELAY_DATA_DIM; ++i)
    {
        file.println(*(data + i));
    }

    if (true)
        Print_Delay_data(data);
}

void EepromManager::Copy_Delay_data_from_Eeprom_to_SD(File &file) // private
{
    uint8_t x;
    for (int location = LOCATION_DELAY; location < LOCATION_DELAY + DELAY_DATA_DIM; ++location)
    {
        x = EEPROM.read(location);
        delayMicroseconds(100); // important not to block the process
        file.println(String(x));

        if (false)
        {
            int i = 0;
            Serial.println(F("EepromManager::Copy_Delay_data_from_Eeprom_to_SD(File &file) - Read EEPROM locations:"));
            Serial.print("(2 bytes) samples: ");
            Serial.print(EEPROM.read(LOCATION_DELAY + i++));
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.print("(2 bytes) samples_LR: ");
            Serial.print(EEPROM.read(LOCATION_DELAY + i++));
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.print("instrument_route: ");
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.print("modulation: ");
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.print("depth: ");
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.print("frequency: ");
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.print("(2 bytes) phase_LR: ");
            Serial.print(EEPROM.read(LOCATION_DELAY + i++));
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.print("(2 bytes) loop_gain: ");
            Serial.print(EEPROM.read(LOCATION_DELAY + i++));
            Serial.println(EEPROM.read(LOCATION_DELAY + i++));
            Serial.println();
        }
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
    if (false)
    {
        int i = 0;
        Serial.println(F("EepromManager::Copy_Delay_data_from_SD_to_Eeprom(File &file)- Read EEPROM locations:"));
        Serial.print("(2 bytes) samples: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) samples_LR: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("instrument_route: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("modulation: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("depth: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("frequency: ");
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) phase_LR: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.print("(2 bytes) loop_gain: ");
        Serial.print(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println(EEPROM.read(LOCATION_DELAY + i++));
        Serial.println();
    }
}

void EepromManager::Delete_Delay_data_in_SD(int session)
{
    String filename = Filename_session_delay(session);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *full_path_ = &full_path[0];
        if (SD.exists(full_path_))
        {
            SD.remove(full_path_);

            Serial.print(F("EepromManager::Copy_session_Delay_data_from_Eeprom_to_SD - existing "));
            Serial.print(full_path);
            Serial.println(" has been deleted.");
        }
    }
    else
    {
        Serial.println(F("EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}

void EepromManager::Copy_session_Delay_data_from_RAM_to_SD(int session) // public
{
    String filename = Filename_session_delay(session);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *_full_path = &full_path[0];
        if (SD.exists(_full_path))
        {
            SD.remove(_full_path);

            Serial.print(F("EepromManager::Copy_session_Delay_data_from_Eeprom_to_SD - existing "));
            Serial.print(full_path);
            Serial.println(" has been deleted.");
        }

        Serial.print(F("EepromManager::Copy_session_Delay_data_from_Eeprom_to_SD - Delay_data will be saved in "));
        Serial.print(full_path);
        Serial.println(" in SD.");

        File file = SD.open(_full_path, FILE_WRITE); // creazione del file vuoto
        Copy_Delay_data_from_RAM_to_SD(file);
    }
    else
    {
        Serial.println(F("EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}

void EepromManager::Copy_session_Delay_data_from_Eeprom_to_SD(int session) // public
{
    String filename = Filename_session_delay(session);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *_full_path = &full_path[0];
        if (SD.exists(_full_path))
        {
            SD.remove(_full_path);

            Serial.print(F("EepromManager::Copy_session_Delay_data_from_Eeprom_to_SD - existing "));
            Serial.print(full_path);
            Serial.println(" has been deleted.");
        }

        File file = SD.open(_full_path, FILE_WRITE); // creazione del file
        Copy_Delay_data_from_Eeprom_to_SD(file);

        Serial.print(F("EepromManager::Copy_session_Delay_data_from_Eeprom_to_SD - delay data have been saved in "));
        Serial.print(full_path);
        Serial.println(" on SD.");
    }
    else
    {
        Serial.println(F("EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}
bool EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom(int session) // public
{
    String filename = Filename_session_delay(session);
    String full_path = String("/LILLADELAY/" + filename);
    if (SD.begin(BUILTIN_SDCARD))
    {
        const char *_full_path = &full_path[0];
        if (SD.exists(_full_path))
        {
            Serial.print(F("EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom - delay data file "));
            Serial.print(full_path);
            Serial.println(F(" found; now starts data import."));

            File file = SD.open(_full_path);
            Copy_Delay_data_from_SD_to_Eeprom(file);
            return true;
        }
        else
        {
            Serial.println(F("EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom - delay data not found on SD!"));
            return false;
        }
    }
    else
    {
        Serial.println(F("EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom - SD not present!"));
        return false;
    }
}
void EepromManager::Print_Delay_file_SD(int session) // public
{
    String filename = Filename_session_delay(session);
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
        Serial.println(F("EepromManager::Copy_session_Delay_data_from_SD_to_Eeprom - SD not present!"));
    }
}