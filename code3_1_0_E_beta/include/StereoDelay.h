/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <WaveLFO.h>
#include "Functions.h"

class StereoDelay : public AudioStream
{
private:
    static constexpr int DELAY_CENTRAL_VALUE_STEP = 2;
    static constexpr int MAX_TAPS = 5;

    void Constrain_delay_value(int value);

    unsigned long T[2] = {0};
    audio_block_t *inputQueueArray[2] = {nullptr};
    int sample_write = 0;
    int sample_read = 0;
    int delay_value = 0; // [sample]
    int delay_central_value = 0;
    int delay_central_value_step = 0; // quanti campioni in piu' o in meno ad ogni update()
    int delay_modulation_source = 0;  // 0: none  1: LFO  2: input 1
    float delay_modulation_gain_value = 0;
    int J_delay_central_value_counter = 0; // quanti cicli di update() servono per raggiungere il delay richiesto
    int32_t cache;

    // **  delay_Main_Array[DELAY_FIFO_SAMPLES]  **
    //
    // Start
    // read_sample        write_sample
    // R------------------W----------------------------------------------------|
    // 0<-  delay_value ->                                           (DELAY_FIFO_SAMPLES - 1)
    //
    // Running
    //            read_sample       write_sample
    // rrrrrrrrrrrR-------wwwwwwwwwwW------------------------------------------|
    // 0          <-  delay_value ->                                 (DELAY_FIFO_SAMPLES - 1)
    //
    // Running
    //                       read_sample       write_sample
    // rrrrrrrrrrrrrrrrrrrrrrRwwwwwwwwwwwwwwwwwW-------------------------------|
    // 0                     <-  delay_value ->                      (DELAY_FIFO_SAMPLES - 1)


public:
    StereoDelay(void) : AudioStream(2, inputQueueArray) {}

    // execution:
    // normal: 6micros @600MHz
    // delay_flag: 14.5micros @600MHz
    virtual void update(void);

    void Setup_delay(int value);             // value e' espresso in Samples
    void Set_delay_central_value(int value); // value e' espresso in Samples

    // La variazione della sorgente di modulazione del delay va applicata immediatamente
    void Set_delay_modulation_source(int value); // 0:none 1:LFO 2:input_1

    // La variazione dell'indice di modulazione va applicata immediatamente
    void Set_delay_modulation_gain(float value); // 0.0 --> 4.0
    
    // puntatori esterni
    int16_t *DELAY_fifo = nullptr; // Ricevi puntatore all'array FIFO
    WaveLFO *_LFO = nullptr;       // Ricevi puntatore LFO (nota: WaveLFO non eredita da AudioStream)
};