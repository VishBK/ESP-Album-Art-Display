#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <WiFi.h>

#include "LastFMClient.h"

// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 500;

const char* server = "ws.audioscrobbler.com";

// Attempt to connect to WiFi network
void connectWiFi() {
    ESP_LOGI("wifi", "Connecting");
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    while (WiFi.status() != WL_CONNECTED) {
        ESP_LOGI("wifi", ".");
        delay(1000);
    }
    WiFi.setAutoReconnect(true);
    ESP_LOGI("wifi", "Connected to WiFi network with IP Address: %s", WiFi.localIP().toString().c_str());
}

void blinkLED() {
    for (uint8_t i = 0; i < 2; i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN,LOW);
        delay(250);
    }
}

LastFMClient::LastFMClient(String username, String apiKey) {
    this->username = username;
    this->apiKey = apiKey;
}

void LastFMClient::setup() {
    connectWiFi();
}

uint16_t LastFMClient::update(LastFMData *data) {
    // this->data = data;
    uint16_t httpCode = getReqAlbum();
    return httpCode;
}

uint16_t LastFMClient::getReqAlbum() {
    const String kFullname = String("http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&api_key=") + API_KEY + "&user=" + USERNAME + "&limit=1&format=json";
    HTTPClient http;
    int httpResponseCode = 0;
    uint8_t badRequestCount = 0;
    String resp = "";
    
    // start request
    while (true) {
        // if (badRequestCount >= 10) ESP.restart();
        if (!c.connect(server, 443))
            ESP_LOGE("lastfmAPI", "Server connection failed!");
        else {
            ESP_LOGV("lastfmAPI", "Connected to server!");

            http.begin(kFullname);
            httpResponseCode = http.GET();
            if (httpResponseCode > 0) {
                ESP_LOGD("lastfmAPI", "HTTP Response code: %i", httpResponseCode);
                resp = http.getString();
                break;
            } else {
                ESP_LOGE("lastfmAPI", "Error %i: %s", httpResponseCode, http.errorToString(httpResponseCode));
                blinkLED();
                badRequestCount++;
            }
        }
        delay(kNetworkDelay);
    }
    http.end();
    c.stop();

    // Parse the JSON
    ESP_LOGV("lastfmAPI", "Parsing");
    JsonDocument filter;

    JsonObject filter_recenttracks = filter["recenttracks"]["track"].add<JsonObject>();
    filter_recenttracks["image"][0]["#text"] = true;
    filter_recenttracks["@attr"]["nowplaying"] = true;
    // serializeJson(filter, Serial);   // Prints JSON to Serial

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, resp, DeserializationOption::Filter(filter));

    if (error) {
        ESP_LOGE("lastfmAPI", "deserializeJson() failed: %s", error.c_str());
        return -1;
    }
    JsonObject parsed = doc["recenttracks"]["track"][0];

    // data->isPlaying = bool(parsed["@attr"]["nowplaying"]);
    isPlaying = parsed["@attr"]["nowplaying"];
    // data->artistName = parsed["artist"]["#text"];
    // data->albumName = parsed["album"]["#text"];
    // data->trackName = parsed["name"];
    // data->mbid = parsed["mbid"];
    imageLink = parsed["image"][1]["#text"].as<const char*>();  // Get the medium image size
    // imageLink = doc["url"].as<const char*>();
    // data->image174Href = parsed["image"][2]["#text"];
    // data->image300Href = parsed["image"][3]["#text"];

    // serializeJson(parsed, Serial);
    ESP_LOGV("lastfmAPI", "\nParsed JSON");

    return httpResponseCode;
}

uint8_t* LastFMClient::getReqBMP(String imageUrl) {
    ESP_LOGV("imageDec", "Starting Decode");
    // https -> http
    if (imageUrl.startsWith("https")) {
        imageUrl.remove(4,1);
    }
    // Replace extension with png
    if (!imageUrl.endsWith(".png")) {
        imageUrl.remove(imageUrl.lastIndexOf("."));
        imageUrl += ".png";
    }

    uint8_t badRequestCount = 0;
    uint8_t* imgBuf;
    uint8_t* resp;
    HTTPClient http;

    // Start request 
    int httpResponseCode = -1;
    while (httpResponseCode < 0) {
        // if (badRequestCount >= 10) ESP.restart();
        // Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED) {
            http.begin(imageUrl);
            http.addHeader("User-Agent", "ESP32");
            // start connection and send HTTP header
            httpResponseCode = http.GET();
            if (httpResponseCode > 0) {
                // HTTP header has been sent and Server response header has been handled
                ESP_LOGD("imageDec", "GET code: %d\n", httpResponseCode);

                // file found at server
                if (httpResponseCode == HTTP_CODE_OK) {
                    int sizeImageBytes = http.getSize();
                    resp = (uint8_t*)(malloc(sizeof(uint8_t)*sizeImageBytes));

                    // get tcp stream
                    WriteBufferingStream bufferedHttpClient{http.getStream(), 1024};
                    bufferedHttpClient.readBytes((char*)resp, sizeImageBytes);
                    bufferedHttpClient.flush();
                    ESP_LOGV("imageDec", "Connection closed or file end\n");

                    if (memcmp(resp, pngHeader, 16) == 0) { // Check PNG header
                        // Start decoding PNG                            
                        ESP_LOGD("imageDec", "Decoding\n");
                        if (pngDec.openRAM(resp, sizeImageBytes, NULL) == PNG_SUCCESS) {
                            pngDec.setBuffer((uint8_t *)malloc(pngDec.getBufferSize()));
                            pngDec.decode(NULL, 0);
                        } else {
                            ESP_LOGE("imageDec", "PNG decode failed");
                        }
                        free(resp);
                        break;
                    } else {
                        httpResponseCode = -1;
                        free(resp);
                    }
                }
            } else {
                ESP_LOGE("imageDec", "GET failed, error: %s", http.errorToString(httpResponseCode).c_str());
                blinkLED();
                badRequestCount++;
            }
        } else {
            ESP_LOGE("imageDec", "WiFi Disconnected");
            connectWiFi();
        }
        delay(kNetworkDelay);
    }
    http.end();
    imgBuf = pngDec.getBuffer();
    ESP_LOGD("imageDec", "0x%x\n", (imgBuf[0]<<16) + (imgBuf[1]<<8) + imgBuf[2]);

    return imgBuf;
}
