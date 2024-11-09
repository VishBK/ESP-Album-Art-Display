# ESP32 Album Art Display
Displays the currently playing album's cover art from LastFM on a 64x64 LED display with an ESP32. Switches to a digital clock when not playing music. Digital clock is modified from [here](https://github.com/hwiguna/HariFun_166_Morphing_Clock).

## Parts list
| Part      | Price | Description |
| ----------- | ----------- | ----------- |
| [ESP32-CH9102X](https://www.aliexpress.com/item/32959541446.html) | $3.10 | The microcontroller that drives the display |
| [64x64 LED pixel matrix](https://www.aliexpress.com/item/32757647402.html) | $21.20 | Display for the album art and clock |
| [5V 10A Power supply](https://www.aliexpress.com/item/2251832624591733.html) | $18.32 | Power supply for the ESP and display (may be able to get away with 8A) |
| [Female to female jumper wires (10cm)](https://www.aliexpress.com/item/32825558073.html) | $2.53 | Wires to connect the ESP to the display |
| [Female barrel connector terminal](https://www.aliexpress.com/item/32805447244.html) | $1.07 | Used to connect the power supply to the display |
| [(Optional) M3 Magnetic screws](https://www.aliexpress.com/item/32791055820.html) | $7.49 | Screw in the back of the display to mount on a magnetic surface
| Total (without optionals) | $46.22 | |

## Setup
1. Create a [LastFM](https://www.last.fm/) account
    * Hook up LastFM to your [music platform](https://www.last.fm/about/trackmymusic)
        * For Spotify I would use their [in-app setting](https://www.last.fm/settings/applications)
        * Otherwise, I would use [PanoScrobbler](https://play.google.com/store/apps/details?id=com.arn.scrobble) for Android and [Web Scrobbler](https://web-scrobbler.com/) for browsers
    * Create an [API key](https://www.last.fm/api/authentication)
    * Follow [me](https://www.last.fm/user/VishBK)!
2. Get [PlatformIO](https://platformio.org/) for VSCode
3. Clone repository into PlatformIO projects folder
4. Follow instructions on [Mr. Codetastic's LED library](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA) for wiring and setup
    * `include/rgb_display.h` has the pin mapping
5. Create a `include/secrets.h` file with:
```C
#define SECRET_SSID "WIFI_SSD"
#define SECRET_PASS "WIFI_PASSWORD"
#define API_KEY "LastFM_API_KEY"
#define USERNAME "LastFM_USERNAME"
```
1. Edit `include/config.h` to your liking
2. Upload the code to the ESP32 and ~~watch the errors pile up~~ enjoy!
