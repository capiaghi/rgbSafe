
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
#include <Fonts/Picopixel.h> // Large friendly font
#include "FreeMonoBold7pt7b.h" // Create with https://rop.nl/truetype2gfx/

Safe::Safe() : m_errorCode(RC_OK),
m_ha40p(),
m_lock(),
m_lastDebounceTime(0),
codeNumber(0),
countDirection(CW),
m_lastAngleDeg(0.0),
m_offset(0.0)
{

}

// ----------------------------------------------------------------------------
/// \brief     Initialize Safe
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t Safe::initialize(Adafruit_Protomatter* matrix)
{
	m_errorCode = m_ha40p.initialize();
	if (m_errorCode != RC_OK) return m_errorCode;

	m_errorCode = m_lock.initialize();
	if (m_errorCode != RC_OK) return m_errorCode;

	m_code[0] = FIRST_CODE_ELEMENT;
	m_code[1] = SECOND_CODE_ELEMENT;
	m_code[2] = THIRD_CODE_ELEMENT;
	codeNumber = 0;
	countDirection = CW;
	m_lastAngleDeg = 0.0;

	// Get Offset
	m_errorCode = m_ha40p.getAngleDeg(m_angleDeg);
	m_offset = m_angleDeg;

	// State Machine
	stm_entryFlag = TRUE;
	stm_exitFlag = FALSE;
	stm_newState = STM_STATE_SAFE_INIT;

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
uint8_t Safe::checkCode()
{
	m_errorCode = m_ha40p.getAngleDeg(m_angleDeg);

	switch (stm_actState)
	{
	case STM_STATE_SAFE_INIT:
		// Entry action
		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_SAFE_INIT"));
#endif
			stm_newState = STM_STATE_SAFE_FIRST_DIGIT;
			stm_entryFlag = FALSE;
			stm_exitFlag = TRUE;
		}
		// Get Offset
		m_offset = m_angleDeg;

		// Exit
		if (stm_exitFlag == TRUE)
		{
			//clearScreen();
			stm_exitFlag = FALSE;
			stm_actState = stm_newState;
			stm_entryFlag = TRUE;
		}
		break;

	case STM_STATE_SAFE_FIRST_DIGIT:

		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_SAFE_FIRST_DIGIT"));
#endif
			stm_newState = STM_STATE_SAFE_FIRST_DIGIT;
			stm_entryFlag = FALSE;
			stm_exitFlag = FALSE;
		}

		// Get value to display
		m_Values[0] = uint8_t((m_angleDeg - m_offset) / STEP_SIZE);

		showCode(m_Values);

		if (directionChanged())
		{
			stm_newState = STM_STATE_SAFE_SECOND_DIGIT;
			stm_exitFlag = TRUE;
		}

		break;


	case STM_STATE_SAFE_SECOND_DIGIT:

		if (directionChanged())
		{
			stm_newState = STM_STATE_SAFE_THIRD_DIGIT;
			stm_exitFlag = TRUE;
		}

		break;

	case STM_STATE_SAFE_THIRD_DIGIT:

		if (directionChanged())
		{
			stm_newState = STM_STATE_SAFE_FOURTH_DIGIT;
			stm_exitFlag = TRUE;
		}
		break;

	case STM_STATE_SAFE_FOURTH_DIGIT:

		if (directionChanged())
		{
			stm_newState = STM_STATE_SAFE_CHECK_CODE;
			stm_exitFlag = TRUE;
		}
		break;


	case STM_STATE_SAFE_CHECK_CODE:
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


	/*

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
			openSafe();
			//m_lock.openLock( SAFE_OPEN_TIME_MS );
			return CORRECT_CODE;
		  }
		  codeNumber++;
		}
	  }
	*/
	m_lastAngleDeg = m_angleDeg;
	return m_errorCode;
}

// ----------------------------------------------------------------------------
/// \brief     Accuracy game for Leica Event
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo     
///
uint8_t Safe::accuracyGame()
{
  // https://de.planetcalc.com/1129/
  m_errorCode = m_ha40p.getAngleDeg(m_angleDeg);
  uint8_t degree = (uint8_t) m_angleDeg;
  float rest = m_angleDeg - degree;
  uint8_t minute = (uint8_t)(rest * 60.0);
  Serial.print("Rest Deg: "); Serial.println(rest);
  rest = rest - ((float)minute / 60.0);
  Serial.print("Rest Deg: "); Serial.println(rest);
  uint8_t second = (uint8_t)(rest * 3600.0);
  Serial.print("Angle Deg: "); Serial.println(degree);
  Serial.print("Angle Min: "); Serial.println(minute);
  Serial.print("Angle Sec: "); Serial.println(second);
  
  m_matrix->fillScreen(BLACK); // Fill background black
  m_matrix->setFont(&FreeMonoBold7pt7b);  // Use nice bitmap font
  m_matrix->setCursor(0, 8);
  m_matrix->setTextColor(WHITE);
  m_matrix->println("49");
  m_matrix->drawCircle(20, 1, 1, WHITE); //todo degree symbol
  m_matrix->setCursor(0, 18);
  m_matrix->println("32'"); 
  m_matrix->setCursor(0, 28);
  m_matrix->print("31"); m_matrix->write(34);
  m_matrix->println("");

  // x, y, w, h, color
  m_matrix->fillRect(0, 30, 10, 2, GREEN);

  m_matrix->show();
  
  return m_errorCode;
}

uint8_t Safe::openSafe()
{
	//m_matrix->println("Safe should be open :-)");
	m_lock.openLock(SAFE_OPEN_TIME_MS);

	return RC_OK;
}

uint8_t Safe::setNewCode()
{
	return RC_OK;
}


uint8_t Safe::showCode(uint8_t valuesToDisplay[])
{
	m_matrix->fillScreen(BLACK); // Fill background black
	m_matrix->setFont(&Picopixel);  // Use nice bitmap font
	m_matrix->setCursor(0, 14);

	m_matrix->print(valuesToDisplay[0]);
  m_matrix->print(valuesToDisplay[1]);
  m_matrix->print(valuesToDisplay[2]);
  m_matrix->print(valuesToDisplay[3]);

	m_matrix->show();

#ifdef DEBUG
	Serial.print(valuesToDisplay[0]);
 Serial.print(valuesToDisplay[1]);
 Serial.print(valuesToDisplay[2]);
 Serial.println(valuesToDisplay[3]);
#endif
	return RC_OK;
}

uint8_t Safe::directionChanged()
{
	// Check direction
	if (round(m_angleDeg) > round(m_lastAngleDeg)) // Get nearest integer (close enough for count direction :-))
	{
		countDirection = CW;
	}
	else
	{
		countDirection = CCW;
	}
	return DIRECTION_NOT_CHANGED;
}
