#include "LillaClock.h"

void LillaClock::update(void)
{
    if (!stop_flag)
    {
        if (identity == 0)
        {
            _Filter_Biquad_Manager->Update();
            _Delay_Manager->Update();
            _Midi_reader->Update();
        }
        else
        {
            _Midi_reader->Update();
        }
    }
}

void LillaClock::Start(void)
{
    stop_flag = false;
}

void LillaClock::Stop(void)
{
    stop_flag = true;
}