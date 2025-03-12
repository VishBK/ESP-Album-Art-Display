#ifndef CONFIG_H
#define CONFIG_H

//#define MQTT_USE_SSL 1
//#define USE_ANDROID_AP 1

// Time settings
#define NTP_REFRESH_INTERVAL_SEC 60*60  // How often we refresh the time from the NTP server
#define TIMEZONE_DELTA_SEC 3600 * -6    // Timezone difference from GMT, expressed in seconds
#define TIMEZONE_DST_SEC 3600   // DST delta to apply

// Scrobbling settings
#define CALL_FREQUENCY 5000     // How often to check LastFM in ms
#define STOP_DURATION 1000*10   // When to stop displaying cover art after stopping scrobbling in ms

// How long are informational messages kept on screen
#define TEXT_PERSISTENCE_MS 1000*10

// Screen positioning settings
// Panel size
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64  	// Panel height of 64 will require PIN_E to be defined
#define PANELS_NUMBER 1 	// Number of chained panels, if just a single panel, obviously set to 1

#define PANE_WIDTH PANEL_WIDTH * PANELS_NUMBER
#define PANE_HEIGHT PANEL_HEIGHT

#define BRIGHTNESS 16   // 0-64
#define NIGHT_MODE_TIME 1   // Time to lower brightness
#define DAY_MODE_TIME 8     // Time to increase brightness

// Clock
#define CLOCK_X 6
#define CLOCK_Y 21
#define CLOCK_SEGMENT_HEIGHT 8
#define CLOCK_SEGMENT_WIDTH 8
#define CLOCK_SEGMENT_SPACING 5
#define CLOCK_WIDTH 4*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+4
#define CLOCK_HEIGHT 2*CLOCK_SEGMENT_HEIGHT+3
//Delay in ms for clock animation - should be below 30ms for a segment size of 8
#define CLOCK_ANIMATION_DELAY_MSEC 20

// Day of week
#define DOW_X 6
#define DOW_Y 1
// Date positions
#define DATE_X DOW_X+23
#define DATE_Y DOW_Y
// Width and height are for both DATE and DOW
#define DATE_WIDTH 52
#define DATE_HEIGHT 8

// Weather sensor data
#define SENSOR_DATA_X 0
#define SENSOR_DATA_Y 0
#define SENSOR_DATA_WIDTH 128
#define SENSOR_DATA_HEIGHT 8
#define SENSOR_DATA_COLOR ((0x00 & 0xF8) << 8) | ((0x8F & 0xFC) << 3) | (0x00 >> 3)
#define SENSOR_ERROR_DATA_COLOR ((0xFF & 0xF8) << 8) | ((0x00 & 0xFC) << 3) | (0x00 >> 3)

#define TEXT_COLOR 0xF800

#define BITMAP_X 1
#define BITMAP_Y 21

// Watchdog settings
#define WDT_TIMEOUT 60   // If the WDT is not reset within X seconds, reboot the unit
        // Do NOT set this too low, or the WDT will prevent OTA updates from completing!!

// Weather - today, and 5-day forecast
#define WEATHER_TODAY_X 56
#define WEATHER_TODAY_Y 1

#define WEATHER_FORECAST_X 90
#define WEATHER_FORECAST_Y 44

//Temperature range for today
#define TEMPRANGE_X 0
#define TEMPRANGE_Y 44
#define TEMPRANGE_WIDTH 64
#define TEMPRANGE_HEIGHT 8
#define TEMPRANGE_COLOR ((0x00 & 0xF8) << 8) | ((0xFF & 0xFC) << 3) | (0xFF >> 3)

// How often to refresh weather forecast data
// (limited by API throttling)
#define WEATHER_REFRESH_INTERVAL_SEC 14400

#endif
