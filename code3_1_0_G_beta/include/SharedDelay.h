/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once
#include <Arduino.h>
#include "SharedElements.h"

// DELAY
static constexpr int DELAY_FIFO_SAMPLES = 220672;
static constexpr uint32_t DELAY_FIFO_BYTES = DELAY_FIFO_SAMPLES << 1; // 0x6bc00 - decimale 441.344

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
extern const int PROGMEM delay_samples_table[100];
extern Delay_data_struct Delay_data;
float Delay_feedback(int8_t value);

// funzioni
void Calc_Delay_values(Delay_data_struct data);
void Turn_ON_Delay(bool ON);
void Calc_delay_routing(uint8_t value);
float Calc_delay_samples(int value);
float Calc_delay_samples_LR(int value);
float Calc_delay_depth (uint8_t value);
float Calc_delay_frequency(uint8_t value);
void Print_Delay_data(const byte *data); // data is a pointer to the first byte of the struct Delay_data
void Print_Delay_values(Delay_values_struct Delay_values);