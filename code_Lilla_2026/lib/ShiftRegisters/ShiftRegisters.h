/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

/*
Lilla PCB2025_R2
This model includes 6 Shift Registers (Shifters_manager) MCP23S17 (SPI communication)
- each Shifter address matches with its id (0 to 6)
- Shiters' channels are configured with INPUT_PULLUP
- each Encoder (DT,CLK,PB) is connected to a single Shifter
*/

/*
At startup
- main.cpp creates the Encoders_manager and Pushbuttons_manager objects
- main.cpp creates the ShiftRegisters object (Shifters_manager) sending the &Encoders pointer
- Shifters_manager creates the Shifter[SHIFTERS] ojbects
- main.cpp sends monitored_encoders and monitored_pushbuttons to Shifters_manager

For each loop():
- main.cpp calls Update()
- Shifters_manager calls the monitored_shifters and reads the status of the DT/CLK and "PB" pins of the monitored_encoders and monitored_pushbuttons
- Shifters_manager send the filterd data to EncordersObj and Pushbuttons_manager
- main.cpp calls  EncordersObj and Pushbuttons_manager ...

When Lilla_state changes, also the set of monitored encoders changes:
- main.cpp sends a new monitored_encoders and monitored_pushbuttons to Shifters_manager
*/

#pragma once

#include <Arduino.h>
#include "config.h"
#include <Adafruit_MCP23X17.h> // Shifters
#include "UserInterface.h"
#include "Encoders.h"
#include "Pushbuttons.h"

class ShiftRegisters
{
private:

    static constexpr uint8_t SHIFTER_ADDRESS[SHIFTERS] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25};

    Adafruit_MCP23X17 Shifter[SHIFTERS]; // Physical shift registers
    Encoders &Encoders_manager;
    Pushbuttons &Pushbuttons_manager;

    static constexpr int STATES = 4;
    enum Type
    {
        Last,
        Old,
        Changed,
        Filtered
    };
    uint16_t shifter_channel_value[STATES][SHIFTERS]; // Used for caching Shifter[shifter_id].readGPIOAB(); 0: old read; 1: last read; 2: filtered (old EXOR last)



    uint32_t monitored_encoders;    // Each bit from 0 to 25 corresponds to an encoder: 0b 000000XX XXXXXXXX XXXXXXXX XXXXXXXX  -  X=1: encoder monitored, X=0: encoder excluded
    uint64_t monitored_pushbuttons; // Each bit from 0 to 35 corresponds to a pushbutton: 0b 00000000 00000000 00000000  0000XXXX   XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX  - X=1: pushbutton monitored, X=0: pushbutton excluded
    uint8_t monitored_shifters;     // 0b 00 XXXXXX  -  X=1: shifter monitored, X=0: shifter excluded
    uint16_t monitored_channels[SHIFTERS];

    // Setup physical shifter
    void Start_SPI_for_shifters(void);
    void Setup_physical_channels(void);

    void Reset_shifters_channels(void); // all input declared 1 (open)
    void Reset_monitored_channels(void);
    void Read_channels(const int &id);
    void Filter_channels_changed_values(const int &id);
    void Set_monitored_encoders(const uint32_t &data);
    void Set_monitored_pushbuttons(const uint64_t &data);

public:
    ShiftRegisters(Encoders &EncsObj, Pushbuttons &PbsObj) : Encoders_manager(EncsObj), Pushbuttons_manager(PbsObj)
    {
        Start_SPI_for_shifters();
        Setup_physical_channels();
        Reset_shifters_channels();
    }

    void Set_monitored_encoders_pushbuttons(const uint32_t &enc, const uint64_t &pb);
    void Update(void);
};