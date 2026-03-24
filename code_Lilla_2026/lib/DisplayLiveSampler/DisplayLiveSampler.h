/*
   LILLA Audio Sampler
   Author: Sandro Grassia (info@lillasampler.it)
   www.lillasampler.it
*/

#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>     // https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
#include <Adafruit_ILI9341.h> // 1.5.12 version - Hardware-specific library
#include "DisplayPrimitives.h"
#include <AudioStream.h> // solo per definizione AUDIO_SAMPLE_RATE
#include "SharedLS.h"
#include "SharedMixer.h"
#include "InfoMaster.h"

class DisplayLiveSampler
{
private:
    InfoMaster &Info;

    float LS_K_wave_color;
    float LS_wave_poit_distance_0 = 0;
    int LS_window_A_sample;
    int LS_window_B_sample;

    elapsedMillis LS_blink_timer;
    bool LS_blink_ON = false;

    static constexpr int LS_MV = 4;
    static constexpr uint16_t LS_WAVE_COLOR = 0xE08A;
    static constexpr uint16_t LS_WAVE_ZERO_COLOR = 0x7BCF;
    static constexpr uint16_t LS_WAVE_BOARD = 0xFE40;     // 0xA514
    static constexpr uint16_t LS_X_COLOR = ILI9341_GREEN; // Live Sampling linee verticali di esecuzione
    static constexpr uint16_t LS_Y_COLOR = ILI9341_WHITE; // Live Sampling linee verticali di esecuzione

    void Draw_XY_lines(void);
    void Delete_menu_frames(void);
    void Page_title(void);

    // elapsedMicros localtimer;
    //  int memo[2];

public:
    DisplayLiveSampler(InfoMaster &Obj) : Info(Obj) {}

    // Gestione LED
    void Led_LIVE_SAMPLING(bool on);

    // LIVE_SAMPLING
    void Confirm_EXIT_from_LS(void);
    void Page(void);
    void Feedback(void);
    void Step(void);
    void Buffer_dimension(void);
    void Volume(void);
    void Play_mode(void);
    void Loop_time(void);
    void X_sample_delta(void);
    void Menu(void);
    void Menu_frame(int position);
    void Show_wave(int sound_id);
    uint16_t Get_wave_color(int point);
    void Update_REC_LED(void);
};