#include <ArduinoJson.h>
#include <WiFi.h>
#include <StreamUtils.h>

#include "LastFMClient.h"

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 500;

LastFMClient::LastFMClient(String username, String apiKey) {
    // http.setTimeout(kNetworkTimeout);
    // http.setConnectTimeout(kNetworkTimeout);
    this->username = username;
    this->apiKey = apiKey;
}

void LastFMClient::setup() {
    // Attempt to connect to WiFi network:
    ESP_LOGI("wifi", "Connecting");
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        ESP_LOGI("wifi", ".");
    }
    ESP_LOGI("wifi", "Connected to WiFi network with IP Address: %s", WiFi.localIP());
    // c.setCACert(lastfm_cert);
}

void blinkLED() {
    for (uint8_t i = 0; i < 2; i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN,LOW);
        delay(250);
    }
}

uint16_t LastFMClient::update(LastFMData *data) {
    // this->data = data;
    uint16_t httpCode = getReqAlbum();
    return httpCode;
}

uint16_t LastFMClient::getReqAlbum() {
    const String kFullname = String("http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&api_key=") + API_KEY + "&user=" + USERNAME + "&limit=1&format=json";
    // const String kFullname = String("http://album-art-display.herokuapp.com/get_latest_track_image_url");
    // Name of the server to connect to
    // const String kHostname = "ws.audioscrobbler.com";
    // Path to download (this is the bit after the hostname in the URL)
    // const String kUrl = String("/2.0/?method=user.getrecenttracks&format=json&limit=1") + "&user=" + this->username + "&api_key=" + this->apiKey;
    // const String kPathname = "/2.0/";
    // const String kQuery = String("?method=user.getrecenttracks&format=json&limit=1") + "&user=" + this->username + "&api_key=" + this->apiKey;
    // HttpClient http = HttpClient(c, kHostname);
    HTTPClient http;
    // http.useHTTP10(true);
    int httpResponseCode = 0;
    uint8_t badRequestCount = 0;
    String resp = "";
    
    ESP_LOGD("lastfmAPI", "\nStarting GET call with: %s", kFullname);
    // ESP_LOGD("lastfmAPI", "%s", kHostname+kUrl);
    // configure server and url
    // http.begin(kHostname+kUrl, ca_cert);

    // start request
    while (httpResponseCode <= 0) {
        // if (badRequestCount >= 10) ESP.restart();
        // Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED) {
            // Your Domain name with URL path or IP address with path
            http.begin(kFullname);            
            // http.addHeader("Content-Type", "application/json");
            http.addHeader("User-Agent", "ESP32");
            http.setTimeout(2500);
            http.setConnectTimeout(2500);
            // Send HTTP GET request
            httpResponseCode = http.GET();

            if (httpResponseCode > 0) {
                ESP_LOGD("lastfmAPI", "HTTP Response code: %s", httpResponseCode);
                resp = http.getString();
                break;
            } else {
                ESP_LOGE("lastfmAPI", "Error %s: %s", httpResponseCode, http.errorToString(httpResponseCode));
                blinkLED();
                badRequestCount++;
            }
        } else {
            ESP_LOGE("lastfmAPI", "WiFi Disconnected");
        }
        delay(kNetworkDelay);
    }
    http.end();

    // Parse the JSON
    ESP_LOGV("lastfmAPI", "Parsing");
    StaticJsonDocument<128> filter;

    JsonObject filter_recenttracks = filter["recenttracks"]["track"].createNestedObject();
    filter_recenttracks["image"][0]["#text"] = true;
    filter_recenttracks["@attr"]["nowplaying"] = true;
    serializeJson(filter, Serial);

    StaticJsonDocument<1536> doc;
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
    serializeJson(parsed, Serial);
    
    ESP_LOGV("lastfmAPI", "\nParsed JSON");

    return httpResponseCode;
}

uint8_t* LastFMClient::getReqBMP(String imageUrl) {
    const String kFullname = String("http://album-art-display.herokuapp.com/get_bmp?color_depth=565&url=") + imageUrl;
    // Name of the server to connect to
    // const String kHostname = "album-art-display.herokuapp.com";
    // Path to download (this is the bit after the hostname in the URL)
    // const String kUrl = String("/get_bmp?") + "url=" + imageUrl;
    // HttpClient http = HttpClient(c, kHostname);
    isDataCall = true;
    // String resp = "";
    uint8_t badRequestCount = 0;
    uint16_t sizeImageBytes = 64*64*2;
    uint8_t* resp = (uint8_t*)(malloc(sizeof(uint8_t) * sizeImageBytes));

    ESP_LOGD("imageAPI", "Starting GET call with: %s", kFullname);
    // ESP_LOGD("imageAPI", "%s", kHostname+kUrl);

    HTTPClient http;

    // Start request 
    int httpResponseCode = 0;        
    while (httpResponseCode <= 0) {
        // if (badRequestCount >= 10) ESP.restart();
        // Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED) {
            http.begin(kFullname);
            http.addHeader("User-Agent", "ESP32");
            // start connection and send HTTP header
            httpResponseCode = http.GET();
            if (httpResponseCode > 0) {
                // HTTP header has been send and Server response header has been handled
                ESP_LOGD("imageAPI", "GET code: %d\n", httpResponseCode);

                // file found at server
                if (httpResponseCode == HTTP_CODE_OK) {

                    // // get length of document (is -1 when Server sends no Content-Length header)
                    // int len = http.getSize();

                    // int readBytes = 0;
                    
                    // // create buffer for read
                    // uint8_t buff[128] = { 0 };

                    // // get tcp stream
                    // WiFiClient* stream = http.getStreamPtr();
                    WriteBufferingStream bufferedHttpClient{http.getStream(), 1024};
                    // bufferedHttpClient.flush();

                    bufferedHttpClient.readBytes((char*)resp, sizeImageBytes);

                    // read all data from server
                    // while (http.connected() && (len > 0 || len == -1)) {
                    //     // ESP_LOGD("imageAPI", "%i", len);
                    //     // get available data size
                    //     size_t size = stream->available();

                    //     if (size) {
                    //         // read up to 128 byte
                    //         int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                    //         // write it to Serial
                    //         // ESP_LOGD("imageAPI", "%i, %i", resp, c);
                    //         for (int i = 0; i < c; i++) {
                    //             resp[i+readBytes] = buff[i];
                    //         }
                    //         readBytes += c;

                    //         if (len > 0) {
                    //             len -= c;
                    //         }
                    //     }
                    //     delay(1);
                    // }
                    ESP_LOGD("imageAPI", "\nconnection closed or file end.\n");
                    break;
                }
            } else {
                ESP_LOGE("imageAPI", "GET failed, error: %s", http.errorToString(httpResponseCode).c_str());
                blinkLED();
                badRequestCount++;
            }
        } else {
            ESP_LOGE("imageAPI", "WiFi Disconnected");
        }
        delay(kNetworkDelay);
    }
    http.end();
    // ESP_LOGD("imageAPI", "0x%x\n", (resp[0]<<8) + resp[1]);
    ESP_LOGD("imageAPI", "0x%x\n", (resp[0]<<16) + (resp[1]<<8) + resp[2]);
    return resp;
}
