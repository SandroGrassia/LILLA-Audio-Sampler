/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "StereoDelay.h"

// value e' espresso in Samples
void StereoDelay::Setup_delay(int value)
{
    Constrain_delay_value(value);
    delay_value = value;
    sample_write = delay_value;
    sample_read = 0;
    delay_central_value = delay_value;
}

// delay_value è il valore effettivo del delay;
// Se non c'e' modulazione del delay --> delay_value == delay_central_value
//
// 0-------------------------(delay_central_value)-----------------------------(DELAY_PIPELINE - 128)
//                   (delay_value)
//                         (delay_value)
//                                     (delay_value)
//                                          (delay_value)
//                                   (delay_value)
//                         (delay_value)

// La variazione di delay va applicata gradualmente ad ogni update()
void StereoDelay::Set_delay_central_value(int value) // value = numero di campioni
{
    int delay_central_value_delta;
    Constrain_delay_value(value);

    if (value != delay_central_value)
    {
        delay_central_value_delta = value - delay_central_value;

        if (abs(delay_central_value_delta) < 500)
        {
            // Queste due variabili sono usate nell'aggiornamento di delay_central_value
            // J_delay_central_value_counter diverso per ogni tap
            J_delay_central_value_counter = abs(delay_central_value_delta) / DELAY_CENTRAL_VALUE_STEP; // quanti cicli di update() servono per raggiungere il delay richiesto

            // delay_central_value_step identico per ogni tap
            delay_central_value_step = ((delay_central_value_delta > 0) ? 1 : -1) * DELAY_CENTRAL_VALUE_STEP; // = +/- 2 // quanti campioni in piu' o in meno ad ogni update()
        }
        else if (abs(delay_central_value_delta) < 2000)
        {
            J_delay_central_value_counter = abs(delay_central_value_delta) / (2 * DELAY_CENTRAL_VALUE_STEP);      // quanti cicli di update() servono per raggiungere il delay richiesto
            delay_central_value_step = ((delay_central_value_delta > 0) ? 1 : -1) * 2 * DELAY_CENTRAL_VALUE_STEP; // = +/- 2 // quanti campioni in piu' o in meno ad ogni update()
        }
        else if (abs(delay_central_value_delta) < 5000)
        {
            J_delay_central_value_counter = abs(delay_central_value_delta) / (4 * DELAY_CENTRAL_VALUE_STEP);      // quanti cicli di update() servono per raggiungere il delay richiesto
            delay_central_value_step = ((delay_central_value_delta > 0) ? 1 : -1) * 4 * DELAY_CENTRAL_VALUE_STEP; // = +/- 2 // quanti campioni in piu' o in meno ad ogni update()
        }
        else
        {
            J_delay_central_value_counter = abs(delay_central_value_delta) / (8 * DELAY_CENTRAL_VALUE_STEP);      // quanti cicli di update() servono per raggiungere il delay richiesto
            delay_central_value_step = ((delay_central_value_delta > 0) ? 1 : -1) * 8 * DELAY_CENTRAL_VALUE_STEP; // = +/- 2 // quanti campioni in piu' o in meno ad ogni update()
        }
    }
}

// La variazione della sorgente di modulazione del delay va applicata immediatamente
void StereoDelay::Set_delay_modulation_source(int value) // 0:none 1:LFO 2:input_1
{
    if (value < 0)
    {
        value = 0;
    }
    delay_modulation_source = value;
}

// La variazione dell'indice di modulazione va applicata immediatamente
void StereoDelay::Set_delay_modulation_gain(float value) // 0.0 --> 4.0
{
    if (value < 0)
    {
        value = 0.0;
    }
    else if (value > 4.0)
    {
        value = 4.0;
    }

    delay_modulation_gain_value = value;
}

// execution:
// normal: 6micros @600MHz
// delay_flag: 14.5micros @600MHz
void StereoDelay::update(void)
{
    double D_sample_read;
    int delay_modulation;
    int delay_by_modulation;
    double delay_pitch;
    int delay_delta = 0;
    int I_sample_read_c;
    int I_sample_read_f;

    T[0] = micros();

    // **  audio_block_t  **
    // Audio blocks are represented with this data type, which is a C struct. The only member intended for use in update() is "data", an array of 16 bit integers representing the audio.
    // If "myblock" is a pointer to an audio_block_t, use myblock->data[0] to access the first audio sample, myblock->data[1] to access the second, and so on.
    // The data[] array is always 32 bit aligned in memory, so you can fetch pairs of samples by type casting the address as a pointer to 32 bit data.

    // Set input data structure, receive audio input from ch.0
    audio_block_t *in_block = NULL;

    // Set output data structure
    audio_block_t *out_block = NULL;
    out_block = allocate(); // al contratio di in_block, inizializzato da receiveXXOnly(ch), un block inizialmente vuoto va allocato

    // Write in_block to delay_Main_Array; execution: 1,5micros @600MHz
    // ***** spostato ****

    // Variazione del valore centrale delay_value
    if (J_delay_central_value_counter > 0) // J_delay_central_value_counter e' un contatore
    {
        // valorizzo delay_delta; l'effetto e' un pitch != 1.0 sulla lettura, e avviene piu' avanti
        delay_delta = delay_central_value_step; // +/- 2
        // aggiorno gia' da ora il valore di delay_central_value
        delay_central_value += delay_delta;
        J_delay_central_value_counter--;
    }

    // delay modulation con LFO
    if (delay_modulation_source == 1)
    {
        LFO_ptr->Update();                                                  // 1: periodic (sinus)
        delay_modulation = delay_modulation_gain_value * LFO_ptr->block[0]; // read only the first value
        delay_by_modulation = delay_central_value + delay_modulation;
        delay_by_modulation = constrain(delay_by_modulation, 0, DELAY_FIFO_SAMPLES - AUDIO_BLOCK_SAMPLES);

        delay_delta += delay_by_modulation - delay_value;
        if (delay_delta > (AUDIO_BLOCK_SAMPLES - 10))
        {
            delay_delta = AUDIO_BLOCK_SAMPLES - 10;
        }
    }

    // delay modulation da input_1
    else if (delay_modulation_source == 2)
    {
        in_block = receiveReadOnly(1); // read input 1 (modulation)
        if (in_block)
        {
            delay_modulation = delay_modulation_gain_value * in_block->data[0]; // read only the first value
            delay_by_modulation = delay_central_value + delay_modulation;
            delay_by_modulation = constrain(delay_by_modulation, 0, DELAY_FIFO_SAMPLES - AUDIO_BLOCK_SAMPLES);
            
            delay_delta += delay_by_modulation - delay_value;
            if (delay_delta > (AUDIO_BLOCK_SAMPLES - 10))
            {
                delay_delta = AUDIO_BLOCK_SAMPLES - 10;
            }

            release(in_block);
        }
        else
            delay_delta = 0;
    }

    // Se non c'e' alcuna modulazione del delay e non varia delay_central_value, deve essere: delay_value == delay_central_value.
    // potrebbero non coincidere per via di una modulazione precedente, in ogni caso occorre correggere
    if (delay_modulation_source == 0 && J_delay_central_value_counter == 0)
    {
        // si calcola l'eventuale differenza e la si limita a +/- 1 per ciascun update()
        delay_delta = delay_central_value - delay_value;
        delay_delta = constrain(delay_delta, -1, +1);
    }

    if (delay_delta != 0)
    {
        // 0-----(10)------------------(128) 0------(10)-----------------(128) 0---------------------------(128)
        //
        //       Se delay_delta = 0 leggero' 128 samples e trasmettero' 128 samples (quindi pitch = 1.0):
        //       (s_r)rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrR     <-- delay_pitch = 1.0
        //
        //       Se delay_delta = 10 leggero' 10 samples in meno, ma trasmettero' comunque 128 samples (quindi pitch < 1.0):
        //       (s_r)rrrrrrrrrrrrrrrrrrrrrrrrrrrrrR       <-- delay_pitch = (128 - 10)/128 = 0.921875
        //

        delay_pitch = (AUDIO_BLOCK_SAMPLES - (double)delay_delta) / AUDIO_BLOCK_SAMPLES;
        D_sample_read = sample_read; // campione iniziale
        for (auto i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            I_sample_read_f = floor(D_sample_read);
            if (I_sample_read_f >= 0)
            {
                I_sample_read_c = ceil(D_sample_read);
                cache = *(DELAY_fifo + I_sample_read_f) + (*(DELAY_fifo + I_sample_read_c) - *(DELAY_fifo + I_sample_read_f)) * (D_sample_read - I_sample_read_f);
                out_block->data[i] = Lilla_saturate16(cache);
            }
            else
            {
                cache = *(DELAY_fifo + DELAY_FIFO_SAMPLES - 1) + (*(DELAY_fifo + 0) - *(DELAY_fifo + DELAY_FIFO_SAMPLES - 1)) * (D_sample_read - (-1));
                out_block->data[i] = Lilla_saturate16(cache);
            }

            D_sample_read += delay_pitch;

            if (D_sample_read > (double)(DELAY_FIFO_SAMPLES - 1))
            {
                D_sample_read -= (DELAY_FIFO_SAMPLES);
                // D_sample_read potrebbe essere NEGATIVO; look at this case:
                // (-1)oooooooooooooooooooooooooo(0)-------------------------------------------------------------(DELAY_FIFO_SAMPLES - 1)ooooooo(D_sample_read)ooo(DELAY_FIFO_SAMPLES)
                // the new situation is:
                // (-1)ooo(D_sample_read)oooooooo(0)-------------------------------------------------------------(DELAY_FIFO_SAMPLES - 1)ooooooooooooooooooooooooo(DELAY_FIFO_SAMPLES)
            }
        }
        delay_value += delay_delta;

        // Restore values
        delay_delta = 0;
        
        sample_read = D_sample_read; // D_sample_read > -1 -->  sample_read >= 0
        if (sample_read < 0)         // should be useless
        {
            sample_read = 0;
        }
    }
    
    else
    {
        for (auto i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            out_block->data[i] = *(DELAY_fifo + sample_read);
            ++sample_read;
            if (sample_read == DELAY_FIFO_SAMPLES)
            {
                sample_read = 0;
            }
        }
    }
    transmit(out_block);
    release(out_block);

    // Write in_block to delay_Main_Array; execution: 1,5micros @600MHz
    // ***** spostato ****
    in_block = receiveReadOnly(0);
    for (auto i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
    {
        if (!in_block)
        {
            *(DELAY_fifo + sample_write) = 0;
        }
        else
        {
            *(DELAY_fifo + sample_write) = in_block->data[i];
        }

        sample_write++;
        if (sample_write == DELAY_FIFO_SAMPLES)
            sample_write = 0;
    }
    
    if (in_block)
    {
        release(in_block);
    }

    /*
    if(millis()> 10000 && millis()< 12000)
    {
        T[1] = micros() - T[0];
        Serial.print("StereoDelay.h update() lap-time:");
        Serial.println(T[1]);
    }
    */
}

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

void StereoDelay::Constrain_delay_value(int value)
{
    const int maximum = DELAY_FIFO_SAMPLES - AUDIO_BLOCK_SAMPLES;
    value = constrain(value, 0, maximum);
}