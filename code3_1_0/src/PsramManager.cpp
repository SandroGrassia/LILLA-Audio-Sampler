/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

//riferimento https://github.com/PaulStoffregen/cores/blob/master/teensy4/extmem.c

#include "PsramManager.h"

// Live Sampling
// Dalla PSRAM totale si esclude spazio per i puntatori; lo spazio utile deve essere un multiplo di 256 (2*AUDIO_BLOCK_SAMPLES byte)
// PSRAM_16MB --> 16777216 byte; escludiamo spazio per i puntatori (4*AUDIO_BLOCK_SAMPLES) --> 16776704 byte

int16_t* PsramManager::New_samples_array(uint32_t dimension_bytes) // restituisce il puntatore int16_t* al primo elemento dell'array
{
    int16_t* _array = nullptr;   
    if (dimension_bytes >= 2)
    {
        // alloca l'array su PSRAM
        _array = (int16_t*)extmem_malloc(dimension_bytes);

        // azzera gli elementi dell'array
        for (uint32_t i = 0; i < (dimension_bytes >> 1); ++i)
            *(_array + i) = 0;
    }
    return _array;
}
bool PsramManager::Remove_samples_array(int16_t* _array)
{
    if ((unsigned long)_array > 1879048204) // 0x7000000C è il primo indirizzo disponibile sulla PSRAM
        {
            extmem_free(_array); 
            return true;
        }
    return false;
}