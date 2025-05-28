/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "AmpliOutMuteIn.h"

void AmpliOutMuteIn::update(void)
{
    audio_block_t *in;
    audio_block_t *out = NULL;
    unsigned int channel;

    for (channel = 0; channel < 4; channel++)
    {
        if (!out)
        {
            out = receiveWritable(channel);
            if (out)
                Apply_gain(channel, out->data, multiplier[channel]);
        }
        else
        {
            in = receiveReadOnly(channel);
            if (in)
            {
                Apply_gain_then_add(channel, out->data, in->data, multiplier[channel]);
                release(in);
            }
        }
    }
    if (out)
    {
        Apply_gain(out->data, multiplier_out);
        transmit(out);
        release(out);
    }
}

void AmpliOutMuteIn::gain(unsigned int channel, float value)
{
    if (muted[channel])
    {
        gain_runtime_mem[channel] = value;
        return;
    }
    float gain_diff = value - gain_runtime[channel];
    // Serial.print("Lilla_AmpliOut_MuteIn ** channel: ");
    // Serial.print(channel);
    // Serial.print(" - gain_diff: ");
    // Serial.println(gain_diff);

    if (abs(gain_diff) > 0.002)
    {
        gain_flag[channel] = true;
        gain_samples[channel] = abs(gain_diff) / 0.001f;
        if (gain_samples[channel] % 2 != 0)
            gain_samples[channel] -= 1;

        gain_delta[channel] = (gain_diff > 0 ? 0.001f : -0.001f);
        gain_step[channel] = gain_samples[channel];
    }
}

void AmpliOutMuteIn::gain(float value)
{
    float gain_diff = value - gain_runtime_out;
    if (abs(gain_diff) > 0.002)
    {
        gain_flag_out = true;
        gain_samples_out = abs(gain_diff) / 0.001f;
        if (gain_samples_out % 2 != 0)
            gain_samples_out -= 1;

        gain_delta_out = (gain_diff > 0 ? 0.001f : -0.001f);
        gain_step_out = gain_samples_out;
    }
}

void AmpliOutMuteIn::Mute(unsigned int channel)
{
    gain_runtime_mem[channel] = gain_runtime[channel];
    gain(channel, 0.0);
    muted[channel] = true;
}

void AmpliOutMuteIn::unmute(unsigned int channel)
{
    muted[channel] = false;
    gain(channel, gain_runtime_mem[channel]);
}

void AmpliOutMuteIn::Apply_gain(const int channel, int16_t *data, const int32_t mult)
{
    uint32_t *p = (uint32_t *)data;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);
    mult_1 = mult;
    mult_2 = mult;
    do
    {
        if (gain_flag[channel])
            Get_mults(channel);
        uint32_t tmp32 = *p; // read 2 samples from *data
        int32_t val1 = signed_multiply_32x16b(mult_1, tmp32);
        int32_t val2 = signed_multiply_32x16t(mult_2, tmp32);
        val1 = signed_saturate_rshift(val1, 16, 0);
        val2 = signed_saturate_rshift(val2, 16, 0);
        *p++ = pack_16b_16b(val2, val1);
    } while (p < end);
}

void AmpliOutMuteIn::Apply_gain_then_add(const int channel, int16_t *data, const int16_t *in, const int32_t mult)
{
    uint32_t *dst = (uint32_t *)data;
    const uint32_t *src = (uint32_t *)in;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);
    mult_1 = mult;
    mult_2 = mult;

    do
    {
        if (gain_flag[channel])
            Get_mults(channel);
        uint32_t tmp32 = *src++; // read 2 samples from *data
        int32_t val1 = signed_multiply_32x16b(mult_1, tmp32);
        int32_t val2 = signed_multiply_32x16t(mult_2, tmp32);
        val1 = signed_saturate_rshift(val1, 16, 0);
        val2 = signed_saturate_rshift(val2, 16, 0);
        tmp32 = pack_16b_16b(val2, val1);
        uint32_t tmp32b = *dst;
        *dst++ = signed_add_16_and_16(tmp32, tmp32b);
    } while (dst < end);
}

void AmpliOutMuteIn::Apply_gain(int16_t *data, const int32_t mult)
{
    uint32_t *p = (uint32_t *)data;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);
    mult_1 = mult;
    mult_2 = mult;
    do
    {
        if (gain_flag_out)
            Get_mults();
        uint32_t tmp32 = *p; // read 2 samples from *data
        int32_t val1 = signed_multiply_32x16b(mult_1, tmp32);
        int32_t val2 = signed_multiply_32x16t(mult_2, tmp32);
        val1 = signed_saturate_rshift(val1, 16, 0);
        val2 = signed_saturate_rshift(val2, 16, 0);
        *p++ = pack_16b_16b(val2, val1);
    } while (p < end);
}

void AmpliOutMuteIn::Get_mults()
{
    gain_runtime_out += gain_delta_out;
    multiplier_out = gain_runtime_out * 65536.0f;
    mult_1 = multiplier_out;
    gain_runtime_out += gain_delta_out;
    multiplier_out = gain_runtime_out * 65536.0f;
    mult_2 = multiplier_out;
    gain_step_out -= 2;

    if (gain_step_out <= 0)
        gain_flag_out = false;
}

void AmpliOutMuteIn::Get_mults(const int channel)
{
    gain_runtime[channel] += gain_delta[channel];
    multiplier[channel] = gain_runtime[channel] * 65536.0f;
    mult_1 = multiplier[channel];
    gain_runtime[channel] += gain_delta[channel];
    multiplier[channel] = gain_runtime[channel] * 65536.0f;
    mult_2 = multiplier[channel];
    gain_step[channel] -= 2;

    if (gain_step[channel] <= 0)
        gain_flag[channel] = false;
}