/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#include "DisplayManager.h"

void DisplayManager::Start(void)
{
    // Start Display
    tft.begin();
    tft.setRotation(1);
    tft.setTextWrap(false);
    tft.fillScreen(ILI9341_BLACK);
    canvas.setTextWrap(false);
}

void DisplayManager::Lilla_cover_slow(void)
{
    tft.fillScreen(ILI9341_BLACK);

    // fade-in
    for (int i = 0; i <= 10; ++i)
    {
        Logo(i / 10.0f);
        delay(50);
    }
    for (int i = 0; i <= 20; ++i)
    {
        Cover_text(i / 20.0f);
        delay(50);
    }

    // fade-out
    delay(4000);
    for (int i = 50; i >= 0; --i)
    {
        Cover_text(i / 50.0f);
        delay(30);
    }
    for (int i = 20; i >= 0; --i)
    {
        Logo(i / 20.0f);
        delay(30);
    }

    // all black
    tft.fillScreen(ILI9341_BLACK);
    delay(200);
}

FLASHMEM
void DisplayManager::Show_Session_number(bool change_session)
{
    tft.setCursor(x_pos(12 + 18), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PATCH");
    tft.setCursor(x_pos(17.5 + 18), y_pos(0));
    tft.setTextColor(change_session ? ILI9341_YELLOW : ILI9341_WHITE);
    tft.print(session);
}

FLASHMEM
void DisplayManager::Session_volume(bool change_vol)
{
    tft.setCursor(x_pos(41), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME");
    Session_volume_value(change_vol);
}

FLASHMEM
void DisplayManager::Session_volume_value(bool change_vol)
{
    Cancel_text_reset_cursor(x_pos(47.5), y_pos(0), 4);
    tft.setTextColor(ILI9341_YELLOW); // tft.setTextColor(change_vol ? ILI9341_YELLOW : ILI9341_WHITE);
    tft.print(volume_session / 20.0f, 2);
}

FLASHMEM
void DisplayManager::Loop_rec_advice(int track, bool on)
{
    if (on)
    {
        tft.setCursor(x_pos(Loop_LOOPS_X + 7 * track), y_pos(Loop_HEAD_R));
        tft.setTextColor(ILI9341_WHITE);
        tft.print(track + 1);
        tft.print("-REC");
    }
    else
    {
        Cancel_text_reset_cursor(x_pos(Loop_LOOPS_X + 7 * track), y_pos(Loop_HEAD_R), 5);
    }
}

void DisplayManager::Popup(String text, uint16_t text_color, uint16_t filler_color)
{
    L_POPUP = x_pos(text.length() + 4); // lunghezza
    H_POPUP = y_pos(3);                 // altezza
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, filler_color);
    tft.setCursor(X_POPUP + x_pos(2), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(text_color);
    tft.print(text);
}

void DisplayManager::Board(float col, float row, int chars)
{
    tft.fillRect(x_pos(col) - 4, y_pos(row) - 2, (6 * chars) + 7, 11, 0x9000); // fillRect(uint16_t x0, uint16_t y0, uint16_t width, uint16_t heigh, uint16_t color);
}

void DisplayManager::Frame(float col, float row, int chars, bool print)
{
    tft.drawRect(x_pos(col) - 4, y_pos(row) - 4, (6 * chars) + 7, 15, (print ? FRAME_COLOR : ILI9341_BLACK)); // drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
}

void DisplayManager::Instrument_frame_on_position(float position, bool print)
{
    tft.drawRect(4, Instrument_Y_POSITION(position) - 4, 311, 15, (print ? FRAME_COLOR : ILI9341_BLACK)); // drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
}

void DisplayManager::Led_instrument_PERFORMANCE(uint8_t instrument)
{
    if (Lilla_state == PERFORMANCE || Lilla_state == MIDI_LOOP)
    {
        if (Players_statistics.Read_total_Players_per_instrument(instrument) == 0)
        {
            tft.drawBitmap(X_LED, Instrument_Y_POSITION(position_of_Instrument[instrument]), led_pic, 8, 8, (MX_mute[instrument] ? RED_OFF : GREEN_OFF));
        }
        else
        {
            tft.drawBitmap(X_LED, Instrument_Y_POSITION(position_of_Instrument[instrument]), led_pic, 8, 8, (MX_mute[instrument] ? RED_ON : GREEN_ON));
        }
        return;
    }
}

void DisplayManager::Led_instrument_SOUND_EDIT(uint8_t instrument)
{
    if (Players_statistics.Read_total_Players_per_instrument(instrument) == 0)
    {
        tft.drawBitmap(x_pos(22) - 4, y_pos(0), led_pic, 8, 8, (MX_mute[instrument] ? RED_OFF : GREEN_OFF));
    }
    else
    {
        tft.drawBitmap(x_pos(22) - 4, y_pos(0), led_pic, 6, 8, (MX_mute[instrument] ? RED_ON : GREEN_ON));
    }
    return;
}

void DisplayManager::Led_instrument_INSTRUMENT_VCF(uint8_t instrument)
{
    if (Lilla_state_0 == PERFORMANCE)
    {
        if (Players_statistics.Read_total_Players_per_instrument(instrument) == 0)
        {
            tft.drawBitmap(x_pos(22) - 4, y_pos(0), led_pic, 8, 8, (MX_mute[instrument] ? RED_OFF : GREEN_OFF));
        }
        else
        {
            tft.drawBitmap(x_pos(22) - 4, y_pos(0), led_pic, 6, 8, (MX_mute[instrument] ? RED_ON : GREEN_ON));
        }
        return;
    }
    else if (Lilla_state_0 == LIVE_SAMPLING)
    {
        if (Players_statistics.Read_total_Players_per_instrument(instrument) == 0)
        {
            tft.drawBitmap(x_pos(40) - 4, y_pos(0), led_pic, 8, 8, (MX_mute[instrument] ? RED_OFF : GREEN_OFF));
        }
        else
        {
            tft.drawBitmap(x_pos(40) - 4, y_pos(0), led_pic, 6, 8, (MX_mute[instrument] ? RED_ON : GREEN_ON));
        }
        return;
    }
}

void DisplayManager::Led_instrument_DIRECT_SAMPLING(void)
{
    // SESSIONS_MAX ha 2 instrument
    if (Players_statistics.Read_total_Players_per_instrument(0) == 0 && Players_statistics.Read_total_Players_per_instrument(1) == 0)
    {
        tft.drawBitmap(x_pos(0), y_pos(8), led_pic, 8, 8, ((MX_mute[0] && MX_mute[1]) ? RED_OFF : GREEN_OFF));
    }
    else
    {
        tft.drawBitmap(x_pos(0), y_pos(8), led_pic, 6, 8, ((MX_mute[0] && MX_mute[1]) ? RED_ON : GREEN_ON));
    }
    return;
}

void DisplayManager::Led_instrument_LIVE_SAMPLING(void)
{
    if (Players_statistics.Read_total_Players_per_instrument(0) == 0 && Players_statistics.Read_total_Players_per_instrument(1) == 0)
    {
        tft.drawBitmap(x_pos(0), y_pos(3), led_pic, 8, 8, ((MX_mute[0] && MX_mute[1]) ? RED_OFF : GREEN_OFF));
    }
    else
    {
        tft.drawBitmap(x_pos(0), y_pos(3), led_pic, 6, 8, ((MX_mute[0] && MX_mute[1]) ? RED_ON : GREEN_ON));
    }
    return;
}

void DisplayManager::Led_tuning_tone(int session)
{
    if (Lilla_state == PERFORMANCE)
    {
        tft.drawBitmap(X_LED, Instrument_Y_POSITION(Session[session].instruments), led_pic, 8, 8, (TT_playing ? ILI9341_RED : RED_OFF));
    }
}

FLASHMEM
void DisplayManager::Performance_page(bool change_session, bool change_vol)
{
    instrument_editing_flag = false;
    Session_header(change_session, change_vol);
    Performance_menu(); // displays the menu and updates P_menu_max
    Instruments_header();
    All_Instrument(session);
}

FLASHMEM
void DisplayManager::Show_Performance(void)
{
    Board(0, 0, 11); // Display.Board(float   col, float row, int chars)
    tft.setCursor(x_pos(0), y_pos(0));
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

        tft.setCursor(x_pos(30), y_pos(Y_EFF));
        tft.print("RESOLUTION");

        tft.setCursor(x_pos(30), y_pos(Y_EFF + 1));
        tft.print("DOWNSAMPLING");

        tft.setCursor(x_pos(0), y_pos(Y_EFF + 1));
        tft.print("LPF CUTOFF");
    }

    else
    {
        Y_EFF = 2.5;
        Delete_row(Y_EFF);
        tft.setTextColor(TEXT_COLOR);

        tft.setCursor(x_pos(0), y_pos(Y_EFF));
        tft.print("RESOLUTION");

        tft.setCursor(x_pos(30), y_pos(Y_EFF));
        tft.print("DOWNSAMPLING");

        tft.setCursor(x_pos(0), y_pos(Y_EFF + 1));
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
        Cancel_text_reset_cursor(x_pos(43), y_pos(Y_EFF), 8);
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(Calc_resolution(resolution), 1);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("bit");
        return;
    }

    Y_EFF = 2.5;
    Cancel_text_reset_cursor(x_pos(11), y_pos(Y_EFF), 8);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Calc_resolution(resolution), 1);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("bit");
}

FLASHMEM
void DisplayManager::Downsampling(void)
{
    float Y_EFF;
    if (Lilla_state == MIDI_LOOP)
    {
        Y_EFF = 2.5;
        Cancel_text_reset_cursor(x_pos(43), y_pos(Y_EFF), 9);
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

    Y_EFF = 2.5;
    Cancel_text_reset_cursor(x_pos(43), y_pos(Y_EFF), 9);
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
    Cancel_text_reset_cursor(x_pos(11), y_pos(Y_EFF + 1), 7);

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
void DisplayManager::Show_sound(uint8_t session, uint8_t instrument)
{
    tft.fillScreen(ILI9341_BLACK);
    if (Lilla_state_0 == MIDI_LOOP)
    {
        Loop_midi_loop_title();
    }
    else
    {
        Show_Performance();
    }
    Show_Session_number(false);

    tft.setCursor(x_pos(23), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SOUND");
    tft.setCursor(x_pos(28.5), y_pos(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(instrument + 1);

    tft.setCursor(x_pos(38), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("FILE");
    File(instrument);

    Show_effects();

    tft.setCursor(x_pos(0), y_pos(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MIDI CHANNEL");
    Midi_channel(instrument);

    tft.setCursor(x_pos(18), y_pos(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PITCH");
    Pitch(instrument);

    tft.setCursor(x_pos(31), y_pos(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("GAIN");
    Gain_sound(session, instrument);

    tft.setCursor(x_pos(42), y_pos(4.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PAN");
    Pan(instrument);

    tft.setCursor(x_pos(0), y_pos(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("ATT");
    Attack(instrument);

    tft.setCursor(x_pos(18), y_pos(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("DEC");
    Decay(instrument);

    tft.setCursor(x_pos(31), y_pos(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SUS");
    Sustain(instrument);

    tft.setCursor(x_pos(42), y_pos(5.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("REL");
    Release(instrument);

    tft.setCursor(x_pos(0), y_pos(6.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PLAY MODE");
    Play_mode(instrument);

    tft.setCursor(x_pos(31), y_pos(6.9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("NOCLICK");
    Noclick(instrument, true);

    tft.setCursor(x_pos(0), y_pos(15));
    tft.setTextColor(TEXT_COLOR);
    tft.print("TRIM STEP");
    Trim_step();

    tft.setCursor(x_pos(28), y_pos(15));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MAX PITCH/VOICES");
    Pitch_voices_max(instrument);
    Led_instrument_SOUND_EDIT(instrument);
}

FLASHMEM
void DisplayManager::File(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(43), y_pos(0), 7);
    tft.setTextColor((file_midi_ch_flag ? ILI9341_YELLOW : ILI9341_WHITE));
    tft.print(name_file[Preset[instrument].file]);
}

FLASHMEM
void DisplayManager::Midi_channel(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(12.5), y_pos(4.9), 2);
    tft.setTextColor((!file_midi_ch_flag ? ILI9341_YELLOW : ILI9341_WHITE));
    tft.print(Preset[instrument].midi_channel + 1);
}
FLASHMEM
void DisplayManager::Pitch(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(23.5), y_pos(4.9), 5);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument].pitch, 3);
}

FLASHMEM
void DisplayManager::Gain_sound(uint8_t session, uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(35.5), y_pos(4.9), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Sound[Id_sound(session, instrument)].gain / 20.0);
}

FLASHMEM
void DisplayManager::Pan(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(45.5), y_pos(4.9), 4);
    tft.setTextColor(ILI9341_YELLOW);
    if (Preset[instrument].pan < 0)
    {
        tft.print("L");
    }
    else if (Preset[instrument].pan > 0)
    {
        tft.print("R");
    }
    tft.print(abs(Preset[instrument].pan));
}

FLASHMEM
void DisplayManager::Attack(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(3.5), y_pos(5.9), 10);
    tft.setTextColor(ILI9341_YELLOW);
    switch (Preset[instrument].attack_type)
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
    tft.setCursor(x_pos(8), y_pos(5.9));
    tft.print(Preset[instrument].attack, 2);
    Unity("sec");
}

FLASHMEM
void DisplayManager::Decay(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(21.5), y_pos(5.9), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument].decay, 2);
    Unity("sec");
}

FLASHMEM
void DisplayManager::Sustain(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(34.5), y_pos(5.9), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument].sustain * 100, 0);
    tft.print("%");
}

FLASHMEM
void DisplayManager::Release(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(45.5), y_pos(5.9), 5);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument].release, 1);
    Unity("sec");
}

FLASHMEM
void DisplayManager::Play_mode(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(9.5), y_pos(6.9), 13);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(loop_mode[Preset[instrument].mode]);
    tft.setCursor(x_pos(14), y_pos(6.9));
    tft.print(name_mode[Preset[instrument].mode]);
}

FLASHMEM
void DisplayManager::Noclick(uint8_t instrument, bool value)
{
    Cancel_text_reset_cursor(x_pos(38.5), y_pos(6.9), 4);
    tft.setTextColor((value ? ILI9341_YELLOW : ILI9341_WHITE));
    tft.print(Preset[instrument].Noclick);
    Unity("S");
}

FLASHMEM
void DisplayManager::Trim_step(void)
{
    Cancel_text_reset_cursor(x_pos(9.5), y_pos(15), 6);
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
void DisplayManager::Pitch_voices_max(uint8_t instrument) // max pitch related to which media is read
{
    Cancel_text_reset_cursor(x_pos(44.5), y_pos(15), 8);
    tft.setTextColor(ILI9341_WHITE);
    if (Preset[instrument].file < FIRST_LIVE_SAMPLING_FILE)
    {
        if (Preset[instrument].use_Wavetable)
        {
            tft.print(MAX_PITCH_WAVETABLE);
        }
        else
        {
            tft.print(MAX_PITCH_FLASH[optimization]);
        }
    }
    else
        tft.print(MAX_PITCH_PSRAM);
    tft.print("/");
    if (Preset[instrument].file < FIRST_LIVE_SAMPLING_FILE)
    {
        if (Preset[instrument].use_Wavetable)
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
    Show_Session_number(session);
    Session_volume(true);
    Show_effects();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(18), y_pos(0));
    tft.print("LOOP");
    Loop_loop_id();

    tft.setCursor(x_pos(Loop_HEAD_C - 1), y_pos(Loop_HEAD_R - 1));
    tft.print("METRO");
    tft.setCursor(x_pos(Loop_HEAD_C - 1), y_pos(Loop_HEAD_R));
    tft.print("TRACK");
    tft.setCursor(x_pos(Loop_HEAD_C - 1), y_pos(Loop_HEAD_R + 1));
    tft.print("SLIDE");
    tft.setCursor(x_pos(Loop_HEAD_C - 2), y_pos(Loop_HEAD_R + 2));
    tft.print("TRANSP");
    tft.setCursor(x_pos(Loop_HEAD_C - 1), y_pos(Loop_HEAD_R + 3));
    tft.print("LEVEL");
    tft.setCursor(x_pos(Loop_HEAD_C - 1), y_pos(Loop_HEAD_R + 4));
    tft.print("SOUND");

    tft.setTextColor(ILI9341_WHITE);
    for (int i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        if (Session[session].Instrument[i].used)
        {
            tft.setCursor(x_pos(Loop_HEAD_C + 3), 151 + i * 11);
            tft.print(i + 1);
        }
    }

    for (int i = 0; i < TRACKS; ++i)
    {
        Loop_track_data(i);
    }

    // show LOOP_time
    Loop_time_stretched();
}

FLASHMEM
void DisplayManager::Loop_loop_id(void)
{
    Cancel_text(x_pos(22), y_pos(0), 5); // Cancel_text(int X, int Y, int N)
    tft.setCursor(x_pos(22.5), y_pos(0));
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

    Cancel_text(x_pos(0), y_pos(1), 27); // Cancel_text(int X, int Y, int N)
    tft.setTextColor(MENU_COLOR);
    for (uint8_t element = 0; element < 4; ++element) // menu element
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
            tft.setCursor(x_pos(X_position_Menu_Loop[position]), y_pos(1));
            tft.print(Menu_Loop_char[element]);
            ++position;
        }
    }
}

FLASHMEM
void DisplayManager::Frame_loop_menu(uint8_t position, bool fresh) // after Loop_menu() use fresh = true
{
    static int8_t position_0 = 0;
    static int8_t XpML_0 = 0;
    static int8_t dvML_0 = 0;

    if (!fresh && position_0 >= 0)
    {
        Frame(XpML_0, 1, dvML_0, false); // Frame(X_position_Menu_Loop[position_0], 1, dimension_voice_Menu_Loop[element_Menu_Loop[position_0]], false);
    }

    if (!Menu_Loop[0] && !Menu_Loop[1] && !Menu_Loop[2])
    {
        position_0 = -1;
        return;
    }

    Frame(X_position_Menu_Loop[position], 1, dimension_voice_Menu_Loop[element_Menu_Loop[position]], true);
    choice_loop_menu = element_Menu_Loop[position];

    XpML_0 = X_position_Menu_Loop[position];
    dvML_0 = dimension_voice_Menu_Loop[element_Menu_Loop[position]];
    position_0 = position;
}

void DisplayManager::Delete_all_frame_loop_menu(void)
{
    uint8_t position;
    for (uint8_t element = 0; element < 4; ++element)
    {
        if (Menu_Loop[element])
        {
            position = position_Menu_Loop[element];
            Frame(X_position_Menu_Loop[position], 1, dimension_voice_Menu_Loop[element], false);
        }
    }
}

FLASHMEM
void DisplayManager::Loop_midi_loop_title(void)
{
    Board(0, 0, 9); // Board(float col, float row, int chars)
    tft.setCursor(x_pos(0), y_pos(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("MIDI LOOP");
}

FLASHMEM
void DisplayManager::Loop_track_data(int track)
{
    // Numero
    Cancel_text_reset_cursor(x_pos(Loop_LOOPS_X + 7 * track), y_pos(Loop_HEAD_R), 5);
    if (LOOP_events[track] > 0)
    {
        tft.setTextColor(ILI9341_WHITE);
        tft.print(track + 1); // Comincia da 1
    }

    // Slide
    Cancel_text_reset_cursor(x_pos(Loop_LOOPS_X + 7 * track), y_pos(Loop_HEAD_R + 1), 6);
    if (LOOP_events[track] > 0)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print((float)(LOOP_slide[track]) / 1000.0f, 2);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("s");
    }

    // Pitch
    Cancel_text_reset_cursor(x_pos(Loop_LOOPS_X + 7 * track), y_pos(Loop_HEAD_R + 2), 6);
    if (LOOP_events[track] > 0)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(LOOP_pitch_int[track]);
        tft.setTextColor(ILI9341_ORANGE);
        tft.print("st");
    }

    // Volume
    Cancel_text_reset_cursor(x_pos(Loop_LOOPS_X + 7 * track), y_pos(Loop_HEAD_R + 3), 6);
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
    Cancel_text_reset_cursor(x_pos(Loop_LOOP_TIME), y_pos(Loop_HEAD_R - 1), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print((float)(LOOP_time * LOOP_stretch) / 1000.0);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("s");
}

FLASHMEM
void DisplayManager::Unity(String A)
{
    tft.setTextColor(ILI9341_ORANGE);
    tft.print(A);
}
int DisplayManager::y_pos(float row)
{
    return 4 + (15.0 * row);
}
int DisplayManager::x_pos(float col)
{
    return 4 + (6.0 * col);
}
void DisplayManager::Delete_row(float row)
{
    tft.fillRect(0, y_pos(row) - 4, 320, 15, ILI9341_BLACK); // fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
}
void DisplayManager::Cancel_text(int X, int Y, int N)
{
    tft.fillRect(X, Y, (N * 6), 8, ILI9341_BLACK); // fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
}
void DisplayManager::Cancel_text_reset_cursor(int X, int Y, int N)
{
    Cancel_text(X, Y, N);
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
    tft.setCursor(x_pos(13), y_pos(12));
    tft.print("UPDATE ");
    tft.setTextColor(Calc_color(ILI9341_WHITE, light)); // 16-bit ('565') color settings
    tft.print(FIRMWARE_VERSION);

    tft.setTextColor(Calc_color(TEXT_COLOR, light)); // 16-bit ('565') color settings
    tft.setCursor(x_pos(13), y_pos(13) - 4);
    tft.print("AUDIO MEMORY ");
    tft.setTextColor(Calc_color(ILI9341_WHITE, light)); // 16-bit ('565') color settings
    tft.print(flash_dimension_MB);
    tft.print("MB");

    tft.setTextColor(Calc_color(TEXT_COLOR, light)); // 16-bit ('565') color settings
    tft.setCursor(x_pos(13), y_pos(14) - 8);
    tft.print("LIVE SAMPLER MEMORY ");
    tft.setTextColor(Calc_color(ILI9341_WHITE, light)); // 16-bit ('565') color settings
    tft.print("16MB");
}
uint16_t DisplayManager::Calc_color(uint16_t color_peak, float light) // 16-bit ('565') color settings
{
    float value_float;

    uint16_t red = color_peak >> 11;
    uint16_t green = (color_peak & 0b11111100000) >> 5;
    uint16_t blue = color_peak & 0b11111;

    value_float = red;
    red = value_float * light;

    value_float = green;
    green = value_float * light;

    value_float = blue;
    blue = value_float * light;

    uint16_t value = (red << 11) + (green << 5) + blue;
    return value;
}
int DisplayManager::Instrument_Y_POSITION(int position)
{
    return 4 + 15.0 * (6 + position) + 7;
}
FLASHMEM
void DisplayManager::Delay_page()
{
    tft.fillScreen(ILI9341_BLACK);

    if (Lilla_state_0 == PERFORMANCE)
    {
        Board(0, 0, 17); // DISPLAY_board(float col, float row, int chars)
        tft.setCursor(x_pos(0), y_pos(0));
        tft.setTextColor(ILI9341_WHITE);
        tft.print("PERFORMANCE ");
        tft.setTextColor(ILI9341_WHITE);
        tft.print("DELAY");
    }

    else if (Lilla_state_0 == LIVE_SAMPLING)
    {
        Board(0, 0, 15); // DISPLAY_board(float col, float row, int chars)
        tft.setCursor(x_pos(0), y_pos(0));
        tft.setTextColor(ILI9341_WHITE);
        tft.print("RING TAPE ");
        tft.setTextColor(ILI9341_WHITE);
        tft.print("DELAY");
    }

    tft.setCursor(x_pos(41), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME");
    Session_volume_value(true);

    Show_effects();

    tft.setCursor(x_pos(0), y_pos(Delay_ROW_BASE));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SOURCES");
    D_sounds();

    tft.setCursor(x_pos(0), y_pos(Delay_ROW_BASE + 2));
    tft.setTextColor(TEXT_COLOR);
    tft.print("FEEDBACK");
    D_read_gain();

    tft.setCursor(x_pos(20), y_pos(Delay_ROW_BASE + 2));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VALUE");
    D_delay();

    tft.setCursor(x_pos(0), y_pos(Delay_ROW_BASE + 3));
    tft.setTextColor(TEXT_COLOR);

    tft.setCursor(x_pos(20), y_pos(Delay_ROW_BASE + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VALUE L/R");
    D_delay_LR();

    tft.setCursor(x_pos(0), y_pos(Delay_ROW_BASE + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD SOURCE");
    D_modulation_type();

    tft.setCursor(x_pos(20), y_pos(Delay_ROW_BASE + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD FREQUENCY");
    D_modulation_frequency();

    tft.setCursor(x_pos(0), y_pos(Delay_ROW_BASE + 6));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD DEPTH");
    D_modulation_depth();

    tft.setCursor(x_pos(20), y_pos(Delay_ROW_BASE + 6));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD PHASE L/R");
    D_modulation_phase_LR();
}
FLASHMEM
void DisplayManager::D_sounds(void)
{
    Cancel_text_reset_cursor(x_pos(8), y_pos(Delay_ROW_BASE), 23); // DISPLAY_text(int X, int Y, int N)
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

    for (uint8_t i = 0; i < 8; ++i)
    {
        if (Delay_values.instrument_route[i])
        {
            tft.setTextColor(ILI9341_YELLOW);
            tft.print("S");
            tft.print(i + 1);
            tft.print("  ");
        }
        else
        {
            tft.setTextColor(0x6300);
            tft.print("S");
            tft.print(i + 1);
            tft.print("  ");
        }
    }
}
FLASHMEM
void DisplayManager::D_delay(void)
{
    Cancel_text_reset_cursor(x_pos(25.5), y_pos(Delay_ROW_BASE + 2), 9);
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
    Cancel_text_reset_cursor(x_pos(8.5), y_pos(Delay_ROW_BASE + 2), 8);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(-Delay_feedback(Delay_data.loop_gain) * 100);
    tft.print("%");
}
FLASHMEM
void DisplayManager::D_delay_LR(void)
{
    Cancel_text_reset_cursor(x_pos(29.5), y_pos(Delay_ROW_BASE + 3), 12);
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
    Cancel_text_reset_cursor(x_pos(10.5), y_pos(Delay_ROW_BASE + 5), 6);
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
    Cancel_text_reset_cursor(x_pos(33.5), y_pos(Delay_ROW_BASE + 5), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Delay_values.modulation_frequency);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("Hz");
}
FLASHMEM
void DisplayManager::D_modulation_depth(void) // depth
{
    Cancel_text_reset_cursor(x_pos(9.5), y_pos(Delay_ROW_BASE + 6), 8);
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
    Cancel_text_reset_cursor(x_pos(33.5), y_pos(Delay_ROW_BASE + 6), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Delay_values.modulation_phase_LR);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("deg");
}
inline uint8_t DisplayManager::Id_sound(uint8_t session, uint8_t instrument)
{
    return Session[session].Instrument[instrument].id_sound;
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
    tft.setCursor(X_POPUP + x_pos(3), Y_POPUP + Y_POPUP_TXT);
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
    tft.setCursor(X_POPUP + x_pos(1.5), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("STOP SAMPLING?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + x_pos(5.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + x_pos(9.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}
FLASHMEM
void DisplayManager::DS_page(uint8_t instrument, int recording)
{
    tft.fillScreen(ILI9341_BLACK);

    Board(0, 0, 7); // Display.Board(float   col, float row, int chars)
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(x_pos(0), y_pos(0));
    tft.print("SAMPLER");

    tft.setCursor(x_pos(41), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME");
    DS_update_volume();

    tft.setCursor(x_pos(0), y_pos(4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("AUDIO MEMORY ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(flash_dimension_MB);
    Unity("MB");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(" (");
    tft.print(Get_flash_size() / 88100.0f);
    Unity("sec");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");

    tft.setCursor(x_pos(0), y_pos(5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("- FREE FOR RECORDINGS");
    DS_available_memory();

    tft.setCursor(x_pos(0), y_pos(6));
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
void DisplayManager::DS_bar(uint8_t channel, int value) // 0 <= value <= BAR_ELEMENTS
{
    static int value_old[2] = {0, 0};
    int X0 = (channel == 0 ? DS_VUMETER_BAR_X : DS_VUMETER_BAR_X + DS_VUMETER_BAR_DX);
    float value_float;
    const float BAR_ELEMENTS_float = BAR_ELEMENTS;

    if (value < 0)
    {
        value = 0;
    }

    if (value > value_old[channel])
    {
        for (int i = value_old[channel] + 1; i <= value; ++i)
        {
            value_float = i / BAR_ELEMENTS_float;
            // builds bricks
            tft.drawFastHLine(X0, DS_VUMETER_BAR_Y - i, DS_VUMETER_BAR_DISTANCE, DS_calc_bar_color(value_float));
        }
    }
    else if (value < value_old[channel])
    {
        for (int i = value + 1; i <= value_old[channel]; ++i)
        {
            // remove bricks
            tft.drawFastHLine(X0, DS_VUMETER_BAR_Y - i, DS_VUMETER_BAR_DISTANCE, ILI9341_BLACK);
        }
    }
    value_old[channel] = value;
}
uint16_t DisplayManager::DS_calc_bar_color(float value) // 16-bit ('565') color settings
{
    const float soglia = 0.5;
    uint16_t red = (value >= soglia ? 31 : 31.0f * (value / soglia));                   // (value >= 0.5f ? 31: 31.0f * 2.0f * value)
    uint16_t green = (value <= soglia ? 63 : 63.0f * (1.2f - value) / (1.2f - soglia)); // (value <= 0.5f ? 63: 63.0f * 2.0f * (1.0f - value))
    uint16_t blue = 0;                                                                  // 33 * 0
    /*
      Serial.print("value ");
      Serial.print(value);
      Serial.print("  red ");
      Serial.print(red);
      Serial.print("  green ");
      Serial.print(green);
      Serial.print("  hex ");
      Serial.println( (red << 11) + (green << 5) + blue, HEX);
    */
    return (red << 11) + (green << 5) + blue;
}
FLASHMEM
void DisplayManager::DS_line_out(bool visible)
{
    tft.drawBitmap(DS_START_X + 37, DS_START_Y + 7, DS_freccia_gomito, 13, 23, (visible ? ILI9341_GREEN : GREEN_OFF));
    tft.setCursor(DS_START_X + 55, DS_START_Y + 23);
    tft.setTextColor((visible ? TEXT_COLOR : TEXT_OFF));
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
    Cancel_text_reset_cursor(x_pos(col), y_pos(row), 10);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(Get_packets_free() * 0.743, 1);
    Unity("sec");
}
FLASHMEM
void DisplayManager::DS_raw_available_memory()
{
    col = 21;
    row = 6;
    Cancel_text_reset_cursor(x_pos(col), y_pos(row), 6);
    tft.setTextColor(ILI9341_WHITE);
    tft.print((Get_flash_size() - Get_flash_occupation() - FLASH_FREE_SPACE) / 88200.0f);
    Unity("sec");
}
FLASHMEM
void DisplayManager::DS_hide_recording()
{
    Cancel_text(x_pos(0), y_pos(8), 18);
    Cancel_text(x_pos(0), y_pos(9), 29);
    Cancel_text(x_pos(0), y_pos(10), 18);
    Cancel_text(x_pos(0), y_pos(11), 18);
    Cancel_text(x_pos(0), y_pos(12), 18);
}
FLASHMEM
void DisplayManager::DS_advice_delete(bool value)
{
    tft.setTextColor((value ? ILI9341_YELLOW : ILI9341_BLACK));
    tft.setCursor(x_pos(3.5), y_pos(8));
    tft.print("PLEASE WAIT");
}
FLASHMEM
void DisplayManager::DS_advice_no_conversion(int DS_export, bool value)
{
    tft.setTextColor((value ? TEXT_COLOR : ILI9341_BLACK));
    tft.setCursor(x_pos(0), y_pos(8));
    tft.print("UNABLE TO CREATE RAW FILE");
    tft.setTextColor((value ? ILI9341_YELLOW : ILI9341_BLACK));
    tft.setCursor(x_pos(0), y_pos(9));
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
    tft.setCursor(x_pos(0), y_pos(8));
    tft.print("OPTIONS:");

    tft.setCursor(x_pos(0), y_pos(9));
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
        tft.setCursor(x_pos(0), y_pos(10));
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
    tft.setCursor(x_pos(0), y_pos(8));
    tft.print("SD EXPORT OPTIONS:");

    tft.setCursor(x_pos(0), y_pos(9));
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
        tft.setCursor(x_pos(0), y_pos(10));
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
    tft.setCursor((led ? x_pos(1.5) : x_pos(0)), y_pos(8));
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
    if (led)
    {
        Led_instrument_DIRECT_SAMPLING();
    }

    // FILE LEFT o MONO
    tft.setCursor(x_pos(0), y_pos(9));
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
        tft.setCursor(x_pos(0), y_pos(10));
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

    tft.setCursor(x_pos(0), y_pos(row));
    tft.setTextColor(TEXT_COLOR);
    tft.print("LENGTH ");

    tft.setTextColor(ILI9341_WHITE);
    tft.print(Recording[recording].seconds, 1);
    Unity("sec");
}
FLASHMEM
void DisplayManager::DS_update_recording_seconds(float value)
{
    row = 11;
    if (Recording[recording].stereo)
    {
        Cancel_text_reset_cursor(x_pos(7), y_pos(row), 7);
    }
    else
    {
        Cancel_text_reset_cursor(x_pos(7), y_pos(row - 1), 7);
    }

    tft.setTextColor(ILI9341_WHITE);
    tft.print(value / 1000.0f, 1);
    Unity("sec");
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
    tft.setCursor(x_pos(0), y_pos(row));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME ");

    tft.setTextColor(ILI9341_YELLOW);
    tft.print(volume_session / 20.0f, 2);
}
FLASHMEM
void DisplayManager::DS_update_volume(bool adj)
{
    Cancel_text_reset_cursor(x_pos(48), y_pos(0), 4);
    tft.setTextColor(adj ? ILI9341_YELLOW : ILI9341_WHITE);
    tft.print(volume_session / 20.0f, 2);
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
    uint8_t position = 0; // position on display

    // DS_define_model(); DISTRIBUITO
    Delete_row(1);
    Delete_row(2);
    tft.setTextColor(MENU_COLOR);

    for (uint8_t element = 0; element < DS_MV; ++element) // menu element
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
            tft.setCursor(x_pos(X_position_Menu_DS[position]), y_pos(Y_position_Menu_DS[position]));
            tft.print(Menu_DS_char[element]);
            position++;
        }
    }
}

FLASHMEM
void DisplayManager::DS_frame_menu(uint8_t position)
{
    static uint8_t position_0 = 0;
    Frame(X_position_Menu_DS[position_0], Y_position_Menu_DS[position_0], dimension_voice_Menu_DS[element_Menu_DS[position_0]], false);
    Frame(X_position_Menu_DS[position], Y_position_Menu_DS[position], dimension_voice_Menu_DS[element_Menu_DS[position]], true);
    choice_DS_menu = element_Menu_DS[position];
    position_0 = position;
}

FLASHMEM
void DisplayManager::Show_VFS_packets(void)
{
    Cancel_text_reset_cursor(x_pos(21), y_pos(7), 40);
    Frame(21, 7, 21, true);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print((VFS_packets * PACKET_DIM) / 1048576.0f, 2);
    Unity("MB");
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(" (");
    tft.print(VFS_packets * 0.743, 0);
    Unity("sec");
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(" MONO)");

    Cancel_text_reset_cursor(x_pos(27), y_pos(8), 40);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(((VFS_packets_max - VFS_packets) * PACKET_DIM) / 1048576.0f, 2);
    Unity("MB");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(" (");
    tft.print((VFS_packets_max - VFS_packets) * 0.743, 0);
    Unity("sec");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");
}

FLASHMEM
void DisplayManager::Confirm_EXIT_from_LS(void)
{
    L_POPUP = 106;
    H_POPUP = 47;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP = (240 - H_POPUP) / 2;
    Y_POPUP_TXT = 10;
    Y_POPUP_OPT = 30;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("STOP RECORDING?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + x_pos(5.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + x_pos(9.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::LS_page(void)
{
    //("012345678901234567890"); // Size 1: 21 chars
    tft.fillScreen(ILI9341_BLACK);
    LS_Ring_Tape();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(37), y_pos(0));
    tft.print("LENGTH");
    LS_dim();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(1.5), y_pos(3));
    tft.print("VOLUME");
    Led_instrument_LIVE_SAMPLING();
    LS_volume();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(37), y_pos(3));
    tft.print("FEEDBACK");
    Show_LS_feedback();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(0), y_pos(4));
    tft.print("PLAY MODE");
    Show_LS_mode();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(0), y_pos(5));
    tft.print("START POINT");
    LS_X_sample_delta();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(0), y_pos(6));
    tft.print("STEP");
    Show_LS_X_step();
}

FLASHMEM
void DisplayManager::LS_Ring_Tape(void)
{
    //("012345678901234567890"); // Size 1: 21 chars
    Board(0, 0, 12); // Display.Board(float & col, float row, int chars)
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(x_pos(0), y_pos(0));
    tft.print("LIVE SAMPLER");
}

FLASHMEM
void DisplayManager::Show_LS_feedback(void)
{
    Cancel_text_reset_cursor(x_pos(46), y_pos(3), 6);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(100 * LS_fbk_table[LS_feedback], 2);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("%");
}

FLASHMEM
void DisplayManager::Show_LS_X_step(void)
{
    Cancel_text_reset_cursor(x_pos(5), y_pos(6), 16);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(LS_X_step);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("samples");
}

FLASHMEM
void DisplayManager::LS_dim(void)
{
    Cancel_text_reset_cursor(x_pos(44), y_pos(0), 12);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(LS_buffer_dim / 44100.0f, 1);
    Unity("sec");
}

FLASHMEM
void DisplayManager::LS_volume(void)
{
    Cancel_text_reset_cursor(x_pos(8.5), y_pos(3), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(volume_session / 20.0f);
}

FLASHMEM
void DisplayManager::Show_LS_mode(void)
{
    Cancel_text_reset_cursor(x_pos(10), y_pos(4), 20);
    tft.setTextColor(ILI9341_YELLOW);
    if (LS_mode > 1)
    {
        tft.print(loop_mode[LS_mode]);
        tft.print(" ");
    }
    tft.print(name_mode[LS_mode]);
}

FLASHMEM
void DisplayManager::LS_loop_time(void)
{
    tft.setTextColor(ILI9341_YELLOW);
    if (LS_mode == 2)
    {
        Cancel_text_reset_cursor(x_pos(19), y_pos(4), 9);
        tft.print(LS_XY_delta / 44100.0f, 2);
        Unity("sec");
    }
    else if (LS_mode == 3)
    {
        Cancel_text_reset_cursor(x_pos(23), y_pos(4), 9);
        tft.print(LS_XY_delta / 44100.0f, 2);
        Unity("sec");
    }
}

FLASHMEM
void DisplayManager::LS_X_sample_delta(void)
{
    float local_value = 0;
    Cancel_text_reset_cursor(x_pos(12), y_pos(5), 40);
    tft.setTextColor(ILI9341_YELLOW);
    if (LS_XY_lock)
    {
        tft.print("FIXED ");
        tft.print(LS_X_sample / 44100.0f, 2);
        Unity("sec");
    }
    else
    {
        if (LS_X_delta == 0)
        {
            tft.print("DELAY ");
            tft.print(0.00);
        }

        else if (LS_X_delta > LS_buffer_dim / 2)
        {
            local_value = (LS_buffer_dim - LS_X_delta) / 44100.0f;
            if (local_value >= 0)
            {
                tft.print("DELAY ");
                tft.print(local_value, 2);
            }
            else
            {
                tft.print("ADVANCE ");
                tft.print(-local_value, 2);
            }
        }
        else
        {
            local_value = -LS_X_delta / 44100.0f;
            if (local_value >= 0)
            {
                tft.print("DELAY ");
                tft.print(local_value, 2);
            }
            else
            {
                tft.print("ADVANCE ");
                tft.print(-local_value, 2);
            }
        }
        Unity("sec");
    }
}
FLASHMEM
void DisplayManager::Show_LS_menu(void)
{
    uint8_t position = 0; // position on display
    Delete_row(1);
    tft.setTextColor(MENU_COLOR);
    for (uint8_t element = 0; element < LS_MV; ++element) // menu element
    {
        if (Menu_LS[element])
        {
            if (position == 0)
            {
                X_position_Menu_LS[position] = 0;
            }

            else
            {
                X_position_Menu_LS[position] = X_position_Menu_LS[position - 1] + dimension_voice_Menu_LS[element_Menu_LS[position - 1]] + 1;
            }

            element_Menu_LS[position] = element;
            position_Menu_LS[element] = position;
            tft.setCursor(x_pos(X_position_Menu_LS[position]), y_pos(1));
            tft.setTextColor(MENU_COLOR);
            tft.print(Menu_LS_char[element]);
            position++;
        }
    }
}

FLASHMEM
void DisplayManager::Frame_LS_menu(uint8_t position)
{
    Delete_all_frame_LS_menu();
    Frame(X_position_Menu_LS[position], 1, dimension_voice_Menu_LS[element_Menu_LS[position]], true);
    // choice_LS_menu = element_Menu_LS[position]; // TRASFERITA NEL CODICE main.cpp
}

FLASHMEM
void DisplayManager::Delete_all_frame_LS_menu(void)
{
    uint8_t position;
    for (uint8_t element = 0; element < LS_MV; ++element)
    {
        if (Menu_LS[element])
        {
            position = position_Menu_LS[element];
            Frame(X_position_Menu_LS[position], 1, dimension_voice_Menu_LS[element], false);
        }
    }
}

FLASHMEM
void DisplayManager::Midi_monitor_page(void)
{
    tft.fillScreen(ILI9341_BLACK);

    Board(0, 0, 12); // Display.Board(float col, float row, int chars)
    tft.setCursor(x_pos(0), y_pos(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("MIDI MONITOR");

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(0), y_pos(3));
    tft.print("MIDI CHANNEL");
    tft.setCursor(x_pos(0), y_pos(4));
    tft.print("MESSAGE");
    tft.setCursor(x_pos(0), y_pos(5));
    tft.print("NOTE-NUMBER");
    tft.setCursor(x_pos(0), y_pos(6));
    tft.print("VELOCITY");
    tft.setCursor(x_pos(0), y_pos(7));
    tft.print("VALUE");
    tft.setCursor(x_pos(0), y_pos(8));
    tft.print("NUMBER");
}

FLASHMEM
void DisplayManager::Midi_monitor_frame(void)
{
    Frame(0, 1, 6, true);
}

FLASHMEM
void DisplayManager::Midi_monitor_data(uint8_t incoming_midi_channel, uint8_t incoming_midi_message, int8_t incoming_note_number, int8_t incoming_velocity, int32_t incoming_midi_value, int8_t incoming_number)
{
    const char message_name[12][20] = {{"NoteOn"}, {"NoteOff"}, {"PitchBend"}, {"AfterTouchPoly"}, {"ControlChange"}, {"ProgramChange"}, {"AfterTouchChange"}, {"SystemExclusive"}, {"Unknown"}};

    tft.setTextColor(ILI9341_YELLOW);
    Cancel_text_reset_cursor(x_pos(13), y_pos(3), 2);
    tft.print(incoming_midi_channel + 1);

    Cancel_text_reset_cursor(x_pos(8), y_pos(4), 16);
    tft.print(message_name[incoming_midi_message]);

    Cancel_text(x_pos(12), y_pos(5), 9);
    if (incoming_note_number >= 0)
    {
        tft.setCursor(x_pos(12), y_pos(5));
        tft.print(note_name[incoming_note_number % 12]);
        tft.print((int)(incoming_note_number / 12) + first_octave);
    }

    Cancel_text(x_pos(9), y_pos(6), 9);
    if (incoming_velocity >= 0)
    {
        tft.setCursor(x_pos(9), y_pos(6));
        tft.print(incoming_velocity);
    }

    Cancel_text(x_pos(6), y_pos(7), 9);
    if (incoming_midi_value >= 0)
    {
        tft.setCursor(x_pos(6), y_pos(7));
        tft.print(incoming_midi_value);
    }

    Cancel_text(x_pos(7), y_pos(8), 9);
    if (incoming_number >= 0)
    {
        tft.setCursor(x_pos(7), y_pos(8));
        tft.print(incoming_number);
    }
}

FLASHMEM
void DisplayManager::MX_page(void)
{
#define MX_Y0 3
    tft.fillScreen(ILI9341_BLACK);

    Board(0, 0, 5); // Display.Board(float   col, float row, int chars)
    tft.setCursor(x_pos(0), y_pos(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("MIXER");

    tft.setCursor(x_pos(0), y_pos(MX_Y0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SOURCE");

    tft.setCursor(x_pos(0), y_pos(MX_Y0 + 1));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MUTE");

    tft.setCursor(x_pos(0), y_pos(MX_Y0 + 2));
    tft.setTextColor(TEXT_COLOR);
    tft.print("GAIN");

    tft.setCursor(x_pos(0), y_pos(MX_Y0 + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("PAN");

    tft.setCursor(x_pos(0), y_pos(MX_Y0 + 4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("LINEOUT");

    tft.setCursor(x_pos(0), y_pos(MX_Y0 + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MONITOR");

    tft.setCursor(x_pos(0), y_pos(MX_Y0 + 7));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME LINE OUT");
    MX_MAIN_volume();

    tft.setCursor(x_pos(0), y_pos(MX_Y0 + 8));
    tft.setTextColor(TEXT_COLOR);
    tft.print("VOLUME MONITOR");
    MX_MONITOR_volume();
}

FLASHMEM
void DisplayManager::MX_MAIN_volume(void)
{
    Cancel_text_reset_cursor(x_pos(16), y_pos(MX_Y0 + 7), 4);
    tft.setTextColor(MENU_COLOR);
    tft.print(MAIN_volume / 20.0f, 2);
}

FLASHMEM
void DisplayManager::MX_MONITOR_volume(void)
{
    Cancel_text_reset_cursor(x_pos(15), y_pos(MX_Y0 + 8), 4);
    tft.setTextColor(MENU_COLOR);
    tft.print(PWM_volume / 20.0f, 2);
}

FLASHMEM
void DisplayManager::MX_source_values(uint8_t source)
{
    if (source == 8)
    {
        MX_source_values_write(source);
    }
    else if (Session[session].Instrument[source].used)
    {
        MX_source_values_write(source);
    }
}

FLASHMEM
void DisplayManager::MX_source_values_jump(uint8_t old_source, uint8_t new_source)
{
    MX_source = new_source;

    MX_source_values_write(old_source); // 0
    MX_source_values_write(MX_source);  // 6
}

FLASHMEM
void DisplayManager::MX_source_values_write(uint8_t source)
{
    int local_id_sound;
    if (source == 8)
    {
        tft.setTextColor((source == MX_source ? TEXT_COLOR : 0x6300));
        tft.setCursor(x_pos(MX_X0 + source * 5 - 1), y_pos(MX_Y0));
        tft.print("L_IN");

        tft.drawBitmap(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 1), led_pic, 8, 8, (MX_mute[source] ? ILI9341_RED : RED_OFF)); // Mute
        tft.setTextColor((source == MX_source ? ILI9341_YELLOW : 0x6300));
        tft.setCursor(x_pos(MX_X0 - 1 + source * 5), y_pos(MX_Y0 + 2)); // Gain
        tft.print(DS_gain / 20.0f);

        tft.setTextColor((source == MX_source ? ILI9341_WHITE : 0x6300));
        tft.setCursor(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 3)); // Pan
        tft.print("C");
    }

    else
    {
        local_id_sound = Session[session].Instrument[source].id_sound;

        tft.setTextColor((source == MX_source ? TEXT_COLOR : 0x6300));
        tft.setCursor(x_pos(MX_X0 + source * 5), y_pos(MX_Y0));
        tft.print("S");
        tft.print(source + 1);

        tft.drawBitmap(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 1), led_pic, 8, 8, (MX_mute[source] ? ILI9341_RED : RED_OFF)); // Mute
        tft.setTextColor((source == MX_source ? ILI9341_YELLOW : 0x6300));

        Cancel_text(x_pos(MX_X0 - 1 + source * 5), y_pos(MX_Y0 + 2), 4);
        tft.setCursor(x_pos(MX_X0 - 1 + source * 5), y_pos(MX_Y0 + 2)); // Gain
        tft.print(Sound[local_id_sound].gain / 20.0f);

        if (Sound[local_id_sound].pan == 0)
        {
            Cancel_text(x_pos(MX_X0 - 0.5 + source * 5), y_pos(MX_Y0 + 3), 3);
            tft.setCursor(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 3)); // Pan
            tft.print("C");
        }
        else
        {
            if (abs(Sound[local_id_sound].pan) < 10) // Pan
            {
                tft.setCursor(x_pos(MX_X0 + source * 5), y_pos(MX_Y0 + 3));
            }
            else
            {
                tft.setCursor(x_pos(MX_X0 - 0.5 + source * 5), y_pos(MX_Y0 + 3));
            }

            if (Sound[local_id_sound].pan < 0)
            {
                tft.print("L");
            }
            else if (Sound[local_id_sound].pan > 0)
            {
                tft.print("R");
            }
            tft.print(abs(Sound[local_id_sound].pan));
        }
    }

    if (MX_routing_source[source] > 1) // to Audio-out
    {
        tft.drawBitmap(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 4), led_pic, 8, 8, GREEN_ON);
    }
    else
    {
        tft.drawBitmap(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 4), led_pic, 8, 8, GREEN_OFF);
    }

    if (MX_routing_source[source] == 1 || MX_routing_source[source] == 3) // to Monitor
    {
        tft.drawBitmap(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 5), led_pic, 8, 8, GREEN_ON);
    }
    else
    {
        tft.drawBitmap(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 5), led_pic, 8, 8, GREEN_OFF);
    }
}

FLASHMEM
void DisplayManager::MX_source_values_edit(uint8_t source)
{
    Cancel_text(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 1), 1); // Mute
    Cancel_text(x_pos(MX_X0 - 1 + source * 5), y_pos(MX_Y0 + 2), 4);   // Gain
    Cancel_text(x_pos(MX_X0 - 0.5 + source * 5), y_pos(MX_Y0 + 3), 3); // Pan
    Cancel_text(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 4), 1); // to Audio-out
    Cancel_text(x_pos(MX_X0 + 0.5 + source * 5), y_pos(MX_Y0 + 5), 1); // to Monitor

    MX_source_values_write(source);
}

FLASHMEM
void DisplayManager::Session_header(bool change_session, bool change_vol)
{
    tft.fillScreen(ILI9341_BLACK);
    Show_Performance();
    Show_Session_number(change_session);
    Session_volume(change_vol);
    Show_effects();
}

FLASHMEM
void DisplayManager::Session_volume_color(bool change_session, bool change_vol)
{
    Show_Session_number(change_session);
    Session_volume(change_vol);
    Session_volume_value(change_vol);
}

FLASHMEM
void DisplayManager::Instruments_header(void)
{
    tft.setTextColor(TEXT_COLOR);

    tft.setCursor(x_pos(Instrument_INDENT_X0), y_pos(5));
    tft.print("SOUND");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 5 + Instrument_SPACE_X), y_pos(5)); // (X0i + 2 * Instrument_SPACE_X + 6)
    tft.print("LOCK");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 2 * Instrument_SPACE_X + 9), y_pos(5)); // (x_pos(X0i + 5 + Instrument_SPACE_X)
    tft.print("P");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 3 * Instrument_SPACE_X + 10), y_pos(5));
    tft.print("MIDI");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 4 * Instrument_SPACE_X + 14), y_pos(5));
    tft.print("ROOT-K");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 5 * Instrument_SPACE_X + 20), y_pos(5));
    tft.print("FROM-K");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 6 * Instrument_SPACE_X + 27), y_pos(5));
    tft.print("TO-K");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 7 * Instrument_SPACE_X + 32), y_pos(5));
    tft.print("PAN");

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 8 * Instrument_SPACE_X + 35), y_pos(5));
    tft.print("GAIN");

    tft.drawLine(6, y_pos(5) + 11, 312, y_pos(5) + 11, 0x630C);
}

FLASHMEM
void DisplayManager::Show_Instrument_description(uint8_t session, uint8_t instrument, bool editing)
{
    uint8_t position = position_of_Instrument[instrument];

    Delete_Instrument(position);
    tft.setTextColor((editing ? ILI9341_YELLOW : ILI9341_WHITE));

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 2), Instrument_Y_POSITION(position));
    tft.print(instrument + 1);

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 2 * Instrument_SPACE_X + 5), Instrument_Y_POSITION(position));
    if (Session[session].Instrument[instrument].lock)
    {
        tft.print("X");
    }

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 2 * Instrument_SPACE_X + 9), Instrument_Y_POSITION(position));
    if (Session[session].Instrument[instrument].precedence)
    {
        tft.print("X");
    }

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 1 + 3 * Instrument_SPACE_X + 10), Instrument_Y_POSITION(position));
    tft.print(Get_midi_channel(session, instrument) + 1);

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 4 * Instrument_SPACE_X + 15), Instrument_Y_POSITION(position));
    Note(Session[session].Instrument[instrument].root_key);

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 2 + 5 * Instrument_SPACE_X + 18.5), Instrument_Y_POSITION(position));
    Note(Session[session].Instrument[instrument].from_note);

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 2 + 6 * Instrument_SPACE_X + 25), Instrument_Y_POSITION(position));
    Note(Session[session].Instrument[instrument].to_note);

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 7 * Instrument_SPACE_X + 32), Instrument_Y_POSITION(position));
    if (Sound[Session[session].Instrument[instrument].id_sound].pan < 0)
    {
        tft.print("L");
    }
    else if (Sound[Session[session].Instrument[instrument].id_sound].pan > 0)
    {
        tft.print("R");
    }

    tft.print(abs(Sound[Session[session].Instrument[instrument].id_sound].pan));

    tft.setCursor(x_pos(Instrument_INDENT_X0 + 8 * Instrument_SPACE_X + 35), Instrument_Y_POSITION(position));
    tft.print(Sound[Session[session].Instrument[instrument].id_sound].gain / 20.0f, 2);

    Led_instrument_PERFORMANCE(instrument);
}

FLASHMEM
void DisplayManager::All_Instrument(uint8_t session)
{
    tft.fillRect(0, Instrument_Y_POSITION(0) - 4, 320, 240, ILI9341_BLACK);
    for (uint8_t instrument = 0; instrument < INSTRUMENTS_MAX; instrument++)
    {
        if (Session[session].Instrument[instrument].used)
        {
            Show_Instrument_description(session, instrument, false);
        }
    }

    if (tuning_tone_flag)
    {
        Instrument_TT(session);
    }
}

FLASHMEM
void DisplayManager::Instrument_TT(uint8_t session)
{
    uint8_t position = Session[session].instruments;
    if (tuning_tone_flag)
    {
        uint8_t position = Session[session].instruments;

        tft.setTextColor(ILI9341_WHITE);

        tft.setCursor(x_pos(Instrument_INDENT_X0 + 2), Instrument_Y_POSITION(position));
        tft.print("TUNING-TONE");

        tft.setCursor(x_pos(Instrument_INDENT_X0 + 3 * Instrument_SPACE_X + 10), Instrument_Y_POSITION(position));
        tft.print("ALL");

        tft.setCursor(x_pos(Instrument_INDENT_X0 + 4 * Instrument_SPACE_X + 15), Instrument_Y_POSITION(position)); // root key
        Note(60);

        tft.setCursor(x_pos(2 + Instrument_INDENT_X0 + 5 * Instrument_SPACE_X + 18.5), Instrument_Y_POSITION(position)); // from key
        Note(0);

        tft.setCursor(x_pos(2 + Instrument_INDENT_X0 + 6 * Instrument_SPACE_X + 25), Instrument_Y_POSITION(position)); // to key
        Note(127);

        tft.setCursor(x_pos(Instrument_INDENT_X0 + 7 * Instrument_SPACE_X + 32), Instrument_Y_POSITION(position));
        tft.print("0");

        tft.setCursor(x_pos(Instrument_INDENT_X0 + 8 * Instrument_SPACE_X + 35), Instrument_Y_POSITION(position));
        tft.setTextColor(ILI9341_YELLOW);
        tft.print(tuning_tone_volume / 20.0f, 2);

        Led_tuning_tone(session);
    }
    else
        tft.fillRect(0, Instrument_Y_POSITION(position) - 4, 320, 15, ILI9341_BLACK);
}

FLASHMEM
void DisplayManager::Volume_TT(uint8_t session)
{
    uint8_t position = Session[session].instruments;

    Cancel_text_reset_cursor(x_pos(Instrument_INDENT_X0 + 8 * Instrument_SPACE_X + 35), Instrument_Y_POSITION(position), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(tuning_tone_volume / 20.0f, 2);
}

FLASHMEM
void DisplayManager::Delete_Instrument(int position)
{
    tft.fillRect(5, Instrument_Y_POSITION(position), 307, 7, ILI9341_BLACK);
}

FLASHMEM
void DisplayManager::Note(uint8_t note_number)
{
    tft.print(note_name[note_number % 12]);
    tft.print((int)(note_number / 12.0f) + first_octave);
}

FLASHMEM
void DisplayManager::Performance_menu(void)
{
    uint8_t position = 0; // position on display

    Delete_row(1);
    tft.setTextColor(MENU_COLOR);
    for (uint8_t element = 0; element < 5; ++element) // menu element
    {
        if (Menu_P[element])
        {
            if (position == 0)
            {
                X_position_Menu_P[position] = 0;
            }

            else
            {
                X_position_Menu_P[position] = X_position_Menu_P[position - 1] + dimension_voice_Menu_P[element_Menu_P[position - 1]] + 1;
            }

            Y_position_Menu_P[position] = 1;
            element_Menu_P[position] = element;
            position_Menu_P[element] = position;
            tft.setCursor(x_pos(X_position_Menu_P[position]), y_pos(Y_position_Menu_P[position]));
            tft.print(Menu_P_char[element]);
            position++;
        }
    }
}

FLASHMEM
void DisplayManager::Frame_performance_menu(uint8_t position, bool fresh)
{
    static uint8_t position_0 = 0;
    if (!fresh)
    {
        Frame(X_position_Menu_P[position_0], Y_position_Menu_P[position_0], dimension_voice_Menu_P[element_Menu_P[position_0]], false);
    }

    Frame(X_position_Menu_P[position], Y_position_Menu_P[position], dimension_voice_Menu_P[element_Menu_P[position]], true);
    choice_performance_menu = element_Menu_P[position];
    position_0 = position;
}

void DisplayManager::Delete_all_frame_performance_menu(void)
{
    uint8_t position;
    for (uint8_t element = 0; element < 5; ++element)
    {
        if (Menu_P[element])
        {
            position = position_Menu_P[element];
            Frame(X_position_Menu_P[position], Y_position_Menu_P[position], dimension_voice_Menu_P[element], false);
        }
    }
}

FLASHMEM
void DisplayManager::Confirm_session_change_popup(void)
{
    L_POPUP = 106;
    H_POPUP = 47;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP = (240 - H_POPUP) / 2;
    Y_POPUP_TXT = 10;
    Y_POPUP_OPT = 30;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + x_pos(2), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("SAVE CHANGES?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + x_pos(2), Y_POPUP + Y_POPUP_OPT);
    tft.print("EXIT");
    tft.setCursor(X_POPUP + x_pos(8), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + x_pos(12), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Confirm_session_change_popup_frame(uint8_t value)
{
    Confirm_frame(X_POPUP + x_pos(2), Y_POPUP + Y_POPUP_OPT, 4, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    Confirm_frame(X_POPUP + x_pos(8), Y_POPUP + Y_POPUP_OPT, 2, false);
    Confirm_frame(X_POPUP + x_pos(12), Y_POPUP + Y_POPUP_OPT, 3, false);
    switch (value)
    {
    case 0: // exit
        Confirm_frame(X_POPUP + x_pos(2), Y_POPUP + Y_POPUP_OPT, 4, true);
        break;
    case 1: // no
        Confirm_frame(X_POPUP + x_pos(8), Y_POPUP + Y_POPUP_OPT, 2, true);
        break;
    case 2: // yes
        Confirm_frame(X_POPUP + x_pos(12), Y_POPUP + Y_POPUP_OPT, 3, true);
        break;
    default:
        break;
    }
}

void DisplayManager::Confirm_frame(int X, int Y, uint8_t chars, bool print)
{
    tft.drawRect(X - 4.0, Y - 4.0, (6 * chars) + 7, 15, (print ? ILI9341_WHITE : ILI9341_RED)); // drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
}

FLASHMEM
void DisplayManager::Confirm_save_changes_page(void)
{
    tft.fillRect(12, 40, 104, 50, ILI9341_RED); // does NOT delete frame
    tft.setCursor(x_pos(3), y_pos(4));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("save changes?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(x_pos(7), y_pos(6));
    tft.print("No");
    tft.setCursor(x_pos(11), y_pos(6));
    tft.print("Yes");
}

FLASHMEM
void DisplayManager::Confirm_session_delete_popup(void)
{
    L_POPUP = 106;
    H_POPUP = 47;
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 10;
    Y_POPUP_OPT = 30;

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("CONFIRM DELETE?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + x_pos(5.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + x_pos(9.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Confirm_session_delete_popup_frame(uint8_t value)
{
    Confirm_frame(X_POPUP + x_pos(5.5), Y_POPUP + Y_POPUP_OPT, 2, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    Confirm_frame(X_POPUP + x_pos(9.5), Y_POPUP + Y_POPUP_OPT, 3, false);
    switch (value)
    {
    case 0: // NO
        Confirm_frame(X_POPUP + x_pos(5.5), Y_POPUP + Y_POPUP_OPT, 2, true);
        break;
    case 1: // YES
        Confirm_frame(X_POPUP + x_pos(9.5), Y_POPUP + Y_POPUP_OPT, 3, true);
        break;
    default:
        break;
    }
}
FLASHMEM
void DisplayManager::Settings_page(void)
{
    tft.fillScreen(ILI9341_BLACK);

    Board(0, 0, 5); // Display.Board(float   col, float row, int chars)
    tft.setCursor(x_pos(0), y_pos(0));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("SETUP");

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(0), y_pos(2));
    tft.print("KEY STEP");
    tft.setCursor(x_pos(0), y_pos(3));
    tft.print("FIRST OCTAVE");
    tft.setCursor(x_pos(0), y_pos(4));
    tft.print("OPTIMIZATION");
    tft.setCursor(x_pos(0), y_pos(5));
    tft.print("CONTROL CHANGE ASSIGNMENT");
    // tft.setCursor(x_pos(0), y_pos(6));
    // tft.print("*FUTURE DEVELOPMENTS*");
    tft.setCursor(x_pos(0), y_pos(6));
    tft.print("IMPORT RAW FILES FROM /LILLARAW");
    tft.setCursor(x_pos(0), y_pos(7));
    tft.print("IMPORT CONFIGURATION FROM /LILLASET/lilla.txt");
    tft.setCursor(x_pos(0), y_pos(8));
    tft.print("EXPORT CONFIGURATION TO /LILLASET/lillaold.txt");
    tft.setCursor(x_pos(0), y_pos(9));
    tft.print("FACTORY RESET");

    Key_step();
    First_octave();
    Optimization();
}

FLASHMEM
void DisplayManager::Key_step(void)
{
    Cancel_text_reset_cursor(x_pos(9), y_pos(2), 5);
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
    Cancel_text_reset_cursor(x_pos(13), y_pos(3), 2);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(first_octave);
}

FLASHMEM
void DisplayManager::Optimization(void)
{
    Cancel_text_reset_cursor(x_pos(13), y_pos(4), 29);
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
    Frame(9, 2, 5, false);   // First octave
    Frame(13, 3, 2, false);  // First octave
    Frame(13, 4, 29, false); // Optimization
    Frame(0, 5, 25, false);  // Control Change Assignment
    Frame(0, 6, 31, false);  // Import raw files
    Frame(0, 7, 45, false);  // Import configuration from
    Frame(0, 8, 46, false);  // Export configuration to SD
    Frame(0, 9, 13, false);  // Factory Reset

    switch (value)
    {
    case 0:
        Frame(9, 2, 5, true); // First octave
        break;
    case 1:
        Frame(13, 3, 2, true); // First octave
        break;
    case 2:
        Frame(13, 4, 29, true); // Optimization
        break;
    case 3:
        Frame(0, 5, 25, true); // Control Change
        break;
    case 4:
        Frame(0, 6, 31, true); // Import RAW files
        break;
    case 5:
        Frame(0, 7, 45, true); // Import configuration to SD
        break;
    case 6:
        Frame(0, 8, 46, true); // Export configuration to SD
        break;
    case 7:
        Frame(0, 9, 13, true); // Factory reset
        break;
    default:
        break;
    }
}

FLASHMEM
void DisplayManager::CC_page(void)
{
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(x_pos(0), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("CONTROL CHANGE ASSIGNMENT");

    tft.setCursor(x_pos(0), y_pos(1));
    tft.setTextColor(MENU_COLOR);
    tft.print("RETURN");

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(0), y_pos(3));
    tft.print("GAIN SOUND 1");
    tft.setCursor(x_pos(0), y_pos(4));
    tft.print("GAIN SOUND 2");
    tft.setCursor(x_pos(0), y_pos(5));
    tft.print("GAIN SOUND 3");
    tft.setCursor(x_pos(0), y_pos(6));
    tft.print("GAIN SOUND 4");

    tft.setCursor(x_pos(0), y_pos(7));
    tft.print("GAIN SOUND 5");
    tft.setCursor(x_pos(0), y_pos(8));
    tft.print("GAIN SOUND 6");
    tft.setCursor(x_pos(0), y_pos(9));
    tft.print("GAIN SOUND 7");
    tft.setCursor(x_pos(0), y_pos(10));
    tft.print("GAIN SOUND 8");
    tft.setCursor(x_pos(0), y_pos(11));
    tft.print("LPF CUTOFF");
}

FLASHMEM
void DisplayManager::All_CC_Sound_gain(void)
{
    for (int n = 0; n < 8; ++n)
    {
        Show_CC_Sound_gain(n);
    }
}

FLASHMEM
void DisplayManager::Show_CC_Sound_gain(int value)
{
    Cancel_text_reset_cursor(x_pos(Control_change_X), y_pos(value + 3), 3);
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
    Cancel_text_reset_cursor(x_pos(Control_change_X), y_pos(11), 3);
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
    Frame(0, 1, 6, false); // Return
    for (uint8_t n = 0; n < 9; ++n)
    {
        Frame(Control_change_X, n + 3, 3, false);
    }

    if (value == 0)
    {
        Frame(0, 1, 6, true);
    }
    else
    {
        Frame(Control_change_X, value + 2, 3, true);
    }
}

FLASHMEM
void DisplayManager::Import_raw_files_frame(uint8_t value)
{
    Frame(0, 1, 4, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    Frame(5, 1, 6, false);
    switch (value)
    {
    case 0: // EXIT
        Frame(0, 1, 4, true);
        break;
    case 1: // IMPORT
        Frame(5, 1, 6, true);
        break;
    default:
        break;
    }
}
FLASHMEM
void DisplayManager::Confirm_config_import_popup(void)
{
    L_POPUP = x_pos(46);
    H_POPUP = y_pos(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 10; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = x_pos(19);

    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    //       ("01234567890123456789012345678901234567891098765"); // 45 char
    tft.print("    WARNING: IMPORT CONFIGURATION FROM SD");
    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT + 15);
    tft.print(" WILL DELETE PATCHES, SOUNDS AND RECORDINGS!"); // 43
    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT + 30);
    //       ("01234567890123456789012345678901234567891098765"); // 45 char
    tft.print("        DO YOU REALLY WANT TO PROCEED?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + X_POPUP_OPT + x_pos(4), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Factory_reset_wait_popup(void)
{
    L_POPUP = x_pos(38);
    H_POPUP = y_pos(3); // 64 pixel
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 10; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_YELLOW);
    tft.setCursor(X_POPUP + x_pos(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_BLACK);
    //       ("01234567890123456789012345678901234567");
    tft.print("            FACTORY RESET");
    tft.setCursor(X_POPUP + x_pos(0), Y_POPUP + Y_POPUP_TXT + y_pos(1));
    tft.setTextColor(ILI9341_BLACK);
    //       ("01234567890123456789012345678901234567");
    tft.print("    PLEASE WAIT - DO NOT SWITCH OFF");
}

/*
FLASHMEM
void DisplayManager::Factory_reset_wait_popup(void)
{
    L_POPUP = x_pos(38);
    H_POPUP = y_pos(3); // 64 pixel
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_YELLOW);
    tft.setCursor(X_POPUP + x_pos(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_BLACK);
    //       ("01234567890123456789012345678901234567");
    tft.print("    PLEASE WAIT - DO NOT SWITCH OFF");
}
*/

FLASHMEM
void DisplayManager::SD_missing(uint16_t color)
{
    L_POPUP = x_pos(18);
    H_POPUP = y_pos(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, color);
    tft.setCursor(X_POPUP + x_pos(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    //       ("012345678901234567");
    tft.print("  SD NOT PRESENT");
}

FLASHMEM
void DisplayManager::Config_import_FILE_error_popup(void)
{
    L_POPUP = x_pos(35);
    H_POPUP = y_pos(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);
    tft.setCursor(X_POPUP + x_pos(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_YELLOW);
    //       ("01234567890123456789012345678901234");
    tft.print("  SD/LILLASET/lilla.txt NOT FOUND");
}

FLASHMEM
void DisplayManager::Config_import_REBOOT_popup(void)
{
    L_POPUP = x_pos(38);
    H_POPUP = y_pos(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);
    tft.setCursor(X_POPUP + x_pos(0), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    //           ("01234567890123456789012345678901234567");
    tft.print(" LOADING NEW CONFIGURATION AND REBOOT");
}

FLASHMEM
void DisplayManager::Confirm_config_import_frame(uint8_t value)
{
    Confirm_frame(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT, 2, false); // DISPLAY_confirm_frame(uint8_t col, uint8_t row, uint8_t chars, bool   print)
    Confirm_frame(X_POPUP + X_POPUP_OPT + x_pos(4), Y_POPUP + Y_POPUP_OPT, 3, false);
    switch (value)
    {
    case 0: // NO
        Confirm_frame(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT, 2, true);
        break;
    case 1: // YES
        Confirm_frame(X_POPUP + X_POPUP_OPT + x_pos(4), Y_POPUP + Y_POPUP_OPT, 3, true);
        break;
    default:
        break;
    }
}

FLASHMEM
void DisplayManager::Confirm_config_export_popup(void)
{
    L_POPUP = x_pos(51);
    H_POPUP = y_pos(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 10; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = x_pos(19);

    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    //       ("012345678901234567890123456789012345678901234567890");
    tft.print("        WARNING: EXPORT CONFIGURATION TO SD");
    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT + 15);
    tft.print("  WILL DELETE A PREVIOUS CONFIGURATION FILE SAVED"); // 43
    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT + 30);
    tft.print("        DO YOU REALLY WANT TO PROCEED?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + X_POPUP_OPT + x_pos(4), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Config_export_SD_error_popup(void)
{
    L_POPUP = x_pos(27);
    H_POPUP = y_pos(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);

    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_YELLOW);
    //       ("012345678901234567890123456");
    tft.print(" INSUFFICIENT SPACE IN SD");
}

FLASHMEM
void DisplayManager::Config_export_save_popup(void)
{
    L_POPUP = x_pos(50);
    H_POPUP = y_pos(3);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    Y_POPUP_TXT = 20; // Prima riga testo

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_BLACK);
    tft.setCursor(X_POPUP, Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    //       ("01234567890123456789012345678901234567890123456789");
    tft.print("  CONFIGURATION SAVED: SD/LILLASET/lillaold.txt");
}

FLASHMEM
void DisplayManager::Confirm_factory_reset_popup(void)
{
    L_POPUP = x_pos(49);
    H_POPUP = y_pos(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 15; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = x_pos(19);

    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    //       ("0123456789012345678901234567890123456789012345678");
    tft.print("      WARNING: FACTORY RESET WILL DELETE");
    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT + 15);
    tft.print("      ALL PATCHES, SOUNDS AND RECORDINGS!");
    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT + 30);
    tft.print("        DO YOU REALLY WANT TO PROCEED?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + X_POPUP_OPT, Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + X_POPUP_OPT + x_pos(4), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayManager::Config_reset_popup(void)
{
    L_POPUP = x_pos(49);
    H_POPUP = y_pos(5.2);
    Y_POPUP = (240 - H_POPUP) / 2;
    X_POPUP = (320 - L_POPUP) / 2;
    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED);

    Y_POPUP_TXT = 10; // Prima riga testo
    Y_POPUP_OPT = Y_POPUP_TXT + 50;
    X_POPUP_OPT = x_pos(19);

    tft.setCursor(X_POPUP + x_pos(1), Y_POPUP + Y_POPUP_TXT + 15);
    tft.setTextColor(ILI9341_WHITE);
    //       ("0123456789012345678901234567890123456789012345678");
    tft.print("   PLEASE WAIT. LILLA WILL RESTART AFTER RESET"); // 43
}

FLASHMEM
void DisplayManager::SOUND_EDIT_menu(void)
{
    if (Lilla_state_0 != MIDI_LOOP)
    {
        uint8_t position = 0;

        // Select_sound_edit_menu_elements(); // DISTRIBUITO
        Delete_row(1);

        for (uint8_t element = 0; element < 3; ++element) // menu element
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
                tft.setCursor(x_pos(X_position_Menu_So[position]), y_pos(1));
                tft.setTextColor(MENU_COLOR);
                tft.print(Menu_So_char[element]);
                position++;
            }
        }
    }
}

FLASHMEM
void DisplayManager::Frame_SOUND_EDIT_menu(uint8_t position)
{
    if (Lilla_state_0 != MIDI_LOOP)
    {
        Delete_all_frame_SOUND_EDIT_menu();
        Frame(X_position_Menu_So[position], 1, dimension_voice_Menu_So[element_Menu_So[position]], true);
        choice_sound_menu = element_Menu_So[position];
    }
}

FLASHMEM
void DisplayManager::Delete_all_frame_SOUND_EDIT_menu(void)
{
    uint8_t position;
    for (uint8_t element = 0; element < 3; ++element)
    {
        if (Menu_So[element])
        {
            position = position_Menu_So[element];
            Frame(X_position_Menu_So[position], 1, dimension_voice_Menu_So[element], false);
        }
    }
}

FLASHMEM
void DisplayManager::Instrument_VCF_page(uint8_t session, uint8_t instrument)
{
    uint8_t id_sound = Session[session].Instrument[instrument].id_sound;

    tft.fillScreen(ILI9341_BLACK);
    if (Lilla_state_0 != LIVE_SAMPLING)
    {
        if (Lilla_state_0 != MIDI_LOOP)
        {
            Show_Performance();
        }
        else
        {
            Loop_midi_loop_title();
        }

        Show_Session_number(false);

        tft.setCursor(x_pos(23), y_pos(0));
        tft.setTextColor(TEXT_COLOR);
        tft.print("SOUND");
        tft.setCursor(x_pos(28.5), y_pos(0));
        tft.setTextColor(ILI9341_WHITE);
        tft.print(instrument + 1);

        tft.setCursor(x_pos(43), y_pos(0));
        tft.setTextColor(TEXT_COLOR);
        tft.print("GAIN");
        Show_VCF_gain(id_sound);
    }
    else
    {
        LS_Ring_Tape();
        tft.setCursor(x_pos(41), y_pos(0));
        tft.setTextColor(TEXT_COLOR);
        tft.print("VOLUME");
        Session_volume_value(true); // true: YELLOW
    }

    if (Lilla_state_0 != MIDI_LOOP)
    {
        tft.setCursor(x_pos(0), y_pos(1));
        tft.setTextColor(MENU_COLOR);
        tft.print("RETURN");
        Frame(0, 1, 6, true);
    }

    Show_effects();
    Show_VCF_solo();

    Board(0, Instrument_VCF_TXT_Y + 1.8, 9); // Display.Board(float   col, float row, int chars)
    tft.setCursor(x_pos(0), y_pos(Instrument_VCF_TXT_Y + 1.8));
    tft.setTextColor(ILI9341_WHITE);
    tft.print("VCF + LFO");

    tft.setCursor(x_pos(0), y_pos(Instrument_VCF_TXT_Y + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("FILTER TYPE");
    Show_VCF_filter_type(instrument);

    tft.setCursor(x_pos(0), y_pos(Instrument_VCF_TXT_Y + 4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("CUTOFF (PITCH 1.0)");
    Show_VCF_cutoff(instrument);

    tft.setCursor(x_pos(0), y_pos(Instrument_VCF_TXT_Y + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("RESONANCE");
    Show_VCF_resonance(instrument);

    tft.setCursor(x_pos(Instrument_VCF_TXT_X), y_pos(Instrument_VCF_TXT_Y + 3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MODULATION_SOURCE");
    Show_VCF_lfo_type(instrument);

    tft.setCursor(x_pos(Instrument_VCF_TXT_X), y_pos(Instrument_VCF_TXT_Y + 4));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD FREQ/TIME");
    Show_VCF_lfo_freq_time(instrument);

    tft.setCursor(x_pos(Instrument_VCF_TXT_X), y_pos(Instrument_VCF_TXT_Y + 5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("MOD DEPTH");
    Show_VCF_lfo_index(instrument);

    Led_instrument_INSTRUMENT_VCF(instrument);
}
FLASHMEM
void DisplayManager::Show_VCF_gain(uint8_t id_sound)
{
    Cancel_text_reset_cursor(x_pos(47.5), y_pos(0), 4);
    tft.setTextColor((Lilla_state_0 == LIVE_SAMPLING ? ILI9341_WHITE : ILI9341_YELLOW));
    tft.print(Sound[id_sound].gain / 20.0f);
}

FLASHMEM
void DisplayManager::Show_VCF_solo(void)
{
    Cancel_text_reset_cursor(x_pos(21.5), y_pos(Instrument_VCF_TXT_Y - .1), 8);
    if (solo_flag)
    {
        tft.setTextColor(ILI9341_YELLOW);
        tft.print("* SOLO *");
    }
}

FLASHMEM
void DisplayManager::Show_VCF_filter_type(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(12), y_pos(Instrument_VCF_TXT_Y + 3), 13);
    tft.setTextColor(ILI9341_YELLOW);
    if (Preset[instrument].Filter.use == 1)
    {
        switch (Preset[instrument].Filter.type)
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
void DisplayManager::Show_VCF_cutoff(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(19), y_pos(Instrument_VCF_TXT_Y + 4), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument].Filter.pivot, 0);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("Hz");
}

FLASHMEM
void DisplayManager::Show_VCF_resonance(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(10), y_pos(Instrument_VCF_TXT_Y + 5), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument].Filter.resonance, 2);
}

FLASHMEM
void DisplayManager::Show_VCF_lfo_type(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(Instrument_VCF_TXT_X + 11), y_pos(Instrument_VCF_TXT_Y + 3), 9);
    tft.setTextColor(ILI9341_YELLOW);
    switch (Preset[instrument].Filter.modulation)
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
    Show_VCF_lfo_freq_time(instrument);
}

FLASHMEM
void DisplayManager::Show_VCF_lfo_freq_time(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(Instrument_VCF_TXT_X + 14), y_pos(Instrument_VCF_TXT_Y + 4), 7);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(Preset[instrument].Filter.frequency_time, 2);
    if (Preset[instrument].Filter.periodic)
    {
        Unity("Hz");
    }
    else
    {
        Unity("sec");
    }
}

FLASHMEM
void DisplayManager::Show_VCF_lfo_index(uint8_t instrument)
{
    Cancel_text_reset_cursor(x_pos(Instrument_VCF_TXT_X + 10), y_pos(Instrument_VCF_TXT_Y + 5), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(100 * Preset[instrument].Filter.index, 0);
    tft.print("%");
}

FLASHMEM
void DisplayManager::Show_VCF_pan(uint8_t id_sound)
{
    Cancel_text_reset_cursor(259, Instrument_VCF_TXT_Y + 93, 2);
    if (Sound[id_sound].pan == 0)
    {
        tft.setTextColor(ILI9341_WHITE);
        tft.print("0");
        return;
    }

    tft.setTextColor(ILI9341_WHITE);
    if (Sound[id_sound].pan < 0)
    {
        tft.print("L");
    }
    else if (Sound[id_sound].pan > 0)
    {
        tft.print("R");
    }
    tft.print(abs(Sound[id_sound].pan));
}

FLASHMEM
void DisplayManager::Make_VFS_presentazione(void)
{
    tft.fillScreen(ILI9341_BLACK);

    tft.setCursor(x_pos(0), y_pos(0));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SAMPLER: RECORDING MEMORY DIMENSION");

    tft.setCursor(x_pos(0), y_pos(2));
    tft.setTextColor(ILI9341_YELLOW);
    //        "012345678901234567890 234 X 432 98765432109876543210"; // max 52 char
    tft.print(" PLEASE ASSIGN THE MEMORY SPACE FOR RECORDINGS (AND");
    tft.setCursor(x_pos(0), y_pos(3));
    tft.print("     THE CONSEQUENT SPACE FOR RAW FILES EXPORT)");

    tft.setCursor(x_pos(0), y_pos(5));
    tft.setTextColor(TEXT_COLOR);
    tft.print("TOTAL FLASH MEMORY SPACE ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(Get_flash_size() / 1048576.0f);
    Unity("MB");

    tft.setCursor(x_pos(0), y_pos(6));
    tft.setTextColor(TEXT_COLOR);
    tft.print("RAW FILES IMPORTED ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(Get_flash_occupation() / 1048576.0f);
    Unity("MB");
}

FLASHMEM
void DisplayManager::Make_VFS_assegnazioni(void)
{
    tft.setCursor(x_pos(0), y_pos(7));
    tft.setTextColor(TEXT_COLOR);
    // tft.print("0123456789012345678901234567890....."); // max 52 char
    tft.print("SPACE FOR RECORDINGS");

    tft.setCursor(x_pos(0), y_pos(8));
    tft.setTextColor(TEXT_COLOR);
    // tft.print("0123456789012345678901234567890....."); // max 52 char
    tft.print("SPACE FOR RAW FILES EXPORT");
}

FLASHMEM
void DisplayManager::Make_VFS_restart(void)
{
    tft.setCursor(x_pos(0), y_pos(10));
    tft.setTextColor(ILI9341_GREEN);
    tft.print(ADV_VFS_3);
    tft.setCursor(x_pos(0), y_pos(11));
    tft.print(ADV_VFS_4);
}

FLASHMEM
void DisplayManager::Make_VFS_no_spazio_per_sampler(void)
{
    tft.setCursor(x_pos(0), y_pos(10));
    tft.setTextColor(ILI9341_MAGENTA);
    tft.print(ADV_VFS_0);
    tft.setCursor(x_pos(0), y_pos(11));
    tft.print(ADV_VFS_1);
    tft.setCursor(x_pos(0), y_pos(12));
    tft.print(ADV_VFS_2);
    tft.setCursor(x_pos(0), y_pos(13));
    tft.print(ADV_VFS_3);
    tft.setCursor(x_pos(0), y_pos(14));
    tft.print(ADV_VFS_4);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_titolo(void)
{
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(x_pos(0), y_pos(0));
    tft.print("IMPORT RAW FILES FROM SD TO FLASH MEMORY");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_attesa_SD(void)
{
    tft.setCursor(x_pos(0), y_pos(3));
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("WAITING 10sec FOR SD CARD");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_directory_assente(void)
{
    tft.setCursor(x_pos(0), y_pos(3));
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("NO FILES TO IMPORT: MISSING /LILLARAW DIRECTORY!");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_consistenza_presente(unsigned long SD_raw_volume, int SD_raw_files, int raw_files_volume, int flash_raw_files)
{
    tft.setCursor(x_pos(0), y_pos(3));
    tft.setTextColor(TEXT_COLOR);
    tft.print("SOURCE: SD CARD /LILLARAW");
    tft.setCursor(x_pos(0), y_pos(4));
    tft.print("- RAW FILES");
    tft.setCursor(x_pos(12), y_pos(4));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(SD_raw_files);
    tft.print(" (");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(SD_raw_volume / 1048576.0f, 2);
    Unity("MB");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");

    // Flash chip info
    tft.setCursor(x_pos(0), y_pos(6));
    tft.setTextColor(TEXT_COLOR);
    tft.println("DESTINATION: LILLA FLASH MEMORY");
    tft.setCursor(x_pos(0), y_pos(7));
    tft.println("- DIMENSION");
    tft.setCursor(x_pos(12), y_pos(7));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(flash_dimension_MB);
    Unity("MB");

    tft.setCursor(x_pos(0), y_pos(8));
    tft.setTextColor(TEXT_COLOR);
    tft.print("- RAW FILES");
    tft.setCursor(x_pos(12), y_pos(8));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(flash_raw_files); // Get_raw_files()
    tft.print(" (");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(raw_files_volume / 1048576.0f, 2); // Get_raw_files_volume()
    Unity("MB");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(")");

    tft.setCursor(x_pos(0), y_pos(9));
    tft.setTextColor(TEXT_COLOR);
    tft.print("- RECORDINGS");
    tft.setCursor(x_pos(13), y_pos(9));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(recordings);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_ultimo_avviso(float erasing_time_ms)
{
    tft.setCursor(x_pos(0), y_pos(10));
    tft.setTextColor(TEXT_COLOR);
    tft.println("- DELETING TIME");
    tft.setCursor(x_pos(16), y_pos(10));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(erasing_time_ms / 60000, 1);
    Unity("min");

    // display SD->Flash menu
    tft.setCursor(x_pos(0), y_pos(1));
    tft.setTextColor(MENU_COLOR);
    tft.print("EXIT IMPORT");

    tft.setTextColor(ILI9341_MAGENTA);
    //        "012345678901234567890 234 X 432 98765432109876543210"); // max 52 char
    tft.setCursor(x_pos(22), y_pos(12) - 5);
    tft.print("IMPORTANT");
    tft.setCursor(x_pos(0), y_pos(13) - 5);
    tft.print("- RAW FILES IMPORT WILL DELETE ALL AUDIO FILES AND");
    tft.setCursor(x_pos(0), y_pos(14) - 5);
    tft.print("  RECORDINGS IN LILLA!");
    tft.setCursor(x_pos(0), y_pos(15) - 5);
    tft.print("- LILLA REQUIRES .raw (lowercase!)");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_avvio_copia(void)
{
    // Start erasing flash chip
    tft.fillRect(0, y_pos(11), 320, 240, ILI9341_BLACK);
    tft.setCursor(x_pos(0), y_pos(12) - 5);
    tft.setTextColor(ILI9341_YELLOW);
    //        "012345678901234567890 234 X 432 98765432109876543210"); // max 52 char
    tft.print("PLEASE WAIT: FLASH MEMORY ERASE IS RUNNING.");
    tft.setCursor(x_pos(0), y_pos(13) - 5);
    tft.print("THAN RAW FILES WILL BE COPYED FROM SD/LILLARAW TO ");
    tft.setCursor(x_pos(0), y_pos(14) - 5);
    tft.print("LILLA FLASH MEMORY");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_percentuale_iniziale(void)
{
    tft.drawLine(x_pos(0), BAR_POS_Y, x_pos(0), BAR_POS_Y + 5, ILI9341_YELLOW);
    tft.setCursor(x_pos(0) + 10, BAR_POS_Y);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("0%");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_avanzamento(unsigned char barcount)
{
    tft.drawLine(x_pos(0) + barcount, BAR_POS_Y, x_pos(0) + barcount, BAR_POS_Y + 5, ILI9341_YELLOW);
    tft.setCursor(x_pos(0) + barcount + 10, BAR_POS_Y);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(barcount);
    tft.print("%");
    if (barcount == 100)
    {
        tft.print(" *DONE*");
    }
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_sfondo_popup(void)
{
    // Start copying RAW files from SD to Flash chip
    tft.fillRect(0, 12, 320, 240, ILI9341_BLACK);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_sfondo_elenco(void)
{
    tft.fillRect(0, y_pos(3), 320, 240, ILI9341_BLACK);
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_file_da_copiare(int row, const char *filename, unsigned long length)
{
    tft.setCursor(x_pos(0), y_pos(row));
    tft.setTextColor(TEXT_COLOR);
    tft.print("COPYING FILE ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(filename);
    tft.print("  ");
    tft.print(length / 1024);
    tft.print("KB");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_errore_flash_chip(void)
{
    tft.setTextColor(TEXT_COLOR);
    tft.print("  FLASH MEMORY ERROR");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_errore_flash_full(void)
{
    tft.setTextColor(TEXT_COLOR);
    tft.print("  ERROR: FLASH MEMORY FULL!");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_copia_completata(void)
{
    // Display RAW files list
    tft.fillRect(0, 12, 320, 240, ILI9341_BLACK);
    tft.setCursor(x_pos(0), y_pos(2));
    tft.setTextColor(TEXT_COLOR);
    tft.print("RAW FILES IMPORT COMPLETED. FILE LIST:");
}

FLASHMEM
void DisplayManager::Copy_raw_files_SD_to_Flash_chip_file_copiato(int row, const char *filename, uint32_t filesize)
{
    tft.setCursor(x_pos(0), y_pos(row));
    tft.setTextColor(ILI9341_WHITE);
    tft.print(filename);
    tft.print("  ");
    tft.print(filesize / 1024);
    tft.print("KB");
}

void DisplayManager::Show_LS_ring_tape_wave(int id_sound)
{

    int id_file = Sound[id_sound].file;

    LS_window_A_sample = LS_constrain_position(LS_X_sample - (LS_window_width - 1) / 2);
    LS_window_B_sample = LS_window_A_sample + LS_window_width - 1;

    /*
    Serial.print(F("(LS_buffer_dim - 1): "));
    Serial.print(LS_buffer_dim - 1);
    Serial.print(F("    LS_X_sample: "));
    Serial.print(LS_X_sample);
    Serial.print(F("    LS_window_A_sample: "));
    Serial.print(LS_window_A_sample);
    Serial.print(F("    LS_window_B_sample: "));
    Serial.println(LS_window_B_sample);
    */

    int16_t *X = Info.LS_620_samples_array(id_file, LS_window_A_sample, LS_window_B_sample);

    int yp, yn, y0;
    int i0 = 0;
    y0 = CANVAS_WAVE_0;
    canvas.fillRect(0, 0, WAVE_WIDTH, WAVE_HEIGHT, WAVE_BOARD); // .fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);

    float value_float = WAVE_WIDTH - 1.0;

    LS_K_wave_color = (LS_window_B_sample - LS_window_A_sample) / value_float;

    for (int i = 0; i < WAVE_WIDTH; ++i)
    {
        yp = CANVAS_WAVE_0 - (*(X + i) >> 10);
        yn = CANVAS_WAVE_0 - (*(X + i + WAVE_WIDTH) >> 10);

        uint16_t LS_wave_color = Get_LS_wave_color(i);

        if (yp == CANVAS_WAVE_0 && yn == CANVAS_WAVE_0)
        {
            canvas.drawPixel(i, yp, LS_WAVE_ZERO_COLOR);
            i0 = i;
            y0 = yp;
        }
        else if (yp < CANVAS_WAVE_0 && yn == CANVAS_WAVE_0)
        {
            if (i > 0 && y0 != CANVAS_WAVE_0)
            {
                canvas.drawLine(i0, y0, i, yp, LS_wave_color);
            }
            else
            {
                canvas.drawPixel(i, yp, LS_wave_color);
            }
            i0 = i;
            y0 = yp;
        }
        else if (yp == CANVAS_WAVE_0 && yn > CANVAS_WAVE_0)
        {
            if (i > 0 && y0 != CANVAS_WAVE_0)
            {
                canvas.drawLine(i0, y0, i, yn, LS_wave_color);
            }
            else
            {
                canvas.drawPixel(i, yn, LS_wave_color);
            }
            i0 = i;
            y0 = yn;
        }
        else if (yp < CANVAS_WAVE_0 && yn > CANVAS_WAVE_0)
        {
            if (i > 0 && y0 != CANVAS_WAVE_0)
            {
                canvas.drawLine(i0, y0, i, yn, LS_wave_color);
                canvas.drawLine(i, yp, i, yn, LS_wave_color);
            }
            else
            {
                canvas.drawLine(i, yp, i, yn, LS_wave_color);
            }
            i0 = i;
            y0 = yn;
        }
    }

    canvas.setTextColor(TEXT_COLOR);
    if (!LS_stereo)
    {
        canvas.setCursor((WAVE_WIDTH / 2) - 12, y_pos(0));
        canvas.print("MONO");
    }
    else if (id_sound == SOUNDS_MAX)
    {
        canvas.setCursor((WAVE_WIDTH / 2) - 63, y_pos(0));
        canvas.print("(STEREO) LEFT CHANNEL");
    }
    else
    {
        canvas.setCursor((WAVE_WIDTH / 2) - 66, y_pos(0));
        canvas.print("(STEREO) RIGHT CHANNEL");
    }

    canvas.setTextColor(TEXT_COLOR);
    canvas.setCursor(x_pos(0), T_LOW);
    canvas.print("WINDOW ");
    canvas.setTextColor(ILI9341_YELLOW);
    canvas.print(LS_window_width / 44100.0f, 1);
    canvas.setTextColor(ILI9341_ORANGE);
    canvas.print("sec");

    LS_draw_XY_lines();
    Update_LS_rec();
    tft.drawRGBBitmap(WAVE_X0, WAVE_MAX, canvas.getBuffer(), canvas.width(), canvas.height());

    // Serial.print("  -  ");
    // Serial.println(minitimer);
}

void DisplayManager::Update_LS_rec(void)
{
    if (LS_state != 1)
    {
        DS_blink_ON = false;
    }
    else if (DS_blink_timer >= 500)
    {
        DS_blink_ON = !DS_blink_ON;
        DS_blink_timer = 0;
    }
    canvas.drawBitmap(5, y_pos(0), led_pic, 5, 8, (DS_blink_ON ? ILI9341_RED : RED_OFF));
    canvas.setCursor(x_pos(2), y_pos(0));
    canvas.setTextColor((DS_blink_ON ? ILI9341_RED : RED_OFF));
    canvas.print("REC");
}

void DisplayManager::LS_draw_XY_lines(void)
{
    // draw LS_X_sample
    canvas.drawLine(WAVE_WIDTH / 2, CANVAS_WAVE_0 - 30, WAVE_WIDTH / 2, CANVAS_WAVE_0 + 30, LS_X_COLOR);

    // draw LS_Y_sample
    if (LS_mode > 1) // loops
    {
        int LS_Y_sample_local = LS_Y_sample;
        if (LS_Y_sample_local < LS_window_A_sample)
        {
            LS_Y_sample_local += LS_buffer_dim;
        }

        //     Caso 0: Y NON rientra nella window
        //     0                                        PQ  (FIFO_dim - 1)
        //     |..................................................|
        //                       A===================B  Y

        //     Caso 1: Y incrementato NON rientra nella window
        //     0     PQ                                    (FIFO_dim - 1)
        //     |    (Y)          A===================B            |     Y

        //     Caso 2: Y incrementato rientra nella window
        //     0     PQ                                    (FIFO_dim - 1)
        //     |    (Y)                                 A=========|=====Y======B

        if (LS_Y_sample_local >= LS_window_A_sample && LS_Y_sample_local <= LS_window_B_sample)
        {
            float LS_Y = LS_Y_sample_local - LS_window_A_sample;
            float LS_W = LS_window_width;
            int LS_Y_sample_local_x = (LS_Y / LS_W) * (WAVE_WIDTH - 1); //  window) * 127.0;
            canvas.drawLine(LS_Y_sample_local_x, CANVAS_WAVE_0 - 30, LS_Y_sample_local_x, CANVAS_WAVE_0 + 30, LS_Y_COLOR);
        }
    }
}

uint16_t DisplayManager::Get_LS_wave_color(int point)
{
    uint16_t green;
    int position;
    int distance;
    static float distance_0 = 0;

    position = LS_constrain_position(LS_window_A_sample + point * LS_K_wave_color);
    if (position > LS_Q_sample)
    {
        position -= LS_buffer_dim;
    }
    distance = LS_Q_sample - position;
    green = constrain(63 * ((float)(LS_buffer_dim - distance) / (float)LS_buffer_dim), 0, 63);
    if ((distance - distance_0) > 1000000)
    {
        green = 63;
    }
    distance_0 = distance;
    return (31 << 11) + (green << 5); // (red << 11) + (green << 5) + blue
}

void DisplayManager::Show_wave(uint8_t instrument)
{
    uint8_t id_sound = Session[session].Instrument[instrument].id_sound;
    int yp, yn, y0;
    int NC_A;
    int16_t *X = Info.Sound_620_samples_array(Preset[instrument].file, Preset[instrument].A, Preset[instrument].B);
    float volume_float = Volume_float[Sound[id_sound].gain];

    NC_A = ((WAVE_WIDTH * (Preset[instrument].Noclick < Noclick_max ? Preset[instrument].Noclick : Noclick_max)) / (Preset[instrument].B - Preset[instrument].A));
    if (NC_A == 0 && (Preset[instrument].Noclick > 0))
    {
        NC_A = 1;
    }

    int NC_B = WAVE_WIDTH - NC_A;
    int i0 = 0;
    y0 = CANVAS_WAVE_0; // central position

    // tft.fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    canvas.fillRect(0, 0, WAVE_WIDTH, WAVE_HEIGHT, WAVE_BOARD);
    canvas.fillRect(0, 0, NC_A, WAVE_HEIGHT, WAVE_BOARD_NOCLICK);
    canvas.fillRect(NC_B, 0, NC_A, WAVE_HEIGHT, WAVE_BOARD_NOCLICK);

    for (int i = 0; i < WAVE_WIDTH; ++i)
    {
        // 16bits>>11 = 5bits from -16 to +15
        yp = CANVAS_WAVE_0 - (*(X + i) >> 10) * volume_float; // MAXIMUM delta 32*4 = 128  (−32768 <= int16_t <= +32767)
        yn = CANVAS_WAVE_0 - (*(X + i + WAVE_WIDTH) >> 10) * volume_float;

        if (yp == CANVAS_WAVE_0 && yn == CANVAS_WAVE_0)
        {
            if (i == 0)
            {
                canvas.drawPixel(0, yp, WAVE_COLOR);
            }
            else
            {
                canvas.drawLine(i0, y0, i, CANVAS_WAVE_0, WAVE_COLOR);
                i0 = i;
                y0 = yp;
            }
        }
        else if (yp < CANVAS_WAVE_0 && yn == CANVAS_WAVE_0)
        {
            if (i > 0)
            {
                canvas.drawLine(i0, y0, i, yp, WAVE_COLOR);
            }
            else
                canvas.drawPixel(0, yp, WAVE_COLOR);
            i0 = i;
            y0 = yp;
        }
        else if (yp == CANVAS_WAVE_0 && yn > CANVAS_WAVE_0)
        {
            if (i > 0)
            {
                canvas.drawLine(i0, y0, i, yn, WAVE_COLOR);
            }
            else
            {
                canvas.drawPixel(0, yn, WAVE_COLOR);
            }
            i0 = i;
            y0 = yn;
        }
        else if (yp < CANVAS_WAVE_0 && yn > CANVAS_WAVE_0)
        {
            if (i > 0)
            {
                canvas.drawLine(i0, y0, i, yn, WAVE_COLOR);
                canvas.drawLine(i, yp, i, yn, WAVE_COLOR);
            }
            else
            {
                canvas.drawLine(0, yp, 0, yn, WAVE_COLOR);
            }
            i0 = i;
            y0 = yn;
        }
    }

    if (solo_flag)
    {
        canvas.setCursor(x_pos(21) + 5, y_pos(0));
        canvas.setTextColor(ILI9341_YELLOW);
        canvas.print("* SOLO *");
    }

    canvas.setTextColor(TEXT_COLOR);
    canvas.setCursor(x_pos(0), T_LOW);
    canvas.print("FROM");
    canvas.setTextColor((slicing_mode ? ILI9341_WHITE : ILI9341_YELLOW));
    canvas.setCursor(x_pos(4.5), T_LOW);
    canvas.setTextColor(ILI9341_YELLOW);
    canvas.print(Preset[instrument].A);
    canvas.setTextColor(ILI9341_ORANGE);
    canvas.print("S");

    canvas.setTextColor(TEXT_COLOR);
    canvas.setCursor(x_pos(21), T_LOW);
    canvas.print("TOT");
    canvas.setTextColor((slicing_mode ? ILI9341_WHITE : ILI9341_YELLOW));
    canvas.setCursor(x_pos(24.5), T_LOW);

    float time = (Preset[instrument].B - Preset[instrument].A + 1) / 44100.0f; // sec
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
        String Value = String(Preset[instrument].B, DEC);
        canvas.setCursor(x_pos(46 - Value.length() - 1), T_LOW);
        canvas.setTextColor(TEXT_COLOR);
        canvas.print("TO");
        canvas.setCursor(x_pos(48.5 - Value.length() - 1), T_LOW);
        canvas.setTextColor(ILI9341_YELLOW);
        canvas.print(Value);
        canvas.setTextColor(ILI9341_ORANGE);
        canvas.print("S");
    }

    else // Window fissa: FIRST/WINDOW
    {
        String Value = String(Preset[instrument].B - Preset[instrument].A + 1, DEC);
        canvas.setCursor(x_pos(45 - Value.length() - 1), T_LOW);
        canvas.setTextColor(TEXT_COLOR);
        canvas.print("SLICE");
        canvas.setCursor(x_pos(50.5 - Value.length() - 1), T_LOW);
        canvas.setTextColor(ILI9341_YELLOW);
        canvas.print(Value);
        canvas.setTextColor(ILI9341_ORANGE);
        canvas.print("S");
    }
    tft.drawRGBBitmap(WAVE_X0, WAVE_MAX, canvas.getBuffer(), canvas.width(), canvas.height());
}

void DisplayManager::Loop_led(int Xled, int Yled, bool ONled)
{
    tft.drawBitmap(Xled, Yled, led_pic, 8, 8, ONled ? GREEN_ON : GREEN_OFF);
}

void DisplayManager::Loop_led_metronomo(int Xled, int Yled, bool ONled)
{
    tft.drawBitmap(Xled, Yled, led_pic, 8, 8, ONled ? RED_ON : RED_OFF);
}