/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#include "DisplayManager.h"

void DisplayManager::Lilla_cover_slow(void)
{
    tft.fillScreen(ILI9341_BLACK);

    // fade-in
    for (auto i = 0; i <= 10; ++i)
    {
        Logo(static_cast<float>(i) / 10.0f);
        delay(50);
    }
    for (auto i = 0; i <= 20; ++i)
    {
        Cover_text(static_cast<float>(i) / 20.0f);
        delay(50);
    }

    // fade-out
    delay(4000);
    for (auto i = 50; i >= 0; --i)
    {
        Cover_text(i / 50.0f);
        delay(30);
    }
    for (auto i = 20; i >= 0; --i)
    {
        Logo(static_cast<float>(i) / 20.0f);
        delay(30);
    }

    // all black
    tft.fillScreen(ILI9341_BLACK);
    delay(200);
}

void DisplayManager::Lilla_cover_saturate(void)
{
    for (auto i = 0; i <= 20; ++i)
    {
        tft.fillScreen(Calc_color(ILI9341_WHITE, static_cast<float>(i) / 20.0f));
        Cover_text(1);
        Logo(1);
        delay(50);
    }

    // all black
    tft.fillScreen(ILI9341_BLACK);
    delay(200);
}

FLASHMEM
void DisplayManager::P_Show_Patch_number(bool change_patch)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_PATCH), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PATCH");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_Patch_id), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(change_patch ? ILI9341_YELLOW : ILI9341_WHITE);
    tft.print(Patch_id);
}

FLASHMEM
void DisplayManager::P_Patch_volume(bool change_vol)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_VOLUME), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME");
    P_Patch_volume_value(change_vol);
}

FLASHMEM
void DisplayManager::P_Patch_volume_value(bool change_vol)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(P_column_Volume_value), DisplayPrimitives::display_coordinate_y(0), 4);
    tft.setTextColor(ILI9341_YELLOW); // tft.setTextColor(change_vol ? ILI9341_YELLOW : ILI9341_WHITE);
    tft.print(volume_patch / 20.0f, 2);
}

FLASHMEM
void DisplayManager::Loop_REC_advice(int track, bool on)
{
    if (on)
    {
        tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_LOOPS_X + 7 * track), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R));
        tft.setTextColor(ILI9341_WHITE);
        tft.print(track + 1);
        tft.print("-REC");
    }
    else
    {
        Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Loop_LOOPS_X + 7 * track), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R), 5);
    }
}

void DisplayManager::P_show_delete_Instrument_frame(float line, bool show)
{
    tft.drawRect(P_column_Instrument_frame, P_Instrument_pixels_y(line) - 4, P_chars_width_Instrument_frame, Frame_heigh, (show ? FRAME_COLOR : ILI9341_BLACK)); // drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
}

void DisplayManager::Led_PERFORMANCE_instrument(int instrument_id, bool on)
{
    if (on)
    {
        tft.drawBitmap(P_pixel_x_LED, P_Instrument_pixels_y(P_line_of_instrument[instrument_id]), led_pic, 8, 8, (MX_mute[instrument_id] ? RED_ON : GREEN_ON));
    }
    else
    {
        tft.drawBitmap(P_pixel_x_LED, P_Instrument_pixels_y(P_line_of_instrument[instrument_id]), led_pic, 8, 8, (MX_mute[instrument_id] ? RED_OFF : GREEN_OFF));
    }
}

void DisplayManager::Led_SOUND_EDIT_instrument(int instrument_id, bool on)
{
    if (on)
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(22) - 4, DisplayPrimitives::display_coordinate_y(0), led_pic, 6, 8, (MX_mute[instrument_id] ? RED_ON : GREEN_ON));
    }
    else
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(22) - 4, DisplayPrimitives::display_coordinate_y(0), led_pic, 8, 8, (MX_mute[instrument_id] ? RED_OFF : GREEN_OFF));
    }
    return;
}

void DisplayManager::Led_INSTRUMENT_VCF_instrument(int instrument_id, bool on)
{
    if (Lilla_state_0 == PERFORMANCE)
    {
        if (on)
        {
            tft.drawBitmap(DisplayPrimitives::display_coordinate_x(22) - 4, DisplayPrimitives::display_coordinate_y(0), led_pic, 6, 8, (MX_mute[instrument_id] ? RED_ON : GREEN_ON));
        }
        else
        {
            tft.drawBitmap(DisplayPrimitives::display_coordinate_x(22) - 4, DisplayPrimitives::display_coordinate_y(0), led_pic, 8, 8, (MX_mute[instrument_id] ? RED_OFF : GREEN_OFF));
        }
    }
    else if (Lilla_state_0 == LIVE_SAMPLING)
    {
        if (on)
        {
            tft.drawBitmap(DisplayPrimitives::display_coordinate_x(40) - 4, DisplayPrimitives::display_coordinate_y(0), led_pic, 6, 8, (MX_mute[instrument_id] ? RED_ON : GREEN_ON));
        }
        else
        {
            tft.drawBitmap(DisplayPrimitives::display_coordinate_x(40) - 4, DisplayPrimitives::display_coordinate_y(0), led_pic, 8, 8, (MX_mute[instrument_id] ? RED_OFF : GREEN_OFF));
        }
    }
}

void DisplayManager::Led_DIRECT_SAMPLING(bool on)
{
    // PATCHES_MAX ha 2 instrument
    if (on)
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8), led_pic, 6, 8, ((MX_mute[0] && MX_mute[1]) ? RED_ON : GREEN_ON));
    }
    else
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8), led_pic, 8, 8, ((MX_mute[0] && MX_mute[1]) ? RED_OFF : GREEN_OFF));
    }
    return;
}

void DisplayManager::Led_tuning_tone(int patch_id)
{
    if (Lilla_state == PERFORMANCE)
    {
        tft.drawBitmap(P_pixel_x_LED, P_Instrument_pixels_y(Patch[patch_id].instruments), led_pic, 8, 8, (TT_playing ? ILI9341_RED : RED_OFF));
    }
}

FLASHMEM
void DisplayManager::P_swow_pointer_frame(P_field_description_struct value, bool show)
{
    switch (value.field_name)
    {
    case field_Menu:
        DisplayPrimitives::Frame_by_col_row(P_column_Menu_element[value.element], P_row_Menu_element[value.element], dimension_voice_Menu_P[element_Menu_P[value.element]], show);
        break;

    case field_Patch:
        DisplayPrimitives::Frame_by_col_row(P_column_Patch_id, 0, 3, show);
        break;

    case field_Instrument:
        DisplayPrimitives::Frame_by_pixels(DisplayPrimitives::display_coordinate_x(P_column_Instrument_frame), P_Instrument_pixels_y(value.instrument_line), P_chars_width_Instrument_frame, show);
        break;

    case field_Instrument_inside:
        DisplayPrimitives::Frame_by_pixels(DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[value.element]), P_Instrument_pixels_y(value.instrument_line), P_chars_Instrument_element[value.element], show);
        break;
    }
}

FLASHMEM
void DisplayManager::P_Performance_page(bool change_patch, bool change_vol)
{
    instrument_editing_flag = false;
    P_Patch_header(change_patch, change_vol);
    P_Performance_menu(); // displays the menu and updates P_menu_max
    P_show_Instruments_header();
    P_show_all_Instruments(Patch_id);
}

FLASHMEM
void DisplayManager::P_Show_Performance(void)
{
    DisplayPrimitives::Board(0, 0, 11); // Display.DisplayPrimitives::Board(float col, float row, int chars)
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("PERFORMANCE");
}

FLASHMEM
void DisplayManager::Show_effects()
{
    float Y_EFF;

    if (Lilla_state == MIDI_LOOP)
    {
        Y_EFF = 1.5;
        tft.setTextColor(TEXT_COLOR);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(30), DisplayPrimitives::display_coordinate_y(Y_EFF));
        tft.print("RESOLUTION");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(30), DisplayPrimitives::display_coordinate_y(Y_EFF + 1));
        tft.print("DOWNSAMPLING");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Y_EFF + 1));
        tft.print("LPF CUTOFF");
    }

    else
    {
        Y_EFF = 2.5;
        DisplayPrimitives::Delete_row(Y_EFF);
        tft.setTextColor(TEXT_COLOR);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Y_EFF));
        tft.print("RESOLUTION");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(30), DisplayPrimitives::display_coordinate_y(Y_EFF));
        tft.print("DOWNSAMPLING");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Y_EFF + 1));
        tft.print("LPF CUTOFF");
    }

    Resolution();
    Downsampling();
    Lowpass_filter();
}

FLASHMEM
void DisplayManager::Resolution(void)
{
    float Y_EFF;

    if (Lilla_state == MIDI_LOOP)
    {
        Y_EFF = 1.5;
        Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(43), DisplayPrimitives::display_coordinate_y(Y_EFF), 9);
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(resolution_value[resolution], 1);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("bits");
        return;
    }

    Y_EFF = 2.5;
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(11), DisplayPrimitives::display_coordinate_y(Y_EFF), 8);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(resolution_value[resolution], 1);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("bits");
}

FLASHMEM
void DisplayManager::Downsampling(void)
{
    const float Y_EFF = 2.5;

    if (Lilla_state == MIDI_LOOP)
    {
        Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(43), DisplayPrimitives::display_coordinate_y(Y_EFF), 9);
        tft.setTextColor(ILI9341_YELLOW);
        float down = AUDIO_SAMPLE_RATE / downsampling;

        if (down < 1000)
        {
            tft.print(down, 0);
            tft.setTextColor(ILI9341_ORANGE);
            tft.print("Hz");
        }
        else
        {
            tft.print(down / 1000.0f, 3);
            tft.setTextColor(ILI9341_ORANGE);
            tft.print("kHz");
        }

        return;
    }

    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(43), DisplayPrimitives::display_coordinate_y(Y_EFF), 9);
    tft.setTextColor(ILI9341_YELLOW);
    float down = AUDIO_SAMPLE_RATE / downsampling;

    if (down < 1000)
    {
        tft.print(down, 0);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("Hz");
    }
    else
    {
        tft.print(down / 1000.0f, 3);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("kHz");
    }
}

FLASHMEM
void DisplayManager::Lowpass_filter(void)
{
    float Y_EFF = (Lilla_state == MIDI_LOOP ? 1.5 : 2.5);
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(11), DisplayPrimitives::display_coordinate_y(Y_EFF + 1), 7);

    float F = lowpass_value[lowpass_target];
    tft.setTextColor(ILI9341_YELLOW);

    if (F > 9999)
    {
        tft.print(F / 1000, 0);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("kHz");
    }
    else if (F > 999)
    {
        tft.print(F / 1000.0f, 2);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("kHz");
    }
    else
    {
        tft.print(F, 0);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("Hz");
    }
}

FLASHMEM
void DisplayManager::Show_sound(int patch_id, int instrument_id)
{
    tft.fillScreen(ILI9341_BLACK);

    if (Lilla_state_0 == MIDI_LOOP)
    {
        Loop_midi_loop_title();
    }
    else
    {
        P_Show_Performance();
    }

    P_Show_Patch_number(false);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(23), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SOUND");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(28.5), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(instrument_id + 1);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(38), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("FILE");
    File(instrument_id);

    Show_effects();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MIDI CHANNEL");
    Midi_channel(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(18), DisplayPrimitives::display_coordinate_y(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PITCH");
    Pitch(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(31), DisplayPrimitives::display_coordinate_y(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("GAIN");
    Gain_sound(patch_id, instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(42), DisplayPrimitives::display_coordinate_y(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PAN");
    Pan(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("ATT");
    Attack(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(18), DisplayPrimitives::display_coordinate_y(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("DEC");
    Decay(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(31), DisplayPrimitives::display_coordinate_y(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SUS");
    Sustain(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(42), DisplayPrimitives::display_coordinate_y(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("REL");
    Release(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PLAY MODE");
    Play_mode(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(31), DisplayPrimitives::display_coordinate_y(6.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("NOCLICK");
    Noclick(instrument_id, true);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(15));
    tft.setTextColor(TEXT_COLOR);
    tft.print("TRIM STEP");
    Trim_step();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(28), DisplayPrimitives::display_coordinate_y(15));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MAX PITCH/VOICES");
    Pitch_voices_max(instrument_id);
}

FLASHMEM
void DisplayManager::File(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(43), DisplayPrimitives::display_coordinate_y(0), 7);
    tft.setTextColor((file_midi_ch_flag ? ILI9341_YELLOW : ILI9341_WHITE));
    tft.print(name_file[Preset[instrument_id].file]);
}

FLASHMEM
void DisplayManager::Midi_channel(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(12.5), DisplayPrimitives::display_coordinate_y(4.9), 2);
    tft.setTextColor((!file_midi_ch_flag ? ILI9341_YELLOW : ILI9341_WHITE));
    tft.print(Preset[instrument_id].midi_channel + 1);
}
FLASHMEM
void DisplayManager::Pitch(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(23.5), DisplayPrimitives::display_coordinate_y(4.9), 5);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument_id].pitch, 3);
}

FLASHMEM
void DisplayManager::Gain_sound(int patch_id, int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(35.5), DisplayPrimitives::display_coordinate_y(4.9), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Sound[Sound_Id(patch_id, instrument_id)].gain / 20.0);
}

FLASHMEM
void DisplayManager::Pan(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(45.5), DisplayPrimitives::display_coordinate_y(4.9), 4);
    tft.setTextColor(ILI9341_YELLOW);

    if (Preset[instrument_id].pan < 0)
    {
        tft.print("L");
    }
    else if (Preset[instrument_id].pan > 0)
    {
        tft.print("R");
    }

    tft.print(abs(Preset[instrument_id].pan));
}

FLASHMEM
void DisplayManager::Attack(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(3.5), DisplayPrimitives::display_coordinate_y(5.9), 10);
    tft.setTextColor(ILI9341_YELLOW);

    switch (Preset[instrument_id].attack_type)
    {
    case 0:
        tft.print("SLOW "); // Slow
        break;
    case 1:
        tft.print("FAST "); // Fast
        break;
    default:
        break;
    }

    tft.setCursor(DisplayPrimitives::display_coordinate_x(8), DisplayPrimitives::display_coordinate_y(5.9));
    tft.print(Preset[instrument_id].attack, 2);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayManager::Decay(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(21.5), DisplayPrimitives::display_coordinate_y(5.9), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument_id].decay, 2);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayManager::Sustain(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(34.5), DisplayPrimitives::display_coordinate_y(5.9), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument_id].sustain * 100, 0);
    tft.print("%");
}

FLASHMEM
void DisplayManager::Release(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(45.5), DisplayPrimitives::display_coordinate_y(5.9), 5);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument_id].release, 1);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayManager::Play_mode(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(9.5), DisplayPrimitives::display_coordinate_y(6.9), 13);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(loop_mode[Preset[instrument_id].mode]);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(14), DisplayPrimitives::display_coordinate_y(6.9));
    tft.print(name_mode[Preset[instrument_id].mode]);
}

FLASHMEM
void DisplayManager::Noclick(int instrument_id, bool value)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(38.5), DisplayPrimitives::display_coordinate_y(6.9), 4);
    tft.setTextColor((value ? ILI9341_YELLOW : ILI9341_WHITE));
    tft.print(Preset[instrument_id].Noclick);
    DisplayPrimitives::Unit("S", 1);
}

FLASHMEM
void DisplayManager::Trim_step(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(9.5), DisplayPrimitives::display_coordinate_y(15), 6);
    tft.setTextColor(ILI9341_YELLOW);

    switch (trim_speed)
    {
    case 0:
        tft.print("1");
        break;
    case 1:
        tft.print("10");
        break;
    case 2:
        tft.print("100");
        break;
    case 3:
        tft.print("1K");
        break;
    case 4:
        tft.print("10K");
        break;
    case 5:
        tft.print("TOT/16");
        break;
    default:
        break;
    }
}

FLASHMEM
void DisplayManager::Pitch_voices_max(int instrument_id) // max pitch related to which media is read
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(44.5), DisplayPrimitives::display_coordinate_y(15), 8);
    tft.setTextColor(ILI9341_WHITE);

    if (Preset[instrument_id].file < FIRST_LIVE_SAMPLING_FILE)
    {
        if (Preset[instrument_id].use_Wavetable)
        {
            tft.print(MAX_PITCH_WAVETABLE);
        }
        else
        {
            tft.print(MAX_PITCH_FLASH[optimization]);
        }
    }
    else
    {
        tft.print(MAX_PITCH_PSRAM);
    }

    tft.print("/");

    if (Preset[instrument_id].file < FIRST_LIVE_SAMPLING_FILE)
    {
        if (Preset[instrument_id].use_Wavetable)
        {
            tft.print("16");
        }
        else
        {
            tft.print(POLYPHONY_FLASH[optimization]);
        }
    }
    else
        tft.print("16");
}

FLASHMEM
void DisplayManager::LOOP_page(void)
{
    tft.fillScreen(ILI9341_BLACK);
    Loop_midi_loop_title(); // title
    Loop_menu();
    P_Show_Patch_number(Patch_id);
    P_Patch_volume(true);
    Show_effects();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(18), DisplayPrimitives::display_coordinate_y(0));
    tft.print("LOOP");
    Loop_loop_id();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_HEAD_C - 1), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R - 1));
    tft.print("METRO");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_HEAD_C - 1), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R));
    tft.print("TRACK");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_HEAD_C - 1), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R + 1));
    tft.print("SLIDE");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_HEAD_C - 2), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R + 2));
    tft.print("TRANSP");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_HEAD_C - 1), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R + 3));
    tft.print("LEVEL");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_HEAD_C - 1), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R + 4));
    tft.print("SOUND");

    tft.setTextColor(ILI9341_WHITE);

    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        if (Patch[Patch_id].Instrument[instrument_id].used)
        {
            tft.setCursor(DisplayPrimitives::display_coordinate_x(Loop_HEAD_C + 3), 151 + instrument_id * 11);
            tft.print(instrument_id + 1);
        }
    }

    for (auto track = 0; track < TRACKS; ++track)
    {
        Loop_track_data(track);
    }

    // show LOOP_time
    Loop_time_stretched();
}

FLASHMEM
void DisplayManager::Loop_loop_id(void)
{
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(22), DisplayPrimitives::display_coordinate_y(0), 5); // DisplayPrimitives::Cancel_text(int X, int Y, int N)
    tft.setCursor(DisplayPrimitives::display_coordinate_x(23), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(ILI9341_YELLOW);

    if (LOOP_id < 0)
    {
        tft.print("---");
        return;
    }

    tft.print(LOOP_id);
}

FLASHMEM
void DisplayManager::Loop_menu(void)
{
    uint8_t position = 0; // position on display

    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(1), 27); // DisplayPrimitives::Cancel_text(int X, int Y, int N)
    tft.setTextColor(MENU_COLOR);

    for (auto element = 0; element < 4; ++element) // menu element
    {
        if (Menu_Loop[element])
        {
            if (position == 0)
            {
                X_position_Menu_Loop[position] = 0;
            }

            else
            {
                X_position_Menu_Loop[position] = X_position_Menu_Loop[position - 1] + dimension_voice_Menu_Loop[element_Menu_Loop[position - 1]] + 1;
            }

            element_Menu_Loop[position] = element;
            position_Menu_Loop[element] = position;
            tft.setCursor(DisplayPrimitives::display_coordinate_x(X_position_Menu_Loop[position]), DisplayPrimitives::display_coordinate_y(1));
            tft.print(Menu_Loop_char[element]);
            ++position;
        }
    }
}

FLASHMEM
void DisplayManager::Frame_loop_menu(int position, bool fresh) // after Loop_menu() use fresh = true
{
    if (!fresh && Loop_menu_position_0 >= 0)
    {
        DisplayPrimitives::Frame_by_col_row(Loop_X_position_menu_0, 1, Loop_dimension_voice_menu_0, false); // DisplayPrimitives::Frame_by_col_row(X_position_Menu_Loop[position_0], 1, dimension_voice_Menu_Loop[element_Menu_Loop[position_0]], false);
    }

    if (!Menu_Loop[0] && !Menu_Loop[1] && !Menu_Loop[2])
    {
        Loop_menu_position_0 = -1;
        return;
    }

    DisplayPrimitives::Frame_by_col_row(X_position_Menu_Loop[position], 1, dimension_voice_Menu_Loop[element_Menu_Loop[position]], true);
    choice_loop_menu = element_Menu_Loop[position];

    Loop_X_position_menu_0 = X_position_Menu_Loop[position];
    Loop_dimension_voice_menu_0 = dimension_voice_Menu_Loop[element_Menu_Loop[position]];
    Loop_menu_position_0 = position;
}

void DisplayManager::Delete_all_frame_loop_menu(void)
{
    for (auto element = 0; element < 4; ++element)
    {
        if (Menu_Loop[element])
        {
            DisplayPrimitives::Frame_by_col_row(X_position_Menu_Loop[position_Menu_Loop[element]], 1, dimension_voice_Menu_Loop[element], false);
        }
    }
}

FLASHMEM
void DisplayManager::Loop_midi_loop_title(void)
{
    DisplayPrimitives::Board(0, 0, 9); // DisplayPrimitives::Board(float col, float row, int chars)
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("MIDI LOOP");
}

FLASHMEM
void DisplayManager::Loop_track_data(int track)
{
    // Numero
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Loop_LOOPS_X + 7 * track), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R), 5);
    if (LOOP_events[track] > 0)
    {
        tft.setTextColor(ILI9341_WHITE);
        tft.print(track + 1); // Comincia da 1
    }

    // Slide
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Loop_LOOPS_X + 7 * track), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R + 1), 6);
    if (LOOP_events[track] > 0)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print((float)(LOOP_slide[track]) / 1000.0f, 2);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("s");
    }

    // Pitch
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Loop_LOOPS_X + 7 * track), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R + 2), 7);
    if (LOOP_events[track] > 0)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(LOOP_pitch_int[track]);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("key");
    }

    // Volume
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Loop_LOOPS_X + 7 * track), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R + 3), 6);
    if (LOOP_events[track] > 0)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(LOOP_volume[track], 1);
    }
}

FLASHMEM
void DisplayManager::Loop_time_stretched(void)
{
    // LOOP_time
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Loop_LOOP_TIME), DisplayPrimitives::display_coordinate_y(Loop_HEAD_R - 1), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print((float)(LOOP_time * LOOP_stretch) / 1000.0);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("s");
}

void DisplayManager::Cancel_text_reset_cursor(int X, int Y, int N)
{
    DisplayPrimitives::Cancel_text(X, Y, N);
    tft.setCursor(X, Y);
}

void DisplayManager::Logo(float light)
{
    tft.drawBitmap(0 + Logo_position_DX, 42 + Logo_position_DY, LOGO_0, 168, 49, Calc_color(ILI9341_YELLOW, light));
    tft.drawBitmap(0 + Logo_position_DX, 14 + Logo_position_DY, LOGO_1, 8, 26, Calc_color(ILI9341_RED, light));
    tft.drawBitmap(32 + Logo_position_DX, 9 + Logo_position_DY, LOGO_2, 16, 31, Calc_color(ILI9341_RED, light));
    tft.drawBitmap(48 + Logo_position_DX, 20 + Logo_position_DY, LOGO_3, 16, 20, Calc_color(ILI9341_MAGENTA, light));
    tft.drawBitmap(88 + Logo_position_DX, 0 + Logo_position_DY, LOGO_4, 8, 40, Calc_color(ILI9341_MAGENTA, light));
    tft.drawBitmap(-1 + Logo_position_DX, 100 + Logo_position_DY, audio_sampler, 168, 18, Calc_color(ILI9341_WHITE, light));
}

void DisplayManager::Cover_text(float light)
{
    tft.setTextColor(Calc_color(TEXT_COLOR, light)); // 16-bit ('565') color settings
    tft.setCursor(DisplayPrimitives::display_coordinate_x(13), DisplayPrimitives::display_coordinate_y(12));
    tft.print("UPDATE ");
    tft.setTextColor(Calc_color(ILI9341_WHITE, light)); // 16-bit ('565') color settings
    tft.print(FIRMWARE_VERSION);

    tft.setTextColor(Calc_color(TEXT_COLOR, light)); // 16-bit ('565') color settings
    tft.setCursor(DisplayPrimitives::display_coordinate_x(13), DisplayPrimitives::display_coordinate_y(13) - 4);
    tft.print("AUDIO MEMORY ");
    tft.setTextColor(Calc_color(ILI9341_WHITE, light)); // 16-bit ('565') color settings
    tft.print(flash_dimension_MB);
    tft.print("MB");

    tft.setTextColor(Calc_color(TEXT_COLOR, light)); // 16-bit ('565') color settings
    tft.setCursor(DisplayPrimitives::display_coordinate_x(13), DisplayPrimitives::display_coordinate_y(14) - 8);
    tft.print("LIVE SAMPLER MEMORY ");
    tft.setTextColor(Calc_color(ILI9341_WHITE, light)); // 16-bit ('565') color settings
    tft.print("16MB");
}

uint16_t DisplayManager::Calc_color(uint16_t color_peak, float light) // 16-bit ('565') color settings
{
    // extracts components of peak (regime) value
    uint16_t red = color_peak >> 11;
    uint16_t green = (color_peak & 0b11111100000) >> 5;
    uint16_t blue = color_peak & 0b11111;

    // modulate each components
    red = static_cast<float>(red) * light;
    green = static_cast<float>(green) * light;
    blue = static_cast<float>(blue) * light;

    uint16_t value = (red << 11) + (green << 5) + blue;
    return value;
}

int DisplayManager::P_Instrument_pixels_y(int position)
{
    return 4 + 15.0 * (6 + position) + 7;
}

FLASHMEM
void DisplayManager::Delay_page()
{
    tft.fillScreen(ILI9341_BLACK);

    if (Lilla_state_0 == PERFORMANCE)
    {
        DisplayPrimitives::Board(0, 0, 17); // DISPLAY_board(float col, float row, int chars)
        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
        tft.setTextColor(ILI9341_WHITE);
        tft.print("PERFORMANCE ");
        tft.setTextColor(ILI9341_WHITE);
        tft.print("DELAY");
    }

    else if (Lilla_state_0 == LIVE_SAMPLING)
    {
        DisplayPrimitives::Board(0, 0, 18); // DISPLAY_board(float col, float row, int chars)
        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
        tft.setTextColor(ILI9341_WHITE);
        tft.print("LIVE SAMPLER ");
        tft.setTextColor(ILI9341_WHITE);
        tft.print("DELAY");
    }

    tft.setCursor(DisplayPrimitives::display_coordinate_x(41), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME");
    P_Patch_volume_value(true);

    Show_effects();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SOURCES");
    D_sounds();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 2));
    tft.setTextColor(TEXT_COLOR);
    tft.print("FEEDBACK");
    D_read_gain();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(20), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 2));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VALUE");
    D_delay();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 3));
    tft.setTextColor(TEXT_COLOR);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(20), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VALUE L/R");
    D_delay_LR();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD SOURCE");
    D_modulation_type();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(20), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD FREQUENCY");
    D_modulation_frequency();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 6));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD DEPTH");
    D_modulation_depth();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(20), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 6));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD PHASE L/R");
    D_modulation_phase_LR();
}

FLASHMEM
void DisplayManager::D_sounds(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(8), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE), 23); // DISPLAY_text(int X, int Y, int N)
    if (Lilla_state_0 == LIVE_SAMPLING)
    {
        tft.setTextColor(ILI9341_YELLOW);

        if (Delay_values.instrument_route[0])
        {
            tft.print("RECORDED AUDIO");
        }
        else
        {
            tft.print("NONE");
        }
        return;
    }

    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        if (Delay_values.instrument_route[instrument_id])
        {
            tft.setTextColor(ILI9341_YELLOW);
            tft.print("S");
            tft.print(instrument_id + 1);
            tft.print("  ");
        }
        else
        {
            tft.setTextColor(0x6300);
            tft.print("S");
            tft.print(instrument_id + 1);
            tft.print("  ");
        }
    }
}

FLASHMEM
void DisplayManager::D_delay(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(25.5), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 2), 9);
    tft.setTextColor(ILI9341_YELLOW);

    if (Delay_values.samples < 44100)
    {
        tft.print(Delay_values.samples / 44.1f, 1);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("ms");
    }
    else
    {
        tft.print(Delay_values.samples / 44100.0f, 2);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("sec");
    }
}

FLASHMEM
void DisplayManager::D_read_gain(void) // feedback
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(8.5), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 2), 8);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(-Delay_feedback(Delay_data.loop_gain) * 100);
    tft.print("%");
}

FLASHMEM
void DisplayManager::D_delay_LR(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(29.5), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 3), 12);
    tft.setTextColor(ILI9341_YELLOW);

    if (Delay_values.samples_LR == 0)
    {
        tft.print(0);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("ms L");
    }
    else if (Delay_values.samples_LR > 0)
    {
        tft.print("+");
        tft.print(Delay_values.samples_LR / 44.1f);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("ms R");
    }
    else
    {
        tft.print("+");
        tft.print(-Delay_values.samples_LR / 44.1f);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("ms L");
    }
}

FLASHMEM
void DisplayManager::D_modulation_type(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(10.5), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 5), 6);
    tft.setTextColor(ILI9341_YELLOW);
    if (Delay_values.modulation_source == 0) // nessuna modulazione
    {
        tft.print("NONE");
    }
    else if (Delay_values.modulation_source == 1) // LFO
    {
        tft.print("LFO");
    }
    else
    {
        tft.print("SOURCE");
    }
}

FLASHMEM
void DisplayManager::D_modulation_frequency(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(33.5), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 5), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Delay_values.modulation_frequency);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("Hz");
}

FLASHMEM
void DisplayManager::D_modulation_depth(void) // depth
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(9.5), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 6), 8);
    tft.setTextColor(ILI9341_YELLOW);
    if (Delay_values.modulation_depth <= 1.0f)
    {
        tft.print(Delay_values.modulation_depth * 100, 1);
    }
    else
    {
        tft.print(Delay_values.modulation_depth * 100, 0);
    }
    tft.print("%");
}

FLASHMEM
void DisplayManager::D_modulation_phase_LR(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(33.5), DisplayPrimitives::display_coordinate_y(Delay_ROW_BASE + 6), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Delay_values.modulation_phase_LR);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("deg");
}

inline int DisplayManager::Sound_Id(int patch_id, int instrument_id)
{
    return Patch[patch_id].Instrument[instrument_id].sound_id;
}

FLASHMEM
void DisplayManager::Delay_disabled(void)
{
    L_POPUP = 228; // 106;
    H_POPUP = 28;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP = (240 - H_POPUP) / 2;
    Y_POPUP_TXT = 10;
    Y_POPUP_OPT = 30;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(3), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);

    //"0123456789012345678901234567890123456789109876543210";
    tft.print("DELAY IS DISABLED WHILE SAMPLING");
}

FLASHMEM
void DisplayManager::DS_confirm_EXIT_from_DS(void)
{
    L_POPUP = 106;
    H_POPUP = 47;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP = (240 - H_POPUP) / 2;
    Y_POPUP_TXT = 10;
    Y_POPUP_OPT = 30;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1.5), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("STOP SAMPLING?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(5.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(9.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::DS_page(int recording)
{
    tft.fillScreen(ILI9341_BLACK);

    DisplayPrimitives::Board(0, 0, 7); // Display.DisplayPrimitives::Board(float   col, float row, int chars)
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.print("SAMPLER");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(41), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME");
    DS_update_volume();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("AUDIO MEMORY ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(flash_dimension_MB);
    DisplayPrimitives::Unit("MB", 2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(" (");
    tft.print(Get_flash_size() / 88100.0f);
    DisplayPrimitives::Unit("sec", 3);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("- FREE FOR RECORDINGS");
    DS_available_memory();

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6));
    tft.setTextColor(TEXT_COLOR);
    tft.print("- FREE FOR RAW FILES ");
    DS_raw_available_memory();

    DS_Recording_description(recording, true);
    DS_sampler_IO();
}

FLASHMEM
void DisplayManager::DS_sampler_IO(void)
{
    tft.setCursor(DS_VUMETER_BAR_X + 1, DS_VUMETER_BAR_Y + 4);
    tft.setTextColor(TEXT_COLOR);
    tft.print("L");
    tft.setCursor(DS_VUMETER_BAR_X + DS_VUMETER_BAR_DX, DS_VUMETER_BAR_Y + 4);
    tft.print("R");

    // tft.drawRect(DS_VUMETER_BAR_X - 8, DS_VUMETER_BAR_Y - BAR_ELEMENTS - 6, 36, BAR_ELEMENTS + 36, ILI9341_GREEN);
    tft.drawRect(DS_VUMETER_BAR_X - 1, DS_VUMETER_BAR_Y - BAR_ELEMENTS - 1, DS_VUMETER_BAR_DISTANCE + 2, BAR_ELEMENTS + 2, 0x03E0);
    tft.drawRect(DS_VUMETER_BAR_X - 1 + DS_VUMETER_BAR_DX, DS_VUMETER_BAR_Y - BAR_ELEMENTS - 1, DS_VUMETER_BAR_DISTANCE + 2, BAR_ELEMENTS + 2, 0x03E0);
    DS_bar(0, 0); // 0 <= value <= BAR_ELEMENTS
    DS_bar(1, 0); // 0 <= value <= BAR_ELEMENTS

    // Schema IN OUT
    tft.drawRect(DS_START_X, DS_START_Y, 31, 15, ILI9341_GREEN);
    tft.drawBitmap(DS_START_X - 19, DS_START_Y + 4, DS_freccia, 19, 7, ILI9341_GREEN);
    tft.setCursor(DS_START_X - 63, DS_START_Y + 4);
    tft.setTextColor(TEXT_COLOR);
    tft.print("LINE-IN");

    tft.setCursor(DS_START_X + 4, DS_START_Y + 17);
    tft.setTextColor(TEXT_COLOR);
    tft.print("GAIN");

    // Sampler
    DS_sampler_frame(true);
    DS_sampler_txt(false);
    DS_show_gain();
}

void DisplayManager::DS_bar(int channel, int value) // 0 <= value <= BAR_ELEMENTS
{
    int X0 = (channel == 0 ? DS_VUMETER_BAR_X : DS_VUMETER_BAR_X + DS_VUMETER_BAR_DX);
    float value_float;
    const float BAR_ELEMENTS_float = BAR_ELEMENTS;

    if (value < 0)
    {
        value = 0;
    }

    if (value > DS_VU_meter_value_old[channel])
    {
        for (auto i = DS_VU_meter_value_old[channel] + 1; i <= value; ++i)
        {
            value_float = i / BAR_ELEMENTS_float;
            // builds bricks
            tft.drawFastHLine(X0, DS_VUMETER_BAR_Y - i, DS_VUMETER_BAR_DISTANCE, DS_calc_bar_color(value_float));
        }
    }
    else if (value < DS_VU_meter_value_old[channel])
    {
        for (auto i = value + 1; i <= DS_VU_meter_value_old[channel]; ++i)
        {
            // remove bricks
            tft.drawFastHLine(X0, DS_VUMETER_BAR_Y - i, DS_VUMETER_BAR_DISTANCE, ILI9341_BLACK);
        }
    }
    DS_VU_meter_value_old[channel] = value;
}

uint16_t DisplayManager::DS_calc_bar_color(float value) // 16-bit ('565') color settings
{
    const float soglia = 0.5;
    uint16_t red = (value >= soglia ? 31 : 31.0f * (value / soglia));                   // (value >= 0.5f ? 31: 31.0f * 2.0f * value)
    uint16_t green = (value <= soglia ? 63 : 63.0f * (1.2f - value) / (1.2f - soglia)); // (value <= 0.5f ? 63: 63.0f * 2.0f * (1.0f - value))
    uint16_t blue = 0;                                                                  // 33 * 0

    if (false)
    {
        Serial.print("value ");
        Serial.print(value);
        Serial.print("  red ");
        Serial.print(red);
        Serial.print("  green ");
        Serial.print(green);
        Serial.print("  hex ");
        Serial.println((red << 11) + (green << 5) + blue, HEX);
        Serial.println();
    }

    return (red << 11) + (green << 5) + blue;
}

FLASHMEM
void DisplayManager::DS_line_out(bool visible)
{
    tft.drawBitmap(DS_START_X + 37, DS_START_Y + 7, DS_freccia_gomito, 13, 23, (visible ? ILI9341_GREEN : GREEN_OFF));
    tft.setCursor(DS_START_X + 55, DS_START_Y + 23);
    tft.setTextColor((visible ? TEXT_COLOR : TEXT_OFF_COLOR));
    tft.print("LINE-OUT");
}

FLASHMEM
void DisplayManager::DS_sampler_frame(bool visible)
{
    tft.drawBitmap(DS_START_X + 31, DS_START_Y + 4, DS_freccia, 19, 7, (visible ? ILI9341_GREEN : ILI9341_BLACK));
    tft.drawRoundRect(DS_START_X + 50, DS_START_Y, 51, 15, 3, (visible ? ILI9341_GREEN : ILI9341_BLACK));
}

FLASHMEM
void DisplayManager::DS_sampler_txt(bool color)
{
    tft.setCursor(DS_START_X + 58, DS_START_Y + 4);
    tft.setTextColor(color ? ILI9341_RED : RED_OFF);
    tft.print("RECORD");
}

FLASHMEM
void DisplayManager::DS_available_memory(void)
{
    col = 22;
    row = 5;
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(col), DisplayPrimitives::display_coordinate_y(row), 10);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(VFS_Get_packets_free() * 0.743, 1);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayManager::DS_raw_available_memory()
{
    col = 21;
    row = 6;
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(col), DisplayPrimitives::display_coordinate_y(row), 6);
    tft.setTextColor(ILI9341_WHITE);
    tft.print((Get_flash_size() - Get_flash_occupation() - FLASH_FREE_SPACE) / 88200.0f);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayManager::DS_hide_recording()
{
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8), 18);
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9), 29);
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10), 18);
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(11), 18);
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(12), 18);
}

FLASHMEM
void DisplayManager::DS_advice_delete(bool value)
{
    tft.setTextColor((value ? ILI9341_YELLOW : ILI9341_BLACK));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(3.5), DisplayPrimitives::display_coordinate_y(8));
    tft.print("PLEASE WAIT");
}

FLASHMEM
void DisplayManager::DS_advice_no_conversion(int DS_export, bool value)
{
    tft.setTextColor((value ? TEXT_COLOR : ILI9341_BLACK));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.print("UNABLE TO CREATE RAW FILE");
    tft.setTextColor((value ? ILI9341_YELLOW : ILI9341_BLACK));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9));

    if (DS_export == 0)
    {
        tft.print("AUDIO MEMORY INSUFFICIENT");
    }
    else
    {
        tft.print(".RAW NAMESPACE IS FULL");
    }
}

FLASHMEM
void DisplayManager::DS_conversion_options(int file_L_RAW, int file_R_RAW, int DS_export)
{
    tft.fillRect(0, 120, 320, 120, ILI9341_BLACK); // cancella Recording e grafica
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.print("OPTIONS:");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9));
    if (!Recording[recording].stereo)
    {
        tft.print("- MONO ");
    }
    else
    {
        tft.print("- LEFT ");
    }

    tft.setTextColor(ILI9341_WHITE);
    tft.print(name_file[recording + FIRST_RECORDING_FILE]);
    tft.print(" --> ");
    tft.print(name_file[file_L_RAW]);
    tft.print(" (");
    tft.print(Recording[recording].bytes >> 10);
    tft.print("kB)");

    if (Recording[recording].stereo && DS_export == 2)
    {
        tft.setTextColor(TEXT_COLOR);
        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10));
        tft.print("- RIGHT ");

        tft.setTextColor(ILI9341_WHITE);
        tft.print(name_file[recording + FIRST_RECORDING_FILE + 1]);
        tft.print(" --> ");
        tft.print(name_file[file_R_RAW]);
        tft.print(" (");
        tft.print(Recording[recording].bytes >> 10);
        tft.print("kB)");
    }
}

FLASHMEM
void DisplayManager::DS_export_options(int file_L_RAW, int file_R_RAW, int DS_export)
{

    tft.fillRect(0, 120, 320, 120, ILI9341_BLACK); // cancella Recording e grafica
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.print("SD EXPORT OPTIONS:");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9));

    if (!Recording[recording].stereo)
    {
        tft.print("- MONO ");
    }
    else
    {
        tft.print("- LEFT ");
    }

    tft.setTextColor(ILI9341_WHITE);
    tft.print(name_file[recording + FIRST_RECORDING_FILE]);
    tft.print(" --> ");
    // DISPLAY_DS_export_filename(0);
    tft.print(" (");
    tft.print(Recording[recording].bytes >> 10);
    tft.print("kB)");

    if (Recording[recording].stereo && DS_export == 2)
    {
        tft.setTextColor(TEXT_COLOR);
        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10));
        tft.print("- RIGHT ");

        tft.setTextColor(ILI9341_WHITE);
        tft.print(name_file[recording + FIRST_RECORDING_FILE + 1]);
        tft.print(" --> ");
        // DISPLAY_DS_export_filename(1);
        tft.print(" (");
        tft.print(Recording[recording].bytes >> 10);
        tft.print("kB)");
    }
}

FLASHMEM
void DisplayManager::DS_Recording_description(int recording, bool led)
{
    // RECORDING
    tft.setCursor((led ? DisplayPrimitives::display_coordinate_x(1.5) : DisplayPrimitives::display_coordinate_x(0)), DisplayPrimitives::display_coordinate_y(8));
    tft.setTextColor(TEXT_COLOR);
    tft.print("RECORDING ");

    if (recording >= 0)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(recording);
    }
    else
    {
        tft.setTextColor(ILI9341_WHITE);
        tft.print("NONE");
    }

    // FILE LEFT o MONO
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9));
    tft.setTextColor(TEXT_COLOR);

    if (recording < 0)
    {
        tft.print("TEST_FILE ");
        tft.setTextColor(ILI9341_WHITE);
        tft.print("0.RAW");
    }
    else
    {
        if (Recording[recording].stereo)
        {
            tft.print("LEFT_FILE ");
        }
        else
        {
            tft.print("MONO_FILE ");
        }
        tft.setTextColor(ILI9341_WHITE);
        tft.print(name_file[2 * recording + FIRST_RECORDING_FILE]);
    }

    // FILE RIGHT
    if (recording >= 0 && Recording[recording].stereo)
    {
        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10));
        tft.setTextColor(TEXT_COLOR);
        tft.print("RIGHT_FILE ");
        tft.setTextColor(ILI9341_WHITE);
        tft.print(name_file[2 * recording + FIRST_RECORDING_FILE + 1]);
    }
    DS_recording_seconds();
    if (led)
    {
        DS_update_volume();
    }
}

FLASHMEM
void DisplayManager::DS_recording_seconds(void)
{
    if (recording < 0)
    {
        return;
    }

    row = 11;
    if (!Recording[recording].stereo)
    {
        row -= 1;
    }

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(row));
    tft.setTextColor(TEXT_COLOR);
    tft.print("LENGTH ");

    tft.setTextColor(ILI9341_WHITE);
    tft.print(Recording[recording].seconds, 1);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayManager::DS_update_recording_seconds(float value)
{
    row = 11;
    if (Recording[recording].stereo)
    {
        Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(7), DisplayPrimitives::display_coordinate_y(row), 7);
    }
    else
    {
        Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(7), DisplayPrimitives::display_coordinate_y(row - 1), 7);
    }

    tft.setTextColor(ILI9341_WHITE);
    tft.print(value / 1000.0f, 1);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayManager::DS_volume(void)
{
    row = 12;
    if (!Recording[recording].stereo)
    {
        row = 11;
    }
    if (recording < 0)
    {
        row = 10;
    }
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(row));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME ");

    tft.setTextColor(ILI9341_YELLOW);
    tft.print(volume_patch / 20.0f, 2);
}

FLASHMEM
void DisplayManager::DS_update_volume(bool adj)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(48), DisplayPrimitives::display_coordinate_y(0), 4);
    tft.setTextColor(adj ? ILI9341_YELLOW : ILI9341_WHITE);
    tft.print(volume_patch / 20.0f, 2);
}

FLASHMEM
void DisplayManager::DS_show_gain(void)
{
    Cancel_text_reset_cursor(DS_START_X + 4, DS_START_Y + 4, 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(DS_gain / 20.0f, 2);
}

FLASHMEM
void DisplayManager::DS_menu(void)
{
    auto position = 0; // position on display

    // DS_define_model(); DISTRIBUITO
    DisplayPrimitives::Delete_row(1);
    DisplayPrimitives::Delete_row(2);
    tft.setTextColor(MENU_COLOR);

    for (auto element = 0; element < DS_MV; ++element) // menu element
    {
        if (Menu_DS[element])
        {
            if (position == 0)
            {
                X_position_Menu_DS[position] = 0;
            }
            else
            {
                X_position_Menu_DS[position] = X_position_Menu_DS[position - 1] + dimension_voice_Menu_DS[element_Menu_DS[position - 1]] + 1;
            }

            Y_position_Menu_DS[position] = 1;

            element_Menu_DS[position] = element;
            position_Menu_DS[element] = position;
            tft.setCursor(DisplayPrimitives::display_coordinate_x(X_position_Menu_DS[position]), DisplayPrimitives::display_coordinate_y(Y_position_Menu_DS[position]));
            tft.print(Menu_DS_char[element]);
            position++;
        }
    }
}

FLASHMEM
void DisplayManager::DS_frame_menu(int position)
{
    DisplayPrimitives::Frame_by_col_row(X_position_Menu_DS[DS_frame_menu_position_0], Y_position_Menu_DS[DS_frame_menu_position_0], dimension_voice_Menu_DS[element_Menu_DS[DS_frame_menu_position_0]], false);
    DisplayPrimitives::Frame_by_col_row(X_position_Menu_DS[position], Y_position_Menu_DS[position], dimension_voice_Menu_DS[element_Menu_DS[position]], true);
    choice_DS_menu = element_Menu_DS[position];
    DS_frame_menu_position_0 = position;
}

FLASHMEM
void DisplayManager::Show_VFS_packets(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(21), DisplayPrimitives::display_coordinate_y(7), 40);
    DisplayPrimitives::Frame_by_col_row(21, 7, 21, true);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print((VFS_packets * PACKET_DIM) / 1048576.0f, 2);
    DisplayPrimitives::Unit("MB", 2);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(" (");
    tft.print(VFS_packets * 0.743, 0);
    DisplayPrimitives::Unit("sec", 3);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(" MONO)");

    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(27), DisplayPrimitives::display_coordinate_y(8), 40);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(((VFS_packets_max - VFS_packets) * PACKET_DIM) / 1048576.0f, 2);
    DisplayPrimitives::Unit("MB", 2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(" (");
    tft.print((VFS_packets_max - VFS_packets) * 0.743, 0);
    DisplayPrimitives::Unit("sec", 3);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");
}

FLASHMEM
void DisplayManager::Midi_monitor_page(void)
{
    tft.fillScreen(ILI9341_BLACK);

    DisplayPrimitives::Board(0, 0, 12); // Display.DisplayPrimitives::Board(float col, float row, int chars)
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("MIDI MONITOR");

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3));
    tft.print("MIDI CHANNEL");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(4));
    tft.print("MESSAGE");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(5));
    tft.print("NOTE-NUMBER");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6));
    tft.print("VELOCITY");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(7));
    tft.print("VALUE");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.print("NUMBER");
}

FLASHMEM
void DisplayManager::Midi_monitor_frame(void)
{
    DisplayPrimitives::Frame_by_col_row(0, 1, 6, true);
}

FLASHMEM
void DisplayManager::Midi_monitor_data(uint8_t incoming_midi_channel, uint8_t incoming_midi_message, int8_t incoming_note_number, int8_t incoming_velocity, int32_t incoming_midi_value, int8_t incoming_number)
{
    const char message_name[12][20] = {{"NoteOn"}, {"NoteOff"}, {"PitchBend"}, {"AfterTouchPoly"}, {"ControlChange"}, {"ProgramChange"}, {"AfterTouchChange"}, {"SystemExclusive"}, {"Unknown"}};

    tft.setTextColor(ILI9341_YELLOW);
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(13), DisplayPrimitives::display_coordinate_y(3), 2);
    tft.print(incoming_midi_channel + 1);

    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(8), DisplayPrimitives::display_coordinate_y(4), 16);
    tft.print(message_name[incoming_midi_message]);

    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(12), DisplayPrimitives::display_coordinate_y(5), 9);
    if (incoming_note_number >= 0)
    {
        tft.setCursor(DisplayPrimitives::display_coordinate_x(12), DisplayPrimitives::display_coordinate_y(5));
        tft.print(note_name[incoming_note_number % 12]);
        tft.print((int)(incoming_note_number / 12) + first_octave);
    }

    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(9), DisplayPrimitives::display_coordinate_y(6), 9);
    if (incoming_velocity >= 0)
    {
        tft.setCursor(DisplayPrimitives::display_coordinate_x(9), DisplayPrimitives::display_coordinate_y(6));
        tft.print(incoming_velocity);
    }

    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(6), DisplayPrimitives::display_coordinate_y(7), 9);
    if (incoming_midi_value >= 0)
    {
        tft.setCursor(DisplayPrimitives::display_coordinate_x(6), DisplayPrimitives::display_coordinate_y(7));
        tft.print(incoming_midi_value);
    }

    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(7), DisplayPrimitives::display_coordinate_y(8), 9);
    if (incoming_number >= 0)
    {
        tft.setCursor(DisplayPrimitives::display_coordinate_x(7), DisplayPrimitives::display_coordinate_y(8));
        tft.print(incoming_number);
    }
}

FLASHMEM
void DisplayManager::MX_page(void)
{
    tft.fillScreen(ILI9341_BLACK);

    DisplayPrimitives::Board(0, 0, 5); // Display.DisplayPrimitives::Board(float   col, float row, int chars)
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("MIXER");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(MX_Y0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SOURCE");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(MX_Y0 + 1));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MUTE");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(MX_Y0 + 2));
    tft.setTextColor(TEXT_COLOR);
    tft.print("GAIN");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(MX_Y0 + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PAN");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(MX_Y0 + 4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("LINEOUT");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(MX_Y0 + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MONITOR");

    /*
    tft.setCursor(DisplayPrimitives::x_pos(0), DisplayPrimitives::display_coordinate_y(MX_Y0 + 7));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME LINE OUT");
    MX_MAIN_volume();

    tft.setCursor(DisplayPrimitives::x_pos(0), DisplayPrimitives::display_coordinate_y(MX_Y0 + 8));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME MONITOR");
    MX_MONITOR_volume();
    */
}

FLASHMEM
void DisplayManager::MX_source_values(int source)
{
    if (source == 8)
    {
        MX_source_values_write(source);
    }
    else if (Patch[Patch_id].Instrument[source].used)
    {
        MX_source_values_write(source);
    }
}

FLASHMEM
void DisplayManager::MX_source_values_jump(int old_source, int new_source)
{
    MX_source = new_source;

    MX_source_values_write(old_source); // 0
    MX_source_values_write(MX_source);  // 6
}

FLASHMEM
void DisplayManager::MX_source_values_write(int source)
{
    int local_sound_id;
    if (source == 8)
    {
        tft.setTextColor((source == MX_source ? TEXT_COLOR : 0x6300));
        tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 + source * 5 - 1), DisplayPrimitives::display_coordinate_y(MX_Y0));
        tft.print("L_IN");

        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 1), led_pic, 8, 8, (MX_mute[source] ? ILI9341_RED : RED_OFF)); // Mute
        tft.setTextColor((source == MX_source ? ILI9341_YELLOW : 0x6300));
        tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 - 1 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 2)); // Gain
        tft.print(DS_gain / 20.0f);

        tft.setTextColor((source == MX_source ? ILI9341_WHITE : 0x6300));
        tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 3)); // Pan
        tft.print("C");
    }

    else
    {
        local_sound_id = Patch[Patch_id].Instrument[source].sound_id;

        tft.setTextColor((source == MX_source ? TEXT_COLOR : 0x6300));
        tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0));
        tft.print("S");
        tft.print(source + 1);

        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 1), led_pic, 8, 8, (MX_mute[source] ? ILI9341_RED : RED_OFF)); // Mute
        tft.setTextColor((source == MX_source ? ILI9341_YELLOW : 0x6300));

        DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(MX_X0 - 1 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 2), 4);
        tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 - 1 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 2)); // Gain
        tft.print(Sound[local_sound_id].gain / 20.0f);

        if (Sound[local_sound_id].pan == 0)
        {
            DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(MX_X0 - 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 3), 3);
            tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 3)); // Pan
            tft.print("C");
        }
        else
        {
            if (abs(Sound[local_sound_id].pan) < 10) // Pan
            {
                tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 3));
            }
            else
            {
                tft.setCursor(DisplayPrimitives::display_coordinate_x(MX_X0 - 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 3));
            }

            if (Sound[local_sound_id].pan < 0)
            {
                tft.print("L");
            }
            else if (Sound[local_sound_id].pan > 0)
            {
                tft.print("R");
            }
            tft.print(abs(Sound[local_sound_id].pan));
        }
    }

    if (MX_routing_source[source] > 1) // to Audio-out
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 4), led_pic, 8, 8, GREEN_ON);
    }
    else
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 4), led_pic, 8, 8, GREEN_OFF);
    }

    if (MX_routing_source[source] == 1 || MX_routing_source[source] == 3) // to Monitor
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 5), led_pic, 8, 8, GREEN_ON);
    }
    else
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 5), led_pic, 8, 8, GREEN_OFF);
    }
}

FLASHMEM
void DisplayManager::MX_source_values_edit(int source)
{
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 1), 1); // Mute
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(MX_X0 - 1 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 2), 4);   // Gain
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(MX_X0 - 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 3), 3); // Pan
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 4), 1); // to Audio-out
    DisplayPrimitives::Cancel_text(DisplayPrimitives::display_coordinate_x(MX_X0 + 0.5 + source * 5), DisplayPrimitives::display_coordinate_y(MX_Y0 + 5), 1); // to Monitor

    MX_source_values_write(source);
}

FLASHMEM
void DisplayManager::P_Patch_header(bool change_patch, bool change_vol)
{
    tft.fillScreen(ILI9341_BLACK);
    P_Show_Performance();
    P_Show_Patch_number(change_patch);
    P_Patch_volume(change_vol);
    Show_effects();
}

FLASHMEM
void DisplayManager::Patch_volume_color(bool change_patch, bool change_vol)
{
    P_Show_Patch_number(change_patch);
    P_Patch_volume(change_vol);
    P_Patch_volume_value(change_vol);
}

FLASHMEM
void DisplayManager::P_show_Instruments_header(void)
{
    tft.setTextColor(TEXT_COLOR);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_SOUND_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title));
    tft.print("SOUND");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_LOCK_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title)); // (X0i + 2 * Instrument_SPACE_X + 6)
    tft.print("LOCK");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_P_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title)); // (DisplayPrimitives::x_pos(X0i + 5 + Instrument_SPACE_X)
    tft.print("P");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_MIDI_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title));
    tft.print("MIDI");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_ROOT_K_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title));
    tft.print("ROOT-K");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_FROM_K_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title));
    tft.print("FROM-K");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_TO_K_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title));
    tft.print("TO-K");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_PAN_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title));
    tft.print("PAN");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_GAIN_title), DisplayPrimitives::display_coordinate_y(P_row_Instrument_title));
    tft.print("GAIN");

    tft.drawLine(6, DisplayPrimitives::display_coordinate_y(P_row_Instrument_title) + 11, 312, DisplayPrimitives::display_coordinate_y(P_row_Instrument_title) + 11, 0x630C);
}

FLASHMEM
void DisplayManager::P_show_Instrument_description(int patch_id, int instrument_id, bool editing)
{
    // P_Delete_Instrument(position);
    P_show_Sound(instrument_id, editing);
    P_show_Lock(patch_id, instrument_id, editing);
    P_show_Precedence(patch_id, instrument_id, editing);
    P_show_Midi(patch_id, instrument_id, editing);
    P_show_RootKey(patch_id, instrument_id, editing);
    P_show_FromKey(patch_id, instrument_id, editing);
    P_show_ToKey(patch_id, instrument_id, editing);
    P_show_Pan(patch_id, instrument_id, editing);
    P_show_Gain(patch_id, instrument_id, editing);
}

FLASHMEM
void DisplayManager::P_show_Sound(int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Sound);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 1);
    tft.print(instrument_id + 1);
}

FLASHMEM
void DisplayManager::P_show_Lock(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[0]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 1);
    if (Patch[patch_id].Instrument[instrument_id].lock)
    {
        tft.print("Y");
    }
    else
    {
        tft.print("N");
    }
}

FLASHMEM
void DisplayManager::P_show_Precedence(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[1]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 1);
    if (Patch[patch_id].Instrument[instrument_id].precedence)
    {
        tft.print("X");
    }
    else
    {
        tft.print("N");
    }
}

FLASHMEM
void DisplayManager::P_show_Midi(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[2]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 2);
    tft.print(Get_midi_channel(patch_id, instrument_id) + 1);
}

FLASHMEM
void DisplayManager::P_show_RootKey(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[3]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 4);
    Note(Patch[patch_id].Instrument[instrument_id].root_key);
}

FLASHMEM
void DisplayManager::P_show_FromKey(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[4]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 4);
    Note(Patch[patch_id].Instrument[instrument_id].from_note);
}

FLASHMEM
void DisplayManager::P_show_ToKey(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[5]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 4);
    Note(Patch[patch_id].Instrument[instrument_id].to_note);
}

FLASHMEM
void DisplayManager::P_show_Pan(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[6]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 2);
    if (Sound[Patch[patch_id].Instrument[instrument_id].sound_id].pan < 0)
    {
        tft.print("L");
    }
    else if (Sound[Patch[patch_id].Instrument[instrument_id].sound_id].pan > 0)
    {
        tft.print("R");
    }
    tft.print(abs(Sound[Patch[patch_id].Instrument[instrument_id].sound_id].pan));
}

FLASHMEM
void DisplayManager::P_show_Gain(int patch_id, int instrument_id, bool editing)
{
    auto position = P_line_of_instrument[instrument_id];
    auto y_display = P_Instrument_pixels_y(position);

    auto x_display = DisplayPrimitives::display_coordinate_x(P_column_Instrument_element[7]);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    Cancel_text_reset_cursor(x_display, y_display, 4);
    tft.print(Sound[Patch[patch_id].Instrument[instrument_id].sound_id].gain / 20.0f, 2);
}

FLASHMEM
void DisplayManager::P_show_all_Instruments(int patch_id)
{
    tft.fillRect(0, P_Instrument_pixels_y(0) - 4, 320, 240, ILI9341_BLACK);

    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        if (Patch[patch_id].Instrument[instrument_id].used)
        {
            P_show_Instrument_description(patch_id, instrument_id, true);
        }
    }

    if (tuning_tone_flag)
    {
        P_show_TuningTone_instrument(patch_id);
    }
}

FLASHMEM
void DisplayManager::P_show_TuningTone_instrument(int patch_id)
{
    auto position = Patch[patch_id].instruments;

    if (tuning_tone_flag)
    {
        tft.setTextColor(ILI9341_WHITE);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_INDENT_X0 + 2), P_Instrument_pixels_y(position));
        tft.print("TUNING-TONE");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_INDENT_X0 + 3 * Instrument_SPACE_X + 10), P_Instrument_pixels_y(position));
        tft.print("ALL");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_INDENT_X0 + 4 * Instrument_SPACE_X + 15), P_Instrument_pixels_y(position)); // root key
        Note(60);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(2 + Instrument_INDENT_X0 + 5 * Instrument_SPACE_X + 18.5), P_Instrument_pixels_y(position)); // from key
        Note(0);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(2 + Instrument_INDENT_X0 + 6 * Instrument_SPACE_X + 25), P_Instrument_pixels_y(position)); // to key
        Note(127);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_INDENT_X0 + 7 * Instrument_SPACE_X + 32), P_Instrument_pixels_y(position));
        tft.print("0");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_INDENT_X0 + 8 * Instrument_SPACE_X + 35), P_Instrument_pixels_y(position));
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(tuning_tone_volume / 20.0f, 2);

        Led_tuning_tone(patch_id);
    }
    else
    {
        tft.fillRect(0, P_Instrument_pixels_y(position) - 4, 320, 15, ILI9341_BLACK);
    }
}

FLASHMEM
void DisplayManager::P_show_gain_TT(int patch_id)
{
    auto position = Patch[patch_id].instruments;

    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Instrument_INDENT_X0 + 8 * Instrument_SPACE_X + 35), P_Instrument_pixels_y(position), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(tuning_tone_volume / 20.0f, 2);
}

FLASHMEM
void DisplayManager::P_Delete_Instrument(int position)
{
    tft.fillRect(5, P_Instrument_pixels_y(position), 307, 7, ILI9341_BLACK);
}

FLASHMEM
void DisplayManager::Note(int note_number)
{
    tft.print(note_name[note_number % 12]);
    tft.print((int)(note_number / 12.0f) + first_octave);
}

FLASHMEM
void DisplayManager::P_Performance_menu(void)
{
    auto position = 0; // position on display

    DisplayPrimitives::Delete_row(1);
    tft.setTextColor(MENU_COLOR);

    for (auto element = 0; element < 5; ++element) // menu element
    {
        if (Menu_P[element])
        {
            if (position == 0)
            {
                P_column_Menu_element[position] = 0;
            }

            else
            {
                P_column_Menu_element[position] = P_column_Menu_element[position - 1] + dimension_voice_Menu_P[element_Menu_P[position - 1]] + 1;
            }

            P_row_Menu_element[position] = 1;
            element_Menu_P[position] = element;
            position_Menu_P[element] = position;
            tft.setCursor(DisplayPrimitives::display_coordinate_x(P_column_Menu_element[position]), DisplayPrimitives::display_coordinate_y(P_row_Menu_element[position]));
            tft.print(Menu_P_char[element]);
            position++;
        }
    }
}

FLASHMEM
void DisplayManager::P_Frame_performance_menu(int position, bool fresh)
{
    if (!fresh)
    {
        DisplayPrimitives::Frame_by_col_row(P_column_Menu_element[P_menu_frame_on_element_0], P_row_Menu_element[P_menu_frame_on_element_0], dimension_voice_Menu_P[element_Menu_P[P_menu_frame_on_element_0]], false);
    }

    DisplayPrimitives::Frame_by_col_row(P_column_Menu_element[position], P_row_Menu_element[position], dimension_voice_Menu_P[element_Menu_P[position]], true);
    choice_performance_menu = element_Menu_P[position];
    P_menu_frame_on_element_0 = position;
}

void DisplayManager::P_Delete_all_frame_performance_menu(void)
{
    uint8_t position;

    for (auto element = 0; element < 5; ++element)
    {
        if (Menu_P[element])
        {
            position = position_Menu_P[element];
            DisplayPrimitives::Frame_by_col_row(P_column_Menu_element[position], P_row_Menu_element[position], dimension_voice_Menu_P[element], false);
        }
    }
}

FLASHMEM
void DisplayManager::P_Confirm_patch_change_popup(void)
{
    L_POPUP = 106;
    H_POPUP = 47;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP = (240 - H_POPUP) / 2;
    Y_POPUP_TXT = 10;
    Y_POPUP_OPT = 30;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(2), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("SAVE CHANGES?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(2), Y_POPUP + Y_POPUP_OPT);
    tft.print("EXIT");
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(8), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(12), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::P_Confirm_patch_change_popup_frame(int value)
{
    P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(2), Y_POPUP + Y_POPUP_OPT, 4, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(8), Y_POPUP + Y_POPUP_OPT, 2, false);
    P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(12), Y_POPUP + Y_POPUP_OPT, 3, false);

    switch (value)
    {
    case 0: // exit
        P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(2), Y_POPUP + Y_POPUP_OPT, 4, true);
        break;
    case 1: // no
        P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(8), Y_POPUP + Y_POPUP_OPT, 2, true);
        break;
    case 2: // yes
        P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(12), Y_POPUP + Y_POPUP_OPT, 3, true);
        break;
    default:
        break;
    }
}

void DisplayManager::P_Confirm_frame(int X, int Y, int chars, bool print)
{
    tft.drawRect(X - 4.0, Y - 4.0, (6 * chars) + 7, 15, (print ? ILI9341_WHITE : ILI9341_RED)); // drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
}

FLASHMEM
void DisplayManager::P_Confirm_patch_delete_popup(void)
{
    L_POPUP = 106;
    H_POPUP = 47;
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 10;
    Y_POPUP_OPT = 30;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("CONFIRM DELETE?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(5.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(9.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::P_Confirm_patch_delete_popup_frame(int value)
{
    P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(5.5), Y_POPUP + Y_POPUP_OPT, 2, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(9.5), Y_POPUP + Y_POPUP_OPT, 3, false);

    switch (value)
    {
    case 0: // NO
        P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(5.5), Y_POPUP + Y_POPUP_OPT, 2, true);
        break;
    case 1: // YES
        P_Confirm_frame(X_POPUP + DisplayPrimitives::display_coordinate_x(9.5), Y_POPUP + Y_POPUP_OPT, 3, true);
        break;
    default:
        break;
    }
}
FLASHMEM
void DisplayManager::Settings_page(void)
{
    tft.fillScreen(ILI9341_BLACK);

    DisplayPrimitives::Board(0, 0, 5); // Display.DisplayPrimitives::Board(float   col, float row, int chars)
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("SETUP");

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(2));
    tft.print("KEY STEP");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3));
    tft.print("FIRST OCTAVE");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(4));
    tft.print("OPTIMIZATION");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(5));
    tft.print(F("CONTROL CHANGE ASSIGNMENT"));
    // tft.setCursor(DisplayPrimitives::x_pos(0), DisplayPrimitives::display_coordinate_y(6));
    // tft.print("*FUTURE DEVELOPMENTS*");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6));
    tft.print(F("IMPORT RAW FILES FROM /LILLARAW"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(7));
    tft.print(F("IMPORT CONFIGURATION FROM /LILLASET/lilla.txt"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.print(F("EXPORT CONFIGURATION TO /LILLASET/lillaold.txt"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9));
    tft.print(F("FACTORY RESET"));

    Key_step();
    First_octave();
    Optimization();
}

FLASHMEM
void DisplayManager::Key_step(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(9), DisplayPrimitives::display_coordinate_y(2), 5);
    tft.setTextColor(ILI9341_YELLOW);

    switch (key_step)
    {
    case 0:
        tft.print(" 1st");
        break;

    case 1:
        tft.print("1/2st");
        break;

    case 2:
        tft.print("1/4st");
        break;

    case 3:
        tft.print("1/8st");
        break;

    default:
        tft.print(" 1st");
        break;
    }
}

FLASHMEM
void DisplayManager::First_octave(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(13), DisplayPrimitives::display_coordinate_y(3), 2);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(first_octave);
}

FLASHMEM
void DisplayManager::Optimization(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(13), DisplayPrimitives::display_coordinate_y(4), 29);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("POLYPHONY ");
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(POLYPHONY_FLASH[optimization]);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print(",  MAX PITCH ");
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(MAX_PITCH_FLASH[optimization]);
}

FLASHMEM
void DisplayManager::Settings_frame(int8_t value)
{
    DisplayPrimitives::Frame_by_col_row(9, 2, 5, false);   // First octave
    DisplayPrimitives::Frame_by_col_row(13, 3, 2, false);  // First octave
    DisplayPrimitives::Frame_by_col_row(13, 4, 29, false); // Optimization
    DisplayPrimitives::Frame_by_col_row(0, 5, 25, false);  // Control Change Assignment
    DisplayPrimitives::Frame_by_col_row(0, 6, 31, false);  // Import raw files
    DisplayPrimitives::Frame_by_col_row(0, 7, 45, false);  // Import configuration from
    DisplayPrimitives::Frame_by_col_row(0, 8, 46, false);  // Export configuration to SD
    DisplayPrimitives::Frame_by_col_row(0, 9, 13, false);  // Factory Reset

    switch (value)
    {
    case 0:
        DisplayPrimitives::Frame_by_col_row(9, 2, 5, true); // First octave
        break;
    case 1:
        DisplayPrimitives::Frame_by_col_row(13, 3, 2, true); // First octave
        break;
    case 2:
        DisplayPrimitives::Frame_by_col_row(13, 4, 29, true); // Optimization
        break;
    case 3:
        DisplayPrimitives::Frame_by_col_row(0, 5, 25, true); // Control Change
        break;
    case 4:
        DisplayPrimitives::Frame_by_col_row(0, 6, 31, true); // Import RAW files
        break;
    case 5:
        DisplayPrimitives::Frame_by_col_row(0, 7, 45, true); // Import configuration to SD
        break;
    case 6:
        DisplayPrimitives::Frame_by_col_row(0, 8, 46, true); // Export configuration to SD
        break;
    case 7:
        DisplayPrimitives::Frame_by_col_row(0, 9, 13, true); // Factory reset
        break;
    default:
        break;
    }
}

FLASHMEM
void DisplayManager::CC_page(void)
{
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("CONTROL CHANGE ASSIGNMENT");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(1));
    tft.setTextColor(MENU_COLOR);
    tft.print("RETURN");

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3));
    tft.print("GAIN SOUND 1");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(4));
    tft.print("GAIN SOUND 2");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(5));
    tft.print("GAIN SOUND 3");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6));
    tft.print("GAIN SOUND 4");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(7));
    tft.print("GAIN SOUND 5");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.print("GAIN SOUND 6");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9));
    tft.print("GAIN SOUND 7");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10));
    tft.print("GAIN SOUND 8");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(11));
    tft.print("LPF CUTOFF");
}

FLASHMEM
void DisplayManager::All_CC_Sound_gain(void)
{
    for (auto instrument_id = 0; instrument_id < INSTRUMENTS_MAX; ++instrument_id)
    {
        Show_CC_Sound_gain(instrument_id);
    }
}

FLASHMEM
void DisplayManager::Show_CC_Sound_gain(int value)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Control_change_X), DisplayPrimitives::display_coordinate_y(value + 3), 3);
    tft.setTextColor(ILI9341_YELLOW);

    if (CC_Sound_gain[value] > 0)
    {
        tft.print(CC_Sound_gain[value]);
    }
    else
    {
        tft.print(" -");
    }
}

FLASHMEM
void DisplayManager::Show_CC_lowpass_filter(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Control_change_X), DisplayPrimitives::display_coordinate_y(11), 3);
    tft.setTextColor(ILI9341_YELLOW);

    if (CC_lowpass_filter > 0)
    {
        tft.print(CC_lowpass_filter);
    }
    else
    {
        tft.print(" -");
    }
}

FLASHMEM
void DisplayManager::Frame_CC_page_menu(int value)
{
    DisplayPrimitives::Frame_by_col_row(0, 1, 6, false); // Return

    for (auto n = 0; n < 9; ++n)
    {
        DisplayPrimitives::Frame_by_col_row(Control_change_X, n + 3, 3, false);
    }

    if (value == 0)
    {
        DisplayPrimitives::Frame_by_col_row(0, 1, 6, true);
    }
    else
    {
        DisplayPrimitives::Frame_by_col_row(Control_change_X, value + 2, 3, true);
    }
}

FLASHMEM
void DisplayManager::Import_raw_files_frame(uint8_t value)
{
    DisplayPrimitives::Frame_by_col_row(0, 1, 4, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    DisplayPrimitives::Frame_by_col_row(5, 1, 6, false);

    switch (value)
    {
    case 0: // EXIT
        DisplayPrimitives::Frame_by_col_row(0, 1, 4, true);
        break;
    case 1: // IMPORT
        DisplayPrimitives::Frame_by_col_row(5, 1, 6, true);
        break;
    default:
        break;
    }
}
FLASHMEM
void DisplayManager::Confirm_config_import_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(46);
    H_POPUP = DisplayPrimitives::display_coordinate_y(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 10; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = DisplayPrimitives::display_coordinate_x(19);

    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);

    //       ("01234567890123456789012345678901234567891098765"); // 45 char
    tft.print(F("    WARNING: IMPORT CONFIGURATION FROM SD"));
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT + 15);
    tft.print(F(" WILL DELETE PATCHES, SOUNDS AND RECORDINGS!")); // 43
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT + 30);

    //       ("01234567890123456789012345678901234567891098765"); // 45 char
    tft.print(F("        DO YOU REALLY WANT TO PROCEED?"));
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + X_POPUP_OPT + DisplayPrimitives::display_coordinate_x(4), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Factory_reset_wait_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(38);
    H_POPUP = DisplayPrimitives::display_coordinate_y(3); // 64 pixel
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 10; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_YELLOW);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_BLACK);

    //       ("01234567890123456789012345678901234567");
    tft.print(F("            FACTORY RESET"));
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(0), Y_POPUP + Y_POPUP_TXT + DisplayPrimitives::display_coordinate_y(1));
    tft.setTextColor(ILI9341_BLACK);

    //       ("01234567890123456789012345678901234567");
    tft.print(F("    PLEASE WAIT - DO NOT SWITCH OFF"));
}

FLASHMEM
void DisplayManager::Encoder_pushbutton_test_board(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(32);
    H_POPUP = DisplayPrimitives::display_coordinate_y(6) - 4; // 64 pixel
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_WHITE);

    tft.setTextColor(ILI9341_RED);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + 4);
    tft.print(F("ENCODERS AND PUSHBUTTONS TEST"));
}

void DisplayManager::Encoder_pushbutton_test_result(const int device, const int element, const int value)
{
    Encoder_pushbutton_test_board();

    struct Result
    {
        int device;
        int element;
        int value;
    };
    static Result memo[5] = {};

    memo[4] = memo[3];
    memo[3] = memo[2];
    memo[2] = memo[1];
    memo[1] = memo[0];
    memo[0] = {device, element, value};

    tft.setTextColor(ILI9341_BLACK);

    for (auto i = 0; i < 5; ++i)
    {
        tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + DisplayPrimitives::display_coordinate_y(5 - i));
        if (memo[i].device == 1)
        {
            tft.print("encoder ");
            tft.print(memo[i].element);
            tft.print(" value ");
            tft.print(memo[i].value);
        }
        if (memo[i].device == 2)
        {
            tft.print("pushbutton ");
            tft.print(memo[i].element);
            tft.print(" pressed");
        }
    }
}

FLASHMEM
void DisplayManager::SD_missing(uint16_t color)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(18);
    H_POPUP = DisplayPrimitives::display_coordinate_y(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, color);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);

    //       ("012345678901234567");
    tft.print(F("  SD NOT PRESENT"));
}

FLASHMEM
void DisplayManager::Config_import_FILE_error_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(35);
    H_POPUP = DisplayPrimitives::display_coordinate_y(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_YELLOW);

    //       ("01234567890123456789012345678901234");
    tft.print(F("  SD/LILLASET/lilla.txt NOT FOUND"));
}

FLASHMEM
void DisplayManager::Config_import_REBOOT_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(38);
    H_POPUP = DisplayPrimitives::display_coordinate_y(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);

    //           ("01234567890123456789012345678901234567");
    tft.print(F(" LOADING NEW CONFIGURATION AND REBOOT"));
}

FLASHMEM
void DisplayManager::Confirm_config_import_frame(uint8_t value)
{
    P_Confirm_frame(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT, 2, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    P_Confirm_frame(X_POPUP + X_POPUP_OPT + DisplayPrimitives::display_coordinate_x(4), Y_POPUP + Y_POPUP_OPT, 3, false);

    switch (value)
    {
    case 0: // NO
        P_Confirm_frame(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT, 2, true);
        break;
    case 1: // YES
        P_Confirm_frame(X_POPUP + X_POPUP_OPT + DisplayPrimitives::display_coordinate_x(4), Y_POPUP + Y_POPUP_OPT, 3, true);
        break;
    default:
        break;
    }
}

FLASHMEM
void DisplayManager::Confirm_config_export_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(51);
    H_POPUP = DisplayPrimitives::display_coordinate_y(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 10; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = DisplayPrimitives::display_coordinate_x(19);

    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);

    //         ("012345678901234567890123456789012345678901234567890");
    tft.print(F("        WARNING: EXPORT CONFIGURATION TO SD"));
    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT + 15);
    tft.print(F("  WILL DELETE A PREVIOUS CONFIGURATION FILE SAVED")); // 43
    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT + 30);
    tft.print(F("        DO YOU REALLY WANT TO PROCEED?"));
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + X_POPUP_OPT + DisplayPrimitives::display_coordinate_x(4), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Config_export_SD_error_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(27);
    H_POPUP = DisplayPrimitives::display_coordinate_y(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);

    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_YELLOW);

    //       ("012345678901234567890123456");
    tft.print(F(" INSUFFICIENT SPACE IN SD"));
}

FLASHMEM
void DisplayManager::Config_export_save_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(50);
    H_POPUP = DisplayPrimitives::display_coordinate_y(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);
    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);

    //       ("01234567890123456789012345678901234567890123456789");
    tft.print(F("  CONFIGURATION SAVED: SD/LILLASET/lillaold.txt"));
}

FLASHMEM
void DisplayManager::Confirm_factory_reset_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(49);
    H_POPUP = DisplayPrimitives::display_coordinate_y(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 15; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = DisplayPrimitives::display_coordinate_x(19);

    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);

    //       ("0123456789012345678901234567890123456789012345678");
    tft.print(F("      WARNING: FACTORY RESET WILL DELETE"));
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT + 15);
    tft.print(F("      ALL PATCHES, SOUNDS AND RECORDINGS!"));
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT + 30);
    tft.print(F("        DO YOU REALLY WANT TO PROCEED?"));
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + X_POPUP_OPT + DisplayPrimitives::display_coordinate_x(4), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Config_reset_popup(void)
{
    L_POPUP = DisplayPrimitives::display_coordinate_x(49);
    H_POPUP = DisplayPrimitives::display_coordinate_y(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 10; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = DisplayPrimitives::display_coordinate_x(19);

    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT + 15);
    tft.setTextColor(ILI9341_WHITE);

    //       ("0123456789012345678901234567890123456789012345678");
    tft.print(F("   PLEASE WAIT. LILLA WILL RESTART AFTER RESET")); // 43
}

FLASHMEM
void DisplayManager::SOUND_EDIT_menu(void)
{
    if (Lilla_state_0 != MIDI_LOOP)
    {
        auto position = 0;

        // Select_sound_edit_menu_elements(); // DISTRIBUITO
        DisplayPrimitives::Delete_row(1);

        for (auto element = 0; element < 3; ++element) // menu element
        {
            if (Menu_So[element])
            {
                if (position == 0)
                {
                    X_position_Menu_So[position] = 0;
                }

                else
                {
                    X_position_Menu_So[position] = X_position_Menu_So[position - 1] + dimension_voice_Menu_So[element_Menu_So[position - 1]] + 1;
                }

                element_Menu_So[position] = element;
                position_Menu_So[element] = position;
                tft.setCursor(DisplayPrimitives::display_coordinate_x(X_position_Menu_So[position]), DisplayPrimitives::display_coordinate_y(1));
                tft.setTextColor(MENU_COLOR);
                tft.print(Menu_So_char[element]);
                position++;
            }
        }
    }
}

FLASHMEM
void DisplayManager::Frame_SOUND_EDIT_menu(int position)
{
    if (Lilla_state_0 != MIDI_LOOP)
    {
        Delete_all_frame_SOUND_EDIT_menu();
        DisplayPrimitives::Frame_by_col_row(X_position_Menu_So[position], 1, dimension_voice_Menu_So[element_Menu_So[position]], true);
        choice_sound_menu = element_Menu_So[position];
    }
}

FLASHMEM
void DisplayManager::Delete_all_frame_SOUND_EDIT_menu(void)
{
    int position;

    for (auto element = 0; element < 3; ++element)
    {
        if (Menu_So[element])
        {
            position = position_Menu_So[element];
            DisplayPrimitives::Frame_by_col_row(X_position_Menu_So[position], 1, dimension_voice_Menu_So[element], false);
        }
    }
}

FLASHMEM
void DisplayManager::Instrument_VCF_page(int patch_id, int instrument_id)
{
    auto sound_id = Patch[patch_id].Instrument[instrument_id].sound_id;

    tft.fillScreen(ILI9341_BLACK);
    if (Lilla_state_0 != LIVE_SAMPLING)
    {
        if (Lilla_state_0 != MIDI_LOOP)
        {
            P_Show_Performance();
        }
        else
        {
            Loop_midi_loop_title();
        }

        P_Show_Patch_number(false);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(23), DisplayPrimitives::display_coordinate_y(0));
        tft.setTextColor(TEXT_COLOR);
        tft.print("SOUND");
        tft.setCursor(DisplayPrimitives::display_coordinate_x(28.5), DisplayPrimitives::display_coordinate_y(0));
        tft.setTextColor(ILI9341_WHITE);
        tft.print(instrument_id + 1);

        tft.setCursor(DisplayPrimitives::display_coordinate_x(43), DisplayPrimitives::display_coordinate_y(0));
        tft.setTextColor(TEXT_COLOR);
        tft.print("GAIN");
        Show_VCF_gain(sound_id);
    }
    else
    {
        //("012345678901234567890"); // Size 1: 21 chars
        DisplayPrimitives::Board(0, 0, 12); // Display.DisplayPrimitives::Board(float & col, float row, int chars)
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
        tft.print("LIVE SAMPLER");

        tft.setCursor(DisplayPrimitives::display_coordinate_x(41), DisplayPrimitives::display_coordinate_y(0));
        tft.setTextColor(TEXT_COLOR);
        tft.print("VOLUME");
        P_Patch_volume_value(true); // true: YELLOW
    }

    if (Lilla_state_0 != MIDI_LOOP)
    {
        tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(1));
        tft.setTextColor(MENU_COLOR);
        tft.print("RETURN");
        DisplayPrimitives::Frame_by_col_row(0, 1, 6, true);
    }

    Show_effects();
    Show_VCF_solo();

    DisplayPrimitives::Board(0, Instrument_VCF_TXT_Y + 1.8, 9); // Display.DisplayPrimitives::Board(float   col, float row, int chars)
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 1.8));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("VCF + LFO");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("FILTER TYPE");
    Show_VCF_filter_type(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("CUTOFF (PITCH 1.0)");
    Show_VCF_cutoff(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("RESONANCE");
    Show_VCF_resonance(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_VCF_TXT_X), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MODULATION_SOURCE");
    Show_VCF_lfo_type(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_VCF_TXT_X), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD FREQ/TIME");
    Show_VCF_lfo_freq_time(instrument_id);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(Instrument_VCF_TXT_X), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD DEPTH");
    Show_VCF_lfo_index(instrument_id);
}
FLASHMEM
void DisplayManager::Show_VCF_gain(int sound_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(47.5), DisplayPrimitives::display_coordinate_y(0), 4);
    tft.setTextColor((Lilla_state_0 == LIVE_SAMPLING ? ILI9341_WHITE : ILI9341_YELLOW));
    tft.print(Sound[sound_id].gain / 20.0f);
}

FLASHMEM
void DisplayManager::Show_VCF_solo(void)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(21.5), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y - .1), 8);
    if (solo_flag)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print("* SOLO *");
    }
}

FLASHMEM
void DisplayManager::Show_VCF_filter_type(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(12), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 3), 13);
    tft.setTextColor(ILI9341_YELLOW);

    if (Preset[instrument_id].Filter.use == 1)
    {
        switch (Preset[instrument_id].Filter.type)
        {
        case 0: // LP
            tft.print("LOWPASS");
            break;
        case 1: // HP
            tft.print("HIGHPASS");
            break;
        case 2: // BP
            tft.print("BANDPASS");
            break;
        case 3: // Notch
            tft.print("NOTCH");
            break;
        default:
            break;
        }
    }
    else
        tft.print("NONE (BYPASS)");
}

FLASHMEM
void DisplayManager::Show_VCF_cutoff(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(19), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 4), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument_id].Filter.pivot, 0);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("Hz");
}

FLASHMEM
void DisplayManager::Show_VCF_resonance(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(10), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 5), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument_id].Filter.resonance, 2);
}

FLASHMEM
void DisplayManager::Show_VCF_lfo_type(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Instrument_VCF_TXT_X + 11), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 3), 9);
    tft.setTextColor(ILI9341_YELLOW);

    switch (Preset[instrument_id].Filter.modulation)
    {
    case 0:
        tft.print("NONE");
        break;
    case 1:
        tft.print("RISING");
        break;
    case 2:
        tft.print("FALLING");
        ;
        break;
    case 3:
        tft.print("LFO");
        break;
    case 4:
        tft.print("LFO+CC7");
        break;
    default:
        break;
    }

    Show_VCF_lfo_freq_time(instrument_id);
}

FLASHMEM
void DisplayManager::Show_VCF_lfo_freq_time(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Instrument_VCF_TXT_X + 14), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 4), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument_id].Filter.frequency_time, 2);

    if (Preset[instrument_id].Filter.periodic)
    {
        DisplayPrimitives::Unit("Hz", 2);
    }
    else
    {
        DisplayPrimitives::Unit("sec", 3);
    }
}

FLASHMEM
void DisplayManager::Show_VCF_lfo_index(int instrument_id)
{
    Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(Instrument_VCF_TXT_X + 10), DisplayPrimitives::display_coordinate_y(Instrument_VCF_TXT_Y + 5), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(100 * Preset[instrument_id].Filter.index, 0);
    tft.print("%");
}

FLASHMEM
void DisplayManager::Show_VCF_pan(int sound_id)
{
    Cancel_text_reset_cursor(259, Instrument_VCF_TXT_Y + 93, 2);

    if (Sound[sound_id].pan == 0)
    {
        tft.setTextColor(ILI9341_WHITE);
        tft.print("0");
        return;
    }

    tft.setTextColor(ILI9341_WHITE);

    if (Sound[sound_id].pan < 0)
    {
        tft.print("L");
    }
    else if (Sound[sound_id].pan > 0)
    {
        tft.print("R");
    }

    tft.print(abs(Sound[sound_id].pan));
}

FLASHMEM
void DisplayManager::Make_VFS_presentation(void)
{
    tft.fillScreen(ILI9341_BLACK);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print(F("SAMPLER: RECORDING MEMORY DIMENSION"));

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(2));
    tft.setTextColor(ILI9341_YELLOW);
    //        "012345678901234567890 234 X 432 98765432109876543210"; // max 52 char
    tft.print(F(" PLEASE ASSIGN THE MEMORY SPACE FOR RECORDINGS (AND"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3));
    tft.print(F("     THE CONSEQUENT SPACE FOR RAW FILES EXPORT)"));

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(5));
    tft.setTextColor(TEXT_COLOR);
    tft.print(F("TOTAL FLASH MEMORY SPACE "));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(Get_flash_size() / 1048576.0f);
    DisplayPrimitives::Unit("MB", 2);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6));
    tft.setTextColor(TEXT_COLOR);
    tft.print(F("RAW FILES IMPORTED "));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(Get_flash_occupation() / 1048576.0f);
    DisplayPrimitives::Unit("MB", 2);
}

FLASHMEM
void DisplayManager::Make_VFS_assignments(void)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(7));
    tft.setTextColor(TEXT_COLOR);

    // tft.print("0123456789012345678901234567890....."); // max 52 char
    tft.print(F("SPACE FOR RECORDINGS"));

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.setTextColor(TEXT_COLOR);

    // tft.print("0123456789012345678901234567890....."); // max 52 char
    tft.print(F("SPACE FOR RAW FILES EXPORT"));
}

FLASHMEM
void DisplayManager::Make_VFS_restart(void)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10));
    tft.setTextColor(ILI9341_GREEN);
    tft.print(ADV_VFS_3);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(11));
    tft.print(ADV_VFS_4);
}

FLASHMEM
void DisplayManager::Make_VFS_not_enough_memory_for_sampler(void)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10));
    tft.setTextColor(ILI9341_MAGENTA);
    tft.print(ADV_VFS_0);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(11));
    tft.print(ADV_VFS_1);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(12));
    tft.print(ADV_VFS_2);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(13));
    tft.print(ADV_VFS_3);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(14));
    tft.print(ADV_VFS_4);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_titolo(void)
{
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.print(F("IMPORT RAW FILES FROM SD TO FLASH MEMORY"));
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_waiting_for_SD(void)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3));
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(F("WAITING 10sec FOR SD CARD"));
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_lillaraw_missing(void)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3));
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(F("NO FILES TO IMPORT: MISSING /LILLARAW DIRECTORY!"));
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_files_report(unsigned long SD_raw_volume, int SD_raw_files, int raw_files_volume, int flash_raw_files)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3));
    tft.setTextColor(TEXT_COLOR);
    tft.print(F("SOURCE: SD CARD /LILLARAW"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(4));
    tft.print(F("- RAW FILES"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(12), DisplayPrimitives::display_coordinate_y(4));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(SD_raw_files);
    tft.print(" (");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(SD_raw_volume / 1048576.0f, 2);
    DisplayPrimitives::Unit("MB", 2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");

    // Flash chip info
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6));
    tft.setTextColor(TEXT_COLOR);
    tft.println(F("DESTINATION: LILLA FLASH MEMORY"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(7));
    tft.println(F("- DIMENSION"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(12), DisplayPrimitives::display_coordinate_y(7));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(flash_dimension_MB);
    DisplayPrimitives::Unit("MB", 2);

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(8));
    tft.setTextColor(TEXT_COLOR);
    tft.print("- RAW FILES");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(12), DisplayPrimitives::display_coordinate_y(8));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(flash_raw_files); // Get_raw_files()
    tft.print(" (");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(raw_files_volume / 1048576.0f, 2); // Get_raw_files_volume()
    DisplayPrimitives::Unit("MB", 2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");

    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(9));
    tft.setTextColor(TEXT_COLOR);
    tft.print(F("- RECORDINGS"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(13), DisplayPrimitives::display_coordinate_y(9));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(recordings);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_last_warning(float erasing_time_ms)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(10));
    tft.setTextColor(TEXT_COLOR);
    tft.println(F("- DELETING TIME"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(16), DisplayPrimitives::display_coordinate_y(10));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(erasing_time_ms / 60000, 1);
    DisplayPrimitives::Unit("min", 3);

    // display SD->Flash menu
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(1));
    tft.setTextColor(MENU_COLOR);
    tft.print("EXIT IMPORT");

    tft.setTextColor(ILI9341_MAGENTA);

    //        "012345678901234567890 234 X 432 98765432109876543210"); // max 52 char
    tft.setCursor(DisplayPrimitives::display_coordinate_x(22), DisplayPrimitives::display_coordinate_y(12) - 5);
    tft.print("IMPORTANT");
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(13) - 5);
    tft.print(F("- RAW FILES IMPORT WILL DELETE ALL AUDIO FILES AND"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(14) - 5);
    tft.print(F("  RECORDINGS IN LILLA!"));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(15) - 5);
    tft.print(F("- LILLA REQUIRES .raw (lowercase!)"));
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_job_start(void)
{
    // Start erasing flash chip
    tft.fillRect(0, DisplayPrimitives::display_coordinate_y(11), 320, 240, ILI9341_BLACK);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(12) - 5);
    tft.setTextColor(ILI9341_YELLOW);

    //        "012345678901234567890 234 X 432 98765432109876543210"); // max 52 char
    tft.print(F("PLEASE WAIT: FLASH MEMORY ERASE IS RUNNING."));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(13) - 5);
    tft.print(F("THAN RAW FILES WILL BE COPYED FROM SD/LILLARAW TO "));
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(14) - 5);
    tft.print("LILLA FLASH MEMORY");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_initial_percentage(void)
{
    tft.drawLine(DisplayPrimitives::display_coordinate_x(0), BAR_POS_Y, DisplayPrimitives::display_coordinate_x(0), BAR_POS_Y + 5, ILI9341_YELLOW);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0) + 10, BAR_POS_Y);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("0%");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_progress(unsigned char barcount)
{
    tft.drawLine(DisplayPrimitives::display_coordinate_x(0) + barcount, BAR_POS_Y, DisplayPrimitives::display_coordinate_x(0) + barcount, BAR_POS_Y + 5, ILI9341_YELLOW);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0) + barcount + 10, BAR_POS_Y);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(barcount);
    tft.print("%");

    if (barcount == 100)
    {
        tft.print(" *DONE*");
    }
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_popup_landscape(void)
{
    // Start copying RAW files from SD to Flash chip
    tft.fillRect(0, 12, 320, 240, ILI9341_BLACK);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_list_landscape(void)
{
    tft.fillRect(0, DisplayPrimitives::display_coordinate_y(3), 320, 240, ILI9341_BLACK);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_files_to_copy(int row, const char *filename, unsigned long length)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(row));
    tft.setTextColor(TEXT_COLOR);
    tft.print("COPYING FILE ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(filename);
    tft.print("  ");
    tft.print(length / 1024);
    tft.print("KB");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_flash_error(void)
{
    tft.setTextColor(TEXT_COLOR);
    tft.print("  FLASH MEMORY ERROR");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_flash_full_error(void)
{
    tft.setTextColor(TEXT_COLOR);
    tft.print("  ERROR: FLASH MEMORY FULL!");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_job_done(void)
{
    // Display RAW files list
    tft.fillRect(0, 12, 320, 240, ILI9341_BLACK);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(2));
    tft.setTextColor(TEXT_COLOR);
    tft.print(F("RAW FILES IMPORT COMPLETED. FILE LIST:"));
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_file_copied(int row, const char *filename, uint32_t filesize)
{
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(row));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(filename);
    tft.print("  ");
    tft.print(filesize / 1024);
    tft.print("KB");
}

void DisplayManager::Show_wave(int instrument_id)
{
    auto sound_id_local = Patch[Patch_id].Instrument[instrument_id].sound_id;
    int yp, yn, y0;
    int NC_A;
    int16_t *X = Info.Sound_620_samples_array(Preset[instrument_id].file, Preset[instrument_id].A, Preset[instrument_id].B);
    float volume_float = Volume_float[Sound[sound_id_local].gain];

    NC_A = ((DisplayPrimitives::WAVEBOARD_WIDTH * (Preset[instrument_id].Noclick < Noclick_max ? Preset[instrument_id].Noclick : Noclick_max)) / (Preset[instrument_id].B - Preset[instrument_id].A));

    if (NC_A == 0 && (Preset[instrument_id].Noclick > 0))
    {
        NC_A = 1;
    }

    int NC_B = DisplayPrimitives::WAVEBOARD_WIDTH - NC_A;
    int wave_pixel_0 = 0;
    y0 = DisplayPrimitives::CANVAS_WAVE_0; // central position

    // tft.fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    canvas.fillRect(0, 0, DisplayPrimitives::WAVEBOARD_WIDTH, DisplayPrimitives::WAVEBOARD_HEIGHT, WAVE_BOARD_COLOR);
    canvas.fillRect(0, 0, NC_A, DisplayPrimitives::WAVEBOARD_HEIGHT, CURTAIN_NOCLICK_COLOR);
    canvas.fillRect(NC_B, 0, NC_A, DisplayPrimitives::WAVEBOARD_HEIGHT, CURTAIN_NOCLICK_COLOR);

    for (auto wave_pixel = 0; wave_pixel < DisplayPrimitives::WAVEBOARD_WIDTH; ++wave_pixel)
    {
        // 16bits>>11 = 5bits from -16 to +15
        yp = DisplayPrimitives::CANVAS_WAVE_0 - (*(X + wave_pixel) >> 10) * volume_float; // MAXIMUM delta 32*4 = 128  (−32768 <= int16_t <= +32767)
        yn = DisplayPrimitives::CANVAS_WAVE_0 - (*(X + wave_pixel + DisplayPrimitives::WAVEBOARD_WIDTH) >> 10) * volume_float;

        if (yp == DisplayPrimitives::CANVAS_WAVE_0 && yn == DisplayPrimitives::CANVAS_WAVE_0)
        {
            if (wave_pixel == 0)
            {
                canvas.drawPixel(0, yp, WAVE_COLOR);
            }
            else
            {
                canvas.drawLine(wave_pixel_0, y0, wave_pixel, DisplayPrimitives::CANVAS_WAVE_0, WAVE_COLOR);
                wave_pixel_0 = wave_pixel;
                y0 = yp;
            }
        }

        else if (yp < DisplayPrimitives::CANVAS_WAVE_0 && yn == DisplayPrimitives::CANVAS_WAVE_0)
        {
            if (wave_pixel > 0)
            {
                canvas.drawLine(wave_pixel_0, y0, wave_pixel, yp, WAVE_COLOR);
            }
            else
            {
                canvas.drawPixel(0, yp, WAVE_COLOR);
            }

            wave_pixel_0 = wave_pixel;
            y0 = yp;
        }

        else if (yp == DisplayPrimitives::CANVAS_WAVE_0 && yn > DisplayPrimitives::CANVAS_WAVE_0)
        {
            if (wave_pixel > 0)
            {
                canvas.drawLine(wave_pixel_0, y0, wave_pixel, yn, WAVE_COLOR);
            }
            else
            {
                canvas.drawPixel(0, yn, WAVE_COLOR);
            }

            wave_pixel_0 = wave_pixel;
            y0 = yn;
        }
        else if (yp < DisplayPrimitives::CANVAS_WAVE_0 && yn > DisplayPrimitives::CANVAS_WAVE_0)
        {
            if (wave_pixel > 0)
            {
                canvas.drawLine(wave_pixel_0, y0, wave_pixel, yn, WAVE_COLOR);
                canvas.drawLine(wave_pixel, yp, wave_pixel, yn, WAVE_COLOR);
            }
            else
            {
                canvas.drawLine(0, yp, 0, yn, WAVE_COLOR);
            }

            wave_pixel_0 = wave_pixel;
            y0 = yn;
        }
    }

    if (solo_flag)
    {
        canvas.setCursor(DisplayPrimitives::display_coordinate_x(21) + 5, DisplayPrimitives::display_coordinate_y(0));
        canvas.setTextColor(ILI9341_YELLOW);
        canvas.print("* SOLO *");
    }

    canvas.setTextColor(TEXT_COLOR);
    canvas.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::Y_FOOTER_TEXT);
    canvas.print("FROM");
    canvas.setTextColor((slicing_mode ? ILI9341_WHITE : ILI9341_YELLOW));
    canvas.setCursor(DisplayPrimitives::display_coordinate_x(4.5), DisplayPrimitives::Y_FOOTER_TEXT);
    canvas.setTextColor(ILI9341_YELLOW);
    canvas.print(Preset[instrument_id].A);
    canvas.setTextColor(ILI9341_ORANGE);
    canvas.print("S");

    canvas.setTextColor(TEXT_COLOR);
    canvas.setCursor(DisplayPrimitives::display_coordinate_x(21), DisplayPrimitives::Y_FOOTER_TEXT);
    canvas.print("TOT");
    canvas.setTextColor((slicing_mode ? ILI9341_WHITE : ILI9341_YELLOW));
    canvas.setCursor(DisplayPrimitives::display_coordinate_x(24.5), DisplayPrimitives::Y_FOOTER_TEXT);

    float time = (Preset[instrument_id].B - Preset[instrument_id].A + 1) / 44100.0f; // sec

    if (time > 1.0)
    {
        canvas.print(time, 2);
        canvas.setTextColor(ILI9341_ORANGE);
        canvas.print("sec");
    }

    else
    {
        canvas.print(time * 1000.0, 2);
        canvas.setTextColor(ILI9341_ORANGE);
        canvas.print("ms");
    }

    if (slicing_mode) // FIRST/LAST
    {
        String Value = String(Preset[instrument_id].B, DEC);
        canvas.setCursor(DisplayPrimitives::display_coordinate_x(46 - Value.length() - 1), DisplayPrimitives::Y_FOOTER_TEXT);
        canvas.setTextColor(TEXT_COLOR);
        canvas.print("TO");
        canvas.setCursor(DisplayPrimitives::display_coordinate_x(48.5 - Value.length() - 1), DisplayPrimitives::Y_FOOTER_TEXT);
        canvas.setTextColor(ILI9341_YELLOW);
        canvas.print(Value);
        canvas.setTextColor(ILI9341_ORANGE);
        canvas.print("S");
    }

    else // Window fissa: FIRST/WINDOW
    {
        String Value = String(Preset[instrument_id].B - Preset[instrument_id].A + 1, DEC);
        canvas.setCursor(DisplayPrimitives::display_coordinate_x(45 - Value.length() - 1), DisplayPrimitives::Y_FOOTER_TEXT);
        canvas.setTextColor(TEXT_COLOR);
        canvas.print("SLICE");
        canvas.setCursor(DisplayPrimitives::display_coordinate_x(50.5 - Value.length() - 1), DisplayPrimitives::Y_FOOTER_TEXT);
        canvas.setTextColor(ILI9341_YELLOW);
        canvas.print(Value);
        canvas.setTextColor(ILI9341_ORANGE);
        canvas.print("S");
    }

    tft.drawRGBBitmap(DisplayPrimitives::X_WAVEBOARD_LEFT, DisplayPrimitives::WAVE_MAX, canvas.getBuffer(), canvas.width(), canvas.height());
}

void DisplayManager::Loop_led(int track, int instrument_id, bool on)
{
    tft.drawBitmap(Loop_LED_X + track * 42, Loop_LED_Y + instrument_id * Loop_LED_DY, led_pic, 8, 8, on ? GREEN_ON : GREEN_OFF);
}

void DisplayManager::Loop_led_metronomo(int Xled, int Yled, bool ONled)
{
    tft.drawBitmap(Xled, Yled, led_pic, 8, 8, ONled ? RED_ON : RED_OFF);
}