/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedDelay.h"

// DELAY

Delay_values_struct Delay_values;
Delay_data_struct Delay_data;

float Delay_feedback(int8_t value) // feedback
{
    const float answer[] = {0, -0.07, -0.18, -0.4, -0.6, -0.65, -0.71, -0.80, -0.92, -0.98};
    value = constrain(value, 0, 10);
    return answer[value];
}

void Calc_Delay_values(Delay_data_struct data)
{
    Calc_delay_routing(data.instrument_route);
    Delay_values.samples = Calc_delay_samples(data.samples);
    Delay_values.samples_LR = Calc_delay_samples_LR(data.samples_LR);
    Delay_values.modulation_source = data.modulation_source;
    Delay_values.modulation_depth = Calc_delay_depth(data.modulation_depth);
    Delay_values.modulation_frequency = Calc_delay_frequency(data.modulation_frequency);
    Delay_values.modulation_phase_LR = data.modulation_phase_LR;
    Delay_values.loop_gain = Delay_feedback(data.loop_gain); // feedback

    if (true)
    {
        Print_Delay_values(Delay_values);
    }
}

FLASHMEM
void Print_Delay_values(Delay_values_struct Delay_values)
{
    Serial.println();
    Serial.println("Print Delay_values");
    Serial.print("Delay_values.samples: ");
    Serial.println(Delay_values.samples);
    Serial.print("Delay_values.samples_LR: ");
    Serial.println(Delay_values.samples_LR);
    Serial.print("Delay_values.modulation_source: ");
    Serial.println(Delay_values.modulation_source);
    Serial.print("Delay_values.modulation_depth: ");
    Serial.println(Delay_values.modulation_depth);
    Serial.print("Delay_values.modulation_frequency: ");
    Serial.println(Delay_values.modulation_frequency);
    Serial.print("Delay_values.modulation_phase_LR: ");
    Serial.println(Delay_values.modulation_phase_LR);
    Serial.print("Delay_values.loop_gain: ");
    Serial.println(Delay_values.loop_gain);

    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        Serial.print("Delay_values.instrument_route[");
        Serial.print(instrument_id);
        Serial.print("]: ");
        Serial.println(Delay_values.instrument_route[instrument_id]);
    }
    Serial.println();
}

FLASHMEM
void Print_Delay_data(const Delay_data_struct &data)
{
    Serial.println();
    Serial.println(F("Print_Delay_data(Delay_data_struct Data):"));

    Serial.print("Delay_data.samples: ");
    Serial.println(data.samples);

    Serial.print("Delay_data.samples_LR: ");
    Serial.println(data.samples_LR);

    Serial.print("Delay_data.instrument_route: ");
    Serial.println(data.instrument_route);

    Serial.print("Delay_data.modulation_source: ");
    Serial.println(data.modulation_source);

    Serial.print("Delay_data.modulation_depth: ");
    Serial.println(data.modulation_depth);

    Serial.print("Delay_data.modulation_frequency: ");
    Serial.println(data.modulation_frequency);

    Serial.print("Delay_data.modulation_phase_LR: ");
    Serial.println(data.modulation_phase_LR);

    Serial.print("Delay_data.loop_gain: ");
    Serial.println(data.loop_gain);
    Serial.println();
}

void Calc_delay_routing(uint8_t value)
{
    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        Delay_values.instrument_route[instrument_id] = (bitRead(value, instrument_id) == 1 ? true : false);
        
        if (false)
        {
            Serial.print("Calc_delay_routing(uint8_t value) - Delay_values.instrument_route[i]: ");
            Serial.println(Delay_values.instrument_route[instrument_id]);
        }
    }
}

void Turn_ON_Delay(bool ON) // switch on/off Delay (using Instrument routing)
{
    static int instrument_route[INSTRUMENTS_MAX];

    if (!ON)
    {
        for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
        {
            instrument_route[instrument_id] = Delay_values.instrument_route[instrument_id];
            Delay_values.instrument_route[instrument_id] = 0;
        }
    }
    
    else
    {
        for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
        {
            Delay_values.instrument_route[instrument_id] = instrument_route[instrument_id];
        }
    }
}

float Calc_delay_samples(int value) // 0 --> 99
{
    value = constrain(value, 0, 99);
    return delay_samples_table[value];
}

float Calc_delay_samples_LR(int value) // 0 --> 50
{
    return value * 44.1;
}

float Calc_delay_depth(int value)
{
    return depth_array[value] / 100.0f; // 0 <= value <= 39
}

float Calc_delay_frequency(int value)
{
    return (value * value / 90.0f); // 0 <= value <= 90
}