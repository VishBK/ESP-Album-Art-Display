// Displays album artwork of currently playing song from your LastFM account

// #include <HTTPClient.h>
#include <ESPNtpClient.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP_Color.h>
#include <SPI.h>
#include <TimeLib.h>
#include <Ticker.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <BH1750.h>

#include "LastFMClient.h"
#include "common.h"
#include "clock.h"
#include "rgb_display.h"
// #include "weather.h"
// #include "colors.h"
// #include "ntp_time.h"


// Define Tasks for Cores
TaskHandle_t Task1;

String currentImageUrl = "";
long lastUpdate = 0;
long lastScrobbling = 0;    // Last time isPlaying was true

Ticker displayTicker;
uint8_t prevHour;
unsigned long prevEpoch;
unsigned long lastNTPUpdate;
unsigned long lastWeatherUpdate;
unsigned long lastLuxUpdate;

bool blinkOn;
bool isClock = true; // Whether the clock should be displayed or not

// Wifi settings
String LASTFM_KEY = API_KEY;
String LASTFM_USERNAME = USERNAME;

// Globals
long callFrequency = 5000;  // How often to check LastFM in ms
long stopDuration = 1000;   // When to stop displaying cover art after stopping scrobbling in ms
long lightCheck = 1000;     // How often to check for ambient light to adjust the display brightness
uint8_t lastBrightness = 0;

LastFMClient client(LASTFM_USERNAME, LASTFM_KEY);

void displayUpdater();

uint8_t* prevImage = nullptr;

BH1750 lightMeter;

// Fades brightness to new brightness value of display
void fadeBrightness(uint8_t curBrightness, uint8_t newBrightness, uint16_t fadeDuration=0) {
    int8_t brightnessDif = (curBrightness-newBrightness);
    int8_t brightnessDir = (brightnessDif < 0) - (brightnessDif > 0);
    uint16_t delayDuration = (fadeDuration != 0) ? (fadeDuration / abs(brightnessDif)) : 1;
    while (curBrightness != newBrightness) {
        curBrightness += brightnessDir;
        display->setPanelBrightness(curBrightness);
        delay(delayDuration);
    }
}

// Draws a bitmap
void drawXbm565(int x, int y, int width, int height, const char *xbm, uint16_t color=0xffff) {
    if (width % 8 != 0) {
        width = ((width / 8) + 1) * 8;
    }
    for (int i = 0; i < width * height / 8; i++ ) {
      unsigned char charColumn = pgm_read_byte(xbm + i);
      for (int j = 0; j < 8; j++) {
        int targetX = (i * 8 + j) % width + x;
        int targetY = (8 * i / (width)) + y;
        if (bitRead(charColumn, j)) {
          display->drawPixel(targetX, targetY, color);
        }
      }
    }
}

// Draws the bitmap using RGB565
void drawBMP_565(uint8_t* image, uint16_t xpos, uint16_t ypos) {
    display->clearScreen();
    ESP_LOGI("draw", "Generating Image...");

    uint8_t* curPixel = image;
    uint8_t* prevPixel = prevImage;
    uint16_t counter = 0;
    uint8_t fade_steps = 48;
    for (uint8_t fade_count = 1; fade_count <= fade_steps; fade_count++) {    
        for (uint16_t y = ypos; y < MATRIX_HEIGHT; y++) {
            for (uint16_t x = xpos; x < MATRIX_WIDTH; x++) {
                uint16_t val = ((*curPixel)<<8) + (*(curPixel+1));
                uint8_t r = 0;
                uint8_t g = 0;
                uint8_t b = 0;
                display->color565to888(val, r, g, b);
                // ESP_Color::Color cur_color(val);
                // ESP_Color::HSLf hsl_color = cur_color.ToHsl(); 
                // float h = hsl_color.H;
                // float s = hsl_color.S;
                // float l = hsl_color.L;
                ESP_LOGV("draw", "R: %u G: %u B: %u", r, g, b);

                uint8_t prev_r = 0;
                uint8_t prev_g = 0;
                uint8_t prev_b = 0;
                // float prev_h = 0;
                // float prev_s = 0;
                // float prev_l = 0;
                if (prevImage) {
                    uint16_t prev_val = ((*prevPixel)<<8) + (*(prevPixel+1));
                    display->color565to888(prev_val, prev_r, prev_g, prev_b);
                    // ESP_Color::Color prev_color(prev_val);
                    // ESP_Color::HSLf prev_hsl_color = prev_color.ToHsl();
                    // prev_h = prev_hsl_color.H;
                    // prev_s = prev_hsl_color.S;
                    // prev_l = prev_hsl_color.L;
                }
                int8_t delta_r = r-prev_r;
                int8_t delta_g = g-prev_g;
                int8_t delta_b = b-prev_b;
                // float delta_h = h-prev_h;
                // float delta_s = s-prev_s;
                // float delta_l = l-prev_l;
                // ESP_Color::Color new_color = ESP_Color::Color::FromHsl(
                //     prev_h+((delta_h*(fade_count))/fade_steps),
                //     prev_s+((delta_s*(fade_count))/fade_steps),
                //     prev_l+((delta_l*(fade_count))/fade_steps)
                // );                
                // display->drawPixelRGB888(x, y, new_color.R_Byte(), new_color.G_Byte(), new_color.B_Byte());
                display->drawPixelRGB888(x, y,
                    prev_r+((delta_r*(fade_count))/fade_steps),
                    prev_g+((delta_g*(fade_count))/fade_steps),
                    prev_b+((delta_b*(fade_count))/fade_steps));
                curPixel += 2;
                prevPixel += 2;
                counter++;
            }
        }
        prevPixel = prevImage;
        curPixel = image;
        counter = 0;
        delay(1);
    }
    free(prevImage);
    prevImage = image;
    // free(image);
    // ESP_LOGI("draw", counter);
    ESP_LOGI("draw", "Done generating");
}

//Draws the bitmap using RGB888
void drawBMP_888(uint8_t* image, uint16_t xpos, uint16_t ypos) {
    // while ( JpegDec.read() == 1) {
    //     uint16_t *pImg;
    //     pImg = JpegDec.pImage;
    //     int mcu_x = JpegDec.MCUx * mcu_w;
    //     int mcu_y = JpegDec.MCUy * mcu_h;
    //     for (uint16_t y = mcu_y; y < mcu_y + mcu_h; y++) {
    //         for (uint16_t x = mcu_x; x < mcu_x + mcu_w; x++) {
    //         uint8_t red = (uint8_t)((((*pImg & 0xF800) >> 11) / 32.0) * 255);
    //         uint8_t green = (uint8_t)((((*pImg & 0x7E0) >> 6) / 32.0) * 255);
    //         uint8_t blue = (uint8_t)((((*pImg & 0x1F) / 32.0) * 255));
            
    //         display.drawPixelRGB888(x, y, red, blue, green);
    //         *pImg++;//Incrementing pImg goes to the next pixel over in the row, when we reach the end of the row
    //         }
    //     }
    // }
    // display->clearScreen();
    ESP_LOGI("draw", "Generating Image...");
    uint8_t* curPixel = image;
    uint8_t* prevPixel = prevImage;
    uint16_t counter = 0;
    uint8_t fade_steps = 48;
    for (uint8_t fade_count = 1; fade_count <= fade_steps; fade_count++) {            
        for (uint16_t y = ypos; y < MATRIX_HEIGHT; y++) {
            for (uint16_t x = xpos; x < MATRIX_WIDTH; x++) {
                // if (*curPixel == '\0') break;
                uint32_t val = ((*curPixel)<<16) + ((*(curPixel+1))<<8) + *(curPixel+2);
                // display.drawPixelRGB888(x, y, (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);                
                uint8_t r = (val >> 16) & 0xFF;
                uint8_t g = (val >> 8) & 0xFF;
                uint8_t b = val & 0xFF;

                uint8_t prev_r = 0;
                uint8_t prev_g = 0;
                uint8_t prev_b = 0;
                if (prevImage) {
                    uint32_t prev_val = ((*prevPixel)<<16) + ((*(prevPixel+1))<<8) + *(prevPixel+2);
                    prev_r = (prev_val >> 16) & 0xFF;
                    prev_g = (prev_val >> 8) & 0xFF;
                    prev_b = prev_val & 0xFF;
                }
                int8_t delta_r = r-prev_r;
                int8_t delta_g = g-prev_g;
                int8_t delta_b = b-prev_b;
                display->drawPixelRGB888(x, y,
                    prev_r+((delta_r*(fade_count))/fade_steps),
                    prev_g+((delta_g*(fade_count))/fade_steps),
                    prev_b+((delta_b*(fade_count))/fade_steps));
                curPixel += 3;
                prevPixel += 3;
                counter++;
            }
        }
        prevPixel = prevImage;
        curPixel = image;
        counter = 0;
        delay(1);
    }
    free(prevImage);
    prevImage = image;
    ESP_LOGI("draw", "Done generating");
}

// Download the image in a separate core from the display drawing
void downloadImage(void* pvParameters) {
  while (true) {
    if (millis() - lastUpdate >= callFrequency) {
        long startTime = millis();
        uint16_t responseCode = client.getReqAlbum();
        lastUpdate = millis();
        if (client.isPlaying) {
            isClock = false;
            lastScrobbling = lastUpdate;
        }
        if (lastUpdate - lastScrobbling >= stopDuration || isClock) {
            if (!isClock) {
                display->clearScreen();   // Clear screen if displaying clock for first time
                prevImage = nullptr;
                clockStartingUp = true;
                currentImageUrl = "";
                displayUpdater();
            }
            isClock = true;
            continue;
        } else {
            String newImageUrl = client.imageLink;
            ESP_LOGD("download", "Updated Image: %s\n", newImageUrl.c_str());
            ESP_LOGD("download", "isPlaying: %s\n", client.isPlaying ? "true" : "false");
            if (newImageUrl == "NONE" || !client.isPlaying) {
                continue;
            }
            else if ((newImageUrl != currentImageUrl) && (responseCode > 0)) {
                ESP_LOGI("download", "New Image. Downloading it");
                uint8_t* bmp_bytes = client.getReqBMP(newImageUrl);
                currentImageUrl = newImageUrl;                
                drawBMP_565(bmp_bytes, 0, 0);
                // drawBMP_888(bmp_bytes, 0, 0);
            }
        }
        long timeDelta = millis()-startTime;
        ESP_LOGI("download", "\n====================================");
        ESP_LOGI("download", "Took %lu milliseconds to draw image", timeDelta);
        ESP_LOGI("download", "\n====================================");
    }
    delay(1);
  }
}

void setup() {
    // Initialize serial
    Serial.begin(115200);

    // Turn off blue LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // I2C for light sensor
    Wire.begin(I2C_SDA, I2C_SCL);
    if (lightMeter.begin()) {
        ESP_LOGV("BH1750", "BH1750 initialised");
    } else {
        ESP_LOGE("BH1750", "Error initialising BH1750");
    }

    diplayInit();
    display->setRotation(3);

    // Serial.println("Fill screen: RED");
    // display->fillScreenRGB888(255, 0, 0);
    // delay(1000);

    // Serial.println("Fill screen: GREEN");
    // display->fillScreenRGB888(0, 255, 0);
    // delay(1000);

    // Serial.println("Fill screen: BLUE");
    // display->fillScreenRGB888(0, 0, 255);
    // delay(1000);

    // Serial.println("Fill screen: Neutral White");
    // display->fillScreenRGB888(64, 64, 64);
    // delay(1000);
    // display->clearScreen();

    logStatusMessage("LastFM...");
    client.setup();

    logStatusMessage("Time...");
    configTime(TIMEZONE_DELTA_SEC, TIMEZONE_DST_SEC, "ro.pool.ntp.org");
    lastNTPUpdate = millis();
    if (getLocalTime(&timeinfo)) {
        logStatusMessage("Time!");
    } else {
        logStatusMessage("No time!");
    }
    
    // logStatusMessage("Getting weather...");
    // getAccuWeatherData();
    // lastWeatherUpdate = millis();
    // logStatusMessage("Weather recvd!");

    // logStatusMessage("Setting up watchdog...");
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    // logStatusMessage("Watchdog set up!");

    // logStatusMessage(WiFi.localIP().toString());
    // drawTestBitmap();
    // displayWeatherData();
    
    // displayTicker.attach_ms(30, displayUpdater);
    
    // Put downloadImage function on seperate core
    xTaskCreatePinnedToCore(
        downloadImage,  /* Task function. */
        "Task1",        /* name of task. */
        10000,          /* Stack size of task */
        NULL,           /* parameter of the task */
        0,              /* priority of the task */
        &Task1,         /* Task handle to keep track of created task */
        0               /* pin task to core 0 */
    );
}

void loop() {
    if (isClock) {
        // Periodically refresh NTP time
        if (millis() - lastNTPUpdate > 1000*NTP_REFRESH_INTERVAL_SEC) {
            logStatusMessage("NTP Refresh");
            configTime(TIMEZONE_DELTA_SEC, TIMEZONE_DST_SEC, "ro.pool.ntp.org");
            lastNTPUpdate = millis();
        }

        // Periodically refresh weather forecast
        // if (millis() - lastWeatherUpdate > 1000 * WEATHER_REFRESH_INTERVAL_SEC) {
        //     logStatusMessage("Weather refresh");
        //     getAccuWeatherData();
        //     displayWeatherData();
        //     lastWeatherUpdate = millis();
        // }

        // Do we need to clear the status message from the screen?
        if (logMessageActive) {
            if (millis() > messageDisplayMillis + LOG_MESSAGE_PERSISTENCE_MSEC) {
                clearStatusMessage();
            }
        }
        displayUpdater();
    }

    if (millis() - lastLuxUpdate > lightCheck) {
        float lux = lightMeter.readLightLevel();
        uint8_t newBrightness = min(max(static_cast<int>((lux)*2), 5), 64);    // Set max between 5-64
        if (newBrightness != lastBrightness) {   
            ESP_LOGD("Light Sensor", "Lux: %f", lux);
            ESP_LOGD("Light Sensor", "Brightness: %u", newBrightness);
            fadeBrightness(lastBrightness, newBrightness);
            lastBrightness = newBrightness;
        }
        lastLuxUpdate = millis();
    }

    // Reset the watchdog timer as long as the main task is running
    esp_task_wdt_reset();
    delay(30);
}

void displayUpdater() {
  if (!getLocalTime(&timeinfo)) {
    logStatusMessage("Failed to get time!");
    return;
  }

  unsigned long epoch = mktime(&timeinfo);
  if (epoch != prevEpoch) {
    displayClock();
    prevEpoch = epoch;
  }
}
