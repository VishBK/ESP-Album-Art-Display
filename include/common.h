#ifndef COMMON_H
#define COMMON_H

#include <ESPNtpClient.h>

#include "rgb_display.h"


#define LED_BUILTIN 2

extern int status;

//Time of last status update
extern unsigned long lastStatusSend;

//Log message persistence
//Is a log message currently displayed?
extern bool logMessageActive;
//When was the message shown?
extern unsigned long messageDisplayMillis;

// NTP
extern bool wifiFirstConnected;

extern bool syncEventTriggered; // True if a time event has been triggered

//Current time and date
extern struct tm timeinfo;

//Flags to trigger display section updates
extern bool clockStartingUp;

//Just a heartbeat for the watchdog...
extern bool heartBeat;

//Weather data
extern uint8_t forecast5Days[5];
extern int8_t minTempToday;
extern int8_t maxTempToday;

//RGB display
extern MatrixPanel_I2S_DMA* display;

#endif

