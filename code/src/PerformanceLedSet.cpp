/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "PerformanceLedSet.h"

void PerformanceLedSet::Request_all_LED_switch_off(void)
{
    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        led_activity[instrument_id] = -2; // request switch OFF
    }
}

void PerformanceLedSet::Request_LED_switch(int instrument_id, bool on)
{
    if (on)
    {
        if (led_activity[instrument_id] < 0)
        {
            led_activity[instrument_id] = 2; // request switch ON
        }
    }
    else
    {
        if (led_activity[instrument_id] > 0)
        {
            led_activity[instrument_id] = -2; // request switch OFF
        }
    }
}

int PerformanceLedSet::Read_LED_activity(int instrument_id)
{
    return led_activity[instrument_id];
}

void PerformanceLedSet::Write_LED_activity(int instrument_id, bool on)
{
    led_activity[instrument_id] = (on ? 1 : -1);
}

void PerformanceLedSet::Restore_all_LED(void)
{
    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        if (abs(led_activity[instrument_id]) == 1)
        {
           led_activity[instrument_id] = 2 * led_activity[instrument_id];
        }
    }
}