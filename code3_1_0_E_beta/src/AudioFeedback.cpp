/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

// Lilla_LiveSampler_Feedback offers:
// - output0 = input0 + gain_float*input1; gain_float changes with no zip-effect

#include "AudioFeedback.h"

void AudioFeedback::update(void)
{
    if (book_change_gain_flag)
    {
        if (!change_gain_flag)
        {
            book_change_gain_flag = false;
            change_gain_flag = true;
            if (gain_target > gain)
                gain_delta = 1; // 64
            else
                gain_delta = -1;                 // -64
            gain_step = abs(gain_target - gain); // multiplo di 128 perche' viene decrementato di 128 ad ogni step di Apply_gain_then_add

            // Serial.print(" gain_step:");
            // Serial.println(gain_step);
        }
    }

    audio_block_t *X_0 = NULL;
    audio_block_t *X_1 = NULL;

    X_0 = receiveWritable(0); // Input steam
    X_1 = receiveReadOnly(1); // Feedback stream

    if (X_0 && X_1)
    {
        transmit(X_0);
        if (change_gain_flag)
        {
            Apply_changing_gain_than_add(X_0->data, X_1->data); // X_0 = X_0 + gain_float*X_1
            gain += 128 * gain_delta;
            gain_step -= 128;

            /*
            Serial.print("Lilla_LiveSampler_Feedback - identity:");
            Serial.print(identity);
            Serial.print(" gain_step:");
            Serial.print(gain_step);
            Serial.print(" gain:");
            Serial.println(gain);
            */

            if (gain_step <= 0)
                change_gain_flag = false;
        }
        else if (gain != 0)
            Apply_gain_then_add(X_0->data, X_1->data); // X_0 = X_0 + X_1
    }
    if (X_0)
        release(X_0);
    if (X_1)
        release(X_1);
}
void AudioFeedback::value(float gain_in)
{
    int gain_target_local = gain_in * MULTI_UNITYGAIN;
    gain_target_local = constrain(gain_target_local, 0, MULTI_UNITYGAIN);
    gain_target_local = 128 * (gain_target_local / 128); // 1 * 128 = 128  -- gain_target_local = 8192 * (gain_target_local / 8192); // 64 * 128samples = 8182

    /*
    Serial.print("Lilla_LiveSampler_Feedback - identity:");
    Serial.print(identity);
    Serial.print(" actual gain_target:");
    Serial.print(gain_target);
    Serial.print(" gain_target_local:");
    Serial.print(gain_target_local);
    Serial.print(" delta:");
    Serial.println(gain_target_local - gain_target);
    */

    if (gain_target_local != gain_target)
    {
        book_change_gain_flag = true;
        gain_target = gain_target_local;
    }
}
void AudioFeedback::Apply_gain_then_add(int16_t *data, const int16_t *in)
{
    uint32_t *dst = (uint32_t *)data;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);
    const uint32_t *src = (uint32_t *)in;
    do
    {
        uint32_t tmp32 = *src++; // read 2 samples from *data
        int32_t val1 = signed_multiply_32x16b(gain, tmp32);
        int32_t val2 = signed_multiply_32x16t(gain, tmp32);
        val1 = signed_saturate_rshift(val1, 16, 0);
        val2 = signed_saturate_rshift(val2, 16, 0);
        tmp32 = pack_16b_16b(val2, val1);
        uint32_t tmp32b = *dst;
        *dst++ = signed_add_16_and_16(tmp32, tmp32b);
    } while (dst < end);
}
void AudioFeedback::Apply_changing_gain_than_add(int16_t *data, const int16_t *in)
{
    uint32_t *dst = (uint32_t *)data;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);
    const uint32_t *src = (uint32_t *)in;
    mult_1 = gain;
    mult_2 = gain;
    gain_local = gain;
    do
    {
        if (change_gain_flag)
        {
            Get_mults();
        }
        uint32_t tmp32 = *src++; // read 2 samples from *data
        int32_t val1 = signed_multiply_32x16b(mult_2, tmp32);
        int32_t val2 = signed_multiply_32x16t(mult_1, tmp32);
        val1 = signed_saturate_rshift(val1, 16, 0);
        val2 = signed_saturate_rshift(val2, 16, 0);
        tmp32 = pack_16b_16b(val2, val1);
        uint32_t tmp32b = *dst;
        *dst++ = signed_add_16_and_16(tmp32, tmp32b);
    } while (dst < end);
}
void AudioFeedback::Get_mults(void)
{
    gain_local += gain_delta;
    mult_1 = gain_local;
    gain_local += gain_delta;
    mult_2 = gain_local;
}