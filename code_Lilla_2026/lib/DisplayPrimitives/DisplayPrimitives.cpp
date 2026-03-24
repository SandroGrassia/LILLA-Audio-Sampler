/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#include "DisplayPrimitives.h"

namespace DisplayPrimitives
{
    int display_coordinate_y(const float row)
    {
        return 4 + (15.0 * row);
    }

    int display_coordinate_x(const float col)
    {
        return 4 + (6.0 * col);
    }

    void Frame_by_col_row(const float col, const float row, const int chars, const bool show)
    {
        Frame_by_pixels(display_coordinate_x(col), display_coordinate_y(row), chars, show);
    }

    void Frame_by_pixels(const int X, const int Y, const int chars, const bool show)
    {
       tft.drawRect(X - 4, Y - 4, (6 * chars) + 7, 15, (show ? FRAME_COLOR : ILI9341_BLACK)); // drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
    }

    void Popup(String text, uint16_t text_color, uint16_t filler_color)
    {
        int L_POPUP = display_coordinate_x(text.length() + 4); // lunghezza
        int H_POPUP = display_coordinate_y(3);                 // altezza
        int Y_POPUP = (240 - H_POPUP) / 2;
        int X_POPUP = (320 - L_POPUP) / 2;
        int Y_POPUP_TXT = 20; // riga testo

        tft.fillRoundRect(X_POPUP, Y_POPUP, L_POPUP, H_POPUP, 4, filler_color);
        tft.setCursor(X_POPUP + display_coordinate_x(2), Y_POPUP + Y_POPUP_TXT);
        tft.setTextColor(text_color);
        tft.print(text);
    }

    void Board(const float col, const float row, const int chars)
    {
        tft.fillRect(display_coordinate_x(col) - 4, display_coordinate_y(row) - 2, (6 * chars) + 7, 11, 0x9000); // fillRect(uint16_t x0, uint16_t y0, uint16_t width, uint16_t heigh, uint16_t color);
    }

    void Cancel_text(const int X, const int Y, const int N)
    {
        tft.fillRect(X, Y, (N * 6), 8, ILI9341_BLACK); // fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    }

    void Cancel_text_reset_cursor(const int X, const int Y, const int N)
    {
        Cancel_text(X, Y, N);
        tft.setCursor(X, Y);
    }

    void Unit(const char *what, const int lenght)
    {
        tft.setTextColor(ILI9341_ORANGE);
        for (auto i = 0; i < lenght; ++i)
        {
            tft.print(*(what + i));
        }
    }

    void Delete_row(const float row)
    {
        tft.fillRect(0, display_coordinate_y(row) - 4, 320, 15, ILI9341_BLACK); // fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    }
}