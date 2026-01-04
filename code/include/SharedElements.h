/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <FS.h>
#include "config.h"


// Main constants
static constexpr int PLAYERS = 16;
static constexpr int SAMPLES_VOLUME = 5000; // rampa per cambio gain - deve essere pari
static constexpr int BLOCK_MIN = 674;       // (at least AUDIO_BLOCK_SAMPLES * MAX_PITCH_FLASH) ; below this lenght, samples are copied from flash to RAM and tune is tracked with inner_tune
static constexpr int NOCLICK_DIM = 300;     // max number of samples included in cross-fade time in NoClick array creation
static constexpr int PATCHES_MAX = 24;      // max number of Patchs stored in EEPROM
static constexpr int INSTRUMENTS_MAX = 8;   // mux number of Instruments per Patch
static constexpr int SOUNDS_MAX = 85;       // max number of Sounds stored in EEPROM

// Polyphony and max pitch
static constexpr double MIN_PITCH = 0.01;                     // minimum value for pitch
static constexpr int POLYPHONY_FLASH[4] = {16, 12, 8, 4};     // [optimization]
static constexpr float MAX_PITCH_FLASH[4] = {1.65, 3, 4, 10}; // [optimization]
static constexpr float MAX_PITCH_WAVETABLE = 24.0;            // maximum value for pitch when playing from RAM
static constexpr float MAX_PITCH_PSRAM = 12.0;                // maximum value for pitch when playing from PSRAM

// SETUP
extern int key_step; // 0: 1semitono - 1: 1/2semitono - 2: 1/4semitono - 3: 1/8semitono
extern uint8_t optimization;
extern int8_t first_octave;

// FILES
static constexpr int NAME_FILE_SIZE = 10;
static constexpr int RAW_FILES = 323; // nomi dei file audio (n.raw, m.rec, x.liv) esclusi i packet (Px.raw)
static constexpr int FIRST_RECORDING_FILE = 260;

// Array dei nomi dei file .raw .rec (prodotti dal Sampler) e .liv (array usati da Live Sampling)
static constexpr char name_file[RAW_FILES][NAME_FILE_SIZE] =
    {
        // raw files on Flash chip
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

        // Direct_Sampling (60 VFS-files on Flash chip)
        // FIRST_RECORDING_FILE = 260
        // 260 - 319 (2 files per each Recording)
        "0.rec", "1.rec", "2.rec", "3.rec", "4.rec", "5.rec", "6.rec", "7.rec", "8.rec", "9.rec", "10.rec", "11.rec", "12.rec", "13.rec", "14.rec", "15.rec", "16.rec", "17.rec", "18.rec", "19.rec",
        "20.rec", "21.rec", "22.rec", "23.rec", "24.rec", "25.rec", "26.rec", "27.rec", "28.rec", "29.rec", "30.rec", "31.rec", "32.rec", "33.rec", "34.rec", "35.rec", "36.rec", "37.rec", "38.rec", "39.rec",
        "40.rec", "41.rec", "42.rec", "43.rec", "44.rec", "45.rec", "46.rec", "47.rec", "48.rec", "49.rec", "50.rec", "51.rec", "52.rec", "53.rec", "54.rec", "55.rec", "56.rec", "57.rec", "58.rec", "59.rec",

        // Live_Sampling, arrays on PSRAM chip: int16_t* FIFO[FIFO_SAMPLES], LS_buffer_L_ptr[FIFO_LR_SAMPLES], LS_buffer_R_ptr[FIFO_LR_SAMPLES],
        // FIRST_LIVE_SAMPLING_FILE = 320
        // 320, 321, 322
        "Mono.liv", "Left.liv", "Right.liv"};

// RAW FILES COPY
static constexpr int BAR_POS_Y = 225; // y_pos(15)

// GESTIONE DELLA MEMORIA FLASH ESTERNA
extern int flash_dimension_MB;
int FLASHMEM Get_flash_size(void);       // definita in main.cpp
int Get_packets_free(void);              // definita in main.cpp ma possibile trasferirla qui
int FLASHMEM Get_flash_occupation(void); // definita in main.cpp

// LILLA STATE
extern uint8_t Lilla_state;
extern uint8_t Lilla_state_0;
enum LillaStates
{
    PERFORMANCE,
    SOUND_EDIT,
    SETUP,
    MIDI_MONITOR,
    CC_SETTINGS,
    DELAY_SETTINGS,
    INSTRUMENT_VCF,
    DIRECT_SAMPLING,
    LIVE_SAMPLING,
    MIXER,
    MIDI_LOOP,
};

// PATCH
// Variabili runtime
static constexpr int VOLUME_1 = 29;
struct Instrument_filter_data_struct
{
    uint8_t use;            // yes/no
    uint8_t type;           // filter type 0 --> 3
    uint8_t pivot;          // 0 --> 100 filter frequency/note frequency
    uint8_t resonance;      // 0 --> 40
    uint8_t modulation;     // waveform 0 -> 3
    uint8_t index;          // 1 --> 20 modulation_index
    uint8_t frequency_time; // 0 --> 20
};
static constexpr uint8_t SIZE_OF_INSTRUMENT_FILTER_DATA = sizeof(Instrument_filter_data_struct); // 7

struct Instrument_struct
{
    bool used;
    uint8_t sound_id;
    uint8_t root_key;
    uint8_t from_note;
    uint8_t to_note;
    bool precedence;
    bool lock;
    Instrument_filter_data_struct Filter;
};
static constexpr uint8_t SIZE_OF_INSTRUMENT = sizeof(Instrument_struct); // 14

struct Patch_struct
{
    bool used;
    uint8_t instruments;
    Instrument_struct Instrument[INSTRUMENTS_MAX];
} __attribute__((__packed__));
static constexpr uint8_t SIZE_OF_PATCH = sizeof(Patch_struct); // 2 + 8 * 14 = 114
extern Patch_struct Patch[PATCHES_MAX + 1];                    // last used by Direct Sampling for "preascolto" and Live Sampling

struct Instrument_filter_values_struct
{
    uint8_t use;          // 0/1
    uint8_t type;         // 0 --> 3
    float pivot;          // 0 --> 30
    float resonance;      // 0 --> 40
    uint8_t modulation;   // 0:none 1:rising 2:falling 3:sinus 4:Aftertouch-sinus index
    float index;          // 1 --> 10
    uint8_t periodic;     // 0: aperiodic -> 1: periodic
    float frequency_time; // 0 --> 20
};

struct Sound_struct // 22 bytes
{
    bool used;
    uint16_t file;
    uint8_t mode;
    int8_t pitch; // -128 + 127
    uint32_t A;
    uint32_t B;
    uint16_t Noclick;
    int8_t pan;
    uint8_t data; // bit4-3-2-1: midi_channel bit0: Attack ramp ("0" Slow, "1" Fast)
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t gain;
} __attribute__((__packed__));
static constexpr uint8_t SIZE_OF_SOUND = sizeof(Sound_struct); // 22
extern Sound_struct Sound[SOUNDS_MAX + 2];                     // last 2 used by Direct Sampling for "preascolto" and Live Sampling

// PERFORMANCE
static constexpr char PROGMEM note_name[12][3] = {{"C"}, {"C#"}, {"D"}, {"D#"}, {"E"}, {"F"}, {"F#"}, {"G"}, {"G#"}, {"A"}, {"A#"}, {"B"}};
extern uint8_t Patch_id;
extern int volume_patch;
extern uint8_t map_instrument_for_note[16][128];
extern bool key_state[16][128]; // usato solo a fini statistici; key premuti su ciascun canale midi; rilevato attaverso il conteggio dei NoteOn
extern bool file_midi_ch_flag;  // quale funzionalita' regola l'encoder 2
extern int8_t position_of_Instrument[INSTRUMENTS_MAX];
extern float pitch_from_note[128];
extern bool display_instrument_volume_flag;
extern uint8_t instrument_volume_changed; // [instrument_id]

// menu
extern bool instrument_editing_flag;
extern uint8_t choice_performance_menu;
extern const char Menu_P_char[][12];
extern const uint8_t dimension_voice_Menu_P[];
extern int X_position_Menu_P[];   // argument is position
extern int Y_position_Menu_P[];   // argument is position
extern uint8_t element_Menu_P[];  // argument is position
extern uint8_t position_Menu_P[]; // argument is element
extern bool Menu_P[];

// Tuning tone
extern int tuning_tone_volume;
extern uint8_t tuning_tone_last_note;
extern bool tuning_tone_flag;
extern bool TT_playing;
extern bool TT_led_flag;

// SOUND
extern uint8_t trim_speed;
extern uint16_t Noclick_max;
extern bool solo_flag;
extern bool slicing_mode;
extern const char name_mode[6][8];
extern char loop_mode[6][5];

// Menu
extern uint8_t choice_sound_menu;
extern const char Menu_So_char[][7];
extern const uint8_t dimension_voice_Menu_So[3];
extern uint8_t X_position_Menu_So[]; // argument is position
extern uint8_t element_Menu_So[];    // argument is position
extern uint8_t position_Menu_So[];   // argument is element
extern bool Menu_So[];

// Player mode
enum LillaPlayModes
{
    ONCE_FWD,     // 0
    ONCE_REV,     // 1
    LOOP_FWD,     // 2
    LOOP_FWD_REV, // 3
    LOOP_REV_FWD, // 4
    LOOP_REV      // 5
};

// ARRAY DI COSTANTI
static constexpr int16_t Sine_wave[257] =
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
static constexpr float waveform_attack[11] = {0, 0.0002136, 0.002106, 0.006012, 0.013886, 0.029511, 0.079989, 0.189978, 0.34, 0.600024, 1.0};
static constexpr float Volume_float[41] =
    {0, 0.00015, 0.0005, 0.002, 0.008, 0.02, 0.037, 0.058, 0.08, 0.10125,
     0.125, 0.15125, 0.18, 0.21125, 0.245, 0.28125, 0.32, 0.36125, 0.405, 0.45125,
     0.5, 0.55125, 0.605, 0.66125, 0.72, 0.78125, 0.845, 0.91125, 0.98, 1.0,
     1.125, 1.20125, 1.28, 1.36125, 1.445, 1.53125, 1.62, 1.71125, 1.805, 1.90125, 2};
static constexpr float exp_table[11] = {0.0, 0.5000, 0.7500, 0.8750, 0.9375, 0.9688, 0.9844, 0.9922, 0.9961, 0.9980, 1.0};
static constexpr float sin_table[11] = {0.0, 0.0245, 0.0955, 0.2061, 0.3455, 0.5, 0.6545, 0.7939, 0.9045, 0.9755, 1.0};
static constexpr float decay_table[11] = {1.0, 0.9755, 0.9045, 0.7939, 0.6545, 0.5, 0.3455, 0.2061, 0.0955, 0.0245, 0.0};
static constexpr float release_table[11] = {1.0, 0.4991, 0.2487, 0.1234, 0.0608, 0.0295, 0.0139, 0.0060, 0.0021, 0.0002, 0.0};

// array compilati al setup()
extern float m_exp_table[10];
extern float m_sin_table[10];
extern float m_decay_table[10];
extern float m_release_table[10];
extern float pan_gain_L_table[33];
extern float pan_gain_R_table[33];

// funzioni
uint8_t Get_midi_channel(int patch_id, int instrument_id);

// PRESET
// E' il the data-set sent to a Player; it's a complete description of a sound that has to be played
struct Preset_struct
{
    float volume;
    int8_t pan;
    uint16_t sound_id;
    uint16_t file;
    uint16_t midi_channel;
    float pitch;
    uint8_t mode;
    int A;
    int B;
    bool use_Wavetable;
    int Noclick;
    uint8_t attack_type;
    float attack;
    float decay;
    float sustain;
    float release;
    bool precedence;
    bool lock;
    Instrument_filter_values_struct Filter;
};
extern Preset_struct Preset[INSTRUMENTS_MAX];

// AUDIOPLAYER
extern elapsedMicros security_timer;                // Protezione Audiostream update()
extern int16_t *Noclick_pointer[INSTRUMENTS_MAX];   // each Noclick instance contains 2 arrays
extern int16_t *Wavetable_pointer[INSTRUMENTS_MAX]; // each Wavetable instance contains 2 arrays

// funzioni
void Update_map_Instrument_for_notes(int from_note, int to_note, int instrument_id); // aggiorna la mappatura tra tutte Instrument e le coppie midi_channel/note_number e relative
void Reset_keys_state();

// LPF FILTER
static constexpr int LPF_MAX = 39;
extern bool lowpass_flag;
extern bool lowpass_direction;
extern int lowpass;        // 0 <= lowpass_log_value <= 39
extern int lowpass_target; // 0 <= lowpass_log_value <= 39
extern bool display_lowpass_flag;
constexpr int lowpass_value[LPF_MAX + 1] =
    {40, 48, 56, 65, 75, 90, 100, 125, 145, 170,
     200, 230, 270, 320, 380, 440, 520, 600, 720, 840,
     980, 1150, 1350, 1600, 1900, 2200, 2500, 3000, 3500, 4100,
     4900, 5700, 6700, 7900, 9200, 10000, 12500, 14500, 17500, 20000}; // 0 -> 39

// RESOLUTION DOWNSAMPLING
static constexpr int RES_MAX = 79;
constexpr float resolution_value[RES_MAX + 1] =
    {
        16, 15, 14, 13, 12.5, 12, 11.5, 11, 10.5, 10,
        9.9, 9.8, 9.7, 9.6, 9.5, 9.4, 9.3, 9.2, 9.1, 9,
        8.9, 8.8, 8.7, 8.6, 8.5, 8.4, 8.3, 8.2, 8.1, 8,
        7.9, 7.8, 7.7, 7.6, 7.5, 7.4, 7.3, 7.2, 7.1, 7,
        6.9, 6.8, 6.7, 6.6, 6.5, 6.4, 6.3, 6.2, 6.1, 6,
        5.9, 5.8, 5.7, 5.6, 5.5, 5.4, 5.3, 5.2, 5.1, 5,
        4.9, 4.8, 4.7, 4.6, 4.5, 4.4, 4.3, 4.2, 4.1, 4,
        3.9, 3.8, 3.7, 3.6, 3.5, 3.4, 3.3, 3.2, 3.1, 3};
extern int resolution;
extern int downsampling;

// PITCH BEND - AFTER TOUCH - VIBRATO
constexpr double VIBRATO_STEP = 0.5568; // (64 * 0.0029) / vibrato_period; vibrato frequency = 6   64 steps --> 1.1136
extern float pitch_bend_value[16];
extern float after_touch_channel_value[16];

// CONTROL CHANGE
extern uint8_t CC_Sound_gain[INSTRUMENTS_MAX];
extern uint8_t CC_lowpass_filter;
extern uint8_t CC_midi_controller;

// STAMPA
void PRINT(String who, String what, float value);

/*
semitones	pitch	    delta pitch
12	        2	        1
11	        1.887748625	0.887748625
10	        1.781797436	0.781797436
9	        1,681792831	0,681792831
8	        1,587401052	0,587401052
7	        1,498307077	0,498307077
6	        1,414213562	0,414213562
5	        1,334839854	0,334839854
4	        1,25992105	0,25992105
3	        1,189207115	0,189207115
2	        1,122462048	0,122462048
1       	1,059463094	0,059463094
1/4         1.014545335 0.014545335
1/5         1.011619440 0.011619440
1/6         1.009673533 0.009673533
0	        1	        0
- 1/6       0.990419147 0.009580853
-1/5        0.988514020 0.011485980
-1	        0.943874313	-0,056125687
-2	        0,890898718	-0,109101282
-3	        0,840896415	-0,159103585
-4	        0,793700526	-0,206299474
-5	        0,749153538	-0,250846462
-6	        0,707106781	-0,292893219
-7	        0,667419927	-0,332580073
-8	        0,629960525	-0,370039475
-9	        0,594603558	-0,405396442
-10	        0,561231024	-0,438768976
-11	        0,529731547	-0,470268453
-12     	0,5	        -0,5
*/