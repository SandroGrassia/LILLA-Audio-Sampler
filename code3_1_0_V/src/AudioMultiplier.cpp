/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "AudioMultiplier.h"

void AudioMultiplier::update(void)
{

    // **  audio_block_t  **
    // Audio blocks are represented with this data type, which is a C struct. The only member intended for use in update() is "data", an array of 16 bit integers representing the audio.
    // If "myblock" is a pointer to an audio_block_t, use myblock->data[0] to access the first audio sample, myblock->data[1] to access the second, and so on.
    // The data[] array is always 32 bit aligned in memory, so you can fetch pairs of samples by type casting the address as a pointer to 32 bit data.

    audio_block_t *blocka, *blockb;
    uint32_t *pa, *pb, *end;
    uint32_t a12, a34; //, a56, a78;
    uint32_t b12, b34; //, b56, b78;

    blocka = receiveWritable(0);
    blockb = receiveReadOnly(1);

    // **  release  **
    // Your update function has ownership of all audio blocks obtained by receiveReadOnly(), receiveWritable(), and allocate(). You must call release() for every non-NULL pointer obtained from those 3 functions

    if (!blocka)
    {
        if (blockb)
        {
            release(blockb);
        }
        return;
    }
    if (!blockb)
    {
        release(blocka);
        return;
    }
    pa = (uint32_t *)(blocka->data);
    pb = (uint32_t *)(blockb->data);
    end = pa + AUDIO_BLOCK_SAMPLES / 2;
    while (pa < end)
    {
        a12 = *pa;
        a34 = *(pa + 1);
        b12 = *pb++;
        b34 = *pb++;
        a12 = pack_16b_16b(signed_saturate_rshift(multiply_16tx16t(a12, b12), 16, 15), signed_saturate_rshift(multiply_16bx16b(a12, b12), 16, 15));
        a34 = pack_16b_16b(signed_saturate_rshift(multiply_16tx16t(a34, b34), 16, 15), signed_saturate_rshift(multiply_16bx16b(a34, b34), 16, 15));
        *pa++ = a12;
        *pa++ = a34;
    }
    transmit(blocka);
    release(blocka);
    release(blockb);
}