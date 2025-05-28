// Array per memorizzare i campioni prelevati dai file

#include "SamplesBasket.h"
 
   void SamplesBasket::Delete(void)
   {
    for (int i = 0; i < dim; ++i)
    *(data + i) = 0;
   };
   void SamplesBasket::Setup(void)
   {
    data = (int16_t *) malloc(dim);
   };