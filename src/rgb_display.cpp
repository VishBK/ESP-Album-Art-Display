#include <algorithm>
#include <random>

#include "rgb_display.h"
#include "common.h"

bool isTextDisplayed = false;   // Is a text currently displayed?
bool isScrollText = false;      // Is text currently scrolling?
int16_t text_x = 0;     // Scrolling text current x position
String textOnDisplay;   // The last text shown
unsigned long textDisplayMs = 0; // When was the last text shown?

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
    HUB75_I2S_CFG::i2s_pins _pins={
        R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN
    };
    HUB75_I2S_CFG mxconfig(
        PANEL_WIDTH,    // width
        PANEL_HEIGHT,   // height
        CHAIN_LENGTH,   // chain length
        _pins           // pin mapping
    );
    mxconfig.clkphase = false;

    // Create matrix object
    display = new MatrixPanel_I2S_DMA(mxconfig);

    // Allocate memory and start DMA display
    if (not display->begin()) Serial.println("****** I2S memory allocation failed ***********");

    display->setPanelBrightness(BRIGHTNESS);
}

void fadeBrightness(uint8_t curBrightness, uint8_t newBrightness, uint16_t fadeDuration) {
    int8_t brightnessDif = (curBrightness-newBrightness);
    int8_t brightnessDir = (brightnessDif < 0) - (brightnessDif > 0);
    uint16_t delayDuration = (fadeDuration != 0) ? (fadeDuration / abs(brightnessDif)) : 1;
    while (curBrightness != newBrightness) {
        curBrightness += brightnessDir;
        display->setPanelBrightness(curBrightness);
        delay(delayDuration);
    }
}

void displayText(const char *text, bool scroll, bool wait) {
    ESP_LOGD("displayText", "%s", text);
    textOnDisplay = text;
    isScrollText = scroll;
    isTextDisplayed = true;

    uint8_t size = 1;

    display->setTextSize(size);   // size 1 == 8 pixels high
    display->setTextWrap(false);  // Don't wrap at end of line - will do ourselves
    display->setTextColor(TEXT_COLOR);

    display->fillRect(0, PANEL_HEIGHT-size*8, PANEL_WIDTH, size*8, 0);
    if (!scroll) {
        isTextDisplayed = true;
        display->setCursor(0, PANEL_HEIGHT-size*8);   // Write on last line
    } else {
        // display->setCursor(text_x, PANEL_HEIGHT-size*8);
    }
    display->setCursor(0, PANEL_HEIGHT-size*8);
    display->print(text);        
    textDisplayMs = millis();

    if (wait) {
        delay(TEXT_PERSISTENCE_MS);
        while (isScrollText) {
            updateText();
            delay(50);
        }
    }
}

void updateText() {
    if (isTextDisplayed && (millis() > textDisplayMs + TEXT_PERSISTENCE_MS)) {    
        uint8_t size = 1;
        display->fillRect(0, PANEL_HEIGHT-size*8, PANEL_WIDTH, size*8, 0);

        if (isScrollText) {
            uint8_t len = textOnDisplay.length()*6;
            text_x--;
            if (text_x + len == 0) {
                isScrollText = false;
                isTextDisplayed = false;
                return;
            }
            display->setCursor(text_x, PANEL_HEIGHT-size*8);
            display->print(textOnDisplay.c_str());
        } else {
            isTextDisplayed = false;
        }
    }
}

void drawBMP(uint8_t* image, uint8_t pixelType, uint8_t xpos, uint8_t ypos) {
    ESP_LOGI("drawBMP", "Generating Image...");
    uint8_t increment = 0;
    switch(pixelType) {
        case RGB565:    // Increment 2 for (r=5, g=6, b=5)
            increment = 2;
            break;
        case RGB888:    // Increment 3 for (r=8, g=8, b=8)
            increment = 3;
            break;
        case RGB8888:   // Increment 4 for (r=8, g=8, b=8, a=8)
            increment = 4;
            break;
        default:
            ESP_LOGE("drawBMP", "Invalid pixel type");
            return;
    }

    uint8_t* curPixel = image;
    for (uint8_t y = ypos; y < PANEL_HEIGHT; y++) {
        for (uint8_t x = xpos; x < PANEL_WIDTH; x++) {
            uint8_t r = curPixel[0];
            uint8_t g = curPixel[1];
            uint8_t b = curPixel[2];
            if (pixelType == RGB565) {
                uint16_t color = curPixel[0]<<8 | curPixel[1];
                display->color565to888(color, r, g, b);
            }

             display->drawPixelRGB888(x, y, r, g, b);

            curPixel += increment;
        }
    }
    ESP_LOGI("drawBMP", "Done generating");
}

void drawBmpDissolve(uint8_t* image, uint8_t pixelType, uint16_t transitionTime, uint8_t xpos, uint8_t ypos) {
    ESP_LOGI("drawBmpDissolve", "Generating Image...");
    uint8_t increment = 0;
    switch(pixelType) {
        case RGB565:    // Increment 2 for (r=5, g=6, b=5)
            increment = 2;
            break;
        case RGB888:    // Increment 3 for (r=8, g=8, b=8)
            increment = 3;
            break;
        case RGB8888:   // Increment 4 for (r=8, g=8, b=8, a=8)
            increment = 4;
            break;
        default:
            ESP_LOGE("drawBmpDissolve", "Invalid pixel type");
            return;
    }

    // Create shuffled pixel coordinates for dissolve effect
    uint16_t* pixelCoords = (uint16_t*) malloc(NUM_LEDS * sizeof(uint16_t));
    for (uint16_t i=0; i < NUM_LEDS; ++i) {
        pixelCoords[i] = i;
    }
    std::shuffle(&pixelCoords[0], &pixelCoords[NUM_LEDS], std::mt19937{std::random_device{}()});

    uint32_t loop_delay = (transitionTime*1000) / (NUM_LEDS);
    ESP_LOGD("drawBmpDissolve", "delay: %d", loop_delay);
    for (uint16_t i=0; i < NUM_LEDS; ++i) {
        unsigned long cur_ms = micros();
        uint16_t curCoord = pixelCoords[i];
        uint8_t x = curCoord % PANEL_WIDTH;
        uint8_t y = curCoord / PANEL_HEIGHT;

        uint32_t curPixel = curCoord*increment;
        uint8_t r = image[curPixel];
        uint8_t g = image[curPixel+1];
        uint8_t b = image[curPixel+2];
        if (pixelType == RGB565) {
            uint16_t color = image[curPixel]<<8 | image[curPixel+1];
            display->color565to888(color, r, g, b);
        }

        // ESP_LOGD("drawBmpDissolve", "Drawing (%d, %d, %d) at (%d, %d)", r, g, b, x, y);
        display->drawPixelRGB888(x, y, r, g, b);

        unsigned long time_spent = micros() - cur_ms;
        if (time_spent < loop_delay) {
           esp_rom_delay_us(loop_delay-time_spent); 
        }
    }
    free(pixelCoords);
    ESP_LOGI("drawBmpDissolve", "Done generating");
}

void drawColorDissolve(uint16_t color, uint16_t transitionTime, uint8_t xpos, uint8_t ypos) {
    ESP_LOGI("drawColorDissolve", "Generating color 0x%x", color);

    // Create shuffled pixel coordinates for dissolve effect
    uint16_t* pixelCoords = (uint16_t*) malloc(NUM_LEDS * sizeof(uint16_t));
    for (uint16_t i=0; i < NUM_LEDS; ++i) {
        pixelCoords[i] = i;
    }
    std::shuffle(&pixelCoords[0], &pixelCoords[NUM_LEDS], std::mt19937{std::random_device{}()});

    uint32_t loop_delay = (transitionTime*1000) / (NUM_LEDS);
    ESP_LOGD("drawColorDissolve", "delay: %d", loop_delay);
    for (uint16_t i=0; i < NUM_LEDS; ++i) {
        unsigned long cur_ms = micros();
        uint8_t x = pixelCoords[i] % PANEL_WIDTH;
        uint8_t y = pixelCoords[i] / PANEL_HEIGHT;

        display->drawPixel(x, y, color);

        unsigned long time_spent = micros() - cur_ms;
        if (time_spent < loop_delay) {
           esp_rom_delay_us(loop_delay-time_spent); 
        }
    }
    free(pixelCoords);
    ESP_LOGI("drawColorDissolve", "Done generating");
}

void displayForecastData() {
    // TODO: Display weather data
}
