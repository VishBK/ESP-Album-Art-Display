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

#define NUM_LEDS PANEL_HEIGHT*PANEL_WIDTH

// Pixel types
enum {
    RGB565,
    RGB888,
    RGB8888
};

/**
 * @brief Gets a color value from a 0-255 range.
 * 
 * The colors are a transition from r - g - b - back to r. Used for cycling colors smoothly.
 * From: https://gist.github.com/davidegironi/3144efdc6d67e5df55438cc3cba613c8
 *
 * @param pos Color position input.
 */
uint16_t colorWheel(uint8_t pos);

/**
 * Initializes and starts the display.
 */
void diplayInit();

/**
 * @brief Draws a BMP image on the display.
 *
 * @param image A pointer to the image bytes.
 * @param pixelType Pixel compression type (565, 888, or 8888)
 * @param xpos X position to start drawing.
 * @param ypos Y position to start drawing.
 */
void drawBMP(uint8_t* image, uint8_t pixelType, uint8_t xpos, uint8_t ypos);

/**
 * @brief Draws a BMP image on the display by picking random pixels to draw.
 * 
 * @param image A pointer to the image bytes.
 * @param pixelType Pixel compression type (565, 888, or 8888)
 * @param transitionTime The length of time of the transition process in ms.
 * @param xpos X position to start drawing.
 * @param ypos Y position to start drawing.
 */
void drawBmpDissolve(uint8_t* image, uint8_t pixelType, uint16_t transitionTime, uint8_t xpos=0, uint8_t ypos=0);

/**
 * @brief Draws a solid color on the display by picking random pixels to draw.
 * 
 * @param color RGB 565 color to draw.
 * @param transitionTime The length of time of the transition process in ms.
 * @param xpos X position to start drawing.
 * @param ypos Y position to start drawing.
 */
void drawColorDissolve(uint16_t color, uint16_t transitionTime, uint8_t xpos=0, uint8_t ypos=0);

/**
 * @brief Changes brightness of the display by fading to the desired value.
 * 
 * @param curBrightness Starting brightness.
 * @param newBrightness Ending brightness.
 * @param fadeDuration The length of time of the fade in ms.
 */
void fadeBrightness(uint8_t curBrightness, uint8_t newBrightness, uint16_t fadeDuration=0);

/**
 * @brief Displays the text on the display.
 *
 * Displays the given text on the bottom of the display with no wrapping.
 * Text can be scrolled if too long. If scroll is used, must repeatedly call updateText() to marquee the text.
 * If wait is used, updateText() is not needed, but the function will only clear the text after TEXT_PERSISTANCE_MS ms,
 * holding up other functions. Should only be used for important messages.
 *
 * @param text The text to display.
 * @param scroll Whether to marquee the text when it is too long. Need to follow up with updateText().
 * @param wait Whether to wait for the text to finish instead of relying on updateText().
 */
void displayText(const char *text, bool scroll=false, bool wait=false);

/**
 * @brief Updates the text on the display.
 *
 * Only needs to be used if scrolling text without waiting with displayText().
 * Will move the scrolled text over after TEXT_PERSISTANCE_MS millseconds.
 */
void updateText();

#endif
