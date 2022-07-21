#pragma once

#include <stdint.h>
#include <Adafruit_Protomatter.h>
#include "Encoder.hpp"
#include "Lock.hpp"


class Safe
{
public:
    Safe();
    uint8_t initialize();
    uint8_t checkCode(Adafruit_Protomatter *matrix);
    uint8_t setNewCode();

private:
    static constexpr float ANGLE_HYSTERESYS_DEG = 1.0f;        // Hysteresys of angle in degree
    static const uint32_t DEBOUNCE_DELAY_ENCODER = 1000;  // This time defines, how long the correct code must be present
    static const uint32_t SAFE_OPEN_TIME_MS = 5000;
      
    static const uint8_t CW = 0;    // Clock wise rotation
    static const uint8_t CCW = 1;   // Counter clock wise rotation

    static constexpr float  FIRST_CODE_ELEMENT = 90.0f;
    static constexpr float  SECOND_CODE_ELEMENT = 180.0f;
    static constexpr float  THIRD_CODE_ELEMENT = 270.0f;
    static const uint8_t    NUMBER_OF_CODE_ELEMENTS = 3;
    
    uint8_t m_errorCode;
    float m_angleDeg;
    float m_lastAngleDeg;
    uint32_t m_lastDebounceTime;
    float m_code[NUMBER_OF_CODE_ELEMENTS];
    uint8_t codeNumber;
    uint8_t countDirection;

    // Angle Encoder HA40+ -----------------------------------------------------
    Encoder m_ha40p;

    // Lock-style Solenoid -----------------------------------------------------
    Lock m_lock;

    uint8_t openSafe(Adafruit_Protomatter *matrix );
 
};
