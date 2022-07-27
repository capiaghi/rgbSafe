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
#include <Adafruit_SPIFlash.h>
#include <Adafruit_TinyUSB.h>
#include <SPI.h>
#include "config.hpp"
#include "ButtonHandler.hpp"
#include "Safe.hpp"

// Include pictures
#include "hexagon.c"


// Private types **************************************************************
/// \brief Used States
/// \details States for the main state machine TBD
typedef enum stm_state_e
{
	STM_STATE_STARTUP,               /// Startup of the safe
	STM_STATE_SAFE_MODE,             /// Checks the code input and opens safe
	STM_STATE_SERIAL_PASS_THROUGH,   /// Passthrough: UART1 <-> UART
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

// SafeClass
Safe safe;

// Button
ButtonHandler enterButton;

char sensVal[50];
char     str[50];                // Buffer to hold scrolling message text
int16_t  textX = matrix.width(), // Current text position (X)
textY,                  // Current text position (Y)
textMin,                // Text pos. (X) when scrolled off left edge
hue = 0;

// FLASH FILESYSTEM STUFF --------------------------------------------------

// External flash macros for QSPI or SPI are defined in board variant file.
#if defined(EXTERNAL_FLASH_USE_QSPI)
Adafruit_FlashTransport_QSPI flashTransport;
#elif defined(EXTERNAL_FLASH_USE_SPI)
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS,
	EXTERNAL_FLASH_USE_SPI);
#else
#error No QSPI/SPI flash are defined in your board variant.h!
#endif

Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem filesys;     // Filesystem object from SdFat

Adafruit_USBD_MSC usb_msc; // USB mass storage object


// FUNCTIONS REQUIRED FOR USB MASS STORAGE ---------------------------------
static bool msc_changed = true; // Is set true on filesystem changes
// Callback on READ10 command.
int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
	return flash.readBlocks(lba, (uint8_t*)buffer, bufsize / 512) ? bufsize : -1;
}

// Callback on WRITE10 command.
int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
	digitalWrite(LED_BUILTIN, HIGH);
	return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback on WRITE10 completion.
void msc_flush_cb(void) {
	flash.syncBlocks();   // Sync with flash
	filesys.cacheClear(); // Clear filesystem cache to force refresh
	digitalWrite(LED_BUILTIN, LOW);
	msc_changed = true;
}

// Get number of files in a specified path that match extension ('filter').
// Pass in absolute path (e.g. "/" or "/gifs") and extension WITHOUT period
// (e.g. "gif", NOT ".gif").
int16_t numFiles(const char* path, const char* filter) {
	File dir = filesys.open(path);
	if (!dir) return -1;
	char filename[256];
	for (int16_t num_files = 0;;) {
		File entry = dir.openNextFile();
		if (!entry) return num_files; // No more files
		entry.getName(filename, sizeof(filename) - 1);
		entry.close();
		if (!entry.isDirectory() &&       // Skip directories
			strncmp(filename, "._", 2)) { // and Mac junk files
			char* extension = strrchr(filename, '.');
			if (extension && !strcasecmp(&extension[1], filter)) num_files++;
		}
	}
	return -1;
}

// Return name of file (matching extension) by index (0 to numFiles()-1)
char* filenameByIndex(const char* path, const char* filter, int16_t index) {
	static char filename[256]; // Must be static, we return a pointer to this!
	File entry, dir = filesys.open(path);
	if (!dir) return NULL;
	while (entry = dir.openNextFile()) {
		entry.getName(filename, sizeof(filename) - 1);
		entry.close();
		if (!entry.isDirectory() &&       // Skip directories
			strncmp(filename, "._", 2)) { // and Mac junk files
			char* extension = strrchr(filename, '.');
			if (extension && !strcasecmp(&extension[1], filter)) {
				if (!index--) {
					return filename;
				}
			}
		}
	}
	return NULL;
}



void setup() {

	// State Machine
	stm_entryFlag = TRUE;
	stm_exitFlag = FALSE;
	stm_newState = STM_STATE_STARTUP;

	// USB mass storage / filesystem setup (do BEFORE Serial init)
	flash.begin();
	// Set disk vendor id, product id and revision
	usb_msc.setID("Adafruit", "External Flash", "1.0");
	// Set disk size, block size is 512 regardless of spi flash page size
	usb_msc.setCapacity(flash.pageSize() * flash.numPages() / 512, 512);
	usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
	usb_msc.setUnitReady(true); // MSC is ready for read/write
	usb_msc.begin();
	filesys.begin(&flash); // Start filesystem on the flash

	// Serial: Interface to PC
	// Serial1: HA40+ Encoder 
	Serial.begin(UART_SPEED);
	Serial1.begin(UART_SPEED);

	// Initialize matrix
	ProtomatterStatus status = matrix.begin();
	Serial.print("Protomatter begin() status: ");
	Serial.println((int)status);
	if (status != PROTOMATTER_OK) {
		for (;;);
	}
	matrix.fillScreen(0);
	matrix.show();

	enterButton.initialize();
	safe.initialize(&matrix);

	delay(1000);

	// Set up the scrolling message...
	sprintf(str, "HTC Rules!");
	matrix.setFont(&FreeSansBold18pt7b); // Use nice bitmap font
	matrix.setTextWrap(false);           // Allow text off edge
	matrix.setTextColor(WHITE);         // White
  matrix.setRotation( ROT90 );

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


	//void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
	// http://adafruit.github.io/Adafruit-GFX-Library/html/class_adafruit___g_f_x.html#a805a15f1b3ea9eff5d1666b8e6db1c56
	// Save via gimp -> c file
	//matrix.drawRGBBitmap(0, 0, (const uint16_t *)hexagonLogo.pixel_data, hexagonLogo.width, hexagonLogo.height);
	//matrix.show();

  //https://learn.adafruit.com/adafruit-protomatter-rgb-matrix-library/arduino-library
  //https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
	float angleTest = 0.0;
	//matrix.println("TEST");
	//matrix.println(angleTest);
	//matrix.show();

	Serial.println("Init complete");
	showHTCRules();

	stm_actState = STM_STATE_STARTUP;

}
void loop()
{

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
#endif
			Serial.println("rgbSafe");
			Serial.print("Software Version: ");
			Serial.println(SOFTWARE_VERSION);

			//matrix.drawRGBBitmap(0, 0, (const uint16_t*)hexagonLogo.pixel_data, hexagonLogo.width, hexagonLogo.height);
			//matrix.show();

			delay(2000);

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

	case STM_STATE_SAFE_MODE:
		// Entry action
		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_SAFE_MODE"));
#endif
			stm_entryFlag = FALSE;
			//stm_exitFlag = TRUE;
		}


    #ifdef WOLFI
		if (safe.checkCode() == CORRECT_CODE) {} //todo
    #else
    safe.accuracyGame();
    #endif

    


		if (enterButton.getEnterButtonState())
		{
			stm_newState = STM_STATE_SERIAL_PASS_THROUGH;
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

	case STM_STATE_SERIAL_PASS_THROUGH:
		// Entry action
		if (stm_entryFlag == TRUE)
		{
#ifdef DEBUG
			Serial.println(F("Entered STM_STATE_SERIAL_PASS_THROUGH"));
#endif
			stm_entryFlag = FALSE;
			//stm_exitFlag = TRUE;
		}

		SerialPassthrough();

		if (enterButton.getEnterButtonState())
		{
			stm_newState = STM_STATE_STARTUP;
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
		//==============================================================================
		// DEFAULT
		//==============================================================================
	default:

		break;
	}

}

void showHTCRules()
{
	matrix.setFont(&FreeSansBold18pt7b);  // Use nice bitmap font
	matrix.setTextWrap(false);            // Allow text off edge
	matrix.setTextColor(WHITE);           // White
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

void SerialPassthrough()
{
	if (Serial.available()) {         // If anything comes in Serial (USB),
		Serial1.write(Serial.read());   // read it and send it out Serial1
	}

	if (Serial1.available()) {     // If anything comes in Serial1
		Serial.write(Serial1.read());   // read it and send it out Serial (USB)
	}
}
