/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h> 

// Firmware version
constexpr char FIRMWARE_VERSION[] = "6_TEST_4 22/03/2026";

// Hardware versions
/*
Lilla PCB2025_R2
This model includes 6 Shift Registers (Shifters) MCP23S17 (SPI communication)
- each Shifter address matches with its id (0 to 6)
- Shiters' channels are configured with INPUT_PULLUP
- each Encoder (DT,CLK,PB) is connected to a single Shifter
*/
#define PCB_2025_R2

// Bus SPI1 pins
// Communication with Display and Shift register chips
static constexpr int SPI1_SCLK = 27;        // SCK
static constexpr int SPI1_MOSI = 26;        // SDA
static constexpr int SPI1_MISO = 39;        // only Shift Registers
static constexpr int SPI1_DC = 29;          // A0
static constexpr int SPI1_RST = 30;         // RESET
static constexpr int SPI1_DISPLAY_CS = 38;  // Display
static constexpr int SPI1_SHIFTERS_CS = 37; // Shift Registers

// GateIn and GateOut pins
static constexpr int GATE_IN_pin = 31;
static constexpr int GATE_OUT_pin = 22;

// Configure GateIn and GateOut GPIO pins
void Setup_GATE_pins(void);
