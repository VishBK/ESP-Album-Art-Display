// Displays album artwork of currently playing song from your LastFM account

#include <ESPNtpClient.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP_Color.h>
#include <SPI.h>
#include <TimeLib.h>
#include <Ticker.h>
#include <Wire.h>
#include <esp_task_wdt.h>

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

uint8_t prevHour;
unsigned long prevEpoch;
unsigned long lastNTPUpdate;
unsigned long lastWeatherUpdate;

bool blinkOn;
bool isClock = true; // Whether the clock should be displayed or not
uint8_t* prevImage = nullptr;
LastFMClient client(USERNAME, API_KEY);

void displayUpdater();

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

//Draws the bitmap using RGB888
void drawBMP_888(uint8_t* image, uint16_t xpos, uint16_t ypos) {
    ESP_LOGI("draw", "Generating Image...");

    uint8_t* curPixel = image;
    uint8_t* prevPixel = prevImage;
    uint16_t counter = 0;
    uint8_t fade_steps = 48;
    for (uint8_t fade_count = 1; fade_count <= fade_steps; fade_count++) {    
        for (uint16_t y = ypos; y < PANEL_HEIGHT; y++) {
            for (uint16_t x = xpos; x < PANEL_WIDTH; x++) {
                uint8_t r = curPixel[0];
                uint8_t g = curPixel[1];
                uint8_t b = curPixel[2];
                // ESP_Color::Color cur_color(val);
                // ESP_Color::HSLf hsl_color = cur_color.ToHsl(); 
                // float h = hsl_color.H;
                // float s = hsl_color.S;
                // float l = hsl_color.L;
                // ESP_LOGV("draw", "R: %u G: %u B: %u", r, g, b);

                uint8_t prev_r = 0;
                uint8_t prev_g = 0;
                uint8_t prev_b = 0;
                // float prev_h = 0;
                // float prev_s = 0;
                // float prev_l = 0;
                if (prevImage) {
                    prev_r = prevPixel[0];
                    prev_g = prevPixel[1];
                    prev_b = prevPixel[2];
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
                curPixel += 4;  // Increment 4 for (r,g,b,a)
                prevPixel += 4;
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
    ESP_LOGI("draw", "Done generating");
}

// Download and display the image in a separate core
void downloadImage(void* pvParameters) {
  while (true) {
    // Check if a new album is playing every CALL_FREQUENCY ms
    if (millis() - lastUpdate >= CALL_FREQUENCY) {
        long startTime = millis();
        uint16_t responseCode = client.getReqAlbum();
        lastUpdate = millis();
        // If music is currently playing, turn off the clock
        if (client.isPlaying) {
            isClock = false;
            lastScrobbling = lastUpdate;
        }
        // If last time music played is after the STOP_DURATION or the clock needs to be shown
        if (lastUpdate - lastScrobbling >= STOP_DURATION || isClock) {
            if (!isClock) {     // Clock is starting up after album art
                // display->clearScreen();
                drawColorDissolve(0x0, 250);
                prevImage = nullptr;
                isClockStartingUp = true;
                currentImageUrl = "";
                displayUpdater();
            }
            isClock = true;
            continue;
        } 
        else {  // If music is still playing and the clock is not shown
            String newImageUrl = client.imageLink;
            ESP_LOGD("downloadImage", "Updated Image: %s\n", newImageUrl.c_str());
            ESP_LOGD("downloadImage", "isPlaying: %s\n", client.isPlaying ? "true" : "false");
            if (!client.isPlaying || newImageUrl.length() == 0) {
                continue;
            }
            // If the updated image url is different from the current one (i.e., a different album is playing)
            else if ((newImageUrl != currentImageUrl) && (responseCode > 0)) {
                ESP_LOGI("downloadImage", "New Image. Downloading it");
                uint8_t* bmp_bytes = client.getReqBMP(newImageUrl);
                currentImageUrl = newImageUrl;
                drawBmpDissolve(bmp_bytes, RGB8888, 350);
                free(bmp_bytes);
            }
        }
        ESP_LOGI("downloadImage", "\n====================================");
        ESP_LOGI("downloadImage", "Took %lu ms to draw image", millis()-startTime);
        ESP_LOGI("downloadImage", "\n====================================");
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

    diplayInit();
    display->setRotation(3);

    displayText("LastFM...");
    client.setup();

    displayText("Time...");
    configTime(TIMEZONE_DELTA_SEC, TIMEZONE_DST_SEC, "ro.pool.ntp.org");
    lastNTPUpdate = millis();
    if (!getLocalTime(&timeinfo)) displayText("Time get fail!");

    // displayText("Getting weather...");
    // getAccuWeatherData();
    // lastWeatherUpdate = millis();
    // displayWeatherData();

    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);

    if (timeinfo.tm_hour >= NIGHT_MODE_TIME and timeinfo.tm_hour < DAY_MODE_TIME) {
        display->setPanelBrightness(BRIGHTNESS*0.25);   // Quarter brightness at night
    } else {
        display->setPanelBrightness(BRIGHTNESS);        // Full brightness during the day
    }
    
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
            displayText("NTP Refresh");
            configTime(TIMEZONE_DELTA_SEC, TIMEZONE_DST_SEC, "ro.pool.ntp.org");
            lastNTPUpdate = millis();
        }

        // Periodically refresh weather forecast
        // if (millis() - lastWeatherUpdate > 1000 * WEATHER_REFRESH_INTERVAL_SEC) {
        //     displayText("Weather refresh");
        //     getAccuWeatherData();
        //     displayWeatherData();
        //     lastWeatherUpdate = millis();
        // }

        displayUpdater();
    }

    if (prevHour != timeinfo.tm_hour) {
        prevHour = timeinfo.tm_hour;
        if (prevHour >= NIGHT_MODE_TIME and prevHour < DAY_MODE_TIME) {
            fadeBrightness(BRIGHTNESS, BRIGHTNESS*0.25);    // Quarter brightness at night
        } else {
            fadeBrightness(BRIGHTNESS*0.25, BRIGHTNESS);    // Full brightness during the day
        }
    }

    // Reset the watchdog timer as long as the main task is running
    esp_task_wdt_reset();
    delay(30);
}

void displayUpdater() {
    if (!getLocalTime(&timeinfo)) {
        displayText("Time get failed!", true);
        return;
    }

    unsigned long epoch = mktime(&timeinfo);
    if (epoch != prevEpoch) {
        displayClock();
        prevEpoch = epoch;
    }
    updateText();
}
