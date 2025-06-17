/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "LoopLedSet.h"

void LoopLedSet::Request_all_LED_switch_off(void)
{
    for (auto track = 0; track < TRACKS; ++track)
    {
        Request_track_LED_switch_off(track);
    }
}

void LoopLedSet::Request_track_LED_switch_off(int track)
{
    // also NOT existing instruments
    for (auto instrument = 0; instrument < INSTRUMENTS_MAX; ++instrument)
    {
        led_activity[track][instrument] = -2;
    }
}

void LoopLedSet::Request_LED_switch(int track, int instrument, bool on)
{
    if (on)
    {
        // Serial.print("Request_LED_switch(auto track, auto instrument, bool on) - true - led_activity:");
        // Serial.println(led_activity[track][instrument]);

        if (led_activity[track][instrument] < 0)
        {
            led_activity[track][instrument] = 2; // request switch ON
            // Serial.println("on");
        }
    }
    else
    {
        // Serial.print("Request_LED_switch(auto track, auto instrument, bool on) - false - led_activity:");
        // Serial.println(led_activity[track][instrument]);
        
        if (led_activity[track][instrument] > 0)
        {
            led_activity[track][instrument] = -2; // request switch OFF
            // Serial.println("off");
        }
    }
}

int LoopLedSet::Read_LED_activity(int track, int instrument)
{
    return led_activity[track][instrument];
}

void LoopLedSet::Write_LED_activity(int track, int instrument, bool on)
{
    led_activity[track][instrument] = (on ? 1 : -1);
}