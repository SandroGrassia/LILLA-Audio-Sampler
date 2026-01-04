/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "Pushbuttons.h"

void Pushbuttons::Transmit_position(const uint8_t &pushbutton, const uint8_t &position)
{
    /*
    Serial.print("Transmit_position - pushbutton: ");
    Serial.print(pushbutton);
    Serial.print(" position: ");
    Serial.println(position);
    */
    
    if (millis() > timer[pushbutton])
    {
        output[pushbutton] = matrix[position][state[pushbutton]].output;

        // Serial.print("output[pushbutton]: ");
        // Serial.println(output[pushbutton]);

        if (matrix[position][state[pushbutton]].restart_timer)
        {
            timer[pushbutton] = millis() + PAUSE_PB;
        }
        state[pushbutton] = matrix[position][state[pushbutton]].next_state;
    }
}

bool Pushbuttons::Get_state(const uint8_t &pushbutton)
{
    return (state[pushbutton] == 0? true: false); // 0: down, 1: up
}

bool Pushbuttons::Get_output(const uint8_t &pushbutton)
{
    auto value = output[pushbutton];
    output[pushbutton] = false;
    return value; // true: just pressed down; false: unchanged
}

void Pushbuttons::Reset(void)
{
    for(auto i = 0; i < PUSHBUTTONS; ++i)
    {
        state[i] = up,
        output[i] = false;
        timer[i] = 0;
    }
}