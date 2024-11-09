#ifndef RGB_DISPLAY_H
#define RGB_DISPLAY_H

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "config.h"

// Pins for Codetastic's LED Matrix
#define R1_PIN 23
#define B1_PIN 22
#define R2_PIN 21
#define B2_PIN 19
#define A_PIN 18
#define C_PIN 17
#define CLK_PIN 16
#define OE_PIN 4

#define G1_PIN 32
#define G2_PIN 33
#define E_PIN 25
#define B_PIN 26
#define D_PIN 27
#define LAT_PIN 14

#define NUM_LEDS MATRIX_HEIGHT*MATRIX_WIDTH

uint16_t colorWheel(uint8_t pos); 
//void drawText(int colorWheelOffset);
void diplayInit();
//void display_drawText();
void logStatusMessage(const char *message);
void logStatusMessage(String message);
void clearStatusMessage();

#endif
