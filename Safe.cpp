
// ****************************************************************************
/// \file      Safe.cpp
///
/// \brief     Safe
///
/// \details   Safe Class to control the lock and enocder
///
/// \author    Christoph Capiaghi
///
/// \version   0.1
///
/// \date      20220721
/// 
/// \copyright Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
/// http://www.apache.org/licenses/LICENSE-2.0
///
/// \pre       
///
/// \bug       
///
/// \warning   
///
/// \todo     
///

#include <arduino.h>
#include "Safe.hpp"
#include "config.hpp"

Safe::Safe() :  m_errorCode(RC_OK),
                m_ha40p(),
                m_lock(),
                m_lastDebounceTime(0),
                codeNumber(0),
                countDirection(CW),
                m_lastAngleDeg(0.0)
{

}

// ----------------------------------------------------------------------------
/// \brief     Initialize Safe
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t Safe::initialize()
{
	m_errorCode = m_ha40p.initialize();
	if(m_errorCode != RC_OK) return m_errorCode;

	m_errorCode = m_lock.initialize();
	if(m_errorCode != RC_OK) return m_errorCode;

  m_code[0] = FIRST_CODE_ELEMENT;
  m_code[1] = SECOND_CODE_ELEMENT;
  m_code[2] = THIRD_CODE_ELEMENT;
  codeNumber = 0;
  countDirection = CW;
  m_lastAngleDeg = 0.0;

  return m_errorCode;
}

// ----------------------------------------------------------------------------
/// \brief     Check safe code
/// \detail    Checks the code for the safe. The code must be entered clockwise, hold, counter clockwise, hold, ...
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Safe::checkCode(Adafruit_Protomatter *matrix)
{
  // Read out encoder
	m_errorCode = m_ha40p.getAngleDeg(m_angleDeg);

  // Check direction
  if ( round(m_angleDeg) > round(m_lastAngleDeg) ) // Get nearest integer (close enough for count direction :-))
  {
    countDirection = CW;
  }
  else
  {
    countDirection = CCW;
  }

  // Check rotation. If roation is 
  // First: CW, second: CCW, third: CW, ...
  if (codeNumber % 2 == 0)
  {
    if( countDirection != CW)
    {
      #ifdef DEBUG
        Serial.print(F("Wrong roatation direction: Expected CW, get CCW. Code Number: "));
        Serial.println(codeNumber);
        Serial.print(F("Angle in Degree: "));
        Serial.println(m_angleDeg);
        Serial.print(F("Last Angle in Degree: "));
        Serial.println(m_lastAngleDeg);
      #endif
        codeNumber = 0;
        return INVALID_CODE;
     }
  }
  else
  {
      if( countDirection != CCW)
      {
        #ifdef DEBUG
          Serial.print(F("Wrong roatation direction: Expected CCW, get CW. Code Number: "));
          Serial.println(codeNumber);
          Serial.print(F("Angle in Degree: "));
          Serial.println(m_angleDeg);
          Serial.print(F("Last Angle in Degree: "));
          Serial.println(m_lastAngleDeg);
        #endif
        codeNumber = 0;
        return INVALID_CODE;
      }
  }

  // Check if the value changed whitin an hysteresys
  if ( (m_angleDeg < (m_lastAngleDeg - ANGLE_HYSTERESYS_DEG) ) || (m_angleDeg > (m_lastAngleDeg + ANGLE_HYSTERESYS_DEG) ))
  {
    // reset the debouncing timer
    m_lastDebounceTime = millis();
  }

  // Check if the value was steady
  if ((millis() - m_lastDebounceTime) > DEBOUNCE_DELAY_ENCODER)
  {
    // Reading was stable for a while
    if ( (m_angleDeg < (m_code[codeNumber] - ANGLE_HYSTERESYS_DEG) ) || ( m_angleDeg > (m_code[codeNumber] + ANGLE_HYSTERESYS_DEG) ))
    {
      #ifdef DEBUG
        Serial.print(F("Code Element was correct. Code Element:"));
      #endif
      // Digit was correct
      if ( codeNumber >= NUMBER_OF_CODE_ELEMENTS)
      {
        codeNumber = 0;
        openSafe(matrix);
        //m_lock.openLock( SAFE_OPEN_TIME_MS );
        return CORRECT_CODE;
      }
      codeNumber++;
    }
  }

  m_lastAngleDeg = m_angleDeg;
  return m_errorCode;
}

uint8_t Safe::openSafe(Adafruit_Protomatter *matrix )
{
  matrix->println("Safe should be open :-)");
  m_lock.openLock( SAFE_OPEN_TIME_MS );

  return RC_OK;
}

uint8_t Safe::setNewCode()
{
  return RC_OK;
}
