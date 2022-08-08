#pragma once

#include <stdint.h>
#include "SerialHandler.hpp"

class Encoder
{
public:

    Encoder();
    uint8_t initialize();
    uint8_t getAngleDeg(float &angleDeg);
    uint8_t getAngleGon(float &angleGon);

private:
    SerialHandler m_serialHandler;
    static const uint8_t m_triggerDelayMs = 1;  // Delay between high and low time of trigger signal
    
    //Romer Protocol RX
    // | Address Field | Length | Command | Angle LSB | Angle | Angle | Angle MSB | CRC
    static const uint8_t ROMER_ADDRESS_FIELD_RX       = 0;
    static const uint8_t ROMER_LENGTH_FIELD_RX        = 1;
    static const uint8_t ROMER_COMMAND_FIELD_RX       = 2;
    static const uint8_t ROMER_ANGLE_INFO_FIELD_RX    = 3;
    static const uint8_t ROMER_ANGLE1_LSB_FIELD_RX    = 4;
    static const uint8_t ROMER_ANGLE2_FIELD_RX        = 5;
    static const uint8_t ROMER_ANGLE3_FIELD_RX        = 6;
    static const uint8_t ROMER_ANGLE4_MSB_FIELD_RX    = 7;
    static const uint8_t ROMER_CRC_FIELD_RX           = 8;

    static const uint8_t ROMER_CMD_B_LENGTH_RX        = 0x06;
    static const uint8_t ROMER_CMD_B_RX               = 0x42;

    //Romer Protocol TX
    // | Address Field | Length | Command | Number of Angles | CRC8 
    static const uint8_t ROMER_ADDRESS_FIELD_TX       = 0;
    static const uint8_t ROMER_LENGTH_FIELD_TX        = 1;
    static const uint8_t ROMER_COMMAND_FIELD_TX       = 2;
    static const uint8_t ROMER_NUMBER_FIELD_TX        = 3;
    static const uint8_t ROMER_CRC_FIELD_TX           = 4;

    static const uint8_t ROMER_CMD_B_LENGTH_TX        = 0x03;
    static const uint8_t ROMER_CMD_B_ADDRESS_TX       = 0xF0; // 0xF0: Master (F) sends to all (0) slaves
    static const uint8_t ROMER_CMD_B_NUMBER_OF_ANGLES = 1; // Number of angles to send:
    static const uint8_t DEFAULT_CRC_VALUE            = 0xFF;
    
    uint32_t m_rawAngle;
    
    uint8_t getAngle(uint32_t &rawAngle);
    uint8_t trigger( );
};
