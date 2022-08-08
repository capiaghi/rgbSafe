
// ****************************************************************************
/// \file      SerialHandler.cpp
///
/// \brief     Handels UART or RS485 interface
///
/// \details   
///
/// \author    Christoph Capiaghi
///
/// \version   0.1
///
/// \date      20220802
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
#include "SerialHandler.hpp"
#include "config.hpp"

SerialHandler::SerialHandler() : m_rs485ModeEnable(0)
{
  
}

// ----------------------------------------------------------------------------
/// \brief     Initialize SerialHandler
/// \detail    Initialize SerialHandler. RX active RS485 interface
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t SerialHandler::initialize()
{
  pinMode(TX_ENABLE_PIN, OUTPUT);
  digitalWrite(TX_ENABLE_PIN, LOW);
  Serial1.begin( UART_SPEED , SERIAL_8N1 ); // Default, no parity
  return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Config of UART and start
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
void SerialHandler::begin(unsigned long baudrate, uint16_t config)
{
  Serial1.begin(baudrate , config );
}

void SerialHandler::begin(unsigned long baudrate)
{
  begin(baudrate, SERIAL_8N1);
}


// ----------------------------------------------------------------------------
/// \brief     Initialize SerialHandler
/// \detail    Initialize SerialHandler. RX active RS485 interface
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
void SerialHandler::end()
{
  Serial1.end();
}

// ----------------------------------------------------------------------------
/// \brief     Initialize SerialHandler
/// \detail    Initialize SerialHandler. RX active RS485 interface
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t SerialHandler::available()
{
  return Serial1.available();
}


// ----------------------------------------------------------------------------
/// \brief     Write data over UART or RS485
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t SerialHandler::write(const uint8_t txData)
{
  uint8_t txDataWritten = 0;
  if (m_rs485ModeEnable == 1)
  {
    digitalWrite(TX_ENABLE_PIN, LOW);
    txDataWritten = Serial1.write(txData);
    digitalWrite(TX_ENABLE_PIN, HIGH);
  }
  else 
  {
    txDataWritten = Serial1.write(txData);
  }

#ifdef DEBUG
     Serial.print(F("Serial 1 write: "));
     Serial.println(txData);
#endif

  return txDataWritten;
}

uint8_t SerialHandler::write(const uint8_t txData[], uint8_t txDataLength)
{
#ifdef DEBUG
     Serial.println(F("Serial 1 write: "));
     for (uint8_t i = 0; i < txDataLength; i++)
     {
      Serial.println(txData[i]);
     }
     Serial.println("-------");
     
#endif
  return Serial1.write(txData, txDataLength);
}

// ----------------------------------------------------------------------------
/// \brief     Read data over UART or RS485
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t SerialHandler::read(uint8_t& rxData)
{
  if (m_rs485ModeEnable == 1)
  {
    digitalWrite(TX_ENABLE_PIN, HIGH);
  }
  rxData = Serial1.read();
  return RC_OK;
}

size_t SerialHandler::readBytes( uint8_t *buffer, size_t length)
{
  if (m_rs485ModeEnable == 1)
  {
    digitalWrite(TX_ENABLE_PIN, HIGH);
  }
  return Serial1.readBytes(buffer, length);
}

void SerialHandler::enableRs485Mode()
{
  digitalWrite(TX_ENABLE_PIN, LOW);
  m_rs485ModeEnable = 1;
}

void SerialHandler::disableRs485Mode()
{
  digitalWrite(TX_ENABLE_PIN, HIGH);
  m_rs485ModeEnable = 0;
}
