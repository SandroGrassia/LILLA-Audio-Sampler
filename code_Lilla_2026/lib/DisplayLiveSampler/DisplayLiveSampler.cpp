/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#include "DisplayLiveSampler.h"

void DisplayLiveSampler::Led_LIVE_SAMPLING(bool on)
{
    if (on)
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3), led_pic, 6, 8, ((MX_mute[0] && MX_mute[1]) ? RED_ON : GREEN_ON));
    }
    else
    {
        tft.drawBitmap(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(3), led_pic, 8, 8, ((MX_mute[0] && MX_mute[1]) ? RED_OFF : GREEN_OFF));
    }
    return;
}

FLASHMEM
void DisplayLiveSampler::Confirm_EXIT_from_LS(void)
{
    const int L_POPUP = 106;                 // Larghezza
    const int H_POPUP = 47;                  // Altezza
    const int X_POPUP = (320 - L_POPUP) / 2; // X posizione su display
    const int Y_POPUP = (240 - H_POPUP) / 2; // Y posizione su display
    const int Y_POPUP_TXT = 10;              // prima riga testo
    const int Y_POPUP_OPT = 30;              // riga opzioni

    tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, ILI9341_RED); // does NOT delete frame
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(1), Y_POPUP + Y_POPUP_TXT);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("STOP RECORDING?");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(5.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("NO");
    tft.setCursor(X_POPUP + DisplayPrimitives::display_coordinate_x(9.5), Y_POPUP + Y_POPUP_OPT);
    tft.print("YES");
}

FLASHMEM
void DisplayLiveSampler::Page(void)
{
    //("012345678901234567890"); // Size 1: 21 chars
    tft.fillScreen(ILI9341_BLACK);
    Page_title();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(37), DisplayPrimitives::display_coordinate_y(0));
    tft.print("LENGTH");
    Buffer_dimension();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(1.5), DisplayPrimitives::display_coordinate_y(3));
    tft.print("VOLUME");
    Volume();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(37), DisplayPrimitives::display_coordinate_y(3));
    tft.print("FEEDBACK");
    Feedback();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(4));
    tft.print("PLAY MODE");
    Play_mode();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(5));
    tft.print("START POINT");
    X_sample_delta();

    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(6));
    tft.print("STEP");
    Step();
}

FLASHMEM
void DisplayLiveSampler::Page_title(void)
{
    //("012345678901234567890"); // Size 1: 21 chars
    DisplayPrimitives::Board(0, 0, 12); // Display.DisplayPrimitives::Board(float & col, float row, int chars)
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::display_coordinate_y(0));
    tft.print("LIVE SAMPLER");
}

FLASHMEM
void DisplayLiveSampler::Feedback(void)
{
    DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(46), DisplayPrimitives::display_coordinate_y(3), 6);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(100 * LS_fbk_table[LS_feedback], 2);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("%");
}

FLASHMEM
void DisplayLiveSampler::Step(void)
{
    DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(5), DisplayPrimitives::display_coordinate_y(6), 16);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(LS_X_step);
    tft.setTextColor(ILI9341_ORANGE);
    tft.print("samples");
}

FLASHMEM
void DisplayLiveSampler::Buffer_dimension(void)
{
    DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(44), DisplayPrimitives::display_coordinate_y(0), 12);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(LS_buffer_dim / 44100.0f, 1);
    DisplayPrimitives::Unit("sec", 3);
}

FLASHMEM
void DisplayLiveSampler::Volume(void)
{
    DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(8.5), DisplayPrimitives::display_coordinate_y(3), 4);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(volume_patch / 20.0f);
}

FLASHMEM
void DisplayLiveSampler::Play_mode(void)
{
    DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(10), DisplayPrimitives::display_coordinate_y(4), 20);
    tft.setTextColor(ILI9341_YELLOW);
    if (LS_mode > 1)
    {
        tft.print(loop_mode[LS_mode]);
        tft.print(" ");
    }
    tft.print(name_mode[LS_mode]);
}

FLASHMEM
void DisplayLiveSampler::Loop_time(void)
{
    tft.setTextColor(ILI9341_YELLOW);
    if (LS_mode == 2)
    {
        DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(19), DisplayPrimitives::display_coordinate_y(4), 9);
        tft.print(LS_XY_delta / 44100.0f, 2);
        DisplayPrimitives::Unit("sec", 3);
    }
    else if (LS_mode == 3)
    {
        DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(23), DisplayPrimitives::display_coordinate_y(4), 9);
        tft.print(LS_XY_delta / 44100.0f, 2);
        DisplayPrimitives::Unit("sec", 3);
    }
}

FLASHMEM
void DisplayLiveSampler::X_sample_delta(void)
{
    float local_value = 0;
    DisplayPrimitives::Cancel_text_reset_cursor(DisplayPrimitives::display_coordinate_x(12), DisplayPrimitives::display_coordinate_y(5), 40);
    tft.setTextColor(ILI9341_YELLOW);

    if (LS_XY_lock)
    {
        tft.print("FIXED ");
        tft.print(LS_X_sample / 44100.0f, 2);
        DisplayPrimitives::Unit("sec", 3);
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

        DisplayPrimitives::Unit("sec", 3);
    }
}

FLASHMEM
void DisplayLiveSampler::Menu(void)
{
    auto position = 0; // position on display
    DisplayPrimitives::Delete_row(1);
    tft.setTextColor(MENU_COLOR);

    for (auto element = 0; element < LS_MV; ++element) // menu element
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
            tft.setCursor(DisplayPrimitives::display_coordinate_x(X_position_Menu_LS[position]), DisplayPrimitives::display_coordinate_y(1));
            tft.setTextColor(MENU_COLOR);
            tft.print(Menu_LS_char[element]);
            position++;
        }
    }
}

FLASHMEM
void DisplayLiveSampler::Menu_frame(int position)
{
    Delete_menu_frames();
    DisplayPrimitives::Frame_by_col_row(X_position_Menu_LS[position], 1, dimension_voice_Menu_LS[element_Menu_LS[position]], true);
    // choice_LS_menu = element_Menu_LS[position]; // TRASFERITA NEL CODICE main.cpp
}

FLASHMEM
void DisplayLiveSampler::Delete_menu_frames(void)
{
    int position;

    for (auto element = 0; element < LS_MV; ++element)
    {
        if (Menu_LS[element])
        {
            position = position_Menu_LS[element];
            DisplayPrimitives::Frame_by_col_row(X_position_Menu_LS[position], 1, dimension_voice_Menu_LS[element], false);
        }
    }
}

void DisplayLiveSampler::Show_wave(int sound_id)
{
    int id_file = Sound[sound_id].file;

    LS_window_A_sample = LS_constrain_position(LS_X_sample - (LS_window_width - 1) / 2);
    LS_window_B_sample = LS_window_A_sample + LS_window_width - 1;

    if (false)
    {
        Serial.print(F("(LS_buffer_dim - 1): "));
        Serial.print(LS_buffer_dim - 1);
        Serial.print(F("    LS_X_sample: "));
        Serial.print(LS_X_sample);
        Serial.print(F("    LS_window_A_sample: "));
        Serial.print(LS_window_A_sample);
        Serial.print(F("    LS_window_B_sample: "));
        Serial.println(LS_window_B_sample);
    }

    int16_t *Wave_array = Info.LS_620_samples_array(id_file, LS_window_A_sample, LS_window_B_sample);

    // localtimer = 0;

    canvas.fillRect(0, 0, DisplayPrimitives::WAVEBOARD_WIDTH, DisplayPrimitives::WAVEBOARD_HEIGHT, WAVE_BOARD_COLOR); // .fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);

    LS_K_wave_color = (LS_window_B_sample - LS_window_A_sample) / static_cast<float>(DisplayPrimitives::WAVEBOARD_WIDTH - 1);

    /* canvas coordinates

                                0                                                 WAVEBOARD_WIDTH - 1 (309)
                                ---------------------------------------------------------------
                            0   |                                                             |
                                |                                                             |
                                |                                                             |                                                                                       
                                |                                                             |                                
           CANVAS_WAVE_0 (48)   |-------------------------------------------------------------|
                                |                                                             |
                                |                                                             |                                                                                       
                                |                                                             |                                
                                |                                                             |
    WAVEBOARD_HEIGHT - 1 (96)   |_____________________________________________________________|

    */
    
    int y0 = DisplayPrimitives::CANVAS_WAVE_0;
    int wave_pixel_0 = 0;

    for (auto wave_pixel = 0; wave_pixel < DisplayPrimitives::WAVEBOARD_WIDTH; ++wave_pixel)
    {
        int yp = DisplayPrimitives::CANVAS_WAVE_0 - (*(Wave_array + wave_pixel) >> 10); // valore minimo = 48 - 32 = 16
        int yn = DisplayPrimitives::CANVAS_WAVE_0 - (*(Wave_array + wave_pixel + DisplayPrimitives::WAVEBOARD_WIDTH) >> 10); // valore massimo 48 + 32 = 80

        uint16_t LS_wave_color = Get_wave_color(wave_pixel);

        if (yp == DisplayPrimitives::CANVAS_WAVE_0 && yn == DisplayPrimitives::CANVAS_WAVE_0)
        {
            canvas.drawPixel(wave_pixel, yp, LS_WAVE_ZERO_COLOR);
            wave_pixel_0 = wave_pixel;
            y0 = yp;
        }

        else if (yp < DisplayPrimitives::CANVAS_WAVE_0 && yn == DisplayPrimitives::CANVAS_WAVE_0)
        {
            if (wave_pixel > 0 && y0 != DisplayPrimitives::CANVAS_WAVE_0)
            {
                canvas.drawLine(wave_pixel_0, y0, wave_pixel, yp, LS_wave_color);
            }
            else
            {
                canvas.drawPixel(wave_pixel, yp, LS_wave_color);
            }

            wave_pixel_0 = wave_pixel;
            y0 = yp;
        }

        else if (yp == DisplayPrimitives::CANVAS_WAVE_0 && yn > DisplayPrimitives::CANVAS_WAVE_0)
        {
            if (wave_pixel > 0 && y0 != DisplayPrimitives::CANVAS_WAVE_0)
            {
                canvas.drawLine(wave_pixel_0, y0, wave_pixel, yn, LS_wave_color);
            }
            else
            {
                canvas.drawPixel(wave_pixel, yn, LS_wave_color);
            }

            wave_pixel_0 = wave_pixel;
            y0 = yn;
        }

        else if (yp < DisplayPrimitives::CANVAS_WAVE_0 && yn > DisplayPrimitives::CANVAS_WAVE_0)
        {
            if (wave_pixel > 0 && y0 != DisplayPrimitives::CANVAS_WAVE_0)
            {
                canvas.drawLine(wave_pixel_0, y0, wave_pixel, yn, LS_wave_color);
                canvas.drawLine(wave_pixel, yp, wave_pixel, yn, LS_wave_color);
            }
            else
            {
                canvas.drawLine(wave_pixel, yp, wave_pixel, yn, LS_wave_color);
            }

            wave_pixel_0 = wave_pixel;
            y0 = yn;
        }
    }

    canvas.setTextColor(TEXT_COLOR);

    if (!LS_stereo)
    {
        canvas.setCursor((DisplayPrimitives::WAVEBOARD_WIDTH / 2) - 12, DisplayPrimitives::display_coordinate_y(0));
        canvas.print("MONO");
    }
    else if (sound_id == SOUNDS_MAX)
    {
        canvas.setCursor((DisplayPrimitives::WAVEBOARD_WIDTH / 2) - 63, DisplayPrimitives::display_coordinate_y(0));
        canvas.print("(STEREO) LEFT CHANNEL");
    }
    else
    {
        canvas.setCursor((DisplayPrimitives::WAVEBOARD_WIDTH / 2) - 66, DisplayPrimitives::display_coordinate_y(0));
        canvas.print("(STEREO) RIGHT CHANNEL");
    }

    canvas.setTextColor(TEXT_COLOR);
    canvas.setCursor(DisplayPrimitives::display_coordinate_x(0), DisplayPrimitives::Y_FOOTER_TEXT);
    canvas.print("WINDOW ");
    canvas.setTextColor(ILI9341_YELLOW);
    canvas.print(LS_window_width / 44100.0f, 1);
    canvas.setTextColor(ILI9341_ORANGE);
    canvas.print("sec");

    Draw_XY_lines();
    Update_REC_LED();

    // memo[0] = localtimer; // 530us
    tft.drawRGBBitmap(DisplayPrimitives::X_WAVEBOARD_LEFT, DisplayPrimitives::WAVE_MAX, canvas.getBuffer(), canvas.width(), canvas.height());
    // memo[1] = localtimer; // memo[1] - memo[0] = 44.000us
    
    // Serial.print("Fill canvas, microseconds:");
    // Serial.print(memo[0]);
    // Serial.print(" Fill display, microseconds:");
    // Serial.println(memo[1] - memo[0]);
}

void DisplayLiveSampler::Update_REC_LED(void)
{
    if (LS_state != REC)
    {
        LS_blink_ON = false;
    }
    else if (LS_blink_timer >= 500)
    {
        LS_blink_ON = !LS_blink_ON;
        LS_blink_timer = 0;
    }

    canvas.drawBitmap(5, DisplayPrimitives::display_coordinate_y(0), led_pic, 5, 8, (LS_blink_ON ? ILI9341_RED : RED_OFF));
    canvas.setCursor(DisplayPrimitives::display_coordinate_x(2), DisplayPrimitives::display_coordinate_y(0));
    canvas.setTextColor((LS_blink_ON ? ILI9341_RED : RED_OFF));
    canvas.print("REC");
}

void DisplayLiveSampler::Draw_XY_lines(void)
{
    // draw LS_X_sample
    canvas.drawLine(DisplayPrimitives::WAVEBOARD_WIDTH / 2, DisplayPrimitives::CANVAS_WAVE_0 - 30, DisplayPrimitives::WAVEBOARD_WIDTH / 2, DisplayPrimitives::CANVAS_WAVE_0 + 30, LS_X_COLOR);

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
            int LS_Y_sample_local_x = (LS_Y / LS_W) * (DisplayPrimitives::WAVEBOARD_WIDTH - 1); //  window) * 127.0;
            canvas.drawLine(LS_Y_sample_local_x, DisplayPrimitives::CANVAS_WAVE_0 - 30, LS_Y_sample_local_x, DisplayPrimitives::CANVAS_WAVE_0 + 30, LS_Y_COLOR);
        }
    }
}

uint16_t DisplayLiveSampler::Get_wave_color(int point)
{
    int position = LS_constrain_position(LS_window_A_sample + point * LS_K_wave_color);

    if (position > LS_Q_sample)
    {
        position -= LS_buffer_dim;
    }

    int distance = LS_Q_sample - position;
    uint16_t green = constrain(63 * ((float)(LS_buffer_dim - distance) / (float)LS_buffer_dim), 0, 63);

    if ((distance - LS_wave_poit_distance_0) > 1000000)
    {
        green = 63;
    }

    LS_wave_poit_distance_0 = distance;
    return (31 << 11) + (green << 5); // (red << 11) + (green << 5) + blue
}