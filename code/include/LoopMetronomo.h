/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedLoop.h"
#include "GraphicElements.h"
#include "DisplayManager.h"
#include <SPI.h>
#include "config.h"

class LoopMetronomo
{
private:
    static constexpr int Metro_LED_Y = 64; // Y-PIXEL primo led Metronomo
    static constexpr int Metro_LED_X = 70; // X-PIXEL primo led Metronomo
    int beat;
    int metro_delta_ms[LOOP_metro_leds]; // delta_t in ms tra i passi del metronomo

    // riferimenti esterni
    DisplayManager &Display;

public:
    LoopMetronomo(DisplayManager &Display) : Display(Display) {}

    // riferimento esterno
    Adafruit_ILI9341 *tft_ptr = nullptr;

    void Update(bool with_led = true); // avanza il metronomo e i led
    void Setup(const int LOOP_time);
    void Leds_off(void); // spegne tutti i led
    void Led_ON(int led); // accende il led e assegna beat
    void Set_beat(int value); // assegna il beat attuale
    int Read_beat(void); // legge il beat attuale  
    int Read_metro_delta_ms(void); // delta_t per il prossimo beat

    unsigned long metro_time; // (ms) istante di esecuzione del prossimo evento da eseguire rispetto a LOOP_Clock
};