/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

/*
 *  questa classe utilizza receiveWritable quindi gli oggetti in ingresso
 *  NON DOVREBBERO avere altri collegamenti
 */

#include "StereoGain.h"

void StereoGain::update(void)
{
    out_block_L = receiveWritable(0);
    out_block_R = receiveWritable(1);
    if (!out_block_L || !out_block_R)
    {
        return;
    }

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
    {
        if (gain_flag_L || gain_flag_R)
        {
            UPDATE_gain();
        }

        cache = gain_L * out_block_L->data[i];
        out_block_L->data[i] = cache;

        cache = gain_R * out_block_R->data[i];
        out_block_R->data[i] = cache;
    }

    transmit(out_block_L, 0);
    transmit(out_block_R, 1);
    release(out_block_L);
    release(out_block_R);
    // out_block_L = NULL;
    // out_block_R = NULL;
}
void StereoGain::Set_gain(float value)
{
    if (abs(value - gain_L) > 0.0001)
    {
        gain_flag_L = true;
        gain_delta_L = (value - gain_L) / SAMPLES_VOLUME;
        gain_step_L = SAMPLES_VOLUME - 1;
    }
    if (abs(value - gain_R) > 0.0001)
    {
        gain_flag_R = true;
        gain_delta_R = (value - gain_R) / SAMPLES_VOLUME;
        gain_step_R = SAMPLES_VOLUME - 1;
    }
}
void StereoGain::set_gain_L(float value)
{
    if (abs(value - gain_L) > 0.0001)
    {
        gain_flag_L = true;
        gain_delta_L = (value - gain_L) / SAMPLES_VOLUME;
        gain_step_L = SAMPLES_VOLUME - 1;
    }
}
void StereoGain::set_gain_R(float value)
{
    if (abs(value - gain_R) > 0.0001)
    {
        gain_flag_R = true;
        gain_delta_R = (value - gain_R) / SAMPLES_VOLUME;
        gain_step_R = SAMPLES_VOLUME - 1;
    }
}
void StereoGain::UPDATE_gain(void)
{
    if (gain_flag_L)
    {
        gain_L += gain_delta_L;
        
        --gain_step_L;
        if (gain_step_L == 0)
        {
            gain_flag_L = false;
        }
    }
    
    if (gain_flag_R)
    {
        gain_R += gain_delta_R;
        
        --gain_step_R;
        if (gain_step_R == 0)
        {
            gain_flag_R = false;
        }
    }
}