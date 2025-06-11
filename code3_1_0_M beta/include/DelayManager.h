/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// Questa classe gestisce i parametri di SteroDelay.h

#pragma once

#include <Arduino.h>
#include "SharedDelay.h"
#include "StereoDelay.h"
#include "WaveLFO.h"
#include "AudioGain.h"
#include "PlayersManager.h"

class DelayManager
{
private:
    Delay_data_struct Delay_data_required;

    /*
    Filtraggio dei valori
    progetto del filtro LPF (Bessel Filter): https://www.micromodeler.com/dsp/

        (1 + 0.649z^-1) * 0.02749
    y = -------------------------- x
        1 - 1.611z^-1 + 0.656z^-2

    Risponde al gradino andando a regime in circa 20 step.

    y(t) = 1.611*y(t-1) - 0.656*y(t-2) + 0.02749*x(t) + 0.0178*x(t-1);

    y(t) = d1*y(t-1) + d2*y(t-2) + no*x(t) + n1*x(t-1);
    */
    
    static constexpr double n0 = 0.02748805251448725;
    static constexpr double n1 = 0.0178446670423745;
    static constexpr double d1 = 1.6107672804431383;
    static constexpr double d2 = -0.6561000000000001;

    /*
    enum Delay_parameters
    {
        SAMPLES,
        SAMPLES_LR,
        MODULATION_DEPTH,
        MODULATION_FREQUENCY,
        MODULATION_PHASE_LR,
        LOOP_GAIN,

        INSTRUMENT_ROUTE,
        MODULATION_SOURCE
    };
    */

    double x[DELAY_LPF_ITEMS];
    double x_1[DELAY_LPF_ITEMS];
    double y_1[DELAY_LPF_ITEMS];
    double y_2[DELAY_LPF_ITEMS];

    static constexpr int steps = 30;
    int step;
    
    bool run_flag = false;
    bool flag[DELAY_ITEMS];
    
    bool Start_LPF(int item, double v_0, double v_1); // v_0: valore di partenza   v_1: valore desiderato
    double New_value(int item);

public:
    DelayManager(void) {}

    StereoDelay *Delay_L_ptr = nullptr;
    StereoDelay *Delay_R_ptr = nullptr;
    WaveLFO *LFO_D_ptr[2];
    AudioGain *D_gain_L_feedback_ptr;
    AudioGain *D_gain_R_feedback_ptr;
    PlayersManager *Players_Manager_ptr;
 
    // chiamate da un oggetto AudioStream
    void Update(void); 
    bool New_values(const Delay_data_struct *data); // call using AudioNoInterrupt()
    void Stop(void); // forza l'arresto, run_flag = 0
};