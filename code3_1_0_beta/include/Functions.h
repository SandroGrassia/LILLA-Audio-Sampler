/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include <utility/dspinst.h>
#include <stdint.h>

template <class T>
int MIN_index(T *array, int elements)
{
    T min_element = *array;
    int answer = 0;
    for (int i = 1; i < elements; i++)
    {
        if (*(array + i) < min_element)
        {
            min_element = *(array + i);
            answer = i;
        }
    }
    return answer;
}

// calcola mult x (a + b) dove a e b sono coppie adiacenti di campioni (int16_t)
static inline uint32_t Muliply_sum(const float &mult, const uint32_t &a, const uint32_t &b) __attribute__((always_inline, unused));
static inline uint32_t Muliply_sum(const float &mult, const uint32_t &a, const uint32_t &b)
{
    int32_t Mult = mult * 65536;                      // 1.0 * 65536 moltiplica per 1.0
    uint32_t r = signed_add_16_and_16(a, b);          // computes (((a[31:16] + b[31:16]) << 16) | (a[15:0 + b[15:0])) include calcolo saturazione
    int32_t val_lo = signed_multiply_32x16t(Mult, r); // computes ((mult[31:0] x *_r[31:16]) >> 16) --> mult x r[0]
    int32_t val_hi = signed_multiply_32x16b(Mult, r); // computes ((mult[31:0] * *_r[15:0]) >> 16)  --> mult x r[1]
    val_lo = signed_saturate_rshift(val_lo, 16, 0);
    val_hi = signed_saturate_rshift(val_hi, 16, 0);
    return pack_16b_16b(val_lo, val_hi);
}

// computes limit(val, 2**bits)
static inline int16_t Lilla_saturate16(const int32_t &val) __attribute__((always_inline, unused));
static inline int16_t Lilla_saturate16(const int32_t &val)
{
    int16_t out;
    int32_t tmp;
    asm volatile("ssat %0, %1, %2" : "=r"(tmp) : "I"(16), "r"(val));
    out = (int16_t)(tmp);
    return out;
}