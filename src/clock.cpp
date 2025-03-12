#include "common.h"
#include "digit.h"
#include "rgb_display.h"
#include "clock.h"
#include "colors.h"

/*
Digit digit0(0, 80 - 1 - 11*1, CLOCK_Y, clockDigitColor);
Digit digit1(0, 80 - 1 - 11*2, CLOCK_Y, clockDigitColor);
Digit digit2(0, 80 - 4 - 11*3, CLOCK_Y, clockDigitColor);
Digit digit3(0, 80 - 4 - 11*4, CLOCK_Y, clockDigitColor);
Digit digit4(0, 80 - 7 - 11*5, CLOCK_Y, clockDigitColor);
Digit digit5(0, 80 - 7 - 11*6, CLOCK_Y, clockDigitColor);
*/

uint16_t clockDigitColor = CLOCK_DIGIT_COLOR;
// The Y axis starts at the bottom for the MorphingClock library... :(
Digit digit5(0, CLOCK_X,                                                 PANEL_HEIGHT-CLOCK_Y-2*(CLOCK_SEGMENT_HEIGHT)-3, clockDigitColor);
Digit digit4(0, CLOCK_X + (CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING),   PANEL_HEIGHT-CLOCK_Y-2*(CLOCK_SEGMENT_HEIGHT)-3, clockDigitColor);
Digit digit3(0, CLOCK_X+2*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+3, PANEL_HEIGHT-CLOCK_Y-2*(CLOCK_SEGMENT_HEIGHT)-3, clockDigitColor);
Digit digit2(0, CLOCK_X+3*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+3, PANEL_HEIGHT-CLOCK_Y-2*(CLOCK_SEGMENT_HEIGHT)-3, clockDigitColor);
// Digit digit1(0, CLOCK_X+4*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+6, PANEL_HEIGHT-CLOCK_Y-2*(CLOCK_SEGMENT_HEIGHT)-3, clockDigitColor);
// Digit digit0(0, CLOCK_X+5*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+6, PANEL_HEIGHT-CLOCK_Y-2*(CLOCK_SEGMENT_HEIGHT)-3, clockDigitColor);

int prevss = 0;
int prevmm = 0;
int prevhh = 0;
bool isColon = true;    // Is colon displayed

void setDigitsColor(uint16_t color) {
    digit5.setColor(color);
    digit4.setColor(color);
    digit3.setColor(color);
    digit2.setColor(color);
    // digit1.setColor(color);
    // digit0.setColor(color);
}

void displayClock() {
    int hh = timeinfo.tm_hour;
    if (hh > 12) {
        hh -= 12;
    }
    int mm = timeinfo.tm_min;
    int ss = timeinfo.tm_sec;
    delay(500);

    if (isClockStartingUp) { // If we didn't have a previous time, just draw it without morphing.
        //   digit0.Draw(ss % 10);
        //   digit1.Draw(ss / 10);

        // Set the values manually since setDigitsColor calls Draw()
        digit2.setValue(mm % 10);   // Minutes ones digit
        digit3.setValue(mm / 10);   // Minutes tens digit
        digit4.setValue(hh % 10);   // Hours ones digit
        digit5.setValue(hh / 10);   // Hours ones digit
        // digit2.Draw(mm % 10); // Minutes ones digit
        // digit3.Draw(mm / 10); // Minutes tens digit
        // digit4.Draw(hh % 10); // Hours ones digit
        // digit5.Draw(hh / 10); // Hours tens digit

        uint8_t pos = round(((fmod(timeinfo.tm_hour+9.5,24)*60+mm)/((24*60)/255.)));
        clockDigitColor = colorWheel(pos);
        setDigitsColor(clockDigitColor);
        digit3.DrawColon(0xffff-clockDigitColor);

        displayDate();
        isClockStartingUp = false;
    } 
    else {
        // epoch changes every milliseconds, we only want to draw when digits actually change
        if (ss != prevss) { 
            // int s0 = ss % 10;
            // int s1 = ss / 10;
            // if (s0!=digit0.getValue()) digit0.Morph(s0);
            // if (s1!=digit1.getValue()) digit1.Morph(s1);
            // prevss = ss;

            // Blink colon
            if (isColon) {
                digit3.DrawColon(0);
                isColon = false;
            } else {
                digit3.DrawColon(0xffff-clockDigitColor);   // Invert colon color
                isColon = true;
            }
        }

        if (mm != prevmm) {            
            uint8_t pos = round(((fmod(timeinfo.tm_hour+9.5,24)*60+mm)/((24*60)/255.)));
            clockDigitColor = colorWheel(pos);
            setDigitsColor(clockDigitColor);   
            int m0 = mm % 10;
            int m1 = mm / 10;
            if (m0 != digit2.getValue()) digit2.Morph(m0);
            if (m1 != digit3.getValue()) digit3.Morph(m1);
            displayDate();
            prevmm = mm;
        }
      
        if (hh != prevhh) {
            int h0 = hh % 10;
            int h1 = hh / 10;
            if (h0 != digit4.getValue()) digit4.Morph(h0);
            if (h1 != digit5.getValue()) digit5.Morph(h1);
            prevhh = hh;
        }
    }
}

void displayDate() {
    display->fillRect(DOW_X, DOW_Y, DATE_WIDTH, DATE_HEIGHT, 0);

    display->setTextSize(1);     // size 1 == 8 pixels high
    display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
    uint16_t dateColor = DATE_COLOR;
    switch (timeinfo.tm_wday) {
        case (0): dateColor = SUNDAY_DATE_COLOR; break;
        case (1): dateColor = MONDAY_DATE_COLOR; break;
        case (2): dateColor = TUESDAY_DATE_COLOR; break;
        case (3): dateColor = WEDNESDAY_DATE_COLOR; break;
        case (4): dateColor = THURSDAY_DATE_COLOR; break;
        case (5): dateColor = FRIDAY_DATE_COLOR; break;
        case (6): dateColor = SATURDAY_DATE_COLOR; break;
    }
    display->setTextColor(dateColor);

    display->setCursor(DOW_X, DOW_Y);
    display->print(&timeinfo, "%a");

    display->setCursor(DATE_X, DATE_Y);
    display->print(&timeinfo, "%m/%d");
}
