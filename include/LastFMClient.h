#pragma once
#include <Arduino.h>
#include <FS.h>
#include <HTTPClient.h>
#include <PNGDEC.h>
#include <SPIFFS.h>
#include <WiFi.h>
// #include <WiFiClientSecure.h>

#include "common.h"
#include "secrets.h"    // Enter your sensitive data in the Secret tab/secrets.h

typedef void (*DrawingCallback)();

typedef struct LastFMData {
  boolean isPlaying;
  const char* artistName;
  const char* albumName;
  const char* trackName;
  const char* image300Href;
  const char* image174Href;
  const char* image64Href;
  const char* mbid;
} LastFMData;

class LastFMClient {
  private:
    LastFMData *data;
    bool isDataCall;
    uint16_t currentImageHeight;
    DrawingCallback *drawingCallback;
    String username;
    String apiKey;
    // HttpClient http;
    WiFiClient c;
    // WiFiClientSecure c;
    PNG pngDec;
    const uint8_t pngHeader[16] = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52
    };
  
  public:
    LastFMClient(String username, String apiKey);
    void setup();
    uint16_t update(LastFMData *data);

    uint16_t getReqAlbum();
    uint8_t* getReqBMP(String image_url);
    String imageLink;
    bool isPlaying;

    // void downloadFile(String url, String filename);

    // void setDrawingCallback(DrawingCallback *drawingCallback) {
    //   this->drawingCallback = drawingCallback;
    // }
};
