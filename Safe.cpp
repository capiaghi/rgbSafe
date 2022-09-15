
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
// Include pictures
//https://github.com/moononournation/Arduino_GFX/blob/master/examples/ImgViewer/ImgViewerPROGMEM/ImgViewerPROGMEM.ino
#include "pics/hexagon_28x32.c"
#include "pics/greenSmiley_32x32.c"
#include "pics/redSmiley_32x32.c"
#include "SafeGame.hpp" // UNDEFINED_CODE_ELEMENT

// ----------------------------------------------------------------------------
/// \brief     Turn off rgb led strip
/// \detail    Timer calls this function
/// \warning   
/// \return    
/// \todo      
///
bool turnOffRgbStrip(Adafruit_NeoPixel *neoPixels )
{
  neoPixels->clear();
  neoPixels->show();
  return true; // repeat? true
}


Safe::Safe() : m_errorCode(RC_OK),
m_ha40p(),
m_lock(),
m_offsetDeg(0.0),
m_initStatus(INIT_NOT_COMPLETE)
{

}

// ----------------------------------------------------------------------------
/// \brief     Initialize Safe
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      Polarisation?
///
uint8_t Safe::initialize(Adafruit_Protomatter* matrix, SerialHandler *serialHandler, Adafruit_NeoPixel *neoPixels, Timer<1, millis, Adafruit_NeoPixel *>* rbgStripTimer)
{
  if (m_initStatus == INIT_NOT_COMPLETE)
  {
    m_matrix          = matrix;
    m_neoPixels       = neoPixels;
    m_rbgStripTimer   = rbgStripTimer;
  
    m_errorCode       = m_lock.initialize();
    if (m_errorCode != RC_OK) return m_errorCode;
  
    m_errorCode       = m_ha40p.initialize(serialHandler);
    if (m_errorCode != RC_OK) return m_errorCode;
  
    // Get Offset
    setNullPosition();  
  }
  m_initStatus = INIT_COMPLETE;
  
  return m_errorCode;
}

// ----------------------------------------------------------------------------
/// \brief     Set current position to zero
/// \detail    Used to set offset
/// \warning   
/// \return    RC_Type
/// \todo      
///
void Safe::setNullPosition()
{
  m_errorCode       = m_ha40p.getAngleDeg(m_offsetDeg);
}

// ----------------------------------------------------------------------------
/// \brief     Set current offset
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
void Safe::setOffsetDeg(const float offset)
{
  m_offsetDeg = offset;
}

// ----------------------------------------------------------------------------
/// \brief     Open safe
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Safe::openSafe()
{
  m_neoPixels->clear();

  for (int i = 0; i < 8; i++) {
    m_neoPixels->setPixelColor(i, m_neoPixels->Color(255, 255, 255));
  }
  m_neoPixels->show();
  m_rbgStripTimer->in((RGB_STRIP_ILLUMINATION_TIME_S*1000), turnOffRgbStrip, m_neoPixels);
  
  m_lock.openLock(SAFE_OPEN_TIME_MS);

  return RC_OK;
}


// ----------------------------------------------------------------------------
/// \brief     Display current code of the safe (input)
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Safe::displayCode(const uint8_t * digits)
{
  m_matrix->fillScreen(BLACK);            // Fill background black
  m_matrix->setFont(&FreeMonoBold7pt7b);  // Use nice bitmap font
  m_matrix->setCursor(0, 8);              // Set cursor
  m_matrix->setTextColor(WHITE);

  for (uint8_t digitNumber; digitNumber < NUMBER_OF_CODE_DIGITS; digitNumber++)
  {
    if (digits[digitNumber] == SafeGame::UNDEFINED_CODE_ELEMENT)
    {
        m_matrix->print("X");
    }
    else
    {
      m_matrix->print(digits[digitNumber]);
    }
  }

  m_matrix->show();
  return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Display Angles (Accuracy Game)
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Safe::getAndDisplayAngles(const float targetAngleDeg, float& angleDeg)
{
  uint8_t m_errorCode = RC_OK;
  // Get Encoder Angle and calculate degree, minute and seconds
  // https://de.planetcalc.com/1129/
  m_errorCode = m_ha40p.getAngleDeg(angleDeg);
  angleDeg -= m_offsetDeg;
  if (angleDeg < 0) angleDeg += 360.0;

  m_degree = (uint16_t) angleDeg;
  float rest = angleDeg - m_degree;
  m_minute = (uint16_t)(rest * 60.0);
#ifdef DEBUG
  Serial.print("Rest Deg: "); Serial.println(rest);
#endif
  rest = rest - ((float)m_minute / 60.0);
#ifdef DEBUG
  Serial.print("Rest Deg: "); Serial.println(rest);
#endif
  m_seconds = (uint16_t)(rest * 3600.0);
#ifdef DEBUG
  Serial.print("Angle Deg: "); Serial.println(m_degree);
  Serial.print("Angle Min: "); Serial.println(m_minute);
  Serial.print("Angle Sec: "); Serial.println(m_seconds);
#endif
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
  // Calculate bar graph
  float differenceDeg = abs(targetAngleDeg - angleDeg);
  float barLength = 0.0;
  if (differenceDeg > m_barGraphResolution) differenceDeg = m_barGraphResolution;
  barLength = WIDTH - differenceDeg / m_barGraphResolution * WIDTH;

  m_matrix->fillRect(0, 30, barLength , 2, GREEN);

  m_matrix->show();
  return RC_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Display green smiley
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
void Safe::displayGreenSmiley()
{
  m_matrix->fillScreen(BLACK);
  m_matrix->drawRGBBitmap(0, 0, (const uint16_t*)greenSmiley_32x32, 32, 32);
  m_matrix->show();
}

// ----------------------------------------------------------------------------
/// \brief     Display red smiley
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
void Safe::displayRedSmiley()
{
  m_matrix->fillScreen(BLACK);
  m_matrix->drawRGBBitmap(0, 0, (const uint16_t*)redSmiley_32x32, 32, 32);
  m_matrix->show();
}

// ----------------------------------------------------------------------------
/// \brief     Display hexagon logo
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
void Safe::displayHexagonLogo()
{
  m_matrix->fillScreen(BLACK);
  m_matrix->drawRGBBitmap(0, 0, (const uint16_t*)hexagon_28x32, 28, 32);
  m_matrix->show();
}

void Safe::showHtcRules()
{
  
}


// ----------------------------------------------------------------------------
/// \brief     Reset display (black)
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
void Safe::resetDisplay()
{
    m_matrix->fillScreen(BLACK); // Fill background black
    m_matrix->show();
}

// ----------------------------------------------------------------------------
/// \brief     Set bar graph resolution
/// \detail    
/// \warning   
/// \return    
/// \todo      
///
void Safe::setBarGraphResolution( const float barGraphResolution)
{
  m_barGraphResolution = barGraphResolution;
}


// ----------------------------------------------------------------------------
/// \brief     Get angle in degree
/// \detail    
/// \warning   
/// \return    RC_Type
/// \todo      
///
uint8_t Safe::getAngleDeg( float& angleDeg)
{
  m_errorCode = m_ha40p.getAngleDeg(angleDeg);
  angleDeg -= m_offsetDeg;
  if (angleDeg < 0) angleDeg += 360.0;
  return m_errorCode;
}
