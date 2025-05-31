/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "Mixer_2x1.h"

void Mixer_2x1::update(void) // execution:  micros @600MHz
{
    // T[0] = micros();
    for (int in = 0; in < 2; in++)
    {
        if (!out_block)
        {
            out_block = receiveWritable(in);
        }
        else
        {
            in_block = receiveReadOnly(in);
            if (in_block)
            {
                Add(out_block->data, in_block->data);
                release(in_block);
            }
        }
    }
    if (out_block)
    {
        transmit(out_block);
        release(out_block);
    }
    out_block = NULL; // DO NOT delete!

    /*
    T[1]=micros()-T[0];
    if(millis()>6000 && millis()<10000)
    {
        Serial.print("M2x1-");
        Serial.println(T[1]);
    }
    */
}

void Mixer_2x1::Add(int16_t *data, const int16_t *in)
{
    uint32_t *dst = (uint32_t *)data;
    const uint32_t *src = (uint32_t *)in;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);
    int cicli = 0;

    /*
    Ogni ciclo do copia 4 campioni dall'ingresso all'uscita, con due esecuzioni di somma

    dati i sample di un blocco di sample in uscita:
    [*data, *(data+1), *(data+2),......................., *(data + AUDIO_BLOCK_SAMPLES - 1)]

    e i sample di un blocco di sample in ingresso:
    [*in, *(in+1), *(in+2),......................., *(in + AUDIO_BLOCK_SAMPLES - 1)]

    inizializzo *dst come puntatore a coppie dell'uscita:
    uint32_t *dst = (uint32_t *)data == [*data, *(data+1)]

    e *src come puntatore a coppie dell'ingresso:
    uint32_t *src = (uint32_t *)in == [*in, *(in+1)]

    1° somma
    prendo una variabile di appoggio:
    uint32_t tmp32 = *dst == [*data, *(data+1)]

    poi eseguo la somma (con saturazione gia' inclusa):
    *dst = signed_add_16_and_16(tmp32, *src) -->  [*data + *in, *(data+1) + *(in+1)]

    poi incremento dst e src (seconda coppia di campioni d'ingresso e seconda coppia di campioni d'uscita)

    2° somma
    di nuovo:
    tmp32 = *dst == [*data+2, *(data+3)]

    poi eseguo la somma (con saturazione gia' inclusa):
    *dst = signed_add_16_and_16(tmp32, *src) -->  [*data+2 + *in+2, *(data+3) + *(in+3)]

    poi incremento dst e src e riparto
    */

    do
    {
        uint32_t tmp32 = *dst;
        *dst++ = signed_add_16_and_16(tmp32, *src++); // uint32_t signed_add_16_and_16(uint32_t a, uint32_t b) computes (((a[31:16] + b[31:16]) << 16) | (a[15:0 + b[15:0]))  (saturates)
        tmp32 = *dst;
        *dst++ = signed_add_16_and_16(tmp32, *src++); // uint32_t signed_add_16_and_16(uint32_t a, uint32_t b) computes (((a[31:16] + b[31:16]) << 16) | (a[15:0 + b[15:0]))  (saturates)
        cicli++;
    } while (dst < end);
}