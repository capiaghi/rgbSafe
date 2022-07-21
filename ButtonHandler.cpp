// ****************************************************************************
/// \file      ButtonHandler.cpp
///
/// \brief     Sets and returns Button flags
///
/// \details   Button Handler for buttons, currently only one button supported
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
#include <stdint.h>
#include "ButtonHandler.hpp"
#include "config.hpp"

ButtonHandler::ButtonHandler() : 
    m_buttonStateEnter(HIGH),
    m_lastbuttonStateEnter(HIGH),
    m_buttonEnterFlag(LOW)
{
}

// ----------------------------------------------------------------------------
/// \brief     Initialize buttosn
/// \detail    Turn on Pull-ups, define as input
/// \warning   
/// \return    void
/// \todo      
///
uint8_t ButtonHandler::initialize()
{
    pinMode(BUTTON_ENTER_PIN, INPUT_PULLUP);
    return RC_OK;
}


// ----------------------------------------------------------------------------
/// \brief     Get state of Button Enter
/// \detail    Gets the state of the button and resets the flag
/// \warning   
/// \return    state: 1 pressed, 0 not pressed
/// \todo      
///
uint8_t ButtonHandler::getEnterButtonState()
{
	if (m_buttonEnterFlag == HIGH)
	{
        m_buttonEnterFlag = LOW;
		  return 1;
	}
	else
	{
        m_buttonEnterFlag = LOW;
		return 0;
	}
}


void ButtonHandler::clear()
{
    m_buttonEnterFlag = LOW;
}

// ----------------------------------------------------------------------------
/// \brief     Updates the flags of the buttons
/// \detail    Sets the flags of the 3 buttons
/// \warning   
/// \return    state: 1 pressed, 0 not pressed
/// \todo      
///
void ButtonHandler::update()
{
    // Read out buttons
    int readingEnter = digitalRead(BUTTON_ENTER_PIN);

    // If the switch changed, due to noise or pressing:
    if (readingEnter != m_lastbuttonStateEnter)
    {
        // reset the debouncing timer
        m_lastDebounceTimeEnter = millis();
    }

    if ((millis() - m_lastDebounceTimeEnter) > DEBOUNCE_DELAY)
    {
        // whatever the reading is at, it's been there for longer
        // than the debounce delay, so take it as the actual current state:
        // if the button state has changed:
        if (readingEnter != m_buttonStateEnter)
        {
            m_buttonStateEnter = readingEnter;

            if (m_buttonStateEnter == LOW)
            {
                m_buttonEnterFlag = HIGH;
            }
        }
    }

    m_lastbuttonStateEnter = readingEnter;
}
