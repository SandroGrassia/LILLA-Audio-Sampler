/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once
#include <Arduino.h>



// RELEASE SOFTWARE
extern String FIRMWARE_VERSION; // definita in main.cpp



// ARCHITETTURA
static constexpr int PLAYERS = 16;
static constexpr int SAMPLES_VOLUME = 5000; // rampa per cambio gain - deve essere pari
static constexpr int BLOCK_MIN = 674;       // (at least AUDIO_BLOCK_SAMPLES * MAX_PITCH_FLASH) ; below this lenght, samples are copied from flash to RAM and tune is tracked with inner_tune
static constexpr int NOCLICK_DIM = 300; // max number of samples included in cross-fade time in NoClick array creation
static constexpr int SESSIONS_MAX = 24;   // max number of Sessions stored in EEPROM
static constexpr int INSTRUMENTS_MAX = 8; // mux number of Instruments per Session
static constexpr int SOUNDS_MAX = 85;     // max number of Sounds stored in EEPROM


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
static constexpr int RAW_FILES = 323; // nomi dei file audio (n.raw, m.rec, x.liv) esclusi i packet (Px.raw)
static constexpr int FIRST_RECORDING_FILE = 260;
extern const char name_file[RAW_FILES][10]; // Array dei nomi dei file .raw e .liv (usati da Live Sampling)


// RAW FILES COPY
static constexpr int BAR_POS_Y = 225; // y_pos(15)


// GESTIONE DELLA MEMORIA FLASH ESTERNA
extern int flash_dimension_MB;
int FLASHMEM Get_flash_size(void); // definita in main.cpp
int Get_packets_free(void); // definita in main.cpp ma possibile trasferirla qui
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
    MIDI_LOOP
};



// SESSION
// Variabili runtime
static constexpr int VOLUME_1 = 29;
struct Instrument_filter_data_struct
{
    // uint8_t compound; // bit0:use  bit1,2,3:modulation  bit4,5:filter_type
    uint8_t use;            // yes/no
    uint8_t type;           // filter type 0 --> 3
    uint8_t pivot;          // 0 --> 100 filter frequency/note frequency
    uint8_t resonance;      // 0 --> 40
    uint8_t modulation;     // waveform 0 -> 3
    uint8_t index;          // 1 --> 20 modulation_index
    uint8_t frequency_time; // 0 --> 20
};
struct Instrument_struct
{
    bool used;
    uint8_t id_sound;
    uint8_t root_key;
    uint8_t from_note;
    uint8_t to_note;
    bool precedence;
    bool lock;
    Instrument_filter_data_struct Filter;
};
struct Session_struct
{
    bool used;
    uint8_t instruments;
    Instrument_struct Instrument[8];
} __attribute__((__packed__)); // https://cs50.stackexchange.com/questions/22297/i-am-getting-an-unexpected-sizeof-error

extern Session_struct Session[SESSIONS_MAX + 1]; // last used by Direct Sampling for "preascolto" and Live Sampling
static constexpr uint8_t SIZE_OF_SESSION = sizeof(Session_struct);

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
} __attribute__((__packed__)); // https://cs50.stackexchange.com/questions/22297/i-am-getting-an-unexpected-sizeof-error
extern Sound_struct Sound[SOUNDS_MAX + 2];       // last 2 used by Direct Sampling for "preascolto" and Live Sampling
static constexpr uint8_t SIZE_OF_SOUND = sizeof(Sound_struct);



// PERFORMANCE
extern uint8_t session;
extern int volume_session;
extern uint8_t map_instrument_for_note[16][128];
extern bool key_state[16][128]; // usato solo a fini statistici; key premuti su ciascun canale midi; rilevato attaverso il conteggio dei NoteOn
extern bool file_midi_ch_flag; // quale funzionalita' regola l'encoder 2
extern int8_t position_of_Instrument[INSTRUMENTS_MAX];
extern const char PROGMEM note_name[12][3];
extern float pitch_from_note[128];
extern bool display_instrument_volume_flag;
extern uint8_t instrument_volume_changed;

// menu
extern bool instrument_editing_flag;
extern uint8_t choice_performance_menu;
extern char Menu_P_char[][12];
extern uint8_t dimension_voice_Menu_P[];
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
extern char  Menu_So_char[][7];
extern uint8_t dimension_voice_Menu_So[3];
extern uint8_t X_position_Menu_So[]; // argument is position
extern uint8_t element_Menu_So[];    // argument is position
extern uint8_t position_Menu_So[];   // argument is element
extern bool Menu_So[];

// Player mode
enum LillaPlayModes
{
    ONCE_FWD, // 0
    ONCE_REV, // 1
    LOOP_FWD, // 2
    LOOP_FWD_REV, // 3
    LOOP_REV_FWD, // 4
    LOOP_REV // 5
};



// ARRAY DI COSTANTI
extern const int16_t Sine_wave[257];
extern const float waveform_decay[11];
extern const float waveform_attack[11];
extern const float Volume_float[41];
extern const float exp_table[11];
extern const float sin_table[11];
extern const float decay_table[11];
extern const float release_table[11];

// array compilati al setup()
extern float m_exp_table[10];
extern float m_sin_table[10];
extern float m_decay_table[10];
extern float m_release_table[10];
extern float pan_gain_L_table[33];
extern float pan_gain_R_table[33];

// funzioni
uint8_t Get_midi_channel(uint8_t session, uint8_t instrument);
float Calc_pitch(float value);
float Calc_attack(uint8_t &n); float CALC_decay(uint8_t &n);
float Calc_sustain(uint8_t &n);
float Calc_release(uint8_t &n);



// PRESET
// E' il the data-set sent to a Player; it's a complete description of a sound that has to be played
struct Preset_struct
{
    float volume;
    int8_t pan;
    uint16_t id_sound;
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
extern elapsedMicros big_T; // Protezione Audiostream update()
extern int16_t *Noclick_pointer[INSTRUMENTS_MAX]; // each Noclick instance contains 2 arrays
extern int16_t *Wavetable_pointer[INSTRUMENTS_MAX]; // each Wavetable instance contains 2 arrays

// funzioni
void Update_map_Instrument_for_notes(uint8_t from_note, uint8_t to_note, uint8_t instrument); // aggiorna la mappatura tra tutte Instrument e le coppie midi_channel/note_number e relative
void Reset_keys_state();



// LPF FILTER
static constexpr int LPF_MAX = 39;
extern bool lowpass_flag;
extern bool lowpass_direction;
extern int lowpass;  // 0 <= lowpass_log_value <= 39
extern int lowpass_target; // 0 <= lowpass_log_value <= 39
extern const int lowpass_value[40];
extern bool display_lowpass_flag;


// RESOLUTION DOWNSAMPLING
extern int resolution;
extern int downsampling;
extern float Calc_resolution(int value);


// PITCH BEND - AFTER TOUCH - VIBRATO
constexpr double VIBRATO_STEP = 0.5568; // (64 * 0.0029) / vibrato_period; vibrato frequency = 6   64 steps --> 1.1136
extern float pitch_bend_value[16];
extern float after_touch_channel_value[16];


// CONTROL CHANGE
extern uint8_t CC_Sound_gain[INSTRUMENTS_MAX];
extern uint8_t CC_lowpass_filter;
extern uint8_t CC_midi_controller;


// LED (NoteOn/Off da tastiera)
extern bool led_flag;


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