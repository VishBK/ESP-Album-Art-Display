#include "config.h"
#include "common.h"
#include "rgb_display.h"

//Time of last status update
unsigned long lastStatusSend = 0;

//Time of last client.loop()
unsigned long lastLoop = 0;

// NTP
const uint8_t timeZone = 2;
const uint8_t minutesTimeZone = 0;
bool wifiFirstConnected = false;
//Current time
struct tm timeinfo;

// Flags to trigger display updates
bool clockStartingUp = true;

//Heartbeat marker
bool heartBeat = true;

//Log message persistence
//Is a log message currently displayed?
bool logMessageActive = false;
//When was the message shown?
unsigned long messageDisplayMillis = 0;

// Display
MatrixPanel_I2S_DMA* display = nullptr;
