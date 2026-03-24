/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "Gate.h"

bool GateIn::Read(void)
{
    if (digitalRead(GATE_IN_pin))
    {
        return true;
    }
    return false;
}

void GateOut::Reset(void)
{
    digitalWrite(GATE_OUT_pin, LOW);
    state = false;
}

void GateOut::Write(void)
{
    digitalWrite(GATE_OUT_pin, HIGH);
    state = true;
}

bool GateOut::Read_state(void)
{
    return state;
}