#ifndef COMMON_H
#define COMMON_H

#include <ESPNtpClient.h>

#include "rgb_display.h"


#define LED_BUILTIN 2

// Current time and date
extern struct tm timeinfo;

// Flags to trigger display section updates
extern bool isClockStartingUp;

// Weather data
extern uint8_t forecast5Days[5];
extern int8_t minTempToday;
extern int8_t maxTempToday;

// RGB display
extern MatrixPanel_I2S_DMA* display;

#endif

