#include <ArduinoJson.h>
#include <WiFi.h>
#include <StreamUtils.h>

#include "LastFMClient.h"

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 500;

const char* server = "ws.audioscrobbler.com";
const char* lastfm_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n" \
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n" \
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n" \
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n" \
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n" \
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n" \
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n" \
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n" \
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n" \
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n" \
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n" \
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n" \
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n" \
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n" \
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n" \
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n" \
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n" \
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n" \
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n" \
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n" \
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n" \
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n" \
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n" \
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n" \
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n" \
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n" \
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n" \
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n" \
"jjxDah2nGN59PRbxYvnKkKj9\n" \
"-----END CERTIFICATE-----\n";

LastFMClient::LastFMClient(String username, String apiKey) {
    this->username = username;
    this->apiKey = apiKey;
}

void LastFMClient::setup() {
    // Attempt to connect to WiFi network:
    ESP_LOGI("wifi", "Connecting");
    WiFi.begin(SECRET_SSID, SECRET_PASS, 6);
    while (WiFi.status() != WL_CONNECTED) {
        ESP_LOGI("wifi", ".");
        delay(1000);
    }
    WiFi.setAutoReconnect(true);
    ESP_LOGI("wifi", "Connected to WiFi network with IP Address: %s", WiFi.localIP().toString());
    // c.setCACert(lastfm_cert);
    // c.setInsecure();
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
    const String kFullname = String("http://album-art-display.herokuapp.com/get_bmp?color_depth=565&url=") + imageUrl;

    isDataCall = true;
    uint8_t badRequestCount = 0;
    uint16_t sizeImageBytes = 64*64*2;
    uint8_t* resp = (uint8_t*)(malloc(sizeof(uint8_t) * sizeImageBytes));

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
