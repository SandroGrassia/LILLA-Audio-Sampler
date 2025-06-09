/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// Questa classe fornisce un array per memorizzare i campioni prelevati dai file

#pragma once
#include <Arduino.h>

class SamplesBasket
{
private:
   uint16_t dim;

public:
   SamplesBasket(uint16_t N) : dim(N) // Si istanzia fornendo la dimensione dell'array come numero di samples contenuti
   {
      Setup(); // la prima attivita' e' creare l'array
   }

   void Delete(void);
   void Setup(void);
   int16_t *data;
};