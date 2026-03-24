/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "ShiftRegisters.h"

void ShiftRegisters::Start_SPI_for_shifters(void)
{
    for (auto i = 0; i < SHIFTERS; ++i)
    {
        Shifter[i].begin_SPI(SPI1_SHIFTERS_CS, SPI1_SCLK, SPI1_MISO, SPI1_MOSI, SHIFTER_ADDRESS[i]); // (int8_t cs_pin, int8_t sck_pin, int8_t miso_pin, int8_t mosi_pin, uint8_t _hw_addr = 0x00);

        // Needed??
        Shifter[i].enableAddrPins();
        Shifter[i].enableAddrPins();
    }
}

void ShiftRegisters::Reset_shifters_channels(void)
{
    for (auto i = 0; i < SHIFTERS; ++i)
    {
        shifter_channel_value[Old][i] = 0b1111111111111111;
    }
}

void ShiftRegisters::Reset_monitored_channels(void)
{
    monitored_shifters = 0;
    for (auto i = 0; i < SHIFTERS; ++i)
    {
        monitored_channels[i] = 0;
    }
}

void ShiftRegisters::Setup_physical_channels(void)
{
    for (auto i = 0; i < SHIFTERS; ++i)
    {
        // (input) channels pullup (100kohm)
        for (auto j = 0; j < 16; ++j)
        {
            Shifter[i].pinMode(j, INPUT_PULLUP);
        }
    }
}

void ShiftRegisters::Set_monitored_encoders(const uint32_t &data)
{
    monitored_encoders = data;

    Serial.print("monitored_encoders, BIN: ");
    Serial.println(monitored_encoders, BIN);

    for (auto i = 0; i < ENCODERS; ++i)
    {
        if (bitRead(monitored_encoders, i))
        {
            const auto shifter_id = encoder_physical[i].shifter_id;
            bitWrite(monitored_shifters, shifter_id, 1);
            bitWrite(monitored_channels[shifter_id], encoder_physical[i].DT_shifter_channel, 1);
            bitWrite(monitored_channels[shifter_id], encoder_physical[i].CLK_shifter_channel, 1);
        }
    }
}

void ShiftRegisters::Set_monitored_pushbuttons(const uint64_t &data)
{
    monitored_pushbuttons = data;

    Serial.print("monitored_pushbuttons, BIN: ");
    Serial.println(monitored_pushbuttons, BIN);

    for (auto i = 0; i < PUSHBUTTONS; ++i)
    {
        if (bitRead(monitored_pushbuttons, i))
        {
            const auto shifter_id = pushbutton_physical[i].shifter_id;
            bitWrite(monitored_shifters, shifter_id, 1);
            bitWrite(monitored_channels[shifter_id], pushbutton_physical[i].shifter_channel, 1);
        }
    }
}

void ShiftRegisters::Set_monitored_encoders_pushbuttons(const uint32_t &enc, const uint64_t &pb)
{
    Reset_monitored_channels();
    Set_monitored_encoders(enc);
    Set_monitored_pushbuttons(pb);

    if (true)
    {
        Serial.println("Monitored shift register chips channeles");
        Serial.println("b7  b6  b5  b4  b3  b2  b1  b0  a7  a6  a5  a4  a3  a2  a1  a0");
        for (auto i = 0; i < SHIFTERS; ++i)
        {
            for (auto j = (SHIFTER_CHANNELS - 1); j >= 0; --j)
            {
                Serial.print(bitRead(monitored_channels[i], j));
                Serial.print("   ");
            }
            Serial.println();
        }
    }
}

void ShiftRegisters::Read_channels(const int &shifter_id)
{
    shifter_channel_value[Last][shifter_id] = Shifter[shifter_id].readGPIOAB();
}

void ShiftRegisters::Filter_channels_changed_values(const int &shifter_id)
{
    shifter_channel_value[Changed][shifter_id] = shifter_channel_value[Last][shifter_id] ^ shifter_channel_value[Old][shifter_id];

    // di questi filtra quelli relativi ai soli encoder e pushbutton monitorati
    shifter_channel_value[Filtered][shifter_id] = shifter_channel_value[Changed][shifter_id] & monitored_channels[shifter_id];

    shifter_channel_value[Old][shifter_id] = shifter_channel_value[Last][shifter_id];
    // Serial.println(shifter_channel_value[Filter][shifter_id], BIN);
}

void ShiftRegisters::Update(void)
{
    // scan all shift registers
    for (auto shifter_id = 0; shifter_id < SHIFTERS; ++shifter_id)
    {
        // read only the monitored shift registers
        if (bitRead(monitored_shifters, shifter_id))
        {
            Read_channels(shifter_id); // read physical channels
            Filter_channels_changed_values(shifter_id);

            if (false)
            {
                if (shifter_channel_value[Filtered][shifter_id] != 0)
                {
                    Serial.print("shifter_id: ");
                    Serial.println(shifter_id);
                    Serial.print(shifter_channel_value[Old][shifter_id], BIN);
                    Serial.print(" - ");
                    Serial.println(shifter_channel_value[Last][shifter_id], BIN);
                }
            }

            // parse channels
            for (auto channel = 0; channel < SHIFTER_CHANNELS; ++channel)
            {
                if (bitRead(shifter_channel_value[Filtered][shifter_id], channel))
                {
                    const auto encoder = Shifter_channel_to_encoder_pushbutton[shifter_id][channel].encoder_id;

                    // Serial.print("encoder: ");
                    // Serial.println(encoder);

                    const auto pushbutton = Shifter_channel_to_encoder_pushbutton[shifter_id][channel].pushbutton_id;
                    if (encoder > -1)
                    {
                        const int DT_channel = encoder_physical[encoder].DT_shifter_channel;
                        const int CLK_channel = encoder_physical[encoder].CLK_shifter_channel;

                        /*
                        Serial.print("DT_channel/CLK_channel: ");
                        Serial.print(DT_channel);
                        Serial.print(" / ");
                        Serial.println(CLK_channel);

                        Serial.print("DT/CLK: ");
                        Serial.print(bitRead(shifter_channel_value[Last][shifter_id], DT_channel));
                        Serial.print(" / ");
                        Serial.println(bitRead(shifter_channel_value[Last][shifter_id], CLK_channel));
                        */

                        Encoders_manager.Transmit_DT_CLK(encoder, bitRead(shifter_channel_value[Last][shifter_id], DT_channel), bitRead(shifter_channel_value[Last][shifter_id], CLK_channel));
                    }
                    else if (pushbutton > -1)
                    {
                        const auto channel = pushbutton_physical[pushbutton].shifter_channel;

                        /*
                        Serial.print("(pushbutton) channel: ");
                        Serial.println(channel);

                        Serial.print("(pushbutton) channel value: ");
                        Serial.println(bitRead(shifter_channel_value[Last][shifter_id], channel));
                        */

                        Pushbuttons_manager.Transmit_position(pushbutton, bitRead(shifter_channel_value[Last][shifter_id], channel));
                    }
                }
            }
        }
    }
}