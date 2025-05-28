/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once
#include <Arduino.h>

// Colori Display
// 16-bit ('565') color settings http://www.barth-dev.de/online/rgb565-color-picker/ and https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
static constexpr uint16_t TEXT_COLOR = 0x07DA;
static constexpr uint16_t TEXT_OFF = 0x02AA;
static constexpr uint16_t FRAME_COLOR = 0xF800; // red
static constexpr uint16_t MENU_COLOR = 0xFFE0;  // yellow
static constexpr uint16_t WAVE_COLOR = 0xE08A;
static constexpr uint16_t WAVE_FRAME = 0x926C;
static constexpr uint16_t WAVE_BOARD = 0x2124;
static constexpr uint16_t WAVE_BOARD_NOCLICK = 0x3111;
static constexpr uint16_t GREEN_ON = 0x07E0;
static constexpr uint16_t RED_ON = 0xF800;
static constexpr uint16_t GREEN_OFF = 0x02C1;
static constexpr uint16_t RED_OFF = 0x70A4;//0x8000;
/*
  Black            = 0x0000,
  White            = 0xFFFF,
  BrightRed        = 0xF800,
  BrightGreen      = 0x07E0,
  BrightBlue       = 0x001F,
  BrightCyan       = 0x07FF,
  BrightMagenta    = 0xF81F,
  Yellow           = 0xFFF0,
  BrightYellow     = 0xFFE0,
  LightYellow      = 0xFFF2,
  Gold             = 0xFEA0,
  Blue             = 0x0010,
  Green            = 0x0400,
  Cyan             = 0x0410,
  Red              = 0x8000,
  Magenta          = 0x8010,
  Brown            = 0xFC00,
  LightGray        = 0x8410,
  DarkGray         = 0x4208,
  LightBlue        = 0x841F,
  LightGreen       = 0x87F0,
  LightCyan        = 0x87FF,
  LightRed         = 0xFC10,
  LightMagenta     = 0xFC1F,
  SaddleBrown      = 0x8A22,
  Sienna           = 0xA285,
  Peru             = 0xCC27,
  Burlywood        = 0xDDD0,
  Wheat            = 0xF7BB,
  Tan              = 0xD5B1,
  Orange           = 0xFDC9,
  DarkOrange       = 0xFC60,
  LightOrange      = 0xFE40,
  Gray242          = 0xF79E,
  Gray229          = 0xE73C,
  Gray204          = 0xCE79,
  Gray192          = 0xC618,
  Gray160          = 0xA514,
  Gray128          = 0x8410,
  Gray96           = 0x630C,
  Gray32           = 0x2104,
  Gray10           = 0x0841

#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198

*/

// Bitmaps
// tool: lcd-image-converter.exe
// drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)

extern const unsigned char LOGO_0[];
extern const unsigned char PROGMEM LOGO_1[];
extern const unsigned char PROGMEM LOGO_2[];
extern const unsigned char PROGMEM LOGO_3[];
extern const unsigned char PROGMEM LOGO_4[];
extern const unsigned char PROGMEM audio_sampler[];
extern const unsigned char PROGMEM led_pic[]; // 5x8
extern const unsigned char PROGMEM DS_freccia[]; // 19X7
extern const unsigned char PROGMEM DS_freccia_gomito[]; // 13x23
extern const unsigned char PROGMEM LS_play_fwd[]; //
extern const unsigned char PROGMEM LS_play_rev[]; //