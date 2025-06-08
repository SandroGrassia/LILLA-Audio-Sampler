/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */
#pragma once

#include <Arduino.h>
#include "SharedElements.h"
#include "SharedLoop.h"

class PlayersStatistics
{
private:
    int total_Players_per_instrument[INSTRUMENTS_MAX]; // usato da DISPLAY_led_instrument(instrument): quanti Player stanno suonando l'Instrument
    int total_Players_per_instrument_old[INSTRUMENTS_MAX];

    int8_t total_Players_per_track_instrument[TRACKS][INSTRUMENTS_MAX]; // usato da DISPLAY_led_instrument(instrument): quanti Player stanno suonando nella track l'instrument
    int8_t total_Players_per_track_instrument_old[TRACKS][INSTRUMENTS_MAX];

public:
    PlayersStatistics(void)
    {
        Reset_total_Players_per_instrument();
        Reset_total_Players_per_track_instrument();
    }

    void Reset_total_Players_per_instrument(void);
    int Read_total_Players_per_instrument(int instrument);
    void Inc_total_Players_per_instrument(int instrument);
    void Dec_total_Players_per_instrument(int instrument);
    void Set_total_Players_per_instrument_old(int instrument, int value);
    int Read_total_Players_per_instrument_old(int instrument);

    void Reset_total_Players_per_track_instrument(void);
    int Read_total_Players_per_track_instrument(const int& track, const int& instrument);
    void Inc_total_Players_per_track_instrument(const int& track, const int& instrument);
    void Dec_total_Players_per_track_instrument(const int& track, const int& instrument);
    void Set_total_Players_per_track_instrument_old(const int& track, const int& instrument, int value);
    int Read_total_Players_per_track_instrument_old(const int& track, const int& instrument);
};