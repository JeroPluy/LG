/*
  Display.cpp - Library for a OLED SSD1306 Display.
  Created by Jero A. February 13. 2020.
  Released into the public domain.
*/

#ifndef Display_h
#define Display_h

#include "Arduino.h"
#include "Color.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

		class Display
		{
			public:
			  Display();
			  void startScreen();
			  void startMenu();
			  void displayLogo();
			  void countDown();	
			  void textWithNumber(char text1[], char text2[], short number);
			  void endScreen(short hitCounter);
			private :
			  const unsigned char _thLogo[];  
		};
#endif /* Display_h */