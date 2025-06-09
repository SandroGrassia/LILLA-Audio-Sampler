/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "LoopLedSet.h"

void LoopLedSet::Update(int track) // richiede circa 1100 microsecondi
{
    for (int i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        if (Session[session].Instrument[i].used)
        {
            Display.Loop_led(Loop_LED_X + track * 42, Loop_LED_Y + i * Loop_LED_DY, (led[track][i] ? true : false));
        }
    }
}

void LoopLedSet::Update_all(void)
{
    for (int j = 0; j < TRACKS; ++j)
    {
        Update(j);
    }
}
void LoopLedSet::Switch_off_all(void)
{
    for (int j = 0; j < TRACKS; ++j)
    {
        for (int i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            if (Session[session].Instrument[i].used)
            {
                Display.Loop_led(Loop_LED_X + j * 42, Loop_LED_Y + i * Loop_LED_DY, false);
            }
            led[j][i] = false;
        }
    }
}
void LoopLedSet::Switch_off(int track)
{
    for (int i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        if (Session[session].Instrument[i].used)
        {
            Display.Loop_led(Loop_LED_X + track * 42, Loop_LED_Y + i * Loop_LED_DY, false);
        }
        led[track][i] = false;
    }
}
void LoopLedSet::State_reset(void)
{
    for (int j = 0; j < TRACKS; ++j)
    {
        for (int i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            led[j][i] = false;
        }
    }
}
