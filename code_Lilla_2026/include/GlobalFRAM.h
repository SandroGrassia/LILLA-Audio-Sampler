/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#pragma once

#include <Arduino.h>
#include <LillaFRAM_MB85RC_I2C.h>


static constexpr int FRAM_LOCATIONS = 0b1000000000000000;
extern LillaFRAM_MB85RC_I2C FRAMchip;

/*

Wire.h library
https://www.pjrc.com/teensy/td_libs_Wire.html

I2C n.1
SCL1: 16
SDA1: 17

I2C n.2
SCL2: 24
SDA2: 25

Pullup resistor:
"1K to 4.7K resistors are recommended for most applications. The on-chip pullup resistors in Teensy 2.0, Teensy 4.0, Teensy 4.1 are very weak. Usually communication with 1 chip at 100kHz can work, but with poor signal quality. The on-chip resistors are not enough for several chips or higher speeds."

*****************************
*** Lilla case, 256K chip ***
*****************************

| Model          | Density (kB) | Device addressing | Device ID feature | Density code | Memory addressing | Tested |
| **MB85RC256V** | 256          | 7 bits            | Yes               | 0x05         | 15 bits           | Yes    |

256Kbit/8 --> 32KB
from 0: 0b 0000 0000 0000 0000
to 32K - 1 = 2^15 - 1 = 0b 01111111 11111111 (requires 15 bit)

generic memory location (15 bit): 0b 0abcdefg hijklmno

is transposed into:
address (write): 1010ABC0 + A  + 0abcdefg + A + hijklmno
address (read) : 1010ABC1 + A  + 0abcdefg + A + hijklmno

If (default ?):
ABC = 000 --> device address = 0x50

The address becomes:
device address: 10100000 = 0xA0
location: 01111111 11111111


*******************************
***  Example with 16K chip  ***
*******************************

16Kbit/8 --> 2KB
from 0: 0b 0000 0000 0000 0000
to 2KB - 1 = 2^11 - 1 = 0b 00000111 11111111 (requires 11 bit)

Es:
write address 0x25 =>  0b 000 00100101
write address 0x750 => 0b 111 01010000

Chip address translation must match the datasheet. Più complesso perché parte dell'indirizzo (i 3 MSB) entrano nel device address.
According to 16K device's datasheet, the I2C protocol is used the following way:
7 bits device address + R/W bit. Device address is 1010xxx where xxx are the 3 MSB of the memory slot (out of 11 bits)
After that device address, the 8 LSB of the memory slot are send to the bus.

To read or write at 0x025 memory slot, the IC2 device address is 0b 1010 000 x. Which means 0x050
To read or write at 0x750 memory slot, the IC2 device address is 0b 1010 111 x. Which means 0x057

The second byte sent is
    To read or write at 0x025 memory slot, (0b 000 00100101 & 0x000 11111111) => 0b 000 00100101 => 0x025
    To read or write at 0x750 memory slot, (0b 111 01010000 & 0x000 11111111) => 0b 000 01010000 => 0x010

The process (0x025 memory slot)
    wire.beginTranmission(0x50);
    Wire.write(0x25);
    ...

The process (0x750 memory slot)
    wire.beginTranmission(0x57);
    Wire.write(0x10);
    ...

*/