/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// Questa classe controlla parametri di filter_biquad.h

#pragma once

#include <Arduino.h>
#include "SharedElements.h"
#include <filter_biquad.h>

class FilterBiquadManager
{
public:
    FilterBiquadManager(void) {}
    
    void Update(void);

    AudioFilterBiquad* _biquad_L = nullptr;
    AudioFilterBiquad* _biquad_R = nullptr;
};
