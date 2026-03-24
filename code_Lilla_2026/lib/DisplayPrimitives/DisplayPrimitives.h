/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#pragma once

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include "GraphicElements.h"
#include "GlobalDisplay.h"

namespace DisplayPrimitives
{
   // canvas Y-positions on display
   static constexpr int X_WAVEBOARD_LEFT = 5;
   static constexpr int WAVE_MAX = 122;
   static constexpr int WAVEBOARD_WIDTH = 310;
   static constexpr float WAVEBOARD_WIDTH_F = 310.0;
   static constexpr int WAVEBOARD_HEIGHT = 97; // deve essere DISPARI
   static constexpr int Y_FOOTER_TEXT = WAVEBOARD_HEIGHT - 12;

   // wave Y-positions on canvas
   static constexpr int CANVAS_WAVE_MIN = WAVEBOARD_HEIGHT - 1;
   static constexpr int CANVAS_WAVE_0 = CANVAS_WAVE_MIN / 2;

   int display_coordinate_y(const float row);
   int display_coordinate_x(const float col);
   void Frame_by_col_row(const float col, const float row, const int chars, const bool show);
   void Frame_by_pixels(const int X, const int Y, const int chars, const bool show);
   void Popup(String text, uint16_t text_color, uint16_t filler_color);
   void Board(const float col, const float row, const int chars);
   void Cancel_text(const int X, const int Y, const int N);
   void Cancel_text_reset_cursor(const int X, const int Y, const int N);
   void Unit(const char *what, const int lenght);
   void Delete_row(const float row); // delete text row
}