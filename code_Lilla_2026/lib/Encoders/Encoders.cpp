/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "Encoders.h"

void Encoders::Reset(void)
{
    for (auto i = 0; i < ENCODERS; ++i)
    {
        state[i] = a;
        rotation[i] = 0;
    }
}

void Encoders::Transmit_DT_CLK(const uint8_t& encoder, const uint8_t& DT, const uint8_t& CLK)
{
    uint8_t read = DT + (CLK << 1);
    rotation[encoder] = matrix[read][state[encoder] ].rotation;
    state[encoder] = matrix[read][state[encoder] ].next_state;
}

int Encoders::Get_state(const uint8_t& encoder)
{
    return state[encoder] ;
}

int Encoders::Get_rotation(const uint8_t& encoder)
{
    auto value = rotation[encoder];
    rotation[encoder] = 0;
    return value;
}