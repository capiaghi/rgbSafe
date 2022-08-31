// ****************************************************************************
/// \file      rgbSafe.ino
///
/// \brief     Main file for RGB Safe (Leica Event and Wolfi Present)
///
/// \details   Safe which contains some goodies, protected by a passcode.
///            RBG Matrix as display, HA40+ Encoder as safe wheel
///            Librarys for Adafruit Matrix Portal M4:
///            https://learn.adafruit.com/adafruit-matrixportal-m4/arduino-libraries
///
///            https://learn.adafruit.com/adafruit-protomatter-rgb-matrix-library/arduino-library
///            https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
///
///           void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
///           http://adafruit.github.io/Adafruit-GFX-Library/html/class_adafruit___g_f_x.html#a805a15f1b3ea9eff5d1666b8e6db1c56
///           Save via gimp -> c file
///
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

#include <Adafruit_Protomatter.h>
// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
//https://github.com/adafruit/Adafruit-GFX-Library/tree/master/Fonts
#include <Fonts/FreeSansBold18pt7b.h> // Large friendly font
#include <Fonts/Picopixel.h> // Large friendly font
#include <Adafruit_NeoPixel.h>
#include <arduino-timer.h>
#include "config.hpp"
#include "ButtonHandler.hpp"
#include "AccuracyGame.hpp"

// Include pictures
#include "pics/hexagon_28x32.c"
//https://github.com/moononournation/Arduino_GFX/blob/master/examples/ImgViewer/ImgViewerPROGMEM/ImgViewerPROGMEM.ino
#include "pics/greenSmiley_32x32.c"
#include "pics/redSmiley_32x32.c"


// Private types **************************************************************
/// \brief Used States
/// \details States for the main state machine TBD
typedef enum stm_state_e
{
	STM_STATE_STARTUP,               /// Startup of the safe
	STM_STATE_SAFE_MODE,             /// Checks the code input and opens safe
	STM_STATE_RESET_ENCODER_VAL,   /// Passthrough: UART1 <-> UART
} stm_state_t;

// Static variables ***********************************************************
typedef unsigned char stm_bool_t;
static stm_state_t            stm_actState;    // Actual State variable
static stm_state_t            stm_newState;    // New State variable
static stm_bool_t             stm_entryFlag;   // Flag for handling the entry action
static stm_bool_t             stm_exitFlag;    // Flag for handling the exit action

// Create a 32-pixel tall, 32 pixel wide matrix with the defined pins
Adafruit_Protomatter matrix(
	WIDTH, 4, 1, rgbPins, 4, addrPins, clockPin, latchPin, oePin, false);

// Neopixel strip
Adafruit_NeoPixel neoPixels(RGB_STRIP_NUMBER_OF_LEDS, RGB_STRIP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel onBoardNeoPixel(1, RGB_ONBOARD_LED_PIN, NEO_GRB + NEO_KHZ800);

// Accuracy Game
AccuracyGame accuracyGame;

// Button
ButtonHandler enterButton;

// Serial Interface to Encoder
SerialHandler serialHandler;

Timer<1, millis, Adafruit_NeoPixel *> rbgStripTimer; // 1 concurrent tasks, using millis as resolution

Timer<> openSafeTimer; // 1 concurrent tasks, using millis as resolution

uint32_t prevTime = 0; // Used for frames-per-second throttle
uint8_t errorCode = 0;

void setup() {

	// State Machine
	stm_entryFlag = TRUE;
	stm_exitFlag = FALSE;
	stm_newState = STM_STATE_STARTUP;
 
	// Serial: Interface to PC
	// Serial1: HA40+ Encoder 
  #ifdef DEBUG
	Serial.begin(UART_SPEED);
  #endif
	//while (!Serial); // wait for serial port to connect.
	Serial1.begin(UART_SPEED);

	// Initialize matrix
	ProtomatterStatus status = matrix.begin();
#ifdef DEBUG
	Serial.print("Protomatter begin() status: ");
	Serial.println((int)status);
#endif
	if (status != PROTOMATTER_OK) {
		for (;;);
	}
	matrix.fillScreen(BLACK);
  matrix.setRotation( ROT90 );
	matrix.show();

	serialHandler.initialize();
	serialHandler.enableRs485Mode();

	enterButton.initialize();

	neoPixels.begin();
	neoPixels.setBrightness(100);
	neoPixels.show(); // Initialize all pixels to 'off'

  onBoardNeoPixel.begin();
  onBoardNeoPixel.setBrightness(5);
  onBoardNeoPixel.setPixelColor(0, onBoardNeoPixel.Color(0, 255, 0));
  onBoardNeoPixel.show(); // Initialize all pixels to 'off'

#ifdef SHOW_HTC
  showHTCRules();
#endif
  
  errorCode = accuracyGame.initialize(&matrix, &serialHandler, &neoPixels, &rbgStripTimer);

  if (errorCode != 0) errorHandler();

#ifdef DEBUG
	Serial.println("Init complete");
#endif



	stm_actState = STM_STATE_STARTUP;

}
void loop()
{
  rbgStripTimer.tick(); // tick timer
  openSafeTimer.tick();
  // Limit the animation frame rate to MAX_FPS.  Because the subsequent sand
  // calculations are non-deterministic (don't always take the same amount
  // of time, depending on their current states), this helps ensure that
  // things constant in the simulation.
  uint32_t t;
  while(((t = micros()) - prevTime) < (1000000L / MAX_FPS));
  prevTime = t;

  
	// Check button state
#ifndef NO_BUTTON
	enterButton.update();
#endif


	switch (stm_actState)
	{
		//==============================================================================
		// STM_STATE_STARTUP
		//==============================================================================
	case STM_STATE_STARTUP:
		/// - Path 1 \n
		/// Startup:
		/// Initialize Thermocouples
		/// Check for Errors

		// Entry action
		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_STARTUP"));
			Serial.println("rgbSafe");
			Serial.print("Software Version: ");
			Serial.println(SOFTWARE_VERSION);
#endif

			matrix.fillScreen(BLACK);
			matrix.drawRGBBitmap(0, 0, (const uint16_t*)hexagon_28x32, 28, 32);
			matrix.show();

			delay(DISPLAY_LOGO_MS);

			stm_newState = STM_STATE_SAFE_MODE;
			stm_entryFlag = FALSE;
			stm_exitFlag = TRUE;
		}

		// Exit
		if (stm_exitFlag == TRUE)
		{
			//clearScreen();
			stm_exitFlag = FALSE;
			stm_actState = stm_newState;
			stm_entryFlag = TRUE;
		}
		break;
///-------------------------------------------
	case STM_STATE_SAFE_MODE:
		// Entry action
		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_SAFE_MODE"));
#endif
      openSafeTimer.cancel();
			stm_entryFlag = FALSE;
		}

		errorCode = accuracyGame.run();

    if (errorCode != 0) errorHandler();

		if (enterButton.getEnterButtonState())
		{
			stm_newState = STM_STATE_RESET_ENCODER_VAL;
			stm_exitFlag = TRUE;
		}
		// Exit
		if (stm_exitFlag == TRUE)
		{
			//clearScreen();
			stm_exitFlag = FALSE;
			stm_actState = stm_newState;
			stm_entryFlag = TRUE;
		}
		break;
    
///-------------------------------------------
	case STM_STATE_RESET_ENCODER_VAL:
		// Entry action
		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_RESET_ENCODER_VAL"));
#endif
			stm_entryFlag = FALSE;
			//stm_exitFlag = TRUE;
		}
    accuracyGame.reset();
    
    openSafeTimer.in(EMERGENCY_SAFE_OPEN_MS, changeState);

    if (enterButton.getEnterButtonState())
    {
      accuracyGame.openSafe();
    }

		// Exit
		if (stm_exitFlag == TRUE)
		{
			//clearScreen();
			stm_exitFlag = FALSE;
			stm_actState = stm_newState;
			stm_entryFlag = TRUE;
		}
		break;
		//==============================================================================
		// DEFAULT
		//==============================================================================
	default:

		break;
	}

}

bool changeState(void *)
{
    stm_newState = STM_STATE_SAFE_MODE;
    stm_exitFlag = TRUE;
  return true;
}

void errorHandler() {
  onBoardNeoPixel.setPixelColor(0, onBoardNeoPixel.Color(255, 0, 0));
  onBoardNeoPixel.show(); // Initialize all pixels to 'off'
}

void showHTCRules()
{
  char sensVal[50];
  char     str[50];                // Buffer to hold scrolling message text
  int16_t  textX = matrix.width(), // Current text position (X)
  textY,                            // Current text position (Y)
  textMin,                         // Text pos. (X) when scrolled off left edge
  hue = 0;

 // Set up the scrolling message...
  sprintf(str, "HTC Rules!"); 
  matrix.setFont(&FreeSansBold18pt7b);  // Use nice bitmap font
  matrix.setTextWrap(false);            // Allow text off edge
  matrix.setTextColor(WHITE);           // White

  int16_t  x1, y1;
  uint16_t w, h;
  matrix.getTextBounds(str, 0, 0, &x1, &y1, &w, &h); // How big is it?
  textMin = -w; // All text is off left edge when it reaches this point
  textY = matrix.height() / 2 - (y1 + h / 2); // Center text vertically
  // Note: when making scrolling text like this, the setTextWrap(false)
  // call is REQUIRED (to allow text to go off the edge of the matrix),
  // AND it must be BEFORE the getTextBounds() call (or else that will
  // return the bounds of "wrapped" text).
  // Example from doublebuffer_scrolltext

	float framesPerSecond = 50.0;
	uint32_t delayInMs = round(1000.0 / framesPerSecond);
	uint32_t counter = round(runTimeS_WelcomeScreen * framesPerSecond);


	for (uint32_t i = 0; i < counter; i++)
	{
		matrix.fillScreen(BLACK);
		matrix.setTextColor(WHITE);
		matrix.setCursor(textX, textY);
		matrix.print(str);
		// Update text position for next frame. If text goes off the
		// left edge, reset its position to be off the right edge.
		if ((--textX) < textMin) textX = matrix.width();
		matrix.show();
		delay(delayInMs); // 20 milliseconds = ~50 frames/second
	}
}
