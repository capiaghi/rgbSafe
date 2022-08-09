
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
  enableRx();
  Serial1.begin( UART_SPEED , SERIAL_8N1 ); // Default, no parity
  #ifdef DEBUG
  Serial.println("SerialHandler init");
  #endif
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
  while (!Serial1);
}

void SerialHandler::begin(unsigned long baudrate)
{
  Serial1.begin(baudrate, SERIAL_8N1);
  while (!Serial1);
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
    enableTx();
    txDataWritten = Serial1.write(txData);
    Serial1.flush(); // Waits for the transmission of outgoing serial data to complete.
    delayMicroseconds(100);
  }
  else 
  {
    txDataWritten = Serial1.write(txData);
  }

#ifdef DEBUG
     Serial.print(F("Serial 1 write: "));
     Serial.println(txData);
#endif

  enableRx(); //todo

  return txDataWritten;
}

uint8_t SerialHandler::write(const uint8_t txData[], uint8_t txDataLength)
{
  uint8_t txDataWritten = 0;
#ifdef DEBUG
     Serial.println(F("Serial 1 write: "));
     for (uint8_t i = 0; i < txDataLength; i++)
     {
      Serial.println(txData[i]);
     }
     Serial.println("-------");
     
#endif
  if (m_rs485ModeEnable == 1)
  {
    enableTx();
    txDataWritten = Serial1.write(txData, txDataLength);
    Serial1.flush();
    enableRx();
    }
    else 
    {
      txDataWritten = Serial1.write(txData, txDataLength);
    }
    return txDataWritten;
}

// ----------------------------------------------------------------------------
/// \brief     Read data over UART or RS485
/// \detail    
/// \warning   
/// \return    Serial1 data
/// \todo      
///
uint8_t SerialHandler::read()
{
  if (m_rs485ModeEnable == 1)
  {
    enableRx();
  }
  return Serial1.read();
}

/// <summary>
/// Read Bytes from Serial1 port
/// </summary>
/// <param name="buffer">Buffer for data storage</param>
/// <param name="length">Length of the buffer</param>
/// <returns></returns>
size_t SerialHandler::readBytes( uint8_t *buffer, const size_t length)
{
  if (m_rs485ModeEnable == 1)
  {
    enableRx();
  }
  return Serial1.readBytes(buffer, length);
}

/// <summary>
/// Enables RS485 Mode: nRE, DE support
/// </summary>
void SerialHandler::enableRs485Mode()
{
  enableRx();
  m_rs485ModeEnable = 1;
}

/// <summary>
/// Disable RS485 Mode
/// </summary>
void SerialHandler::disableRs485Mode()
{
  enableRx();
  m_rs485ModeEnable = 0;
}

/// <summary>
/// Enable RX of the RS-485 driver. Disables TX.
/// </summary>
void SerialHandler::enableRx()
{
  digitalWrite(TX_ENABLE_PIN, LOW);
  delayMicroseconds(DELAY_US_PIN_STATE);
}

/// <summary>
/// Enable TX of the RS-485 driver. Disables RX.
/// </summary>
void SerialHandler::enableTx()
{
  digitalWrite(TX_ENABLE_PIN, HIGH);
  delayMicroseconds(DELAY_US_PIN_STATE);
}
