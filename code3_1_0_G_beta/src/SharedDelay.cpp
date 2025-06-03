/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedDelay.h"

// DELAY

Delay_values_struct Delay_values;
Delay_data_struct Delay_data;

const int PROGMEM delay_samples_table[100] =
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
        Print_Delay_values(Delay_values);
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
    for (int i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        Serial.print("Delay_values.instrument_route[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(Delay_values.instrument_route[i]);
    }
    Serial.println();
}

FLASHMEM
void Print_Delay_data(const byte *data)
{       
        Serial.println();
        Serial.println(F("Print Delay_data"));
        byte data_LSB;
        byte data_MSB;
        int16_t result_int;
        uint16_t result_uint;
        int i = 0;

        Serial.print("uint16_t samples: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_uint = data_MSB << 8 | data_LSB;
        Serial.println(result_uint);

        Serial.print("int16_t samples_LR: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_int = data_MSB << 8 | data_LSB;
        Serial.println(result_int);

        Serial.print("instrument_route: ");
        Serial.println(*(data + i++));

        Serial.print("modulation: ");
        Serial.println(*(data + i++));

        Serial.print("depth: ");
        Serial.println(*(data + i++));

        Serial.print("frequency: ");
        Serial.println(*(data + i++));

        Serial.print("uint16_t phase_LR: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_uint = data_MSB << 8 | data_LSB;
        Serial.println(result_uint);

        Serial.print("uint16_t loop_gain: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_uint = data_MSB << 8 | data_LSB;
        Serial.println(result_uint);
        Serial.println();
}

void Calc_delay_routing(uint8_t value)
{
    for (int i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        Delay_values.instrument_route[i] = (bitRead(value, i) == 1 ? true : false);
        if (false)
        {
            Serial.print("Calc_delay_routing(uint8_t value) - Delay_values.instrument_route[i]: ");
            Serial.println(Delay_values.instrument_route[i]);
        }
    }
}

void Turn_ON_Delay(bool ON) // switch on/off Delay (using Instrument routing)
{
    static int instrument_route[INSTRUMENTS_MAX];
    if (!ON)
        for (int i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            instrument_route[i] = Delay_values.instrument_route[i];
            Delay_values.instrument_route[i] = 0;
        }
    else
        for (int i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            Delay_values.instrument_route[i] = instrument_route[i];
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

float Calc_delay_depth(uint8_t value)
{
    const float read_depth_array[40] = {
        0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1.0,
        2, 3, 4, 5, 6, 8, 10, 12, 14, 16,
        18, 20, 24, 28, 32, 36, 40, 44, 48, 52,
        56, 60, 65, 70, 75, 80, 85, 90, 95, 100};
    return read_depth_array[value] / 100.0f; // 0 <= value <= 39
}

float Calc_delay_frequency(uint8_t value)
{
    return (value * value / 90.0f); // 0 <= value <= 90
}