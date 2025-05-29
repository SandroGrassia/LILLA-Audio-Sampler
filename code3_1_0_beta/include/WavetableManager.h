/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <SerialFlash.h>
#include "SharedElements.h"
#include "StereoLiveSampler.h"
#include "LillaSerialFlash.h"

class WavetableManager
{
private:
    static constexpr int WAVETABLE_DIM = 674;
    static constexpr uint16_t FULL_WAVETABLE_DIM = 2 * WAVETABLE_DIM;
    void READ_Samples(int file_id, int16_t *destination, int seek_in, int samples_in);
    int16_t Wavetable[FULL_WAVETABLE_DIM] = {0};
    int16_t cache_1[WAVETABLE_DIM] = {0};
    int16_t cache_2[WAVETABLE_DIM] = {0};
    int length = 0;  
    
public:
    WavetableManager(void) {}

    //puntatori esterni
    StereoLiveSampler* _LiveSampler; // Wavetable[i]._LiveSampler = &LiveSampler;

    int16_t* get_pointer(void);
    
    bool Make(int file_id, int8_t mode, int A_Flash_sample, int B_Flash_sample, uint16_t delta_Noclick, int16_t *p_Noclick);
};