
// ****************************************************************************
/// \file      Encoder.cpp
///
/// \brief     HA40+ Encoder Control
///
/// \details   Controlls the HA40+ Encoder using UART 
///
/// \author    Christoph Capiaghi
///
/// \version   0.1
///
/// \date      20220720
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
#include "Encoder.hpp"
#include "config.hpp"

Encoder::Encoder() : m_rawAngle(0)
{
  
}

// ----------------------------------------------------------------------------
/// \brief     Initialize Encoder
/// \detail    Set trigger output
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::initialize()
{
  pinMode(BUTTON_ENTER_PIN, OUTPUT);
  digitalWrite(BUTTON_ENTER_PIN, LOW);
  return 0;
}

// ----------------------------------------------------------------------------
/// \brief     Get Encoder Angle in Degree
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::getAngleDeg(float angleDeg)
{
  getAngle( m_rawAngle );
  angleDeg = 23.2;
  //angleDeg = (float)((m_rawAngle / (2^32)) * (2*PI));
  return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Get Encoder Angle in Gon
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::getAngleGon(float angleGon)
{
  getAngle( m_rawAngle );
  angleGon = 35.7;
  //angleGon = (float)((m_rawAngle / (2^32)) * (400.0));
  return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Send trigger to Encoder
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::trigger()
{
  digitalWrite(BUTTON_ENTER_PIN, HIGH);
  delay(m_triggerDelayMs);
  digitalWrite(BUTTON_ENTER_PIN, LOW);
  return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Get Encoder raw Angle in Degree
/// \detail    angleDeg = angleRaw / 2^32 *2pi
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::getAngle(uint32_t rawAngle)
{
  trigger();
  Serial1.print("Code to get Angle");
  rawAngle = 0;
  return RC_OK;
}
