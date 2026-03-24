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

void PlayersStatistics::Inc_total_Players_per_track_instrument(const int &track, const int &instrument_id)
{
    ++total_Players_per_track_instrument[track][instrument_id];

    // Serial.print("Inc_total_Players_per_track_instrument(const int &track, const int &instrument_id) - value: ");
    // Serial.println(total_Players_per_track_instrument[track][instrument_id]);

    if (total_Players_per_track_instrument[track][instrument_id] == 1)
    {
        // Serial.println("invio richiesta LED_ON");
        Loop_led_set_ptr->Request_LED_switch(track, instrument_id, true);
    }
}

void PlayersStatistics::Dec_total_Players_per_track_instrument(const int &track, const int &instrument_id)
{
    if (total_Players_per_track_instrument[track][instrument_id] > 0)
    {
        --total_Players_per_track_instrument[track][instrument_id];

        // Serial.print("Dec_total_Players_per_track_instrument(const int &track, const int &instrument_id) - value: ");
        // Serial.println(total_Players_per_track_instrument[track][instrument_id]);

        if (total_Players_per_track_instrument[track][instrument_id] == 0)
        {
            // Serial.println("invio richiesta LED_OFF");
            Loop_led_set_ptr->Request_LED_switch(track, instrument_id, false);
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

int PlayersStatistics::Read_total_Players_per_instrument(int instrument_id)
{
    return total_Players_per_instrument[instrument_id];
}

void PlayersStatistics::Inc_total_Players_per_instrument(int instrument_id)
{
    ++total_Players_per_instrument[instrument_id];

    if (false)
    {
        for (auto i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            Serial.print(total_Players_per_instrument[i]);
            Serial.print(" ");
        }
        Serial.println();
    }

    if (total_Players_per_instrument[instrument_id] == 1)
    {
        // Serial.println("invio richiesta LED_ON");   
        Performance_led_set_ptr->Request_LED_switch(instrument_id, true);
    }

}

void PlayersStatistics::Dec_total_Players_per_instrument(int instrument_id)
{
    if (total_Players_per_instrument[instrument_id] > 0)
    {
        --total_Players_per_instrument[instrument_id];

        if (total_Players_per_instrument[instrument_id] == 0)
        {
            // Serial.println("invio richiesta LED_OFF");
            Performance_led_set_ptr->Request_LED_switch(instrument_id, false);
        }
    }

    if (false)
    {
        for (auto i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            Serial.print(total_Players_per_instrument[i]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void PlayersStatistics::Set_total_Players_per_instrument_old(int instrument_id, int value)
{
    total_Players_per_instrument_old[instrument_id] = value;
}

int PlayersStatistics::Read_total_Players_per_instrument_old(int instrument_id)
{
    return total_Players_per_instrument_old[instrument_id];
}