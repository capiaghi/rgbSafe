
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
#include <stdint.h>
extern "C" {
#include "crc8.h"
}


Encoder::Encoder() : m_serialHandler(), m_rawAngle(0)
{

}

// ----------------------------------------------------------------------------
/// \brief     Initialize Encoder
/// \detail    Set encoder to 8-bit, ASCII Mode
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::initialize()
{
#ifdef DEBUG
Serial.println("Encoder Init");
#endif

  m_serialHandler.initialize();
  m_serialHandler.enableRs485Mode();
  setEightBitMode();

  sendRomerGCmd();
  uint8_t recBufferLength = 8;
  uint8_t recBuffer[recBufferLength];
  readRomerCmd(recBuffer, recBufferLength);
 
   return 0;
}

// ----------------------------------------------------------------------------
/// \brief     Get Encoder Angle in Degree
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::getAngleDeg(float &angleDeg)
{
	getAngle(m_rawAngle);
	angleDeg = (float)((m_rawAngle / pow(2,32)) * (360.0));
 #ifdef DEBUG
     Serial.print(F("Encoder Angle in Degree: "));
     Serial.println(angleDeg, 8);
#endif
	return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Get Encoder Angle in Rad
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::getAngleRad(float &angleDeg)
{
  getAngle(m_rawAngle);
  angleDeg = (float)((m_rawAngle / pow(2,32)) * (2*PI));
 #ifdef DEBUG
     Serial.print(F("Encoder Angle in Degree: "));
     Serial.println(angleDeg, 8);
#endif
  return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Get Encoder Angle in Gon
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::getAngleGon(float &angleGon)
{
	getAngle(m_rawAngle);
	angleGon = (float)((m_rawAngle / pow(2,32)) * (400.0));
	return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Send hardware trigger to Encoder
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Encoder::trigger()
{
  // Not supported
	//digitalWrite(TRIGGER_PIN, HIGH);
	//delay(m_triggerDelayMs);
	//digitalWrite(TRIGGER_PIN, LOW);
	return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Get Encoder raw Angle in Degree
/// \detail    Send Romer CMD B -> SW Trigger and get Trigger
/// \warning   
/// \return    RC_Type
/// \todo  
///    
uint8_t Encoder::getAngle(uint32_t &rawAngle)
{
// First: Send CMD
    sendRomerBCmd();

// Now get angle
 //Romer Protocol
// | Address Field | Length | Command | Angle info field | Angle LSB | Angle | Angle | Angle MSB | CRC
// Supported Protocol
// | 0xXX         | 0x06   | 0x42    | XX                | XX        |  XX   | XX    | XX        | XX
// No crc check implemented yet
    /*
  uint8_t recBufferLength = 9;
  uint8_t recBuffer[recBufferLength];
  uint8_t retries = 0;
  while(m_serialHandler.available() <= 0)
  {
    if(retries > 10) return RC_INV_UART1_TIMEOUT;
    delay(1);
    retries++;
#ifdef DEBUG
      Serial.print(F("Number of retries (UART1 RX): "));
      Serial.println(retries);
#endif
  }
  m_serialHandler.readBytes(recBuffer, recBufferLength);

#ifdef DEBUG
      Serial.println(F("Received bytes (UART1 RX): "));
      for(uint8_t i = 0; i < recBufferLength; i++)
      {
        Serial.print(recBuffer[i], HEX);
        Serial.println("");
      }
#endif
*/
  uint8_t recBufferLength = 9;
  uint8_t recBuffer[recBufferLength];
  //Romer Protocol
  // | Address Field | Length | Command | Register LSB | Register | Register | Register MSB | CRC
  // Supported Protocol
  // | 0xXX         | 0x06   | 0x47    | XX            |  XX      | XX       | XX           | XX
  readRomerCmd(recBuffer, recBufferLength);

  if(recBuffer[ROMER_LENGTH_FIELD_RX] != ROMER_CMD_B_LENGTH_RX) return RC_INV_UART1_LENGTH;
  if(recBuffer[ROMER_COMMAND_FIELD_RX] != ROMER_CMD_B_RX)       return RC_INV_UART1_COMMAND;

  // 4* uint8_t to uint32_t
  rawAngle = (recBuffer[ROMER_ANGLE4_MSB_FIELD_RX]<<24) | (recBuffer[ROMER_ANGLE3_FIELD_RX]<<16) | (recBuffer[ROMER_ANGLE2_FIELD_RX]<<8) | recBuffer[ROMER_ANGLE1_LSB_FIELD_RX];
#ifdef DEBUG
      Serial.print(F("Raw angle (UART1 RX): "));
      Serial.println(rawAngle);
#endif
	return RC_OK;
}


void Encoder::sendRomerBCmd()
{
    // First: Send CMD
//Romer Protocol
// | Address Field | Length | Command | Number of Angles | CRC8 
// Supported Protocol
// | 0xF0          | 0x03   | 0x42    | 1                | CRC8
// 0xF0: Master to all Encoder
// Length of the command in bytes, excluding the address and the length byte itself
    uint8_t cmd[] = {ROMER_CMD_B_ADDRESS_TX, ROMER_CMD_B_LENGTH_TX, ROMER_CMD_B_RX, ROMER_CMD_B_NUMBER_OF_ANGLES, DEFAULT_CRC_VALUE};

  // The CRC8 byte is calculated from the whole command
    cmd[ROMER_CRC_FIELD_TX] = CSV_CalcCRC8(cmd, (uint32_t)(sizeof(cmd) / sizeof(cmd[0])) - 1); // size - 1: Without crc
    m_serialHandler.write(cmd, sizeof(cmd));
}

void Encoder::sendRomerGCmd()
{
    uint8_t cmd[] = { ROMER_CMD_B_ADDRESS_TX, ROMER_CMD_READ_REGISTER_LENGTH_TX, ROMER_CMD_G_TX, ROMER_REGISTER_ADDRESS_LSB, ROMER_REGISTER_ADDRESS_MSB, ROMER_CMD_G_NUMBER_OF_REGISTER, DEFAULT_CRC_VALUE };
    // The CRC8 byte is calculated from the whole command
    cmd[ROMER_CRC_FIELD_G_COMMAND_TX] = CSV_CalcCRC8(cmd, (uint32_t)(sizeof(cmd) / sizeof(cmd[0])) - 1); // size - 1: Without crc
    m_serialHandler.write(cmd, sizeof(cmd));
}

/// <summary>
/// Reads a specific romer command
/// </summary>
/// <param name="recBuffer">Receive Buffer</param>
/// <param name="recBufferLength">Length of the Receive Buffer</param>
/// <returns></returns>
uint8_t Encoder::readRomerCmd(uint8_t recBuffer[], uint8_t recBufferLength)
{
    uint8_t retries = 0;
    while (m_serialHandler.available() <= 0)
    {
        if (retries > 10) return RC_INV_UART1_TIMEOUT;
        delay(1);
        retries++;
#ifdef DEBUG
        Serial.print(F("Number of retries (UART1 RX): "));
        Serial.println(retries);
#endif
    }
    m_serialHandler.readBytes(recBuffer, recBufferLength);

#ifdef DEBUG
    Serial.println(F("Received bytes (UART1 RX): "));
    for (uint8_t i = 0; i < recBufferLength; i++)
    {
        Serial.print(recBuffer[i], HEX);
        Serial.println("");
    }
#endif
    return RC_OK;
}

void Encoder::setEightBitMode()
{
    // Set 8 bit mode, binary
    // Usage: It is necessary to write in 9 bit mode with the MSB 1
    // Trick: Write in 8 bit mode and add partiy bit (even or odd -> 1 is neccessary)
    m_serialHandler.end();
    m_serialHandler.begin(UART_SPEED, SERIAL_8O1); // odd parity

    m_serialHandler.write(0xFF);
    m_serialHandler.write(0xFF);
    m_serialHandler.write(0xAA);
    m_serialHandler.write(0xAA);
    m_serialHandler.end();
    m_serialHandler.begin(UART_SPEED, SERIAL_8N1); // Default, no parity
}