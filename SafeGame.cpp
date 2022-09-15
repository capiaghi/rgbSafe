
// ****************************************************************************
/// \file      SafeGame.cpp
///
/// \brief     SafeGame
///
/// \details   SafeGame Class to control the lock and enocder
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
#include "SafeGame.hpp"
#include "config.hpp"

// ----------------------------------------------------------------------------
/// \brief     Default constructor SafeGame
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
SafeGame::SafeGame() : m_errorCode(RC_OK),
                       m_offsetDeg(0.0),
                       m_currentDigit(1)
{

}

// ----------------------------------------------------------------------------
/// \brief     Initialize SafeGame
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t SafeGame::initialize(Safe* safe)
{
  // State Machine
  stm_entryFlag     = TRUE;
  stm_exitFlag      = FALSE;
  stm_newState      = STM_STATE_SAFE_INIT;

  m_safe            = safe;

  // Get Offset
  m_errorCode       = m_safe->getAngleDeg(m_angleDeg);
  m_offsetDeg       = m_angleDeg;

  // Init with not yet defined
  for (uint8_t digit; digit < NUMBER_OF_CODE_ELEMENTS; digit++)
  {
      m_currentCode[digit] = UNDEFINED_CODE_ELEMENT;
  }

  m_correctCode[0] = FIRST_CODE_ELEMENT;
  m_correctCode[1] = SECOND_CODE_ELEMENT;
  m_correctCode[2] = THIRD_CODE_ELEMENT;
  m_correctCode[3] = FOURTH_CODE_ELEMENT;
  
  m_currentDigit = 0;
  m_lastCodeElement = 0;
  m_sign = -1;
  return m_errorCode;
}

void SafeGame::reset()
{
  m_safe->resetDisplay();
  stm_actState    = STM_STATE_SAFE_INIT;
  stm_entryFlag   = TRUE;
  stm_exitFlag    = FALSE;
}


// ----------------------------------------------------------------------------
/// \brief     Check safe code
/// \detail    Checks the code for the safe. The code must be entered clockwise, hold, counter clockwise, hold, ...
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t SafeGame::run()
{
  switch (stm_actState)
  {
  // ----------------------------------------------------------------------------
  // STM_STATE_SAFE_INIT
  // ----------------------------------------------------------------------------
  case STM_STATE_SAFE_INIT:
    // Entry action
    if (stm_entryFlag == TRUE)
    {
#ifdef DEBUG
      Serial.println(F("Entered STM_STATE_SAFE_INIT"));
#endif
      stm_newState  = STM_STATE_SAFE_FIRST_DIGIT;
      stm_entryFlag = FALSE;
      stm_exitFlag  = TRUE;
    }
    
    // Init with not yet defined
    for (uint8_t digit; digit < NUMBER_OF_CODE_ELEMENTS; digit++)
    {
        m_currentCode[digit] = UNDEFINED_CODE_ELEMENT;
    }
    m_sign = (-1);
    m_safe->setNullPosition(); // Set current offset to zero position
    m_safe->displayCode(m_currentCode); /// Show XXXX



#ifdef DEBUG
    Serial.println("Offset");
    Serial.println(m_offsetDeg);
#endif

    // Exit
    if (stm_exitFlag == TRUE)
    {
      //clearScreen();
      stm_exitFlag  = FALSE;
      stm_actState  = stm_newState;
      stm_entryFlag = TRUE;
    }
    break;

  // ----------------------------------------------------------------------------
  // STM_STATE_SAFE_FIRST_DIGIT
  // ----------------------------------------------------------------------------
  case STM_STATE_SAFE_FIRST_DIGIT:

    if (stm_entryFlag == TRUE)
    {
#ifdef DEBUG
      Serial.println(F("Entered STM_STATE_SAFE_FIRST_DIGIT"));
#endif
      stm_entryFlag = FALSE;
      stm_exitFlag = FALSE;
      m_currentCodeElement = 0;
      m_lastCodeElement = 0;
    }

    m_errorCode = m_safe->getAngleDeg(m_angleDeg); // Get position
    
    if ( m_sign == -1)
    {
      m_angleDeg = 360.0 - m_angleDeg;
    }

    m_code_found = false;
    for (uint8_t i = 0; i < NUMBER_OF_CODE_DISK_ELEMENTS && !m_code_found; i++)
    {
      m_currentCodeElementAngle = STEP_SIZE*i;
      
      if (m_angleDeg < (m_currentCodeElementAngle + TOLERANCE) && m_angleDeg > (m_currentCodeElementAngle - TOLERANCE))
      {
        m_currentCodeElement = i;
        m_code_found = true;
      }
    }
    if (stm_entryFlag == TRUE)
    {
        m_lastCodeElement = m_currentCodeElement;
    }

    

#ifdef DEBUG
      Serial.println(F("m_currentCodeElement"));
      Serial.println(m_currentCodeElement);
      Serial.println(F("m_lastCodeElement"));
      Serial.println(m_lastCodeElement);
#endif

    if (m_currentCodeElement > m_lastCodeElement || (m_currentCodeElement == 0 && (m_lastCodeElement == (NUMBER_OF_CODE_DISK_ELEMENTS - 1)))) // Handle also wrap around
    {
      m_lastCodeElement             = m_currentCodeElement;
      m_currentCode[m_currentDigit] = m_currentCodeElement;
      m_safe->displayCode(m_currentCode);
    } 
    else if (m_currentCodeElement < m_lastCodeElement)
    {
      m_lastCodeElement             = m_currentCodeElement;
      if (m_currentDigit >= NUMBER_OF_CODE_ELEMENTS - 1)
      {
        stm_newState  = STM_STATE_SAFE_CHECK_CODE;
        stm_entryFlag = FALSE;
        stm_exitFlag  = TRUE;
        m_currentDigit = 0;
      }
      else
      {
        m_sign *= (-1); // Change sign
        m_safe->setNullPosition();
        m_lastCodeElement = 0;
        m_currentCodeElement = 0;
        m_currentDigit++;
      }
    }

    // Exit
    if (stm_exitFlag == TRUE)
    {
      stm_exitFlag = FALSE;
      stm_actState = stm_newState;
      stm_entryFlag = TRUE;
    }

    break;

  // ----------------------------------------------------------------------------
  // STM_STATE_SAFE_CHECK_CODE
  // ----------------------------------------------------------------------------
  case STM_STATE_SAFE_CHECK_CODE:

    if (stm_entryFlag == TRUE)
    {
#ifdef DEBUG
      Serial.println(F("Entered STM_STATE_SAFE_CHECK_CODE"));
#endif
      stm_entryFlag = FALSE;
      stm_exitFlag = FALSE;
    }

    // Check Code
    m_codeValid = true;
    for (uint8_t digit; digit < NUMBER_OF_CODE_ELEMENTS; digit++)
    {
        if (m_currentCode[digit] != m_correctCode[digit])
        {
          m_codeValid = false;
          break;
        }
    }

    if (m_codeValid)
    {
      stm_newState  = STM_STATE_SAFE_CORRECT_CODE;
      stm_entryFlag = FALSE;
      stm_exitFlag  = TRUE;
    }
    else
    {
      stm_newState  = STM_STATE_SAFE_WRONG_CODE;
      stm_entryFlag = FALSE;
      stm_exitFlag  = TRUE;
    }

    // Exit
    if (stm_exitFlag == TRUE)
    {
      stm_exitFlag = FALSE;
      stm_actState = stm_newState;
      stm_entryFlag = TRUE;
    }
    break;



  // ----------------------------------------------------------------------------
  // STM_STATE_SAFE_CORRECT_CODE
  // ----------------------------------------------------------------------------
  case STM_STATE_SAFE_CORRECT_CODE:

    if (stm_entryFlag == TRUE)
    {
#ifdef DEBUG
      Serial.println(F("Entered STM_STATE_SAFE_CORRECT_CODE"));
#endif
      stm_entryFlag = FALSE;
      stm_exitFlag = FALSE;
    }

    m_safe->displayGreenSmiley();
    m_safe->openSafe();
    stm_newState = STM_STATE_SAFE_INIT;
    stm_entryFlag = FALSE;
    stm_exitFlag = TRUE;


    // Exit
    if (stm_exitFlag == TRUE)
    {
      stm_exitFlag = FALSE;
      stm_actState = stm_newState;
      stm_entryFlag = TRUE;
    }
    break;

  // ----------------------------------------------------------------------------
  // STM_STATE_SAFE_WRONG_CODE
  // ----------------------------------------------------------------------------
  case STM_STATE_SAFE_WRONG_CODE:

    if (stm_entryFlag == TRUE)
    {
#ifdef DEBUG
      Serial.println(F("Entered STM_STATE_SAFE_WRONG_CODE"));
#endif
      stm_entryFlag = FALSE;
      stm_exitFlag = FALSE;
    }

    m_safe->displayRedSmiley();
    delay(2000);
    stm_newState = STM_STATE_SAFE_INIT;
    stm_entryFlag = FALSE;
    stm_exitFlag = TRUE;
    
    // Exit
    if (stm_exitFlag == TRUE)
    {
      stm_exitFlag = FALSE;
      stm_actState = stm_newState;
      stm_entryFlag = TRUE;
    }
    break;

  //==============================================================================
  // DEFAULT
  //==============================================================================
  default:
#ifdef DEBUG
    Serial.println(F("Entered STM_STATE_SAFE_DEFAULT"));
#endif
    break;

  }
  return m_errorCode;
}
