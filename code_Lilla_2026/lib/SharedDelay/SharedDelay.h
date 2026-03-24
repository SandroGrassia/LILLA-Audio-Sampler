/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedElements.h"
#include "config.h"

// DELAY
static constexpr int DELAY_FIFO_SAMPLES = 220672;
static constexpr uint32_t DELAY_FIFO_BYTES = DELAY_FIFO_SAMPLES << 1; // 0x6bc00 - decimale 441.344
static constexpr float depth_array[40] = {
        0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1.0,
        2, 3, 4, 5, 6, 8, 10, 12, 14, 16,
        18, 20, 24, 28, 32, 36, 40, 44, 48, 52,
        56, 60, 65, 70, 75, 80, 85, 90, 95, 100};

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

static constexpr int DELAY_ITEMS = 8; // all Delay parameters
static constexpr int DELAY_LPF_ITEMS = 6; // parameters from SAMPLES to LOOP_GAIN are filtered with LPFs 

struct Delay_data_struct // DELAY_DATA_DIM byte
{ 
    uint16_t samples;
    int16_t samples_LR;
    uint8_t instrument_route;
    uint8_t modulation_source;
    uint8_t modulation_depth;
    uint8_t modulation_frequency;
    uint16_t modulation_phase_LR;
    uint16_t loop_gain;
};
static constexpr int DELAY_DATA_DIM = sizeof(Delay_data_struct);

static constexpr int Delay_data_limits[DELAY_LPF_ITEMS][2] = {
    {0, 99}, // SAMPLES
    {-10, 10}, // SAMPLES_LR
    {0, 39}, // MODULATION_DEPTH
    {0, 90}, // MODULATION_FREQUENCY
    {0, 359}, // MODULATION_PHASE_LR
    {0, 9} // LOOP_GAIN
};

struct Delay_values_struct
{
    float loop_gain;
    float samples;
    float samples_LR;
    bool instrument_route[INSTRUMENTS_MAX];
    uint8_t modulation_source; // 0: none 1:wave 2:signal
    float modulation_depth;        // 0.0 --> 1.0 modulation index
    float modulation_frequency;    // only for waveform
    uint16_t modulation_phase_LR;
};

extern Delay_values_struct Delay_values;
static constexpr int PROGMEM delay_samples_table[100] =
    {0, 11, 22, 33, 44, 55, 66, 77, 88, 110, 132, 154, 176, 221,
     265, 309, 353, 397, 485, 573, 662, 750, 838, 1058, 1279, 1499,
     1720, 1940, 2381, 2822, 3263, 3704, 4145, 4586, 5027, 5468, 5909,
     6350, 7232, 8114, 8996, 9878, 10760, 11642, 12524, 13406, 14876,
     16346, 17816, 19286, 20756, 22226, 23696, 25166, 26636, 28106,
     29576, 31046, 32516, 34721, 36926, 39131, 41336, 43541, 45746,
     47951, 50156, 52361, 54566, 56771, 58976, 61181, 63386, 65591,
     67796, 70001, 72206, 74411, 76616, 81026, 85436, 89846, 94256,
     98666, 103076, 107486, 111896, 116306, 120716, 125126, 132476,
     139826, 147176, 154526, 161876, 169226, 176576, 183926, 196000, 220500};

extern Delay_data_struct Delay_data;
float Delay_feedback(int8_t value);

// funzioni
void Calc_Delay_values(Delay_data_struct data);
void Turn_ON_Delay(bool ON);
void Calc_delay_routing(uint8_t value);
float Calc_delay_samples(int value);
float Calc_delay_samples_LR(int value);
float Calc_delay_depth (int value);
float Calc_delay_frequency(int value);
void Print_Delay_data(const Delay_data_struct &data);
void Print_Delay_values(Delay_values_struct Delay_values);