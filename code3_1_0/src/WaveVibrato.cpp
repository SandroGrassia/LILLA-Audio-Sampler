/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "WaveVibrato.h"

/*
Vibrato is a FM effect; the modulating oscillator has T = constant, and the amplitude
is proportional to the Vibrato midi value (0 --> 127).

When Vibrato midi command is recived, the Main code:
- set a local "vibrato_flag" to true/false
- updates Vibrato object: MAKE_vibrato_array(vibrato_value)
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

    void WaveVibrato::Make_vibrato_table(void)
    {
        for(uint8_t x = 0; x < 32 ; x ++)
            vibrato_table[x] = sin ((x * M_PI)/ 16.0);
    }
    void WaveVibrato::Update_vibrato_array(float vibrato_value)
    {
        uint8_t x;
        float vibrato_k = (vibrato_value / 127.0) * MAX_DELTA_PITCH_VIBRATO;
        float vibrato_j = (vibrato_value / 127.0) * MIN_DELTA_PITCH_VIBRATO;
        for(x = 0; x < 16 ; x ++)
                vibrato_array[x] = 1.0 + (vibrato_k * vibrato_table[x]);
        for(x = 16; x < 32 ; x ++)
                vibrato_array[x] = 1.0 + (vibrato_j * vibrato_table[x]);
    }
    float* WaveVibrato::Get_vibrato_array_pointer(void)
    {
        return &vibrato_array[0];
    }
    uint8_t* WaveVibrato::Get_vibrato_array_last_element(void)
    {
        return &vibrato_array_last_element;
    }