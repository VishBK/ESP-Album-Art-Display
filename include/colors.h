// *********************************************************************
// Color definitions
// *********************************************************************
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

// RGB565 color formula: ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
#define CLOCK_DIGIT_COLOR   ((0x00 & 0xF8) << 8) | ((0xFF & 0xFC) << 3) | (0xFF >> 3)

#define DOW_COLOR ((0x00 & 0xF8) << 8) | ((0x40 & 0xFC) << 3) | (0xFF >> 3)
#define DATE_COLOR DOW_COLOR
#define SUNDAY_DATE_COLOR       ((0xFF & 0xF8) << 8) | ((0xD1 & 0xFC) << 3) | (0x00 >> 3)
#define MONDAY_DATE_COLOR       ((0xFF & 0xF8) << 8) | ((0x00 & 0xFC) << 3) | (0x34 >> 3)
#define TUESDAY_DATE_COLOR      ((0xFF & 0xF8) << 8) | ((0x61 & 0xFC) << 3) | (0x00 >> 3)
#define WEDNESDAY_DATE_COLOR    ((0x3C & 0xF8) << 8) | ((0x86 & 0xFC) << 3) | (0x35 >> 3)
#define THURSDAY_DATE_COLOR     ((0x76 & 0xF8) << 8) | ((0x4D & 0xFC) << 3) | (0x7F >> 3)
#define FRIDAY_DATE_COLOR       ((0xDD & 0xF8) << 8) | ((0xE2 & 0xFC) << 3) | (0x1C >> 3)
#define SATURDAY_DATE_COLOR     ((0x28 & 0xF8) << 8) | ((0xD2 & 0xFC) << 3) | (0xD6 >> 3)

// Some standard colors
// uint16_t myRED = display->color565(255, 0, 0);
// uint16_t myGREEN = display->color565(0, 255, 0);
// uint16_t myBLUE = display->color565(48, 73, 255);
// uint16_t myWHITE = display->color565(255, 255, 255);
// uint16_t myYELLOW = display->color565(255, 255, 0);
// uint16_t myCYAN = display->color565(0, 255, 255);
// uint16_t myMAGENTA = display->color565(255, 0, 255);
// uint16_t myORANGE = display->color565(255, 96, 0);
// uint16_t myBLACK = display->color565(0, 0, 0);

// uint16_t myCOLORS[8] = {myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};
