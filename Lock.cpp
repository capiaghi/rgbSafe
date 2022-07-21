
// ****************************************************************************
/// \file      Lock.cpp
///
/// \brief     Lock-style Solenoid control
///
/// \details   Controlls Lock
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
#include "Lock.hpp"
#include "config.hpp"

Lock::Lock()
{
  
}

// ----------------------------------------------------------------------------
/// \brief     Initialize Lock
/// \detail    Initialize lock. Normaly closed
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t Lock::initialize()
{
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW);
  return 0;
}

// ----------------------------------------------------------------------------
/// \brief     Open lock 
/// \detail    Opens the lock for some time
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t Lock::openLock(uint32_t openTimeMs)
{
  if (openTimeMs > MAX_OPEN_TIME_MS)    openTimeMs = MAX_OPEN_TIME_MS;
  if (openTimeMs < MIN_OPEN_TIME_MS )   openTimeMs = MIN_OPEN_TIME_MS;
  
  digitalWrite(LOCK_PIN, HIGH);
  delay(openTimeMs);
  digitalWrite(LOCK_PIN, LOW);
  
  return RC_OK;
}
