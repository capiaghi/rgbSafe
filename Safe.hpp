#pragma once

#include <stdint.h>
#include <Adafruit_Protomatter.h>
#include "Encoder.hpp"
#include "Lock.hpp"


class Safe
{
public:
	Safe();
	uint8_t initialize(Adafruit_Protomatter* matrix);
	uint8_t checkCode();
  uint8_t accuracyGame();
	uint8_t setNewCode();

private:
	static constexpr float ANGLE_HYSTERESYS_DEG = 1.0f;        // Hysteresys of angle in degree
	static const uint32_t DEBOUNCE_DELAY_ENCODER = 1000;  // This time defines, how long the correct code must be present
	static const uint32_t SAFE_OPEN_TIME_MS = 5000;

	static const uint8_t CW = 0;    // Clock wise rotation
	static const uint8_t CCW = 1;   // Counter clock wise rotation

	static const uint8_t DIRECTION_CHANGED = 1;
	static const uint8_t DIRECTION_NOT_CHANGED = 0;

	static constexpr float STEP_SIZE = 10.0;

	static constexpr float  FIRST_CODE_ELEMENT = 90.0f;
	static constexpr float  SECOND_CODE_ELEMENT = 180.0f;
	static constexpr float  THIRD_CODE_ELEMENT = 270.0f;
	static const uint8_t    NUMBER_OF_CODE_ELEMENTS = 4;


	// Private types **************************************************************
	/// \brief Used States
	/// \details States for the main state machine TBD
	typedef enum stm_state_e
	{
		STM_STATE_SAFE_INIT,                   /// Offset
		STM_STATE_SAFE_FIRST_DIGIT,            /// First digit of the code
		STM_STATE_SAFE_SECOND_DIGIT,           /// Second digit of the code
		STM_STATE_SAFE_THIRD_DIGIT,            /// Third digit of the code
		STM_STATE_SAFE_FOURTH_DIGIT,           /// Forth digit of the code
		STM_STATE_SAFE_CHECK_CODE,             /// Check if the input was correct
	} stm_state_t;

	// Static variables ***********************************************************
	typedef unsigned char stm_bool_t;
	stm_state_t            stm_actState;    // Actual State variable
	stm_state_t            stm_newState;    // New State variable
	stm_bool_t             stm_entryFlag;   // Flag for handling the entry action
	stm_bool_t             stm_exitFlag;    // Flag for handling the exit action



	uint8_t m_errorCode;
	float m_angleDeg;
	float m_offset;
	uint8_t m_Values[NUMBER_OF_CODE_ELEMENTS];
	float m_lastAngleDeg;
	uint32_t m_lastDebounceTime;
	float m_code[NUMBER_OF_CODE_ELEMENTS];
	uint8_t codeNumber;
	uint8_t countDirection;

	Adafruit_Protomatter* m_matrix;

	// Angle Encoder HA40+ -----------------------------------------------------
	Encoder m_ha40p;

	// Lock-style Solenoid -----------------------------------------------------
	Lock m_lock;

	uint8_t openSafe();

	uint8_t showCode(uint8_t valuesToDisplay[]);
	uint8_t directionChanged();


};
