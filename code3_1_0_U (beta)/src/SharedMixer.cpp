/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedMixer.h"

// MIXER

uint8_t MX_source = 0; // 0-->7: Sound 8: InputDevice
uint8_t PWM_volume = VOLUME_1;
uint8_t MAIN_volume = VOLUME_1;
uint8_t MX_routing_source[9] = {3, 3, 3, 3, 3, 3, 3, 3, 3}; // 0-->7: Sound 8: InputDevice; 1 --> source routed to PWM output (monitor); 2 --> source routed to Audio Board output;  3 --> source routed to both
bool MX_mute[9] = {false, false, false, false, false, false, false, false, false};