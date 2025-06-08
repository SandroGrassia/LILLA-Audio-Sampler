/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "LoopMetronomo.h"

void LoopMetronomo::Setup(const int LOOP_time)
{
    // calcola i valori LOOP_metro_delta_ms
    for (int i = 0; i < (LOOP_metro_leds - 1); ++i)
    {
        metro_delta_ms[i] = LOOP_time / LOOP_metro_leds;
    }
    metro_delta_ms[LOOP_metro_leds - 1] = LOOP_time - (LOOP_metro_leds - 1) * (LOOP_time / LOOP_metro_leds);
}

void LoopMetronomo::Leds_off(void)
{
    for (int i = 0; i < LOOP_metro_leds; ++i)
    {
        Display.Loop_led_metronomo(Metro_LED_X + i * 12, Metro_LED_Y, false);
    }
}

void LoopMetronomo::Led_ON(int led)
{
    if (led >= 0 && led < LOOP_metro_leds)
    {
        beat = led;
        Display.Loop_led_metronomo(Metro_LED_X + beat * 12, Metro_LED_Y, true);
        Serial.print("primo beat: ");
        Serial.println(beat);
    }
}

void LoopMetronomo::Set_beat(int value)
{
    beat = value;
    Leds_off();
}

void LoopMetronomo::Update(bool with_led)
{
    if (beat == (LOOP_metro_leds - 1))
    {
        if (with_led)
        {
            Display.Loop_led_metronomo(Metro_LED_X, Metro_LED_Y, true);
            Display.Loop_led_metronomo(Metro_LED_X + (LOOP_metro_leds - 1) * 12, Metro_LED_Y, false);
        }
        beat = 0;
    }
    else
    {
        if (with_led)
        {
            Display.Loop_led_metronomo(Metro_LED_X + (beat + 1) * 12, Metro_LED_Y, true);
            Display.Loop_led_metronomo(Metro_LED_X + (beat) * 12, Metro_LED_Y, false);
        }
        ++beat;
    }
}

int LoopMetronomo::Read_beat(void)
{
    return beat;
}

int LoopMetronomo::Read_metro_delta_ms(void)
{
    return metro_delta_ms[beat];
}