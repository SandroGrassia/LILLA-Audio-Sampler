/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */
#pragma once

#include <Arduino.h>
#include "SharedElements.h"
#include "SharedLoop.h"
#include "LoopLedSet.h"
#include "PerformanceLedSet.h"
#include "config.h"

class LoopLedSet;

class PlayersStatistics
{
private:
    int total_Players_per_instrument[INSTRUMENTS_MAX]; // usato da DISPLAY_led_instrument(instrument_id): quanti Player stanno suonando l'Instrument
    int total_Players_per_instrument_old[INSTRUMENTS_MAX];

    int8_t total_Players_per_track_instrument[TRACKS][INSTRUMENTS_MAX]; // usato da DISPLAY_led_instrument(instrument_id): quanti Player stanno suonando nella track l'instrument_id
    int8_t total_Players_per_track_instrument_old[TRACKS][INSTRUMENTS_MAX];
    

public:
    PlayersStatistics(void)
    {
        Reset_total_Players_per_instrument();
        Reset_total_Players_per_track_instrument();
    }
    
    LoopLedSet* Loop_led_set_ptr = nullptr;
    PerformanceLedSet* Performance_led_set_ptr = nullptr; 

    void Reset_total_Players_per_instrument(void);
    int Read_total_Players_per_instrument(int instrument_id);
    void Inc_total_Players_per_instrument(int instrument_id);
    void Dec_total_Players_per_instrument(int instrument_id);
    void Set_total_Players_per_instrument_old(int instrument_id, int value);
    int Read_total_Players_per_instrument_old(int instrument_id);

    void Reset_total_Players_per_track_instrument(void);
    void Inc_total_Players_per_track_instrument(const int& track, const int& instrument_id);
    void Dec_total_Players_per_track_instrument(const int& track, const int& instrument_id);
};