/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "PerformanceLedSet.h"

void PerformanceLedSet::Request_all_LED_switch_off(void)
{
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        led_activity[instrument_local] = -2; // request switch OFF
    }
}

void PerformanceLedSet::Request_LED_switch(int instrument, bool on)
{
    if (on)
    {
        if (led_activity[instrument] < 0)
        {
            led_activity[instrument] = 2; // request switch ON
            Serial.print("Request_LED_switch(int track, int instrument, bool on): on, instrument: ");
            Serial.println(instrument);
        }
    }
    else
    {
        if (led_activity[instrument] > 0)
        {
            led_activity[instrument] = -2; // request switch OFF
            Serial.print("Request_LED_switch(int track, int instrument, bool on): off, instrument: ");
            Serial.println(instrument);
        }
    }
}

int PerformanceLedSet::Read_LED_activity(int instrument)
{
    return led_activity[instrument];
}

void PerformanceLedSet::Write_LED_activity(int instrument, bool on)
{
    led_activity[instrument] = (on ? 1 : -1);
}

void PerformanceLedSet::Restore_all_LED(void)
{
    for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
    {
        if (abs(led_activity[instrument_local]) == 1)
        {
           led_activity[instrument_local] = 2 * led_activity[instrument_local];
        }
    }
}