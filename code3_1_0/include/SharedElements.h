/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once
#include <Arduino.h>



// RELEASE SOFTWARE
extern String FIRMWARE_VERSION; // definita in main.cpp



// HARDWARE
static constexpr int ENCODERS = 26;
static constexpr int PUSHBUTTONS = 36;



// ARCHITETTURA
static constexpr int PLAYERS = 16;
static constexpr int SAMPLES_VOLUME = 5000; // rampa per cambio gain - deve essere pari
static constexpr int BLOCK_MIN = 674;       // (at least AUDIO_BLOCK_SAMPLES * MAX_PITCH_FLASH) ; below this lenght, samples are copied from flash to RAM and tune is tracked with inner_tune
static constexpr int NOCLICK_DIM = 300; // max number of samples included in cross-fade time in NoClick array creation

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



// GESTIONE EEPROM
static constexpr int SESSIONS_MAX = 24;   // max number of Sessions stored in EEPROM
static constexpr int INSTRUMENTS_MAX = 8; // mux number of Instruments per Session
static constexpr int SOUNDS_MAX = 85;     // max number of Sounds stored in EEPROM
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
    uint8_t id_sound;
    uint8_t root_key;
    uint8_t from_note;
    uint8_t to_note;
    uint8_t info;                                // bit0: precedence, bit1: lock
    EEPROM_Instrument_filter_data_struct Filter; // 5 bytes
};
struct EEPROM_Session_struct // 90 bytes
{
    bool used; // 0:deleted  1:active
    uint8_t instruments;
    EEPROM_Instrument_struct Instrument[8]; // 8X11= 88 bytes
} __attribute__((__packed__));              // https://cs50.stackexchange.com/questions/22297/i-am-getting-an-unexpected-sizeof-error
extern EEPROM_Session_struct EEPROM_Session;
static constexpr uint8_t size_of_EEPROM_Session = sizeof(EEPROM_Session);




// GESTIONE DELLA MEMORIA PSRAM ESTERNA

// Live Sampling
/*
Dalla PSRAM totale si esclude spazio per i puntatori; lo spazio utile deve poter essere divisibile per  2 (L/R) poi per 256 (128 samples)
PSRAM_16MB --> 16777216 byte; escludiamo spazio per i puntatori (4*AUDIO_BLOCK_SAMPLES) --> 16776704 byte pari a 190sec; dedichiamo 1724*256=441344 byte (circa 5sec) per ciascun canale del Delay, restano 15894016 byte per Live Sampling
*/
static constexpr int LS_MONO_SAMPLES = 7946752;
static constexpr uint32_t LS_MONO_BYTES = LS_MONO_SAMPLES << 1; // 0xf28400 - 15.893.504
static constexpr int LS_STEREO_SAMPLES = 3973376; 
static constexpr uint32_t LS_STEREO_BYTES = LS_STEREO_SAMPLES << 1; // 0x794200 - decimale 7.946.752
static constexpr int FIRST_LIVE_SAMPLING_FILE = 320;

// Delay
static constexpr int DELAY_FIFO_SAMPLES = 220672;
static constexpr uint32_t DELAY_FIFO_BYTES = DELAY_FIFO_SAMPLES << 1; // 0x6bc00 - decimale 441.344



// FILES
static constexpr int RAW_FILES = 323; // nomi dei file audio (n.raw, m.rec, x.liv) esclusi i packet (Px.raw)
static constexpr int FIRST_RECORDING_FILE = 260;
extern const char name_file[RAW_FILES][10]; // Array dei nomi dei file .raw e .liv (usati da Live Sampling)




// GESTIONE DELLA MEMORIA FLASH ESTERNA
extern int flash_dimension_MB;
int FLASHMEM Get_flash_size(void); // definita in main.cpp
int Get_packets_free(void); // definita in main.cpp ma possibile trasferirla qui
int FLASHMEM Get_flash_occupation(void); // definita in main.cpp

// VFS/Virtual File System
static constexpr int PACKETS = 1024;
static constexpr int PACKET_DIM = 65536;  // number of bytes in a packet = 256 blocks of 128 samples (256 bytes)
static constexpr int PACKET_BLOCKS = 256; // number of AUDIO_BLOCKs in a packet
static constexpr int PACKETS_LAG = 3;     // Live Sampler: number of packets always EMPTY between last packet recorderd and first packet recorded
static constexpr int FLASH_FREE_SPACE = 131072;  // 512 AUDIO_BLOCKs - minimum space free in Flash chip
static constexpr int VFS_PACKETS_MIN = 20;       // 20x256x2.9ms = 14.8s
static constexpr int VFS_PACKETS_MAX = 512;      // maximum number of Packet in Flash chip <= PACKETS
static constexpr int VFS_PACKETS_DS = 512;       // <= VFS_PACKETS_MAX / 2 ; maximum number of Packets for Direct Sampler
static constexpr int RECORDINGS = 30; // number of recordings allowed; a single recording is made up with a group of 64KB "packets" stored in Flash chip
extern int VFS_FAT_table[VFS_PACKETS_DS]; // contiene il Recording_id cui appartiene il packet
extern int VFS_packets; // number of Packets on FLASH (maximum is VFS_PACKETS_MAX)
extern int VFS_packets_max;
extern const char PROGMEM name_packet[PACKETS][10]; // array dei nomi dei file .rec (packet usati da Direct Sampling)
struct EEPROM_VFS_Recording // 4 byte
{
    uint16_t first_packet; // if stereo is Left channel's first packet (Right channel's first packet is .first_packet + 1)
    uint8_t packets;       // packets per channel
    uint8_t info;
};
extern EEPROM_VFS_Recording EEPROM_Recording[RECORDINGS];
static constexpr uint8_t size_of_EEPROM_Recording = sizeof(EEPROM_Recording[0]);
struct VFS_Recording // runtime
{
    int first_packet; // if stereo is Left channel's first packet (Right channel's first packet is .first_packet + 1)
    int packets;      // packets per channel
    int bytes;        // per channel
    float seconds;
    bool stereo;
    bool consistent;
};
extern VFS_Recording Recording[RECORDINGS];





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
static constexpr uint8_t size_of_Sound = sizeof(Sound[0]);





// PERFORMANCE
extern uint8_t session;
extern int volume_session;
extern uint8_t map_instrument_for_note[16][128];
extern bool key_state[16][128]; // usato solo a fini statistici; key premuti su ciascun canale midi; rilevato attaverso il conteggio dei NoteOn
extern bool file_midi_ch_flag; // quale funzionalita' regola l'encoder 2
extern int8_t position_of_Instrument[];
extern const char PROGMEM note_name[][3];
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





// SOUND EDIT
extern uint8_t trim_speed;
extern uint16_t Noclick_max;
extern bool solo_flag;
extern bool slicing_mode;
extern const char name_mode[6][8];
extern char loop_mode[6][5];

// Menu
extern uint8_t choice_sound_menu;
extern char  Menu_So_char[3][7];
extern uint8_t dimension_voice_Menu_So[3];
extern uint8_t X_position_Menu_So[3]; // argument is position
extern uint8_t element_Menu_So[3];    // argument is position
extern uint8_t position_Menu_So[3];   // argument is element
extern bool Menu_So[3];

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

// array di costanti
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






// DIRECT SAMPLING
// VU-meter a barre
static constexpr int BAR_ELEMENTS = 50; // barre del VU-meter stereo

// menu
static constexpr int DS_MV = 12; // elementi di Menu_DS[]
extern bool Menu_DS[DS_MV];
extern const char Menu_DS_char[DS_MV][19];
extern const uint8_t dimension_voice_Menu_DS[DS_MV]; // dimensione degli elementi di Menu_DS[] 
extern uint8_t X_position_Menu_DS[DS_MV]; 
extern uint8_t Y_position_Menu_DS[DS_MV]; 
extern uint8_t element_Menu_DS[DS_MV]; 
extern uint8_t position_Menu_DS[DS_MV];
extern uint8_t choice_DS_menu;

// variabili
extern int recordings;
extern int recording;
extern int DS_gain;
extern elapsedMillis DS_blink_timer;
extern bool DS_blink_ON;





// LIVE SAMPLING
// Menu
constexpr int LS_MV = 4;
extern bool Menu_LS[LS_MV];
extern const char Menu_LS_char[][12];
extern const uint8_t dimension_voice_Menu_LS[LS_MV];
extern uint8_t X_position_Menu_LS[LS_MV]; // argument is position
extern uint8_t element_Menu_LS[LS_MV];    // argument is position
extern uint8_t position_Menu_LS[LS_MV];   // argument is element

// variabili
constexpr float LS_fbk_table[9] = {0, 0.01, 0.03, 0.07, 0.1, 0.2, 0.4, 0.8, 0.9};
extern int LS_state;
extern bool LS_stereo;
extern int LS_buffer_dim;
extern uint8_t LS_mode; // play mode
extern int LS_window_width; // samples from LS_window_A_sample to LS_window_B_sample

extern int LS_Q_sample; // ultima posizione registrata su LS_buffer_L/R
extern bool LS_XY_lock; // play bloccato sul virtual tape
extern int LS_X_step; // step di avanzamento
extern int LS_X_delta; // distanza tra LS_Q_sample e LS_X_sample
extern int LS_X_sample; // posizione di partenza play
extern int LS_Y_sample; // posizione di fine loop 
extern int LS_XY_delta; // distanza tra LS_X_sample e LS_Y_sample

// waveform
static constexpr int WAVE_WIDTH = 310;
int LS_constrain_position(int value);

// feedback
extern int LS_feedback; // feedback interno al Live Sampler

// DELAY
enum Delay_parameters
    {
        SAMPLES,
        SAMPLES_LR,
        MODULATION_DEPTH,
        MODULATION_FREQUENCY,
        MODULATION_PHASE_LR,
        LOOP_GAIN,

        INSTRUMENT_ROUTE,
        MODULATION_SOURCE
    };

static constexpr int Delay_LPF_items = 6; // Delay_LPF_items trattati con il LPF, da SAMPLES a LOOP_GAIN
static constexpr int Delay_items = 8; // tutti i parametri 

struct Delay_data_struct // delay_data_dim byte
{ 
    uint16_t samples;
    uint16_t samples_LR;
    uint8_t instrument_route;
    uint8_t modulation_source;
    uint8_t modulation_depth;
    uint8_t modulation_frequency;
    uint16_t modulation_phase_LR;
    uint16_t track_gain;
};

static constexpr int delay_data_dim = sizeof(Delay_data_struct);
static constexpr int Delay_samples_max = 99;
static constexpr int Delay_samples_LR_max = 99;
static constexpr int Delay_depth_max = 99;
static constexpr int Delay_frequency_max = 99;
static constexpr int Delay_phase_LR_max = 99;
static constexpr int Delay_loop_gain_max = 9;

static constexpr int Delay_data_limits[Delay_LPF_items][2] = {
    {0, 99}, // SAMPLES
    {-10, 10}, // SAMPLES_LR
    {0, 39}, // MODULATION_DEPTH
    {0, 90}, // MODULATION_FREQUENCY
    {0, 359}, // MODULATION_PHASE_LR
    {0, 9} // LOOP_GAIN
};

struct Delay_values_struct
{
    float track_gain;
    float samples;
    float samples_LR;
    bool instrument_route[INSTRUMENTS_MAX];
    uint8_t modulation_source; // 0: none 1:wave 2:signal
    float modulation_depth;        // 0.0 --> 1.0 modulation index
    float modulation_frequency;    // only for waveform
    uint16_t modulation_phase_LR;
};

extern Delay_values_struct Delay_values;
extern const int PROGMEM delay_samples_table[100];
extern Delay_data_struct Delay_data;
float Delay_feedback(int8_t value);


// funzioni
void Calc_Delay_values(Delay_data_struct data);
void Turn_ON_Delay(bool ON);
void Calc_delay_routing(uint8_t value);
float Calc_delay_samples(int value);
float Calc_delay_samples_LR(int value);
float Calc_delay_depth(uint8_t &value);
float Calc_delay_frequency(uint8_t &value);

// MIDI LOOP
static constexpr int NO_TRACK = -1; // track virtuale per note suonate da tastiera nello stato MIDI_LOOP
static constexpr int TRACKS = 6;        // numero di track
static constexpr int LOOP_EVENTS = 40; // numero massimo di eventi in un track
static constexpr int LOOP_UI_A = 1;    // primo encoder prima fila
static constexpr int LOOP_UI_B = 9;    // primo encoder seconda fila
static constexpr int LOOP_UI_C = 17;   // primo encoder terza fila
static constexpr int LOOP_metro_leds = 4; // 4 = 4/4
static constexpr int MIDI_LOOP_FILES = 1000;

struct LOOP_struct // verificata 12 byte 
{
    int time;    // ms
    uint8_t midi_channel;
    uint8_t note_number;
    uint8_t velocity;
    bool note_on;
};
extern LOOP_struct LOOP_element[TRACKS][LOOP_EVENTS];
extern byte LOOP_events[TRACKS];
extern int LOOP_slide[TRACKS]; // slittamento temporale in ms
extern int LOOP_pitch_int[TRACKS]; // slittamento pitch -400....0....+400
extern float LOOP_stretch; // stretch comune a tutti i track

// LOOP play/stop
extern bool LOOP_track_run[TRACKS]; // se "true" il track e' in esecuzione
extern int LOOP_play_event[TRACKS]; // indice del prossimo evento da eseguire
extern uint32_t LOOP_play_time[TRACKS]; // (ms) istante di esecuzione del prossimo evento da eseguire rispetto a LOOP_Clock
extern float LOOP_volume[TRACKS];
extern uint16_t LOOP_time; // (ms) durata del track master (0) comune a tutti i track

// LOOP learn
extern int LOOP_learning_track;
extern bool LOOP_learn_flag;
extern int LOOP_elements;
extern elapsedMillis LOOP_learn_clock; // utilizzato per calcolare la durata di track learn
extern int LOOP_clock_memo;
extern int LOOP_last_event;

// LOOP leds
extern bool LOOP_led_flag[TRACKS];

// LOOP_metronomo
extern bool LOOP_metronomo_run; // se "true" i led del metronomo sono visualizzati

// LOOP metronomo, richieste da MidiReader a Main
extern bool LOOP_metronomo_flag_IN[2]; // accendi led_0, switch led del metronomo

// LOOP metronomo, richieste da Main a MidiReader
// extern bool LOOP_metronomo_flag_OUT; // run metronomo
// extern unsigned long LOOP_metronomo_update_time; // prossimo istante di aggiornamento

// LOOP Menu
extern int Loop_menu_max;
extern uint8_t choice_loop_menu;
extern const char Menu_Loop_char[4][12];
extern const uint8_t dimension_voice_Menu_Loop[4];
extern uint8_t X_position_Menu_Loop[4]; // argument is position
extern bool Menu_Loop[4];
extern uint8_t element_Menu_Loop[4]; // argument is position
extern uint8_t position_Menu_Loop[4]; // argument is element

// LOOP funzioni
unsigned long LOOP_Clock_time_from_virtual_time(int T_evento); // definita in main.cpp
unsigned long LOOP_Clock(void); // definita in main.cpp

// LOOP salvataggio su SD
extern int LOOP_id; // loop_id actually displayed




// MIXER
extern uint8_t MX_source;
extern uint8_t PWM_volume;
extern uint8_t MAIN_volume;
extern uint8_t MX_routing_source[];
extern bool MX_mute[];




// MIDI MONITOR
extern uint8_t MM_midi_channel;
extern uint8_t MM_note_number;
extern uint8_t MM_velocity;
extern uint8_t MM_midi_controller;
extern uint8_t MM_midi_value;
extern uint8_t MM_pitch_bend_least;
extern uint8_t MM_pitch_bend_most;
extern uint8_t MM_least_bits;
extern uint8_t MM_most_bits;
extern uint8_t midi_message_received;
extern bool display_wait; // used in MIDI_MONITOR, SETUP




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
extern int lowpass;        // 0 <= lowpass_log_value <= 39
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