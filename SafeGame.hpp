#pragma once

#include <stdint.h>
#include "Safe.hpp"
#include <arduino-timer.h>

class SafeGame
{
public:
  SafeGame();
  uint8_t initialize(Safe *safe);
  void reset();
  uint8_t run();

  static const uint8_t  UNDEFINED_CODE_ELEMENT    = 255;

private:

  static constexpr float ANGLE_HYSTERESYS_DEG = 1.0f;        // Hysteresys of angle in degree
  static const uint32_t DEBOUNCE_DELAY_ENCODER = 1000;  // This time defines, how long the correct code must be present
  static const uint32_t SAFE_OPEN_TIME_MS = 5000;

  static const uint8_t CW = 0;    // Clock wise rotation
  static const uint8_t CCW = 1;   // Counter clock wise rotation

  static const uint8_t DIRECTION_CHANGED = 1;
  static const uint8_t DIRECTION_NOT_CHANGED = 0;

 
  static const uint8_t NUMBER_OF_CODE_DISK_ELEMENTS = 10;               /// Elements of the physical code disk, 0 ... 9
  static constexpr float STEP_SIZE = 360.0 / NUMBER_OF_CODE_DISK_ELEMENTS;
  static constexpr float TOLERANCE = STEP_SIZE/2.0 - STEP_SIZE*0.1; // 10 % of step size

  static const uint8_t FIRST_CODE_ELEMENT        = 1;    /// Secret code, first digit
  static const uint8_t SECOND_CODE_ELEMENT       = 9;    /// Secret code, second digit
  static const uint8_t THIRD_CODE_ELEMENT        = 5;    /// Secret code, third digit
  static const uint8_t FOURTH_CODE_ELEMENT       = 8;    /// Secret code, forth digit
  
  static const uint8_t NUMBER_OF_CODE_ELEMENTS   = 4;    /// Number of code digits



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
    STM_STATE_SAFE_CORRECT_CODE,           /// Code was correct
    STM_STATE_SAFE_WRONG_CODE,             /// Invalid code
  } stm_state_t;

  // Static variables ***********************************************************
  typedef unsigned char stm_bool_t;
  stm_state_t            stm_actState;    // Actual State variable
  stm_state_t            stm_newState;    // New State variable
  stm_bool_t             stm_entryFlag;   // Flag for handling the entry action
  stm_bool_t             stm_exitFlag;    // Flag for handling the exit action



  uint8_t m_errorCode;
  float m_angleDeg;
  float m_angleDegLast;
  float m_offsetDeg;
  float m_currentCodeElementAngle;
  uint8_t m_currentCode[NUMBER_OF_CODE_ELEMENTS];
  uint8_t m_correctCode[NUMBER_OF_CODE_ELEMENTS];
  uint8_t m_currentCodeElement;
  uint8_t m_lastCodeElement;
  uint8_t m_currentDigit;
  int8_t m_sign;
  bool m_codeValid;
  bool m_code_found;

  
  // RGB Matrix --------------------------------------------------------------
  Safe* m_safe;

  


};
