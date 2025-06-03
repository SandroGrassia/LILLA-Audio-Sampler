/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "SharedElements.h"
#include "SharedLoop.h"
#include "ElementiGrafici.h"
#include "DisplayManager.h"
#include "PlayersStatistics.h"
#include <SPI.h>

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
    DisplayManager &Display;

    static constexpr int Loop_LED_Y = 151; // Y-PIXEL primo led
    static constexpr int Loop_LED_X = 70;  // X-PIXEL primo led
    static constexpr int Loop_LED_DY = 11; // spaziatura Y

public:
    LoopLedSet(DisplayManager &Display) : Display(Display)
    {
        State_reset();
    }
    
    PlayersStatistics *_Players_statistics = nullptr;

    bool led[TRACKS][INSTRUMENTS_MAX]; // matrice dei led

    void Update(int track); // aggiorna i LED di track (richiede circa 1100 us) - Update(track) e Update_all() NON vanno chiamate all'interno delle funzioni update() degli oggetti AudioStream
    void Update_all(void); // aggiorna i LED di tutti i track - Update(track) e Update_all() NON vanno chiamate all'interno delle funzioni update() degli oggetti AudioStream
    void Switch_off_all(void); // spegne i LED di tutti i track
    void Switch_off(int track); // spegne i LED del track
    void State_reset(void); // azzera lo stato dei led dei 6 canali
};