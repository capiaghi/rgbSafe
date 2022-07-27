// ButtonHandler.hpp

#pragma once
#include <stdint.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class ButtonHandler
{

public:
	ButtonHandler();
	uint8_t initialize();
	uint8_t getEnterButtonState();
	void clear();
	void update();

private:
	// Private constants **********************************************************
	uint8_t 	m_buttonStateEnter = HIGH;

	// Last button state
	uint8_t 	m_lastbuttonStateEnter = HIGH;

	// Button pressed flag
	uint8_t 	m_buttonEnterFlag = LOW;

	// the following variables are long's because the time, measured in miliseconds,
	// will quickly become a bigger number than can be stored in an int.
	// the last time the output pin was toggled
	uint32_t m_lastDebounceTimeEnter = 0;
	const uint32_t  DEBOUNCE_DELAY = 50;    // the debounce time; increase if the output flickers

};

