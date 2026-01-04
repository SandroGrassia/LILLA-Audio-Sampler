/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "config.h"

void Setup_GATE_pins(void)
{
    // Gate in/out
    pinMode(GATE_IN_pin, INPUT_PULLUP);
    pinMode(GATE_OUT_pin, OUTPUT);
}
