/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "FilterBiquadManager.h"

void FilterBiquadManager::Update(void)
{
    // **********************************************************************
    //                            LOWPASS FILTER
    // **********************************************************************

    if (lowpass_flag)
    {
        lowpass = (lowpass_direction ? (lowpass + 1) : (lowpass - 1));
        biquad_L_ptr->setLowpass(0, lowpass_value[lowpass], 0.7071);
        biquad_R_ptr->setLowpass(0, lowpass_value[lowpass], 0.7071);
        if (lowpass == lowpass_target)
        {
            lowpass_flag = false;
        }
    }
}