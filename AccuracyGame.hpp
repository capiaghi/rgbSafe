#pragma once

#include <stdint.h>
#include "Safe.hpp"
#include <arduino-timer.h>

class AccuracyGame
{
public:
	AccuracyGame();
	uint8_t initialize(Safe* safe);
	uint8_t run();
  void    reset();
  uint8_t openSafe();

private:

  // Two different levels: Kids (easy) 300 Degree target, hard: 100 Degree target
  // Hysteresys of angle in degree / min and seconds
  static constexpr float TARGET_ANGLE = 100.0;
  static constexpr float ANGLE_HYSTERESYS_DEG_SETTING = 0.0;  /// Angle hyst in deg
  static constexpr float ANGLE_HYSTERESYS_MIN_SETTING = 1.0;  /// Angle hyst in min
  static constexpr float ANGLE_HYSTERESYS_SEC_SETTING = 0.0;  /// Angle hyst in sec
  static constexpr float ANGLE_HYSTERESYS_DEG = ANGLE_HYSTERESYS_DEG_SETTING +  (ANGLE_HYSTERESYS_MIN_SETTING / 60.0) + (ANGLE_HYSTERESYS_SEC_SETTING / 3600); /// Hysteresys of angle in degree

  // Hysteresys of angle in degree / min and seconds
  static constexpr float TARGET_ANGLE_KIDS = 300;
  static constexpr float ANGLE_HYSTERESYS_DEG_SETTING_KIDS = 1.0;  /// Angle hyst in deg
  static constexpr float ANGLE_HYSTERESYS_MIN_SETTING_KIDS = 0.0;  /// Angle hyst in min
  static constexpr float ANGLE_HYSTERESYS_SEC_SETTING_KIDS = 0.0;  /// Angle hyst in sec
  static constexpr float ANGLE_HYSTERESYS_DEG_KIDS = ANGLE_HYSTERESYS_DEG_SETTING_KIDS +  (ANGLE_HYSTERESYS_MIN_SETTING_KIDS / 60.0) + (ANGLE_HYSTERESYS_SEC_SETTING_KIDS / 3600); /// Hysteresys of angle in degree
  
	static const uint32_t DEBOUNCE_DELAY_ENCODER_MS = 5000;		/// This time defines, how long the correct code must be present
	static const uint32_t SAFE_OPEN_TIME_MS = 5000;           /// How long is the safe open (coil)

  static constexpr float BAR_GRAPH_RESOLUTION_DEG = 1.0;      /// Green bar graph resolution in degree
  static constexpr float BAR_GRAPH_RESOLUTION_KIDS_DEG = 5.0;      /// Green bar graph resolution in degree

  static const uint32_t RGB_STRIP_ILLUMINATION_TIME_S = 15; /// How long should be the led in the safe on in seconds

	// Private types **************************************************************
	/// \brief Used States
	/// \details States for the main state machine
	typedef enum stm_state_e
	{
		STM_STATE_ACCURACY_GAME_INIT,               /// Init Accurracy game
		STM_STATE_ACCURACY_GAME_CHECK_VALUE,        /// Check encoder values
		STM_STATE_ACCURACY_GAME_IN_TOLERANCE,       /// Reached target angle within a tolerance (Master Level)
    STM_STATE_ACCURACY_GAME_IN_TOLERANCE_KIDS,  /// Reached target angle within a tolerance (Kids Level)
		STM_STATE_ACCURACY_GAME_OVER,               /// You messed it up -> Game over
		STM_STATE_ACCURACY_GAME_WIN                 /// You did it. 
	} stm_state_t;

	// Static variables ***********************************************************
	typedef unsigned char stm_bool_t;
	stm_state_t            stm_actState;    /// Actual State variable
	stm_state_t            stm_newState;    /// New State variable
	stm_bool_t             stm_entryFlag;   /// Flag for handling the entry action
	stm_bool_t             stm_exitFlag;    /// Flag for handling the exit action

	uint8_t m_errorCode;
	float m_angleDeg;
	float m_offsetDeg;
	uint32_t m_lastDebounceTime;
	uint16_t m_degree;
	uint16_t m_minute;
	uint16_t m_seconds;

	float m_targetAngleDeg;
  float m_targetAngleDegKids;

	Safe* m_safe;


};
