/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "PlayersStatistics.h"

void PlayersStatistics::Reset_total_Players_per_track_instrument(void)
{
    for(auto track = 0; track < TRACKS; ++track)
    {
        for(auto instrument = 0; instrument < INSTRUMENTS_MAX; ++instrument)
        {
            total_Players_per_track_instrument[track][instrument] = 0;
            total_Players_per_track_instrument_old[track][instrument] = 0;
        }
    }
}

int PlayersStatistics::Read_total_Players_per_track_instrument(const int &track, const int &instrument)
{
    return total_Players_per_track_instrument[track][instrument];
}

void PlayersStatistics::Inc_total_Players_per_track_instrument(const int &track, const int &instrument)
{
    ++total_Players_per_track_instrument[track][instrument];
}

void PlayersStatistics::Dec_total_Players_per_track_instrument(const int &track, const int &instrument)
{
    if (total_Players_per_track_instrument[track][instrument] > 0)
    {
        --total_Players_per_track_instrument[track][instrument];
    }
}

void PlayersStatistics::Set_total_Players_per_track_instrument_old(const int &track, const int &instrument, int value)
{
    total_Players_per_track_instrument_old[track][instrument] = value;
}

int PlayersStatistics::Read_total_Players_per_track_instrument_old(const int &track, const int &instrument)
{
    return total_Players_per_track_instrument_old[track][instrument];
}

void PlayersStatistics::Reset_total_Players_per_instrument(void)
{
    for(auto i = 0; i < INSTRUMENTS_MAX; ++i)
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

    if (false)
    {
        for(auto i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            Serial.print(total_Players_per_instrument[i]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void PlayersStatistics::Dec_total_Players_per_instrument(int instrument)
{
    if (total_Players_per_instrument[instrument] > 0)
    {
        --total_Players_per_instrument[instrument];
    }

    if (false)
    {
        for(auto i = 0; i < INSTRUMENTS_MAX; ++i)
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