/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "config.h"

class GateIn
{
private:
public:
    GateIn() {}

    bool Read(void);
};

class GateOut
{
private:
    bool state = false;

public:
    GateOut() {}

    void Reset(void);
    void Write(void);
    bool Read_state(void);
};
