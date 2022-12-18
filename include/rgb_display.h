#ifndef RGB_DISPLAY_H
#define RGB_DISPLAY_H

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "config.h"

// Pins for Faptastic's LED Matrix
#define R1_PIN 15
#define G1_PIN 13
#define B1_PIN 2
#define R2_PIN 4
#define G2_PIN 12
#define B2_PIN 16   // RX2
#define E_PIN 14
#define A_PIN 17    // TX2
#define B_PIN 27
#define C_PIN 5
#define D_PIN 26
#define CLK_PIN 18
#define LAT_PIN 25
#define OE_PIN 19

#define NUM_LEDS MATRIX_HEIGHT*MATRIX_WIDTH

uint16_t colorWheel(uint8_t pos); 
//void drawText(int colorWheelOffset);
void diplayInit();
//void display_drawText();
void logStatusMessage(const char *message);
void logStatusMessage(String message);
void clearStatusMessage();

#endif
