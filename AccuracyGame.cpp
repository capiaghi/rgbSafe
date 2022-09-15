
// ****************************************************************************
/// \file      AccuracyGame.cpp
///
/// \brief     Game
///
/// \details   Game Class to control the lock and enocder
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
#include "AccuracyGame.hpp"
#include "config.hpp"
#include <Fonts/Picopixel.h> // Large friendly font
#include "FreeMonoBold7pt7b.h" // Create with https://rop.nl/truetype2gfx/
#include "pics/redSmiley_32x32.c"
#include "pics/greenSmiley_32x32.c"


AccuracyGame::AccuracyGame() :  m_errorCode(RC_OK),
                                m_offsetDeg(0.0)
{

}

// ----------------------------------------------------------------------------
/// \brief     Initialize Safe
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t AccuracyGame::initialize(Safe* safe)
{
  // State Machine
  stm_entryFlag = TRUE;
  stm_exitFlag = FALSE;
  stm_newState = STM_STATE_ACCURACY_GAME_INIT;

  m_safe = safe;
  m_safe->setBarGraphResolution( BAR_GRAPH_RESOLUTION_DEG);
	return m_errorCode;
}

void AccuracyGame::reset()
{
  m_safe->resetDisplay();
  stm_actState = STM_STATE_ACCURACY_GAME_INIT;
  stm_entryFlag = TRUE;
  stm_exitFlag = FALSE;
}

// ----------------------------------------------------------------------------
/// \brief     Check safe code
/// \detail    Checks the code for the safe. The code must be entered clockwise, hold, counter clockwise, hold, ...
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t AccuracyGame::run()
{
	switch (stm_actState)
	{

  // ----------------------------------------------------------------------------
  // STM_STATE_ACCURACY_GAME_INIT
  // ----------------------------------------------------------------------------    
	case STM_STATE_ACCURACY_GAME_INIT:
		// Entry action
		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_INIT"));
#endif
			stm_newState = STM_STATE_ACCURACY_GAME_CHECK_VALUE;
			stm_entryFlag = FALSE;
			stm_exitFlag = TRUE;
		}
		// Get Offset
    m_safe->setNullPosition();

#ifdef DEBUG
		Serial.println("Offset");
		Serial.println(m_offsetDeg);
#endif

		// Exit
		if (stm_exitFlag == TRUE)
		{
			//clearScreen();
			stm_exitFlag = FALSE;
			stm_actState = stm_newState;
			stm_entryFlag = TRUE;
		}
		break;

  // ----------------------------------------------------------------------------
  // STM_STATE_ACCURACY_GAME_CHECK_VALUE
  // ----------------------------------------------------------------------------    
	case STM_STATE_ACCURACY_GAME_CHECK_VALUE:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_CHECK_VALUE"));
#endif
			stm_entryFlag = FALSE;
			stm_exitFlag  = FALSE;
		}

    m_targetAngleDeg      = TARGET_ANGLE;
    m_targetAngleDegKids  = TARGET_ANGLE_KIDS;
    
    if (m_angleDeg > 200)
    {
      m_safe->setBarGraphResolution( BAR_GRAPH_RESOLUTION_KIDS_DEG);
      m_safe->getAndDisplayAngles(m_targetAngleDegKids, m_angleDeg);
    }
    else
    {
      m_safe->setBarGraphResolution( BAR_GRAPH_RESOLUTION_DEG);
      m_safe->getAndDisplayAngles(m_targetAngleDeg, m_angleDeg);
    }


		// Within tolerance?
		if ((m_angleDeg > (m_targetAngleDeg - ANGLE_HYSTERESYS_DEG)) && (m_angleDeg < (m_targetAngleDeg + ANGLE_HYSTERESYS_DEG)))
		{
			m_lastDebounceTime = millis(); // Reset timer
			stm_newState = STM_STATE_ACCURACY_GAME_IN_TOLERANCE;
			stm_entryFlag = FALSE;
			stm_exitFlag = TRUE;
		}

    // Within tolerance?
    if ((m_angleDeg > (m_targetAngleDegKids - ANGLE_HYSTERESYS_DEG_KIDS)) && (m_angleDeg < (m_targetAngleDegKids + ANGLE_HYSTERESYS_DEG_KIDS)))
    {
      m_lastDebounceTime = millis(); // Reset timer
      stm_newState = STM_STATE_ACCURACY_GAME_IN_TOLERANCE_KIDS;
      stm_entryFlag = FALSE;
      stm_exitFlag = TRUE;
    }

		// Exit
		if (stm_exitFlag == TRUE)
		{
			//clearScreen();
			stm_exitFlag = FALSE;
			stm_actState = stm_newState;
			stm_entryFlag = TRUE;
		}

		break;

  // ----------------------------------------------------------------------------
  // STM_STATE_ACCURACY_GAME_IN_TOLERANCE
  // ----------------------------------------------------------------------------		
		case STM_STATE_ACCURACY_GAME_IN_TOLERANCE:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_IN_TOLERANCE"));
#endif
			stm_entryFlag = FALSE;
			stm_exitFlag = FALSE;
		}

		m_safe->getAndDisplayAngles(m_targetAngleDeg, m_angleDeg);

		// Within tolerance?
		if ((m_angleDeg < (m_targetAngleDeg - ANGLE_HYSTERESYS_DEG)) || (m_angleDeg > (m_targetAngleDeg + ANGLE_HYSTERESYS_DEG)))
		{
			stm_newState = STM_STATE_ACCURACY_GAME_OVER;
			stm_entryFlag = FALSE;
			stm_exitFlag = TRUE;
		}

		// Check if the value was steady
		if ((millis() - m_lastDebounceTime) > DEBOUNCE_DELAY_ENCODER_MS)
		{
			stm_newState = STM_STATE_ACCURACY_GAME_WIN;
			stm_entryFlag = FALSE;
			stm_exitFlag = TRUE;
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
  // STM_STATE_ACCURACY_GAME_IN_TOLERANCE_KIDS
  // ----------------------------------------------------------------------------
    case STM_STATE_ACCURACY_GAME_IN_TOLERANCE_KIDS:

    if (stm_entryFlag == TRUE)
    {
#ifdef DEBUG
      Serial.println(F("Entered STM_STATE_ACCURACY_GAME_IN_TOLERANCE_KIDS"));
#endif
      stm_entryFlag = FALSE;
      stm_exitFlag = FALSE;
    }
    if (m_angleDeg > 200)
    {
      m_safe->setBarGraphResolution( BAR_GRAPH_RESOLUTION_KIDS_DEG);
      m_safe->getAndDisplayAngles(m_targetAngleDegKids, m_angleDeg);
    }
    else
    {
      m_safe->setBarGraphResolution( BAR_GRAPH_RESOLUTION_DEG);
      m_safe->getAndDisplayAngles(m_targetAngleDeg, m_angleDeg);
    }
    m_safe->getAndDisplayAngles(m_targetAngleDegKids, m_angleDeg);

    // Within tolerance?
    if ((m_angleDeg < (m_targetAngleDegKids - ANGLE_HYSTERESYS_DEG_KIDS)) || (m_angleDeg > (m_targetAngleDegKids + ANGLE_HYSTERESYS_DEG_KIDS)))
    {
      stm_newState = STM_STATE_ACCURACY_GAME_OVER;
      stm_entryFlag = FALSE;
      stm_exitFlag = TRUE;
    }

    // Check if the value was steady
    if ((millis() - m_lastDebounceTime) > DEBOUNCE_DELAY_ENCODER_MS)
    {
      stm_newState = STM_STATE_ACCURACY_GAME_WIN;
      stm_entryFlag = FALSE;
      stm_exitFlag = TRUE;
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
  // STM_STATE_ACCURACY_GAME_WIN
  // ----------------------------------------------------------------------------
	case STM_STATE_ACCURACY_GAME_WIN:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_WIN"));
#endif
			stm_entryFlag = FALSE;
			stm_exitFlag = FALSE;
		}

    m_safe->displayGreenSmiley();
		m_safe->openSafe();
		stm_newState = STM_STATE_ACCURACY_GAME_INIT;
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


	case STM_STATE_ACCURACY_GAME_OVER:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_OVER"));
#endif
			stm_entryFlag = FALSE;
			stm_exitFlag = FALSE;
		}

		m_safe->displayRedSmiley();
		delay(2000);
		stm_newState = STM_STATE_ACCURACY_GAME_INIT;
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
