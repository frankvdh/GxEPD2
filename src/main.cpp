// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Display ePaper for Arduino: https://forum.arduino.cc/index.php?topic=436411.0


// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class

#include <math.h>
#define DEBUG
#include "Debug.h"

#define VERSION_STRING "v0.0.1"

#define ENABLE_GxEPD2_GFX 0

// uncomment next line to use class GFX of library GFX instead of Adafruit_GFX
#include "gfx/GFX_Root.h"
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>

#include "GxEPD2_3C.h"
#include "gfx/Fonts/FreeSerif12pt7b.h"
#ifdef RPI
#include "BMPfile.h"
#ifdef WIN32
#else
#include <wiringPi.h>   // for delay()
#endif
#endif

// Below is correct for RPI
GxEPD2_3C<GxEPD2_213c, GxEPD2_213c::HEIGHT> display(GxEPD2_213c(/*CS*/ 8, /*DC=*/ 25, /*RST=*/ 17, /*BUSY=*/ 24));
//GxEPD2_3C<GxEPD2_420c, GxEPD2_420c::HEIGHT> display(GxEPD2_420c(/*CS=10*/ 8, /*DC=*/ 25, /*RST=*/ 17, /*BUSY=*/ 24));

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_3C.h for method setPartialWindow()

#ifdef WIN32
#define wiringPiSetup() Debug("wiringPiSetup()\n")
#define wiringPiI2CSetup(addr) Debug("wiringPiI2CSetup(%d)\n", addr)
#define wiringPiI2CWrite(addr, val) Debug("wiringPiI2CWrite(%d, %02x)", addr, val)
uint16_t values[] = {40, 50, 60, 70, 80, 90, 100, 110, 100, 100, 120, 90, 100};
int value_num = -1;
#define NUM_VALUES (sizeof(values)/sizeof(values[0]))
#define NUM_SAMPLES 1
#else
#include <wiringPi.h> //Import Raspberry Pi WiringPi encoding IO control library
#include <wiringPiI2C.h> //Import Raspberry Pi WiringPi coded I2C control library
#define NUM_SAMPLES 100
#endif // WIN32

#define COLUMN_WIDTH 5
#define MIN_TEMP 40
#define MAX_TEMP 140
#define MAX_OK_TEMP 120
#define MIN_OK_TEMP 80

void printAt(uint16_t x, uint16_t y, char const *text) {
    Debug("printAt(%d, %d, %s)\n", x, y, text);
    display.setCursor(x, y);
    display.print(text);
    Debug("printAt done\n");
}

void printAt(uint16_t x, uint16_t y, int value) {
    char s[20];
    sprintf(s, "%d", value);
    printAt(x, y, s);
}

void splashScreen(void) {
    Debug("splashScreen\n");
    const char *splashScreen = "Fuel Pressure";
    display.setRotation(1);
    display.setFont(&FreeSerif12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(splashScreen, 0, 0, &tbx, &tby, &tbw, &tbh);
    Debug("text bounds: %d, %d, %d, %d\n", tbx, tby, tbw, tbh);
    // center text left-right
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    // centre in top half of screen
    uint16_t y = ((display.height() - tbh) / 4) - tby;
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    printAt(x, y, splashScreen);
    x += tbw; // Remember right bound

    display.getTextBounds(VERSION_STRING, 0, 0, &tbx, &tby, &tbw, &tbh);
    Debug("text bounds: %d, %d, %d, %d\n", tbx, tby, tbw, tbh);
    x -= tbw; // Align right bounds of both strings
    // centre in bottom half of screen
    y = ((display.height() - tbh) * 3 / 4) - tby;
    printAt(x, y, VERSION_STRING);
    display.nextPage();
    Debug("splashScreen done\n");
}

uint16_t scale(int temp) {
    if (temp < MIN_TEMP) temp = MIN_TEMP;
    uint16_t height = (temp - MIN_TEMP) * display.height() / (MAX_TEMP - MIN_TEMP);
    return height;
}
// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()
// showPartialUpdate() purposely uses values that are not multiples of 8 to test this


const uint16_t topLine = scale(MAX_OK_TEMP);
const uint16_t botLine = scale(MIN_OK_TEMP);
uint16_t numBars;
uint16_t *bar;
void drawGraphBackground() {
    display.setRotation(1);
    display.setFont(&FreeSerif12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.firstPage();
    display.fillScreen(GxEPD_WHITE);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    uint16_t x = 5, y = 5;
    printAt(x, y, "Fuel");
    display.getTextBounds("Fuel", x, y, &tbx, &tby, &tbw, &tbh);
    Debug("text bounds: %d, %d, %d, %d\n", tbx, tby, tbw, tbh);
    // centre in top half of screen
    y += tbh * 3/4;
    printAt(x, y, "Pressure");
    y += tbh * 3/4;
    printAt(x, y, "psi");
    display.drawLine(x + tbw +5, botLine, display.width(), botLine, GxEPD_RED);
    display.drawLine(x + tbw +5, topLine, display.width(), topLine, GxEPD_RED);
    printAt(x, 5, MAX_TEMP);
    printAt(x, scale(120)+5, MAX_OK_TEMP);
    printAt(x, scale(60) - 5, MIN_OK_TEMP);
    printAt(x, display.height() - 5, MIN_TEMP);
    display.getTextBounds("Pressure", x, y, &tbx, &tby, &tbw, &tbh);
    display.nextPage();
    numBars = (display.width() - (x + tbw + 5)) / COLUMN_WIDTH;
    bar = (uint16_t *) malloc((numBars+1)*sizeof(uint16_t));
    memset(bar, 0, (numBars+1)*sizeof(uint16_t));
}

void addBar(int temp) {
    uint16_t height = scale(temp);
    Debug("addBar(%d) %d\n", temp, height);
    bar[numBars] = height;
    uint16_t firstChange, lastChange;
    for (firstChange = 0; firstChange <= numBars; firstChange++)
        if (bar[firstChange] != bar[firstChange+1]) break;
    if (firstChange >= numBars) return;   // All bars the same and equal to new bar height -- no change needed
    for (lastChange = numBars; lastChange > firstChange; lastChange--)
        if (bar[lastChange] != bar[lastChange-1]) break;
    Debug("changes from %d (%d < %d) to %d (%d < %d)\n", firstChange, bar[firstChange], bar[firstChange+1], lastChange-1, bar[lastChange-1], bar[lastChange]);
    // Have found the region where the bar values change
    uint16_t minBar = bar[firstChange];
    uint16_t maxBar = bar[firstChange];
    for (int i = firstChange+1; i <= lastChange; i++) {
        if (bar[i] < minBar) minBar = bar[i];
        else if (bar[i] > maxBar) maxBar = bar[i];
    }
    Debug("Min bar = %d, Max bar = %d\n", minBar, maxBar);
    uint16_t box_x = display.width() - (numBars - firstChange)*COLUMN_WIDTH;
    uint16_t box_y = minBar;
    uint16_t box_w = (lastChange-firstChange)*COLUMN_WIDTH;
    uint16_t box_h = maxBar - minBar;
    Debug("change box: %d, %d, %d, %d\n", box_x, box_y, box_w, box_h);
    // Have the region where bars are changing... erase all bars in this region
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    // Draw bars in the changed region, including shifting them left by one
    for (int i = firstChange; i < lastChange; i++) {
        if ((bar[i] = bar[i+1]) == minBar) continue;
        Debug("bar: %d, %d, %d, %d\n", display.width() - (numBars-i)*COLUMN_WIDTH, minBar, COLUMN_WIDTH, bar[i] - minBar);
        display.fillRect(display.width() - i*COLUMN_WIDTH, minBar, COLUMN_WIDTH, bar[i] - minBar, GxEPD_BLACK);
    }

    // Redraw OK limit lines
    if (minBar <= botLine && maxBar >= botLine) {
        Debug("Refresh bottom line\n");
        display.drawLine(box_x, botLine, box_w, botLine, GxEPD_RED);
    }
    if (minBar <= topLine && maxBar >= topLine) {
        Debug("Refresh top line\n");
        display.drawLine(box_x, topLine, box_w, topLine, GxEPD_RED);
    }
    display.nextPage();
}


/* From https://github.com/emakefun/RaspberryPi-Sensor-Board
The MCU I2C address of the expansion board is 0x04, and the registered address is explained as follows:
* bits 0-2 = channel number
* bits 4-5 = function
* 0x10 ~ 0x17: read ADC raw data
* 0x20 ~ 0x27: read input voltage
* 0x30 ~ 0x37: Read the ratio of input voltage to output voltage Input voltage / output voltage
*/

#ifdef WIN32
#else
int analogRead(int channel) {
    wiringPiI2CWrite(0x04, 0x20 + (channel & 0x07)); //Write a byte to the slave
    return wiringPiI2CReadReg16(0x04, 0x20 + (channel & 0x07)); // Read two bytes from the specified address of the slave
}
#endif // WIN32


// From https://learn.adafruit.com/thermistor/using-a-thermistor by Limor Fried, Adafruit Industries
// MIT License - please keep attribution and consider buying parts from Adafruit

#define SERIESRESISTOR 100000   // the value of the 'other' resistor
#define FULLSCALE 0xFFF         // 12-bit A2D
#define THERMISTORCHANNEL 0     // What pin the thermistor is connected to

#define THERMISTORNOMINAL 100000    // resistance at 25 degrees C
#define TEMPERATURENOMINAL 25       // temp. for nominal resistance (almost always 25 C)
#define BCOEFFICIENT 3950           // The beta coefficient of the thermistor (usually 3000-4000)


void loop(void) {

#ifdef WIN32
    value_num++;
    value_num %= NUM_VALUES;
    addBar(values[value_num]);
#else
    int reading = 0;

    for (int i=0; i < NUM_SAMPLES; i++) {
        reading += analogRead(THERMISTORCHANNEL);
        delay(1);
    }
    reading /= NUM_SAMPLES;

    // convert the reading to resistance
    // Vtherm = Rtherm/(Rtherm + Rfixed) * Vfs
    // ADCtherm = Rtherm/(Rtherm+Rfixed) * ADCfs
    // ADCtherm*(Rtherm+Rfixed) = Rtherm * ADCfs
    // ADCtherm*Rtherm+ADCtherm*Rfixed = Rtherm * ADCfs
    // Rtherm*ADCfs - ADCtherm*Rtherm  = ADCtherm * Rfixed
    // Rtherm (ADCfs-ADCtherm) = ADCtherm * Rfixed
    // Rtherm = ADCtherm * Rfixed / (ADCfs - ADCtherm)
    // Rtherm = ADCtherm * 100K /(0xFFF - ADCtherm)
    float resistance = SERIESRESISTOR * reading / (float)(FULLSCALE - reading);
    float steinhart = log(resistance / THERMISTORNOMINAL) / BCOEFFICIENT;        // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart - 273.15;             // invert and convert absolute temp to C

    printf("%5d %10.0f %6.2f\r\n", reading, resistance, steinhart);
#endif // WIN32
    delay(1000);
}

#ifdef RPI
void  Handler(int signo) {
    //System Exit
    Debug("\r\nHandler:exit\r\n");
    SPI.end();

    exit(0);
}
#endif // RPI

int main(void) {
    Debug("main()\n");
    display.init(); // Also does wiringPiSetup()
    wiringPiI2CSetup(0x04);//Open the I2C device, 0x04 is the MCU I2C address on the expansion board
    // first update should be full refresh

#if 0
    // Dump the OTP memory for external analysis
    uint8_t buffer[0x1001];    // One extra byte as dummy
    display.epd2.readOtp(buffer, sizeof(buffer));
    for (uint16_t i = 0; i < sizeof(buffer); i++) {
        if (i % 8 == 0) printf("\n");
        printf("%02x ", buffer[i]);
    }
    exit(0);
#endif
#ifdef WIN32
#else
    delay(1000);
    display.fillScreen(GxEPD_WHITE);
    if (!display.writeBmpFile("oil-can.bmp", 0, display.height()/2, 0, false))
        Debug("Reading oil-can failed\n");
    if (!display.writeBmpFile("thermometer.bmp", 0, display.height()/2, 0, false))
        Debug("Reading thermometer failed");
    delay(1000);
    drawGraphBackground();
    delay(1000);
    display.powerOff();
#endif // WIN32
    drawGraphBackground();
    while (true)
        loop();
    Debug("main() done\n");
    return 0;
}
