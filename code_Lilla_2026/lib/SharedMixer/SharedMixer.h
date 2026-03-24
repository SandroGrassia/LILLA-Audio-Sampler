/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedElements.h"
#include "config.h"


// MIXER

extern uint8_t MX_source;
extern uint8_t PWM_volume;
extern uint8_t MAIN_volume;
extern uint8_t MX_routing_source[];
extern bool MX_mute[];