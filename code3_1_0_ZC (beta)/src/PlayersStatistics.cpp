/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "PlayersStatistics.h"

void PlayersStatistics::Reset_total_Players_per_track_instrument(void)
{
    for (auto track = 0; track < TRACKS; ++track)
    {
        for (auto instrument_local = 0; instrument_local < INSTRUMENTS_MAX; ++instrument_local)
        {
            total_Players_per_track_instrument[track][instrument_local] = 0;
            total_Players_per_track_instrument_old[track][instrument_local] = 0;
        }
    }
}

void PlayersStatistics::Inc_total_Players_per_track_instrument(const int &track, const int &instrument)
{
    ++total_Players_per_track_instrument[track][instrument];

    // Serial.print("Inc_total_Players_per_track_instrument(const int &track, const int &instrument) - value: ");
    // Serial.println(total_Players_per_track_instrument[track][instrument]);

    if (total_Players_per_track_instrument[track][instrument] == 1)
    {
        // Serial.println("invio richiesta LED_ON");
        Loop_led_set_ptr->Request_LED_switch(track, instrument, true);
    }
}

void PlayersStatistics::Dec_total_Players_per_track_instrument(const int &track, const int &instrument)
{
    if (total_Players_per_track_instrument[track][instrument] > 0)
    {
        --total_Players_per_track_instrument[track][instrument];

        // Serial.print("Dec_total_Players_per_track_instrument(const int &track, const int &instrument) - value: ");
        // Serial.println(total_Players_per_track_instrument[track][instrument]);

        if (total_Players_per_track_instrument[track][instrument] == 0)
        {
            // Serial.println("invio richiesta LED_OFF");
            Loop_led_set_ptr->Request_LED_switch(track, instrument, false);
        }
    }
}

void PlayersStatistics::Reset_total_Players_per_instrument(void)
{
    for (auto i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        total_Players_per_instrument[i] = 0;
        total_Players_per_instrument_old[i] = 0;
    }
}

int PlayersStatistics::Read_total_Players_per_instrument(int instrument)
{
    return total_Players_per_instrument[instrument];
}

void PlayersStatistics::Inc_total_Players_per_instrument(int instrument)
{
    ++total_Players_per_instrument[instrument];

    if (true)
    {
        for (auto i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            Serial.print(total_Players_per_instrument[i]);
            Serial.print(" ");
        }
        Serial.println();
    }

    if (total_Players_per_instrument[instrument] == 1)
    {
        // Serial.println("invio richiesta LED_ON");   
        Performance_led_set_ptr->Request_LED_switch(instrument, true);
    }

}

void PlayersStatistics::Dec_total_Players_per_instrument(int instrument)
{
    if (total_Players_per_instrument[instrument] > 0)
    {
        --total_Players_per_instrument[instrument];

        if (total_Players_per_instrument[instrument] == 0)
        {
            // Serial.println("invio richiesta LED_OFF");
            Performance_led_set_ptr->Request_LED_switch(instrument, false);
        }
    }

    if (true)
    {
        for (auto i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            Serial.print(total_Players_per_instrument[i]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void PlayersStatistics::Set_total_Players_per_instrument_old(int instrument, int value)
{
    total_Players_per_instrument_old[instrument] = value;
}

int PlayersStatistics::Read_total_Players_per_instrument_old(int instrument)
{
    return total_Players_per_instrument_old[instrument];
}