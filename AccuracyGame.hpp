#pragma once

#include <stdint.h>
#include <Adafruit_Protomatter.h>
#include <Adafruit_NeoPixel.h>
#include "Encoder.hpp"
#include "Lock.hpp"

class AccuracyGame
{
public:
	AccuracyGame();
	uint8_t initialize(Adafruit_Protomatter* matrix, SerialHandler* serialHandler, Adafruit_NeoPixel* neoPixels);
	uint8_t run();
  void reset();

private:
	static constexpr float ANGLE_HYSTERESYS_DEG = 1.0f;			// Hysteresys of angle in degree
	static const uint32_t DEBOUNCE_DELAY_ENCODER_MS = 5000;		// This time defines, how long the correct code must be present
	static const uint32_t SAFE_OPEN_TIME_MS = 5000;
	static constexpr float TARGET_ANGLE = 100.0;
  static constexpr float BAR_GRAPH_RESOLUTION_DEG = 5;

	// Private types **************************************************************
	/// \brief Used States
	/// \details States for the main state machine TBD
	typedef enum stm_state_e
	{
		STM_STATE_ACCURACY_GAME_INIT,          /// Offset
		STM_STATE_ACCURACY_GAME_CHECK_VALUE,
		STM_STATE_ACCURACY_GAME_IN_TOLERANCE,  /// 
		STM_STATE_ACCURACY_GAME_OVER,           /// Second digit of the code
		STM_STATE_ACCURACY_GAME_WIN
	} stm_state_t;

	// Static variables ***********************************************************
	typedef unsigned char stm_bool_t;
	stm_state_t            stm_actState;    // Actual State variable
	stm_state_t            stm_newState;    // New State variable
	stm_bool_t             stm_entryFlag;   // Flag for handling the entry action
	stm_bool_t             stm_exitFlag;    // Flag for handling the exit action

	uint8_t m_errorCode;
	float m_angleDeg;
	float m_offsetDeg;
	uint32_t m_lastDebounceTime;
	uint16_t m_degree;
	uint16_t m_minute;
	uint16_t m_seconds;

	float m_targetAngleDeg;

	Adafruit_Protomatter* m_matrix;

	Adafruit_NeoPixel* m_neoPixels;

	// Angle Encoder HA40+ -----------------------------------------------------
	Encoder m_ha40p;

	// Lock-style Solenoid -----------------------------------------------------
	Lock m_lock;

	uint8_t openSafe();
	uint8_t getAndDisplayAngles();


};
