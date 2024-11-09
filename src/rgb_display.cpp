#include "rgb_display.h"
#include "common.h"

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
// From: https://gist.github.com/davidegironi/3144efdc6d67e5df55438cc3cba613c8
uint16_t colorWheel(uint8_t pos) {
  if (pos < 85) {
    return display->color565(pos*3, 255 - pos*3, 0);
  } else if (pos < 170) {
    pos -= 85;
    return display->color565(255 - pos*3, 0, pos*3);
  } else {
    pos -= 170;
    return display->color565(0, pos*3, 255 - pos*3);
  }
}

void diplayInit() {
    HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
    HUB75_I2S_CFG mxconfig(
                            PANEL_WIDTH,    // width
                            PANEL_HEIGHT,   // height
                            CHAIN_LENGTH,   // chain length
                            _pins           // pin mapping
    );
    // mxconfig.latch_blanking = 4;
    // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
    // mxconfig.driver = HUB75_I2S_CFG::FM6124;
    mxconfig.clkphase = false;

  // Create matrix object
  display = new MatrixPanel_I2S_DMA(mxconfig);

  // Allocate memory and start DMA display
  if (not display->begin()) Serial.println("****** I2S memory allocation failed ***********");
  
  display->setPanelBrightness(BRIGHTNESS);
}

void logStatusMessage(const char *message) {
  ESP_LOGD("log_message", "%s", message);
  // Clear the last line first!
  display->fillRect(0, 56, 128, 8, 0);

  display->setTextSize(1);     // size 1 == 8 pixels high
  display->setTextWrap(false); // Don't wrap at end of line - will do ourselves

  display->setCursor(0, 56);   // Write on last line

  display->setTextColor(LOG_MESSAGE_COLOR);
  display->print(message);

  messageDisplayMillis = millis();
  logMessageActive = true;
}


void logStatusMessage(String message) {
  ESP_LOGD("log_message", "%s", message);
  // Clear the last line first!
  display->fillRect(0, 56, 128, 8, 0);

  display->setTextSize(1);     // size 1 == 8 pixels high
  display->setTextWrap(false); // Don't wrap at end of line - will do ourselves

  display->setCursor(0, 56);   // Write on last line

  display->setTextColor(display->color444(255,0,0));
  display->print(message);

  messageDisplayMillis = millis();
  logMessageActive = true;
}

void clearStatusMessage() {
   display->fillRect(0, 56, 128, 8, 0); 
   logMessageActive = false;
}

void displayForecastData() {

}
