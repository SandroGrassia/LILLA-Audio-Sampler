/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>

class PsramManager
{
public:
PsramManager (void) {}

int16_t* New_samples_array(uint32_t dimension);
bool Remove_samples_array(int16_t* start_point);
};