/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "Router_16x3.h"

// execution: 17.5micros @600MHz
void Router_16x3::update(void)
{
    // T[0] = micros();

    audio_block_t *in_block;
    audio_block_t *out_block[3] = {NULL};

    out_block[0] = allocate();
    out_block[1] = allocate();
    out_block[2] = allocate();

    uint64_t *killer[3];
    killer[0] = (uint64_t *)(out_block[0]->data);
    killer[1] = (uint64_t *)(out_block[1]->data);
    killer[2] = (uint64_t *)(out_block[2]->data);
    for (uint8_t i = 0; i < 32; ++i)
    {
        *(killer[0] + i) = 0;
        *(killer[1] + i) = 0;
        *(killer[2] + i) = 0;
    }

    for (uint8_t in = 0; in < CH_IN; ++in)
    {
        in_block = receiveReadOnly(in);
        if (in_block)
        {
            if (routing_MX[in] > 1) // 2 e 3 --> l'ingresso va ruotato verso l'Audio Board (out0 o out1 definito da routing_table)
            {
                Add(out_block[routing_table[in]]->data, in_block->data);
            }
            if (routing_MX[in] == 1 || routing_MX[in] == 3) // 1 e 3 --> l'ingresso va ruotato verso il monitor-PWM (out2)
            {
                Add(out_block[2]->data, in_block->data);
            }
        }
        release(in_block);
        in_block = NULL;
    }

    for (uint8_t out = 0; out < CH_OUT; ++out)
    {
        transmit(out_block[out], out);
        release(out_block[out]);
        out_block[out] = NULL;
    }

    // T[1] = micros()-T[0];
    // if(millis()>20000 && millis()<22000)
    // Serial.println(T[1]);
}

void Router_16x3::Add(int16_t *data, const int16_t *in)
{
    uint32_t *dst = (uint32_t *)data;
    const uint32_t *src = (uint32_t *)in;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

    do
    {
        uint32_t tmp32 = *dst;
        *dst++ = signed_add_16_and_16(tmp32, *src++); // uint32_t signed_add_16_and_16(uint32_t a, uint32_t b) computes (((a[31:16] + b[31:16]) << 16) | (a[15:0 + b[15:0]))  gestendo la saturazione
        tmp32 = *dst;
        *dst++ = signed_add_16_and_16(tmp32, *src++);
    } while (dst < end);
}