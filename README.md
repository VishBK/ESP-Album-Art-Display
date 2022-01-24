# ESP32 Album Art Display
Displays the currently playing album art from LastFM on a 64x64 LED display with an ESP32. Switches to a digital clock when not playing music. Digital clock is modified from [here](https://github.com/hwiguna/HariFun_166_Morphing_Clock).

## Parts list
| Part      | Price | Description |
| ----------- | ----------- | ----------- |
| [ESP32-CH9102X](https://www.aliexpress.com/item/32959541446.html) | $5.64 | The microcontroller that drives the display |
| [64x64 LED pixel matrix](https://www.aliexpress.com/item/32757647402.html) | $24.90 | Display for the album art and clock |
| [5V 10A Power supply](https://www.aliexpress.com/item/1005002733082598.html) | $13.48 | Power supply for the ESP and display (may be able to get away with 8A) |
| [Female to female jumper wires (10cm)](https://www.aliexpress.com/item/32825558073.html) | $2.10 | Wires to connect the ESP to the display |
| [Female barrel connector terminal](https://www.aliexpress.com/item/32805447244.html) | $0.83 | Used to connect the power supply to the display |
| [(Optional) Right angle 4-pin connector](https://www.amazon.com/Kentek-Degree-Internal-Computer-Adapter/dp/B07KS4X9YN) | $5.50 | Sticks out less from the back of the display, making it easier to hang on a wall |
| [(Optional) M3 Magnetic screws](https://www.aliexpress.com/item/32791055820.html) | $8.16 | Screw in the back of the display to mount on a magnetic surface
| Total (without optionals) | $46.95 | |

## Libraries
* [Mr. Faptastic's LED Matrix Library for ESP32s](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
