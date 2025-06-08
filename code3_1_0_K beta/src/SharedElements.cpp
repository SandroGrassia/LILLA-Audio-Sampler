/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedElements.h"

// SETUP
int key_step;
uint8_t optimization;
int8_t first_octave;

// FILES
const char name_file[RAW_FILES][10] =
    {
        // Normal files
        "0.raw", "1.raw", "2.raw", "3.raw", "4.raw", "5.raw", "6.raw", "7.raw", "8.raw", "9.raw", "10.raw", "11.raw", "12.raw", "13.raw", "14.raw", "15.raw", "16.raw", "17.raw", "18.raw", "19.raw",
        "20.raw", "21.raw", "22.raw", "23.raw", "24.raw", "25.raw", "26.raw", "27.raw", "28.raw", "29.raw", "30.raw", "31.raw", "32.raw", "33.raw", "34.raw", "35.raw", "36.raw", "37.raw", "38.raw", "39.raw",
        "40.raw", "41.raw", "42.raw", "43.raw", "44.raw", "45.raw", "46.raw", "47.raw", "48.raw", "49.raw", "50.raw", "51.raw", "52.raw", "53.raw", "54.raw", "55.raw", "56.raw", "57.raw", "58.raw", "59.raw",
        "60.raw", "61.raw", "62.raw", "63.raw", "64.raw", "65.raw", "66.raw", "67.raw", "68.raw", "69.raw", "70.raw", "71.raw", "72.raw", "73.raw", "74.raw", "75.raw", "76.raw", "77.raw", "78.raw", "79.raw",
        "80.raw", "81.raw", "82.raw", "83.raw", "84.raw", "85.raw", "86.raw", "87.raw", "88.raw", "89.raw", "90.raw", "91.raw", "92.raw", "93.raw", "94.raw", "95.raw", "96.raw", "97.raw", "98.raw", "99.raw",
        "100.raw", "101.raw", "102.raw", "103.raw", "104.raw", "105.raw", "106.raw", "107.raw", "108.raw", "109.raw", "110.raw", "111.raw", "112.raw", "113.raw", "114.raw", "115.raw", "116.raw", "117.raw", "118.raw", "119.raw",
        "120.raw", "121.raw", "122.raw", "123.raw", "124.raw", "125.raw", "126.raw", "127.raw", "128.raw", "129.raw", "130.raw", "131.raw", "132.raw", "133.raw", "134.raw", "135.raw", "136.raw", "137.raw", "138.raw", "139.raw",
        "140.raw", "141.raw", "142.raw", "143.raw", "144.raw", "145.raw", "146.raw", "147.raw", "148.raw", "149.raw", "150.raw", "151.raw", "152.raw", "153.raw", "154.raw", "155.raw", "156.raw", "157.raw", "158.raw", "159.raw",
        "160.raw", "161.raw", "162.raw", "163.raw", "164.raw", "165.raw", "166.raw", "167.raw", "168.raw", "169.raw", "170.raw", "171.raw", "172.raw", "173.raw", "174.raw", "175.raw", "176.raw", "177.raw", "178.raw", "179.raw",
        "180.raw", "181.raw", "182.raw", "183.raw", "184.raw", "185.raw", "186.raw", "187.raw", "188.raw", "189.raw", "190.raw", "191.raw", "192.raw", "193.raw", "194.raw", "195.raw", "196.raw", "197.raw", "198.raw", "199.raw",
        "200.raw", "201.raw", "202.raw", "203.raw", "204.raw", "205.raw", "206.raw", "207.raw", "208.raw", "209.raw", "210.raw", "211.raw", "212.raw", "213.raw", "214.raw", "215.raw", "216.raw", "217.raw", "218.raw", "219.raw",
        "220.raw", "221.raw", "222.raw", "223.raw", "224.raw", "225.raw", "226.raw", "227.raw", "228.raw", "229.raw", "230.raw", "231.raw", "232.raw", "233.raw", "234.raw", "235.raw", "236.raw", "237.raw", "238.raw", "239.raw",
        "240.raw", "241.raw", "242.raw", "243.raw", "244.raw", "245.raw", "246.raw", "247.raw", "248.raw", "249.raw", "250.raw", "251.raw", "252.raw", "253.raw", "254.raw", "255.raw", "256.raw", "257.raw", "258.raw", "259.raw",

        // Direct_Sampling (60 VFS-files)
        // FIRST_RECORDING_FILE = 260
        // 260 - 319 (2 files per each Recording)
        "0.rec", "1.rec", "2.rec", "3.rec", "4.rec", "5.rec", "6.rec", "7.rec", "8.rec", "9.rec", "10.rec", "11.rec", "12.rec", "13.rec", "14.rec", "15.rec", "16.rec", "17.rec", "18.rec", "19.rec",
        "20.rec", "21.rec", "22.rec", "23.rec", "24.rec", "25.rec", "26.rec", "27.rec", "28.rec", "29.rec", "30.rec", "31.rec", "32.rec", "33.rec", "34.rec", "35.rec", "36.rec", "37.rec", "38.rec", "39.rec",
        "40.rec", "41.rec", "42.rec", "43.rec", "44.rec", "45.rec", "46.rec", "47.rec", "48.rec", "49.rec", "50.rec", "51.rec", "52.rec", "53.rec", "54.rec", "55.rec", "56.rec", "57.rec", "58.rec", "59.rec",

        // Live_Sampling, arrays on PSRAM: int16_t* FIFO[FIFO_SAMPLES], LS_buffer_L_ptr[FIFO_LR_SAMPLES], LS_buffer_R_ptr[FIFO_LR_SAMPLES],
        // FIRST_LIVE_SAMPLING_FILE = 320
        // 320, 321, 322
        "Mono.liv", "Left.liv", "Right.liv"};

// GESTIONE DELLA MEMORIA FLASH ESTERNA
int flash_dimension_MB;

// LILLA STATE
uint8_t Lilla_state;
uint8_t Lilla_state_0;

// SESSION
Session_struct Session[SESSIONS_MAX + 1]; // local copy of sessions
Sound_struct Sound[SOUNDS_MAX + 2];

// PERFORMANCE
uint8_t session = 0;
int volume_session = 29;
uint8_t map_instrument_for_note[16][128] = {0};
bool key_state[16][128] = {0}; // usato solo a fini statistici; key premuti su ciascun canale midi; rilevato attaverso il conteggio dei NoteOn
bool file_midi_ch_flag = true;
int8_t position_of_Instrument[INSTRUMENTS_MAX];
const char note_name[12][3] = {{"C"}, {"C#"}, {"D"}, {"D#"}, {"E"}, {"F"}, {"F#"}, {"G"}, {"G#"}, {"A"}, {"A#"}, {"B"}};
float pitch_from_note[128] = {0};
bool display_instrument_volume_flag = false;
uint8_t instrument_volume_changed = 0;

// menu
bool instrument_editing_flag = false;
uint8_t choice_performance_menu;
char Menu_P_char[5][12] = {{"EXIT"}, {"SAVE"}, {"CLONE"}, {"SAVE_AS_NEW"}, {"DROP"}};
uint8_t dimension_voice_Menu_P[5] = {4, 4, 5, 11, 4};
bool Menu_P[5];
int X_position_Menu_P[5];   // argument is position
int Y_position_Menu_P[5];   // argument is position
uint8_t element_Menu_P[5];  // argument is position
uint8_t position_Menu_P[5]; // argument is element

// Tuning tone
int tuning_tone_volume = 10;
uint8_t tuning_tone_last_note = 0;
bool tuning_tone_flag = false;
bool TT_playing = false;
bool TT_led_flag = false;

// SOUND EDIT
uint8_t trim_speed = 5;
uint16_t Noclick_max;
bool solo_flag = false;
bool slicing_mode = true; // true: slicing AB  - false: slicing A-Samples
const char name_mode[6][8] = {{"FWD"}, {"REV"}, {"FWD"}, {"FWD-REV"}, {"REV-FWD"}, {"REV"}};
char loop_mode[6][5] = {{"once"}, {"once"}, {"loop"}, {"loop"}, {"loop"}, {"loop"}};

// Menu
uint8_t choice_sound_menu;
char Menu_So_char[3][7] = {{"RETURN"}, {"CLONE"}, {"DROP"}};
uint8_t dimension_voice_Menu_So[3] = {6, 5, 4};
uint8_t X_position_Menu_So[3]; // argument is position
uint8_t element_Menu_So[3];    // argument is position
uint8_t position_Menu_So[3];   // argument is element
bool Menu_So[3];

// array di costanti
const int16_t Sine_wave[257] =
    {0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179,
     7962, 8739, 9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732,
     15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403,
     22005, 22594, 23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
     27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956, 30273, 30571,
     30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521,
     32609, 32678, 32728, 32757, 32767, 32757, 32728, 32678, 32609, 32521,
     32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
     30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790,
     26319, 25832, 25329, 24811, 24279, 23731, 23170, 22594, 22005, 21403,
     20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732,
     14010, 13279, 12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179,
     6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
     0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179,
     -7962, -8739, -9512, -10278, -11039, -11793, -12539, -13279, -14010, -14732,
     -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403,
     -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
     -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571,
     -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521,
     -32609, -32678, -32728, -32757, -32767, -32757, -32728, -32678, -32609, -32521,
     -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
     -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790,
     -26319, -25832, -25329, -24811, -24279, -23731, -23170, -22594, -22005, -21403,
     -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732,
     -14010, -13279, -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179,
     -6393, -5602, -4808, -4011, -3212, -2410, -1608, -804, 0};
const float waveform_decay[11] = {1.0, 0.600024, 0.34, 0.189978, 0.079989, 0.029511, 0.013886, 0.006012, 0.002106, 0.0002136, 0};  // {32767,19661,11141,6225,2621,967,455,197,69,7,0};
const float waveform_attack[11] = {0, 0.0002136, 0.002106, 0.006012, 0.013886, 0.029511, 0.079989, 0.189978, 0.34, 0.600024, 1.0}; // {32767,19661,11141,6225,2621,967,455,197,69,7,0};
const float Volume_float[41] =
    {0, 0.00015, 0.0005, 0.002, 0.008, 0.02, 0.037, 0.058, 0.08, 0.10125,
     0.125, 0.15125, 0.18, 0.21125, 0.245, 0.28125, 0.32, 0.36125, 0.405, 0.45125,
     0.5, 0.55125, 0.605, 0.66125, 0.72, 0.78125, 0.845, 0.91125, 0.98, 1.0,
     1.125, 1.20125, 1.28, 1.36125, 1.445, 1.53125, 1.62, 1.71125, 1.805, 1.90125, 2};
const float exp_table[11] = {0.0, 0.5000, 0.7500, 0.8750, 0.9375, 0.9688, 0.9844, 0.9922, 0.9961, 0.9980, 1.0};
const float sin_table[11] = {0.0, 0.0245, 0.0955, 0.2061, 0.3455, 0.5, 0.6545, 0.7939, 0.9045, 0.9755, 1.0};
const float decay_table[11] = {1.0, 0.9755, 0.9045, 0.7939, 0.6545, 0.5, 0.3455, 0.2061, 0.0955, 0.0245, 0.0};
const float release_table[11] = {1.0, 0.4991, 0.2487, 0.1234, 0.0608, 0.0295, 0.0139, 0.0060, 0.0021, 0.0002, 0.0};

// array compilati al setup()
float m_exp_table[10];
float m_sin_table[10];
float m_decay_table[10];
float m_release_table[10];
float pan_gain_L_table[33];
float pan_gain_R_table[33];

// funzioni
uint8_t Get_midi_channel(uint8_t session, uint8_t instrument)
{
    // .data contains midi channel in its bits: 7 6 5 M I D I 0
    return ((Sound[Session[session].Instrument[instrument].id_sound].data & 30) >> 1);
}
float Calc_pitch(float value)
{
    return pow(2.0f, value / 192.0f); // 0: no shift
}
float Calc_attack(uint8_t &n)
{
    return (n / 100.0f);
}
float CALC_decay(uint8_t &n)
{
    return (n / 25.0f);
}
float Calc_sustain(uint8_t &n)
{
    return (n / 50.0f);
}
float Calc_release(uint8_t &n)

{
    return (n / 2.0f);
}

// PRESET
Preset_struct Preset[INSTRUMENTS_MAX];

// PLAYER
elapsedMicros big_T = 0;
int16_t *Noclick_pointer[INSTRUMENTS_MAX] = {0};   // each Noclick instance contains 2 arrays
int16_t *Wavetable_pointer[INSTRUMENTS_MAX] = {0}; // each Wavetable instance contains 2 arrays

// funzioni
void Update_map_Instrument_for_notes(uint8_t from_note, uint8_t to_note, uint8_t instrument) // aggiorna la mappatura tra tutte Instrument e le coppie midi_channel/note_number e relative
{
    for (uint8_t note = 0; note < 128; ++note)
    {
        if (note >= from_note && note <= to_note)
        {
            bitWrite(map_instrument_for_note[Get_midi_channel(session, instrument)][note], instrument, 1);
        }
        else
        {
            bitWrite(map_instrument_for_note[Get_midi_channel(session, instrument)][note], instrument, 0);
        }
    }
}

void Reset_keys_state()
{
    for (uint8_t a = 0; a < 16; ++a)
    {
        for (uint8_t b = 0; b < 128; ++b)
        {
            key_state[a][b] = false; // true: key pressed down
        }
    }
}

// LPF FILTER RESOLUTION DOWNSAMPLING
bool lowpass_flag = false;
bool lowpass_direction = false;
int lowpass = LPF_MAX;
int lowpass_target = LPF_MAX;
const int lowpass_value[40] =
    {40, 48, 56, 65, 75, 90, 100, 125, 145, 170,
     200, 230, 270, 320, 380, 440, 520, 600, 720, 840,
     980, 1150, 1350, 1600, 1900, 2200, 2500, 3000, 3500, 4100,
     4900, 5700, 6700, 7900, 9200, 10000, 12500, 14500, 17500, 20000}; // 0 -> 39
bool display_lowpass_flag = false;
int resolution = 68;
int downsampling = 1;

float Calc_resolution(int value) // 0<= value <= 68 ; 1.0bit <= return <= 16.0bit
{
    if (value < 62)
    {
        return 1.0 + 9 * (value / 62.0f);
    }
    else
    {
        return value - 52;
    }
}

// PITCH BEND - AFTER TOUCH - VIBRATO
float pitch_bend_value[16] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
float after_touch_channel_value[16] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

// CONTROL CHANGE
uint8_t CC_Sound_gain[INSTRUMENTS_MAX] = {0};
uint8_t CC_lowpass_filter = 0;
uint8_t CC_midi_controller = 0;

// LED (NoteOn/Off da tastiera)
bool led_flag = false;

// STAMPA
void PRINT(String who, String what, float value)
{
    Serial.print(who);
    Serial.print(" ");
    Serial.print(what);
    Serial.print(": ");
    Serial.println(value);
}