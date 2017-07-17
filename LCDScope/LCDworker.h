#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 6

#define BLACK HX8357_BLACK
#define WHITE HX8357_WHITE
#define YELLOW HX8357_YELLOW
#define RED HX8357_RED
#define GREEN HX8357_GREEN

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// SoftSPI - note that on some processors this might be *faster* than hardware SPI!
// USE SoftSPI FOR FEATHER M0!!
//Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, MOSI, SCK, TFT_RST, MISO);
int topy = 17;
//int boty = tft.height()-22;
int boty = 320-22;
int leftx = 50;
const char* trigSpell[] = 
     {
     "FREE",
     "PLUS",
     "NEG"};


