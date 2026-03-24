#include "LillaClock.h"

void LillaClock::update(void)
{
    if (!stop_flag)
    {
        if (identity == 0)
        {
            Filter_Biquad_Manager_ptr->Update();
            Delay_Manager_ptr->Update();
            Midi_reader_ptr->Update();
        }
        else
        {
            Midi_reader_ptr->Update();
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