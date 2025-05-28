/* SerialFlash Library - for filesystem-like access to SPI Serial Flash memory
 * https://github.com/PaulStoffregen/SerialFlash
 * Copyright (C) 2015, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this library was funded by PJRC.COM, LLC by sales of Teensy.
 * Please support PJRC's efforts to develop open source software by purchasing
 * Teensy or other genuine PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 /*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 */

#pragma once

#include <Arduino.h>
#include <SerialFlash.h>
#include "SharedElements.h"

class LillaSerialFlashFile : public SerialFlashFile
{
    // Questa classe estende SerialFlashFile includendo un metodo per per l'apertura rapida (fast_open)
    // di un oggetto (rawfile). Utilizzo:
    // Lilla_SerialFlashFile rawfile;
    // rawfile.fast_open(id_file);
    //
    // Per ridefinire l'oggetto come SerialFlashFile:
    // SerialFlashFile rawfile1 = rawfile;

public:
    LillaSerialFlashFile(void) : SerialFlashFile() {} // Costruttore che chiama il costruttore della classe base

    // funzione di apertura da usare al posto di SerialFlash.open(nome_file)
    void fast_open(int id_file);
    void packet_fast_open(int id_packet);
};

class FlashFileRegisterParser
{
    // questa classe contiene il database utilizzato da Lilla_SerialFlashFile::fast_open
private:
    static uint32_t address_array[RAW_FILES + PACKETS];
    static uint32_t length_array[RAW_FILES + PACKETS];
    static uint16_t dirindex_array[RAW_FILES + PACKETS];

public:
    static void Read_all_file_data();
    static uint32_t address(int id_file);
    static uint32_t length(int id_file);
    static uint16_t dirindex(int id_file);
};