#include "config.h"
#include "common.h"
#include "rgb_display.h"


// Current time
struct tm timeinfo;

// Flags to trigger display updates
bool isClockStartingUp = true;

// Display
MatrixPanel_I2S_DMA* display = nullptr;
