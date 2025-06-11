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

    AudioFilterBiquad* biquad_L_ptr = nullptr;
    AudioFilterBiquad* biquad_R_ptr = nullptr;
};
