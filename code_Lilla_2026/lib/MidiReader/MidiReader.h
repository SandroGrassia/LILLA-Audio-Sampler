/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// Lilla_Midi_Reader
// Credits: Francois Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)

#pragma once

#include <Arduino.h>
#include <MIDI.h>
#include "SharedElements.h"
#include "SharedLoop.h"
#include "SharedMM.h"
#include "WaveVibrato.h"
#include "WaveSine.h"
#include "PlayersManager.h"
#include "PlayersStatistics.h"
#include "LoopMetronomo.h"
#include "config.h"

extern MIDI_NAMESPACE::SerialMIDI<HardwareSerial> serialMIDI; // Define your transport - by Francois Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)
extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI; // Define the MIDI interface - by Francois Best (https://github.com/FortySevenEffects/arduino_midi_library/issues/165)

class MidiReader
{
private:
    bool midi_stop_flag = true;
    bool vibrato_flag = false;

    // riferimenti esterni
    LoopMetronomo &LOOP_metronomo;

public:
    MidiReader(LoopMetronomo &LOOP_metronomo) : LOOP_metronomo(LOOP_metronomo) {}

    virtual void Update(void);
    void Begin(void); // inizializza le funzionalità midi
    void Start(void); // inizializza le tastiere virtuali

    /*
    Chiamando .Stop():
    1) non si ricevono i messaggi MIDI, quindi non si inviano ai Player ne' NoteOn ne' NoteOff ne'altri messaggi/comandi
    3) l'elaborazione e la produzione di suoni dovuta a Player, Delay, Mixer etc... continua

    AudioNoInterrupts() + AudioInterrupts():
    1) si utilizzano SOLO nel main.cpp
    2) servono a garantire che il codice contenuto sia eseguito senza interruzione dovuta all'interrupt di Audiostrem
    3) si usa quando quando l'esecuzione parziale delle istruzioni creerebbe problemi
    4) vanno escuse istruzioni che comportano tempi di elaborazione "importanti" (come SerialPrint, operazioni sul Display) perche' l'interrupt di Audiostrem si puo' sospendere per un tempo T << 2900us
    */
    void Stop()
    {
        midi_stop_flag = true;
    }

    WaveVibrato *Vibrato_ptr = nullptr; // Midi_reader.Vibrato_ptr = &Vibrato
    WaveSine *Tone_generator_ptr = nullptr;
    PlayersManager *Players_Manager_ptr = nullptr;
};