/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "AudioPeakDetector.h"

void AudioPeakDetector::update(void)
{
    audio_block_t *block;
    const int16_t *p, *end;
    int32_t min, max;

    block = receiveReadOnly();
    if (!block)
    {
        return;
    }
    p = block->data;
    end = p + AUDIO_BLOCK_SAMPLES;
    min = min_sample;
    max = max_sample;
    do
    {
        int16_t d = *p++;
        // TODO: can we speed this up with SSUB16 and SEL
        // http://www.m4-unleashed.com/parallel-comparison/
        if (d < min)
            min = d;
        if (d > max)
            max = d;
    } while (p < end);
    min_sample = min;
    max_sample = max;
    new_output = true;
    release(block);
}
bool AudioPeakDetector::available(void)
{
    __disable_irq();
    bool flag = new_output;
    if (flag)
        new_output = false;
    __enable_irq();
    return flag;
}
float AudioPeakDetector::read(void)
{
    __disable_irq();
    int min = min_sample;
    int max = max_sample;
    min_sample = 32767;
    max_sample = -32768;
    __enable_irq();
    min = abs(min);
    max = abs(max);
    if (min > max)
        max = min;
    return max / 32767.0f;
}
void AudioPeakDetector::reset()
{
    __disable_irq();
    min_sample = 0;
    max_sample = 0;
    new_output = true;
    __enable_irq();
}