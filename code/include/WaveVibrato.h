/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <spi_interrupt.h>
#include "config.h"

/*
Vibrato is a FM effect; the modulating oscillator has T = constant, and the amplitude
is proportional to the Vibrato midi value (0 --> 127).

When Vibrato midi command is recived:
- set a local "vibrato_flag" to true/false
- updates Vibrato object: Make_vibrato_array(vibrato_value)
- updates all Players: Set_vibrato_flag(value)

When a Player receive the Set_vibrato_flag = TRUE command:
- set a local "vibrato_flag" to true
if Playing = TRUE:
- set a local vibrato_element
and for each update():
- vibrato_element += VIBRATO_STEP ; // 0<= vibrato_element <= 255
- pitch_vibrato = *(p_vibrato_array + vibrato_element);

When a Player receive the SET_vibrato_flag = FALSE command:
- set a local "vibrato_flag" to false
*/

class WaveVibrato
{
private:
    static constexpr float  MAX_DELTA_PITCH_VIBRATO = 0.009673533f;
    static constexpr float  MIN_DELTA_PITCH_VIBRATO = 0.009580853f;
    float vibrato_table[32] = {0};
    float vibrato_array[32] = {0};
    uint8_t vibrato_array_last_element = 0;

public:
    WaveVibrato(void) {}
    
    void Make_vibrato_table(void);
    void Update_vibrato_array(float vibrato_value);
    float* Get_vibrato_array_pointer(void);
    uint8_t* Get_vibrato_array_last_element(void);
};