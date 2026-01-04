/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedElements.h" // per INSTRUMENT_MAX
#include "config.h"

class PerformanceLedSet
{
private:

    int8_t led_activity[INSTRUMENTS_MAX]; // -2: request switch-OFF   -1: led OFF    +1: led ON   -2: request switch-ON

public:
    PerformanceLedSet() {}

    void Request_all_LED_switch_off(void);
    void Request_LED_switch(int instrument_id, bool on); // called by PlayersStatistics - do NOT access to Display
    int Read_LED_activity(int instrument_id); // called by main
    void Write_LED_activity(int instrument_id, bool on); // called by main
    void Restore_all_LED(void);
};