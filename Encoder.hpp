#pragma once

#include <stdint.h>

class Encoder
{
public:

    Encoder();
    uint8_t initialize();
    uint8_t getAngleDeg(float);
    uint8_t getAngleGon(float);

private:

    static const uint8_t m_triggerDelayMs = 1;  // Delay between high and low time of trigger signal

    uint32_t m_rawAngle;
    
    uint8_t getAngle(uint32_t rawAngle);
    uint8_t trigger( );
};
