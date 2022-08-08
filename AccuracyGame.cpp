
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
#include "AccuracyGame.hpp"
#include "config.hpp"
#include <Fonts/Picopixel.h> // Large friendly font
#include "FreeMonoBold7pt7b.h" // Create with https://rop.nl/truetype2gfx/
#include "pics/redSmiley_32x32.c"
#include "pics/greenSmiley_32x32.c"

AccuracyGame::AccuracyGame() : m_errorCode(RC_OK),
m_ha40p(),
m_lock(),
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
uint8_t AccuracyGame::initialize(Adafruit_Protomatter* matrix)
{
	m_errorCode = m_ha40p.initialize();
	if (m_errorCode != RC_OK) return m_errorCode;

	m_errorCode = m_lock.initialize();
	if (m_errorCode != RC_OK) return m_errorCode;

	// Get Offset
	m_errorCode = m_ha40p.getAngleDeg(m_angleDeg);
	m_offsetDeg = m_angleDeg;

	// State Machine
	stm_entryFlag = TRUE;
	stm_exitFlag = FALSE;
	stm_newState = STM_STATE_ACCURACY_GAME_INIT;

	m_matrix = matrix;

	return m_errorCode;
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
		m_errorCode = m_ha40p.getAngleDeg(m_angleDeg);
		m_offsetDeg = m_angleDeg;

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

	case STM_STATE_ACCURACY_GAME_CHECK_VALUE:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_CHECK_VALUE"));
#endif
			stm_entryFlag = FALSE;
			stm_exitFlag = FALSE;
		}

		getAndDisplayAngles();

		m_targetAngleDeg = 100.0; //TBD

		// Within tolerance?
		if ((m_angleDeg > (m_targetAngleDeg - ANGLE_HYSTERESYS_DEG)) && (m_angleDeg < (m_targetAngleDeg + ANGLE_HYSTERESYS_DEG)))
		{
			m_lastDebounceTime = millis(); // Reset timer
			stm_newState = STM_STATE_ACCURACY_GAME_IN_TOLERANCE;
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

		
		case STM_STATE_ACCURACY_GAME_IN_TOLERANCE:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_IN_TOLERANCE"));
#endif
			stm_entryFlag = FALSE;
			stm_exitFlag = FALSE;
		}

		getAndDisplayAngles();

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

	case STM_STATE_ACCURACY_GAME_WIN:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_ACCURACY_GAME_WIN"));
#endif
			stm_entryFlag = FALSE;
			stm_exitFlag = FALSE;
		}

		m_matrix->drawRGBBitmap(0, 0, (const uint16_t*)greenSmiley_32x32, 32, 32);
		m_matrix->show();
		delay(2000);
		openSafe();
		stm_newState = STM_STATE_ACCURACY_GAME_CHECK_VALUE;
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

		m_matrix->drawRGBBitmap(0, 0, (const uint16_t*)redSmiley_32x32, 32, 32);
		m_matrix->show();
		delay(2000);
		stm_newState = STM_STATE_ACCURACY_GAME_CHECK_VALUE;
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


uint8_t AccuracyGame::openSafe()
{
	m_matrix->fillScreen(BLACK); // Fill background black to save power
	m_matrix->show();
	m_lock.openLock(SAFE_OPEN_TIME_MS);

	return RC_OK;
}

uint8_t AccuracyGame::getAndDisplayAngles()
{
  uint8_t m_errorCode = RC_OK;
  // Get Encoder Angle and calculate degree, minute and seconds
  // https://de.planetcalc.com/1129/
  m_errorCode = m_ha40p.getAngleDeg(m_angleDeg);
  m_angleDeg -= m_offsetDeg;
  if (m_angleDeg < 0) m_angleDeg += 360.0;

  m_degree = (uint16_t) m_angleDeg;
  float rest = m_angleDeg - m_degree;
  m_minute = (uint16_t)(rest * 60.0);
  Serial.print("Rest Deg: "); Serial.println(rest);
  rest = rest - ((float)m_minute / 60.0);
  Serial.print("Rest Deg: "); Serial.println(rest);
  m_seconds = (uint16_t)(rest * 3600.0);
  Serial.print("Angle Deg: "); Serial.println(m_degree);
  Serial.print("Angle Min: "); Serial.println(m_minute);
  Serial.print("Angle Sec: "); Serial.println(m_seconds);
  
  m_matrix->fillScreen(BLACK); // Fill background black
  m_matrix->setFont(&FreeMonoBold7pt7b);  // Use nice bitmap font
  m_matrix->setCursor(0, 8);
  m_matrix->setTextColor(WHITE);
  m_matrix->print(m_degree);
  m_matrix->drawCircle(m_matrix->getCursorX() + 2, 1, 1, WHITE); //Degree symobl
  m_matrix->setCursor(0, 18);
  m_matrix->print(m_minute); m_matrix->println("'");
  m_matrix->setCursor(0, 28);
  m_matrix->print(m_seconds); m_matrix->write(34); // 34 -> Symbol "
  m_matrix->println("");

  // x, y, w, h, color
  m_matrix->fillRect(0, 30, 10, 2, GREEN);

  m_matrix->show();
  return RC_OK;
  
}