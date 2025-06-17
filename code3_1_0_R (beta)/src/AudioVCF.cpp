/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// AudioVCF.h does NOT inherit from AudioStream.h

#include "AudioVCF.h"

void AudioVCF::Update(void)
{
    // FILTER execution: n.1 stage: 5micros, n.2 stages: 9micros, n.3 stages: 12micros , n.4 stages: 15micros
    int32_t b0, b1, b2, a1, a2, sum;
    uint32_t in2, out2, bprev, aprev, flag;
    uint32_t *data, *last;
    uint32_t *step_1 = NULL, *step_2 = NULL, *step_3 = NULL;
    int32_t *state;

    last = (uint32_t *)(_block) + AUDIO_BLOCK_SAMPLES / 2;
    if (LFO_connect)
    {
        step_1 = (uint32_t *)(_block) + AUDIO_BLOCK_SAMPLES / 8;
        step_2 = step_1 + AUDIO_BLOCK_SAMPLES / 8;
        step_3 = step_2 + AUDIO_BLOCK_SAMPLES / 8;
    }

    state = &definition[0]; // (uint32_t *)definition
    do
    {
        b0 = *state++;         // b0 pointer to definition[1]
        b1 = *state++;         // b1 pointer to definition[2]
        b2 = *state++;         // b2 pointer to definition[3]
        a1 = *state++;         // b3 pointer to definition[4]
        a2 = *state++;         // b4 pointer to definition[5]
        bprev = *state++;      // bprev pointer to definition[6]
        aprev = *state++;      // aprev pointer to definition[7]
        sum = *state & 0x3FFF; // 14 1s: 0011111111111111

        // create a 32bit pointer to the first 32-bit-element of input queue block->data
        data = last - AUDIO_BLOCK_SAMPLES / 2;
        // Serial.print("f0:");
        // Serial.println(*(_frequency));

        do
        {
            in2 = *data;
            sum = signed_multiply_accumulate_32x16b(sum, b0, in2); // computes (sum + ((a[31:0] * b[31:16]) >> 16))
            sum = signed_multiply_accumulate_32x16t(sum, b1, bprev);
            sum = signed_multiply_accumulate_32x16b(sum, b2, bprev);
            sum = signed_multiply_accumulate_32x16t(sum, a1, aprev);
            sum = signed_multiply_accumulate_32x16b(sum, a2, aprev);
            out2 = signed_saturate_rshift(sum, 16, 14);
            sum &= 0x3FFF;
            sum = signed_multiply_accumulate_32x16t(sum, b0, in2);
            sum = signed_multiply_accumulate_32x16b(sum, b1, in2);
            sum = signed_multiply_accumulate_32x16t(sum, b2, bprev);
            sum = signed_multiply_accumulate_32x16b(sum, a1, out2);
            sum = signed_multiply_accumulate_32x16t(sum, a2, aprev);
            bprev = in2;
            aprev = pack_16b_16b(signed_saturate_rshift(sum, 16, 14), out2);
            // retaining part of the sum is meant to implement the "first order noise shaping" described in this article:
            // http://www.earlevel.com/main/2003/02/28/biquads/
            sum &= 0x3FFF;
            bprev = in2;
            *data++ = aprev;

            if (LFO_connect)
            {
                if (data == step_1)
                {
                    Set_filter(0, *(_frequency + 1));
                    // Serial.print(" f1:");
                    // Serial.print(*(_frequency + 1));
                }
                else if (data == step_2)
                {
                    Set_filter(0, *(_frequency + 2));
                    // Serial.print(" f2:");
                    // Serial.print(*(_frequency + 2));
                }
                else if (data == step_3)
                {
                    Set_filter(0, *(_frequency + 3));
                    // Serial.print(" f3:");
                    // Serial.println(*(_frequency + 3));
                }
            }
        } while (data < last);
        flag = *state & 0x80000000;
        *state++ = sum | flag;
        *(state - 2) = aprev;
        *(state - 3) = bprev;
    } while (flag);
}

// Set the biquad coefficients directly
void AudioVCF::setCoefficients(uint32_t stage, const int *coefficients)
{
    if (stage >= 4)
    {
        return;
    }
    int32_t *dest = definition + (stage << 3);
    // __disable_irq();
    if (stage > 0)
        *(dest - 1) |= 0x80000000;
    *dest++ = *coefficients++;
    *dest++ = *coefficients++;
    *dest++ = *coefficients++;
    *dest++ = *coefficients++ * -1;
    *dest++ = *coefficients++ * -1;
    //*dest++ = 0;
    //*dest++ = 0;  // clearing filter state causes loud pop
    dest += 2;
    *dest &= 0x80000000;
    // __enable_irq();
}

void AudioVCF::setCoefficients(uint32_t stage, const double *coefficients)
{
    int coef[5];
    coef[0] = coefficients[0] * 1073741824.0;
    coef[1] = coefficients[1] * 1073741824.0;
    coef[2] = coefficients[2] * 1073741824.0;
    coef[3] = coefficients[3] * 1073741824.0;
    coef[4] = coefficients[4] * 1073741824.0;
    setCoefficients(stage, coef);
}

// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
// execution: 3micros @600MHz
void AudioVCF::Set_filter(uint32_t stage, float frequency)
{
    // Lowpass
    if (filter_type == 0)
    {
        int coef[5];
        double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
        double sinW0 = sin(w0);
        double alpha = sinW0 / ((double)q_value * 2.0);
        double cosW0 = cos(w0);
        double scale = 1073741824.0 / (1.0 + alpha);
        /* b0 */ coef[0] = ((1.0 - cosW0) / 2.0) * scale;
        /* b1 */ coef[1] = (1.0 - cosW0) * scale;
        /* b2 */ coef[2] = coef[0];
        /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
        /* a2 */ coef[4] = (1.0 - alpha) * scale;
        setCoefficients(stage, coef);
    }

    // Highpass
    else if (filter_type == 1)
    {
        int coef[5];
        double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
        double sinW0 = sin(w0);
        double alpha = sinW0 / ((double)q_value * 2.0);
        double cosW0 = cos(w0);
        double scale = 1073741824.0 / (1.0 + alpha);
        /* b0 */ coef[0] = ((1.0 + cosW0) / 2.0) * scale;
        /* b1 */ coef[1] = -(1.0 + cosW0) * scale;
        /* b2 */ coef[2] = coef[0];
        /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
        /* a2 */ coef[4] = (1.0 - alpha) * scale;
        setCoefficients(stage, coef);
    }

    // Bandpass
    else if (filter_type == 2)
    {
        int coef[5];
        double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
        double sinW0 = sin(w0);
        double alpha = sinW0 / ((double)q_value * 2.0);
        double cosW0 = cos(w0);
        double scale = 1073741824.0 / (1.0 + alpha);
        /* b0 */ coef[0] = alpha * scale;
        /* b1 */ coef[1] = 0;
        /* b2 */ coef[2] = (-alpha) * scale;
        /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
        /* a2 */ coef[4] = (1.0 - alpha) * scale;
        setCoefficients(stage, coef);
    }

    // Notch
    if (filter_type == 3)
    {
        int coef[5];
        double w0 = frequency * (2 * 3.141592654 / AUDIO_SAMPLE_RATE_EXACT);
        double sinW0 = sin(w0);
        double alpha = sinW0 / ((double)q_value * 2.0);
        double cosW0 = cos(w0);
        double scale = 1073741824.0 / (1.0 + alpha);
        /* b0 */ coef[0] = scale;
        /* b1 */ coef[1] = (-2.0 * cosW0) * scale;
        /* b2 */ coef[2] = coef[0];
        /* a1 */ coef[3] = (-2.0 * cosW0) * scale;
        /* a2 */ coef[4] = (1.0 - alpha) * scale;
        setCoefficients(stage, coef);
    }
}