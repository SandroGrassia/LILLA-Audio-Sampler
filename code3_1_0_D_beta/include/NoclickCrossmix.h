/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// NoclickCrossmix.h crea il vettore di cross-mix per loop Forward o loop Reverse

#pragma once

#include <Arduino.h>
#include <SerialFlash.h>
#include "SharedElements.h"
#include "LillaSerialFlash.h"

class NoclickCrossmix
{
private:
    void Read_flash(int file_id, int16_t *destination, int seek_in, int samples_in);
    int16_t Noclick[NOCLICK_DIM];

public:
    NoclickCrossmix(void) {}

    int16_t *get_pointer(void);
    bool Make(int file_id, int32_t A_Flash_sample, int32_t B_Flash_sample, uint16_t delta_Noclick);
};