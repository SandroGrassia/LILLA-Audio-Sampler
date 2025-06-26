/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "UserInterface.h"

// Encoders
EN_struct Encoder[26];
elapsedMillis ENC_timer = 0;
elapsedMillis ENC_nip = 0;
elapsedMillis minitimer = 0;

void Setup_encoders(void)
{
    for (auto i = 0; i < 26; ++i)
    {
        Encoder[i].state = 0;
    }

    Encoder[0].address = 10;
    Encoder[1].address = 8;
    Encoder[2].address = 6;
    Encoder[3].address = 10;
    Encoder[4].address = 8;
    Encoder[5].address = 6;
    Encoder[6].address = 10;
    Encoder[7].address = 8;
    Encoder[8].address = 11;
    Encoder[9].address = 0;
    Encoder[10].address = 2;
    Encoder[11].address = 11;
    Encoder[12].address = 0;
    Encoder[13].address = 2;
    Encoder[14].address = 11;
    Encoder[15].address = 0;
    Encoder[16].address = 12;
    Encoder[17].address = 15;
    Encoder[18].address = 3;
    Encoder[19].address = 12;
    Encoder[20].address = 15;
    Encoder[21].address = 3;
    Encoder[22].address = 12;
    Encoder[23].address = 15;
    Encoder[24].address = 3;
    Encoder[25].address = 2;

#if defined PCB_2022 || defined PCB_2023
    Encoder[0].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[1].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[2].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[3].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[4].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[5].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[6].DT_MUX_pin = MUX_pin[MUX3];
    Encoder[7].DT_MUX_pin = MUX_pin[MUX3];
    Encoder[8].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[9].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[10].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[11].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[12].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[13].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[14].DT_MUX_pin = MUX_pin[MUX3];
    Encoder[15].DT_MUX_pin = MUX_pin[MUX3];
    Encoder[16].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[17].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[18].DT_MUX_pin = MUX_pin[MUX1];
    Encoder[19].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[20].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[21].DT_MUX_pin = MUX_pin[MUX2];
    Encoder[22].DT_MUX_pin = MUX_pin[MUX3];
    Encoder[23].DT_MUX_pin = MUX_pin[MUX3];
    Encoder[24].DT_MUX_pin = MUX_pin[MUX3];
    Encoder[25].DT_MUX_pin = MUX_pin[MUX3];

    Encoder[0].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[1].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[2].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[3].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[4].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[5].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[6].CLK_MUX_pin = MUX_pin[MUX4];
    Encoder[7].CLK_MUX_pin = MUX_pin[MUX4];
    Encoder[8].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[9].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[10].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[11].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[12].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[13].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[14].CLK_MUX_pin = MUX_pin[MUX4];
    Encoder[15].CLK_MUX_pin = MUX_pin[MUX4];
    Encoder[16].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[17].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[18].CLK_MUX_pin = MUX_pin[MUX6];
    Encoder[19].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[20].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[21].CLK_MUX_pin = MUX_pin[MUX5];
    Encoder[22].CLK_MUX_pin = MUX_pin[MUX4];
    Encoder[23].CLK_MUX_pin = MUX_pin[MUX4];
    Encoder[24].CLK_MUX_pin = MUX_pin[MUX4];
    Encoder[25].CLK_MUX_pin = MUX_pin[MUX4];

#else // if defined PCB_2024
    Encoder[0].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[1].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[2].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[3].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[4].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[5].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[6].CLK_MUX_pin = MUX_pin[MUX3];
    Encoder[7].CLK_MUX_pin = MUX_pin[MUX3];
    Encoder[8].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[9].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[10].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[11].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[12].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[13].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[14].CLK_MUX_pin = MUX_pin[MUX3];
    Encoder[15].CLK_MUX_pin = MUX_pin[MUX3];
    Encoder[16].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[17].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[18].CLK_MUX_pin = MUX_pin[MUX1];
    Encoder[19].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[20].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[21].CLK_MUX_pin = MUX_pin[MUX2];
    Encoder[22].CLK_MUX_pin = MUX_pin[MUX3];
    Encoder[23].CLK_MUX_pin = MUX_pin[MUX3];
    Encoder[24].CLK_MUX_pin = MUX_pin[MUX3];
    Encoder[25].CLK_MUX_pin = MUX_pin[MUX3];

    Encoder[0].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[1].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[2].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[3].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[4].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[5].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[6].DT_MUX_pin = MUX_pin[MUX4];
    Encoder[7].DT_MUX_pin = MUX_pin[MUX4];
    Encoder[8].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[9].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[10].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[11].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[12].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[13].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[14].DT_MUX_pin = MUX_pin[MUX4];
    Encoder[15].DT_MUX_pin = MUX_pin[MUX4];
    Encoder[16].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[17].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[18].DT_MUX_pin = MUX_pin[MUX6];
    Encoder[19].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[20].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[21].DT_MUX_pin = MUX_pin[MUX5];
    Encoder[22].DT_MUX_pin = MUX_pin[MUX4];
    Encoder[23].DT_MUX_pin = MUX_pin[MUX4];
    Encoder[24].DT_MUX_pin = MUX_pin[MUX4];
    Encoder[25].DT_MUX_pin = MUX_pin[MUX4];
#endif
}

// Pushbuttons
PB_struct PB[36];
int PB_number = 0;
unsigned long Timer_pushbutton = 0;
elapsedMillis PB_timer = 0;

void Setup_pushbuttons(void)
{
    for (auto i = 0; i < 36; ++i)
    {
        PB[i].state = false;
        PB[i].timer = 0;
    }

    PB[0].address = 9;
    PB[1].address = 7;
    PB[2].address = 5;
    PB[3].address = 9;
    PB[4].address = 7;
    PB[5].address = 5;
    PB[6].address = 9;
    PB[7].address = 7;
    PB[8].address = 14;
    PB[9].address = 1;
    PB[10].address = 4;
    PB[11].address = 14;
    PB[12].address = 1;
    PB[13].address = 4;
    PB[14].address = 14;
    PB[15].address = 1;
    PB[16].address = 13;
    PB[17].address = 1;
    PB[18].address = 4;
    PB[19].address = 13;
    PB[20].address = 1;
    PB[21].address = 4;
    PB[22].address = 13;
    PB[23].address = 1;
    PB[24].address = 5;
    PB[25].address = 4;
    PB[26].address = 13;
    PB[27].address = 14;
    PB[28].address = 15;
    PB[29].address = 0;
    PB[30].address = 1;
    PB[31].address = 2;
    PB[32].address = 3;
    PB[33].address = 4;
    PB[34].address = 5;
    PB[35].address = 6;

    PB[0].P_MUX_pin = MUX_pin[MUX1];
    PB[1].P_MUX_pin = MUX_pin[MUX1];
    PB[2].P_MUX_pin = MUX_pin[MUX1];
    PB[3].P_MUX_pin = MUX_pin[MUX2];
    PB[4].P_MUX_pin = MUX_pin[MUX2];
    PB[5].P_MUX_pin = MUX_pin[MUX2];
    PB[6].P_MUX_pin = MUX_pin[MUX3];
    PB[7].P_MUX_pin = MUX_pin[MUX3];
    PB[8].P_MUX_pin = MUX_pin[MUX1];
    PB[9].P_MUX_pin = MUX_pin[MUX1];
    PB[10].P_MUX_pin = MUX_pin[MUX1];
    PB[11].P_MUX_pin = MUX_pin[MUX2];
    PB[12].P_MUX_pin = MUX_pin[MUX2];
    PB[13].P_MUX_pin = MUX_pin[MUX2];
    PB[14].P_MUX_pin = MUX_pin[MUX3];
    PB[15].P_MUX_pin = MUX_pin[MUX3];
    PB[16].P_MUX_pin = MUX_pin[MUX1];
    PB[17].P_MUX_pin = MUX_pin[MUX6];
    PB[18].P_MUX_pin = MUX_pin[MUX6];
    PB[19].P_MUX_pin = MUX_pin[MUX2];
    PB[20].P_MUX_pin = MUX_pin[MUX5];
    PB[21].P_MUX_pin = MUX_pin[MUX5];
    PB[22].P_MUX_pin = MUX_pin[MUX3];
    PB[23].P_MUX_pin = MUX_pin[MUX4];
    PB[24].P_MUX_pin = MUX_pin[MUX4];
    PB[25].P_MUX_pin = MUX_pin[MUX4];
    PB[26].P_MUX_pin = MUX_pin[MUX7];
    PB[27].P_MUX_pin = MUX_pin[MUX7];
    PB[28].P_MUX_pin = MUX_pin[MUX7];
    PB[29].P_MUX_pin = MUX_pin[MUX7];
    PB[30].P_MUX_pin = MUX_pin[MUX7];
    PB[31].P_MUX_pin = MUX_pin[MUX7];
    PB[32].P_MUX_pin = MUX_pin[MUX7];
    PB[33].P_MUX_pin = MUX_pin[MUX7];
    PB[34].P_MUX_pin = MUX_pin[MUX7];
    PB[35].P_MUX_pin = MUX_pin[MUX7];
}

// MUX
void Setup_Mux_Pins(void)
{

    // Pin collegati all'Address bus dei Mux
    pinMode(MUX_S0_pin, OUTPUT);
    pinMode(MUX_S1_pin, OUTPUT);
    pinMode(MUX_S2_pin, OUTPUT);
    pinMode(MUX_S3_pin, OUTPUT);

    // Pin collegati alle uscite SIG dei Mux
    for (auto i = 0; i < 7; ++i)
    {
        pinMode(MUX_pin[i], INPUT_PULLUP);
    }
}

// lettura
void Write_MUX_address(int address)
{
    digitalWriteFast(MUX_S0_pin, bitRead(address, 0));
    digitalWriteFast(MUX_S1_pin, bitRead(address, 1));
    digitalWriteFast(MUX_S2_pin, bitRead(address, 2));
    digitalWriteFast(MUX_S3_pin, bitRead(address, 3));
}

int Encoder_state(uint8_t encoder, bool write_mux)
{
    if (write_mux)
    {
        Write_MUX_address(Encoder[encoder].address);
        delayMicroseconds(PAUSE_MUX);
    }
    return 2 * digitalRead(Encoder[encoder].DT_MUX_pin) + digitalRead(Encoder[encoder].CLK_MUX_pin);
}

bool Read_pushbutton(int PB_id)
{
    if (PB[PB_id].state && ((millis() - PB[PB_id].timer) > PB_DOWN_PAUSE)) // era down (PB_DOWN_PAUSE e' l'isteresi dopo il push)
    {
        Write_MUX_address(PB[PB_id].address);
        delayMicroseconds(PAUSE_MUX);
        if (digitalRead(PB[PB_id].P_MUX_pin) == HIGH) // HIGH e' up
        {
            PB[PB_id].state = false;
            PB[PB_id].timer = millis();
            return false;
        }
        else // e' ancora down
        {
            return false;
        }
    }

    else if (!PB[PB_id].state && ((millis() - PB[PB_id].timer) > PB_UP_PAUSE)) // era up (PB_UP_PAUSE e' l'isteresi dopo il rilascio)
    {
        Write_MUX_address(PB[PB_id].address);
        delayMicroseconds(PAUSE_MUX);
        if (digitalRead(PB[PB_id].P_MUX_pin) == LOW) // LOW e' down
        {
            PB[PB_id].state = true;
            PB[PB_id].timer = millis();
            return true;
        }
        else // e' ancora up
        {
            return false;
        }
    }
    else // timer ancora non scaduti
    {
        return false;
    }
}

bool Read_pushbutton_fast(int PB_id)
{
    Write_MUX_address(PB[PB_id].address);
    delayMicroseconds(PAUSE_MUX);
    if (digitalRead(PB[PB_id].P_MUX_pin) == LOW)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int Read_pushbutton_UP(int PB_id)
{
    if (PB[PB_id].state && ((millis() - PB[PB_id].timer) > PB_DOWN_PAUSE)) // era down (PB_DOWN_PAUSE e' l'isteresi dopo il push)
    {
        Write_MUX_address(PB[PB_id].address);
        delayMicroseconds(PAUSE_MUX);
        if (digitalRead(PB[PB_id].P_MUX_pin) == HIGH) // HIGH e' up
        {
            PB[PB_id].state = false;
            PB[PB_id].timer = millis();
            return PB_timer; // rilascio
        }
        else // e' ancora down
        {
            return 0;
        }
    }

    else if (!PB[PB_id].state && ((millis() - PB[PB_id].timer) > PB_UP_PAUSE)) // era up (PB_UP_PAUSE e' l'isteresi dopo il rilascio)
    {
        Write_MUX_address(PB[PB_id].address);
        delayMicroseconds(PAUSE_MUX);
        if (digitalRead(PB[PB_id].P_MUX_pin) == LOW) // LOW e' down
        {
            PB_timer = 0;
            PB[PB_id].state = true;
            PB[PB_id].timer = millis();
            return 0; // premuto
        }
        else // e' ancora up
        {
            return 0;
        }
    }
    else // timer ancora non scaduti
    {
        return 0;
    }
}

bool Read_encoder_fast(int encoder)
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
            {
                state = Encoder_state(encoder, false);
            }
            if (state == 0 || state == 1)
            {
                ENC_timer = 0;
                answer = true;
            }
            break;

        case 2:
            minitimer = 0;
            while (minitimer < READ_TIME && state == 2)
            {
                state = Encoder_state(encoder, false);
            }
            if (state == 0 || state == 2)
            {
                ENC_timer = 0;
                answer = true;
            }
            break;

        case 3:
            break;

        default:
            Serial.println("Switch MISSING! 9286");
            break;
        }

        Encoder[encoder].state = state;
        return answer;
    }
}

int Read_encoder_simple(int encoder)
{
    auto state = 0;
    if (ENC_timer < ENC_STOP)
    {
        return 0;
    }
    else
    {
        state = Encoder_state(encoder, true);
        auto result = 0;
        if (Encoder[encoder].state != 3)
        {
            Encoder[encoder].state = state;
            return 0;
        }

        switch (state)
        {
        case 0:
            break;

        case 1:
            minitimer = 0;
            while (minitimer < READ_TIME && state == 1)
            {
                state = Encoder_state(encoder, false);
            }
            if (state == 0 || state == 1)
            {
                ENC_timer = 0;
                ENC_nip = 0;
                result = 1;
            }
            break;

        case 2:
            minitimer = 0;
            while (minitimer < READ_TIME && state == 2)
            {
                state = Encoder_state(encoder, false);
            }
            if (state == 0 || state == 2)
            {
                ENC_timer = 0;
                ENC_nip = 0;
                result = -1;
            }
            break;

        case 3:
            break;

        default:
            Serial.println("Switch MISSING! 9342");
            break;
        }
        
        Encoder[encoder].state = state;
        return result;
    }
}