#pragma once

#include <stdint.h>
#include <Adafruit_Protomatter.h>
#include <Adafruit_NeoPixel.h>
#include "Encoder.hpp"
#include "Lock.hpp"
#include <arduino-timer.h>

class Safe
{
public:
	Safe();
  uint8_t initialize(Adafruit_Protomatter* matrix, SerialHandler *serialHandler, Adafruit_NeoPixel *neoPixels, Timer<1, millis, Adafruit_NeoPixel *>* rbgStripTimer);
	void reset();
  uint8_t run();
  uint8_t openSafe();
  uint8_t getAndDisplayAngles(const float targetAngleDeg, float& angleDeg);
  uint8_t displayCode(const uint8_t * digits);
  void setOffsetDeg(const float offset);
  void resetDisplay();
  uint8_t getAngleDeg( float& angleDeg);
  void displayHexagonLogo();
  void displayGreenSmiley();
  void displayRedSmiley();
  void showHtcRules();
  void setNullPosition();
  void setBarGraphResolution(const float barGraphResolution);

private:

	static const uint32_t SAFE_OPEN_TIME_MS = 5000;

	static const uint8_t NUMBER_OF_CODE_DIGITS     = 4;    /// Number of code digits


  static const uint32_t RGB_STRIP_ILLUMINATION_TIME_S = 15; // How long should be the led in the safe on in seconds

	uint8_t m_errorCode;
	float m_offset;
  float m_offsetDeg;
  int8_t m_currentCode[NUMBER_OF_CODE_DIGITS];
  uint8_t m_initStatus;
  float m_barGraphResolution;
   
 
	float m_lastAngleDeg;
	uint32_t m_lastDebounceTime;
	uint8_t codeNumber;
	uint8_t countDirection;
	uint16_t m_degree;
	uint16_t m_minute;
	uint16_t m_seconds;

  

  // RGB Matrix --------------------------------------------------------------
	Adafruit_Protomatter* m_matrix;

	// Angle Encoder HA40+ -----------------------------------------------------
	Encoder m_ha40p;

	// Lock-style Solenoid -----------------------------------------------------
	Lock m_lock;

  Adafruit_NeoPixel* m_neoPixels;
  Timer<1, millis, Adafruit_NeoPixel *>* m_rbgStripTimer;
  
	uint8_t directionChanged();


};
