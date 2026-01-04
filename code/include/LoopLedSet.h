/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedElements.h"  // only for INSTRUMENTS_MAX
#include "SharedLoop.h"
#include "GraphicElements.h"
#include <SPI.h>
#include "config.h"

/*
LoopLedSet

Update(track) visualizza gli 8 LED di una Track sulla pagina MIDI_LOOP, in base al contenuto dell'array total_Players_per_instrument in PlayersStatistics,
e dal confronto con l'array total_Players_per_instrument_old.

IMPORTANTE: Update(track) e Update_all() NON vanno chiamate all'interno delle funzioni update() degli oggetti AudioStream

Lo stato ON/OFF dei led e' contenuto in led[TRACKS][INSTRUMENTS_MAX].

Per un funzionamento corretto, ad ogni passaggio di contesto (es da PERFORMANCE a MIDI_LOOP e viceversa) total_Players_per_instrument e
total_Players_per_instrument_old vanno azzerati assieme al reset dei Player.
*/

class LoopLedSet
{
private:
    // riferimenti esterni
    int8_t led_activity[TRACKS][INSTRUMENTS_MAX]; // -2: request switch-OFF   -1: led OFF    +1: led ON   -2: request switch-ON

public:
    LoopLedSet()
    {
        Request_all_LED_switch_off();    
    }
      
    void Request_all_LED_switch_off(void); // called by main
    void Request_track_LED_switch_off(int track); // 
    void Request_LED_switch(int track, int instrument_id, bool on); // called by PlayersStatistics - do NOT access to Display
    int Read_LED_activity(int track, int instrument_id); // called by main
    void Write_LED_activity(int track, int instrument_id, bool on); // called by main
};