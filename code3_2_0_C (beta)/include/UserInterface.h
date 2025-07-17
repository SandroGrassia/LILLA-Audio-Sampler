/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once
#include <Arduino.h>


// *************************************************************
// ********     MULTIPLEXERS, ENCODERS, PUSHBUTTONS    *********
// *************************************************************

// LILLA 2022/2023/2024/2025 comprende n.7 multiplexer (MUX)
//
// vista backside:
//
// MUX3     MUX2      MUX1
// MUX4     MUX5      MUX6
//                    MUX7

// HARDWARE
static constexpr int ENCODERS = 26;
static constexpr int PUSHBUTTONS = 36;

// microseconds between send address and read value
static constexpr int PAUSE_MUX = 10;
static constexpr int PB_UP_PAUSE = 50;
static constexpr int PB_DOWN_PAUSE = 50;

// pin del T41 dedicati al bus address dei MUX
static constexpr int MUX_S0_pin = 37;
static constexpr int MUX_S1_pin = 35;
static constexpr int MUX_S2_pin = 34;
static constexpr int MUX_S3_pin = 33;

// corrispondenza tra gli identificativi dei MUX sul PCB (MUX1, MUX2,..., MUX7) e gli identificativi nel codice
static constexpr int MUX1 = 0;
static constexpr int MUX2 = 1;
static constexpr int MUX3 = 2;
static constexpr int MUX4 = 3;
static constexpr int MUX5 = 4;
static constexpr int MUX6 = 5;
static constexpr int MUX7 = 6;

// pin del T41 dedicati alle uscite SIG dei Mux
#ifdef PCB_2022
static constexpr uint8_t MUX_pin[7] = {40, 32, 25, 28, 31, 39, 36};
#else
static constexpr uint8_t MUX_pin[7] = {25, 40, 36, 9, 39, 28, 32};
#endif

static constexpr int READ_TIME = 50;
static constexpr int ENC_STOP = 50; // ATTENZIONE: VALORE DA NON DIMINUIRE AL DI SOTTO DI 30!! milliseconds between consecuive reads
static constexpr int ENC_NIP = 100;
static constexpr int STOP_PUSHBUTTONS = 500; // milliseconds between consecuive read of the same pushbutton

// MUX
void Setup_Mux_Pins(void);

// Scrittura MUX
void Write_MUX_address(int address);

// Encoders
struct EN_struct
{
    uint8_t state;
    uint8_t DT_MUX_pin;
    uint8_t CLK_MUX_pin;
    uint8_t address;
};
extern EN_struct Encoder[26];
extern elapsedMillis ENC_timer;
extern elapsedMillis ENC_nip;
extern elapsedMillis minitimer;

// Lettura encoders
void Setup_encoders(void);
int Encoder_state(uint8_t encoder, bool write_mux);
bool Read_encoder_fast(int encoder);
int Read_encoder_simple(int encoder);

template <class T>
bool Read_encoder(int encoder, T &value, const int highest, const int lowest, int inc)
{
    auto state = 0;
    if (ENC_timer < ENC_STOP)
    {
        return 0;
    }
    else
    {
        state = Encoder_state(encoder, true);
        if (Encoder[encoder].state != 3)
        {
            Encoder[encoder].state = state;
            return 0;
        }

        bool answer = false;
        switch (state)
        {
        case 0:
            break;

        case 1:
            minitimer = 0;
            while (minitimer < READ_TIME && state == 1)
                state = Encoder_state(encoder, false);
            if (state == 0 || state == 1)
            {
                if (value < highest)
                {
                    ENC_timer = 0;
                    value += inc;
                    answer = true;
                }
            }
            break;

        case 2:
            minitimer = 0;
            while (minitimer < READ_TIME && state == 2)
                state = Encoder_state(encoder, false);
            if (state == 0 || state == 2)
            {
                if (value > lowest)
                {
                    ENC_timer = 0;
                    value -= inc;
                    answer = true;
                }
            }
            break;

        case 3:
            break;

        default:
            Serial.println("Switch MISSING! 9405");
            break;
        }
        Encoder[encoder].state = state;
        return answer;
    }
}

template <class T>
bool Read_encoder_inverse(int encoder, T &value, const int highest, const int lowest, int inc)
{
    auto state = 0;
    if (ENC_timer < ENC_STOP)
    {
        return false;
    }
    else
    {
        state = Encoder_state(encoder, true);
        if (Encoder[encoder].state != 3)
        {
            Encoder[encoder].state = state;
            return false;
        }

        bool answer = false;
        switch (state)
        {
        case 0:
            break;

        case 1:
            minitimer = 0;
            while (minitimer < READ_TIME && state == 1)
                state = Encoder_state(encoder, false);
            if (state == 0 || state == 1)
            {
                if (value > lowest)
                {
                    ENC_timer = 0;
                    value -= inc;
                    answer = true;
                }
            }
            break;
        case 2:
            minitimer = 0;
            while (minitimer < READ_TIME && state == 2)
                state = Encoder_state(encoder, false);
            if (state == 0 || state == 2)
            {
                if (value < highest)
                {
                    ENC_timer = 0;
                    value += inc;
                    answer = true;
                }
            }
            break;

        case 3:
            break;

        default:
            Serial.println("Switch MISSING! 9467");
            break;
        }
        Encoder[encoder].state = state;
        return answer;
    }
}


// Pushbuttons
struct PB_struct
{
    bool state;
    uint8_t P_MUX_pin;
    uint8_t address;
    unsigned long timer;
};
extern PB_struct PB[36];
extern int PB_number;
extern unsigned long Timer_pushbutton;
extern elapsedMillis PB_timer;

// Lettura pushbuttons
void Setup_pushbuttons(void);
bool Read_pushbutton_fast(int PB_id);
bool Read_pushbutton(int PB_id);
int Read_pushbutton_UP(int PB_id);