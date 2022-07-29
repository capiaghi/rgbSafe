
// ****************************************************************************
/// \file      Conifg.h
///
/// \brief     Configuration of the Board
///
/// \details    
///
/// \author    Christoph Capiaghi
///
/// \version   0.1
///
/// \date      20220718
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

// Prevent recursive inclusion
#pragma once 

// Include ********************************************************************

// Exported types *************************************************************
/// \brief     Optional description
/// \details   Optional description (more detailed)

//#define WOLFI ( 1 )

#define NO_BUTTON ( 1 )

#define UART_SPEED		( 115200 )
#define DEBUG			( 1 ) // Serial Debug enable

#define HEIGHT			( 32 ) // Matrix height (pixels) - SET TO 64 FOR 64x64 MATRIX!
#define WIDTH			( 32 ) // Matrix width (pixels)

// Rotation of the RGB Matrix
#define ROT0      ( 0 )
#define ROT90     ( 1 )
#define ROT180    ( 2 )
#define ROT270    ( 3 )

const uint8_t kMatrixWidth = 32;       // known working: 16, 32, 48, 64
const uint8_t kMatrixHeight = 32;       // known working: 32, 64, 96, 128

const uint8_t RC_OK = 0;
const uint8_t RC_INV_UART1_LENGTH = 1;
const uint8_t RC_INV_UART1_COMMAND = 2;
const uint8_t RC_INV_UART1_TIMEOUT = 3;


const uint8_t INVALID_CODE = 1;
const uint8_t CORRECT_CODE = 2;

const char SOFTWARE_VERSION[10] = "V1.0";

const uint8_t runTimeS_WelcomeScreen = 4;

// Define Pins  ************************************************************

// Pinout RGB Matrix
static uint8_t rgbPins[] = { 7, 8, 9, 10, 11, 12 };
static uint8_t addrPins[] = { 17, 18, 19, 20, 21 };
static uint8_t clockPin = 14;
static uint8_t latchPin = 15;
static uint8_t oePin = 16;

static const uint8_t TRIGGER_PIN = 23;   // A1
static const uint8_t BUTTON_ENTER_PIN = 24;	// A2
static const uint8_t LOCK_PIN = 25;	// A3

// Define Boolean Variable ************************************************************
#ifndef TRUE
#define TRUE  ( 1 )
#endif
#ifndef FALSE
#define FALSE ( !TRUE )
#endif


// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF
