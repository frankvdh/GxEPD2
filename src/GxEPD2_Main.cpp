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

#define ENABLE_GxEPD2_GFX 0

// uncomment next line to use class GFX of library GFX instead of Adafruit_GFX
#include "gfx/GFX_Root.h"
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>

#include "GxEPD2_BW.h"
#include "gfx/Fonts/FreeMonoBold9pt7b.h"
#ifdef RPI
#include "BMPfile.h"
#include <wiringPi.h>   // for delay()
#endif

#define MAX_DISPLAY_BUFFER_SIZE 5000 // e.g. full height for 200x200
// 3-color e-papers
#define MAX_HEIGHT_3C(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
// Below is correct for RPI
GxEPD2_BW<GxEPD2_213_M21, MAX_HEIGHT_3C(GxEPD2_213_M21)> display(GxEPD2_213_M21(/*CS*/ 8, /*DC=*/ 25, /*RST=*/ 17, /*BUSY=*/ 24));
//GxEPD2_3C<GxEPD2_420c, MAX_HEIGHT_3C(GxEPD2_420c)> display(GxEPD2_420c(/*CS=10*/ 8, /*DC=*/ 25, /*RST=*/ 17, /*BUSY=*/ 24));

// select only one to fit in code space
#include "bitmaps/Bitmaps3c104x212.h" // 2.13" b/w/r
#include "bitmaps/Bitmaps3c400x300.h" // 4.2"  b/w/r

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_3C.h for method setPartialWindow()

const char HelloWorld[] = "Hello World!";
#ifdef RPI
const char HelloArduino[] = "Hi Raspberry Pi!";
#else
const char HelloArduino[] = "Hello Arduino!";
#endif
const char HelloEpaper[] = "Hello E-Paper!";

void helloWorld(void) {
    Debug("helloWorld\n");
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
    Debug("text bounds: %d, %d, %d, %d\n", tbx, tby, tbw, tbh);
    // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(HelloWorld);
    } while (display.nextPage());
    Debug("helloWorld done\n");
}

void helloWorldForDummies() {
    //Debug("helloWorld\n");
    const char text[] = "Hello World!";
    // most e-papers have width < height (portrait) as native orientation, especially the small ones
    // in GxEPD2 rotation 0 is used for native orientation (most TFT libraries use 0 fix for portrait orientation)
    // set rotation to 1 (rotate right 90 degrees) to have enough space on small displays (landscape)
    display.setRotation(1);
    // select a suitable font in Adafruit_GFX
    display.setFont(&FreeMonoBold9pt7b);
    // on e-papers black on white is more pleasant to read
    display.setTextColor(GxEPD_BLACK);
    // Adafruit_GFX has a handy method getTextBounds() to determine the boundary box for a text for the actual font
    int16_t tbx, tby;
    uint16_t tbw, tbh; // boundary box window
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
    // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;
    // full window mode is the initial mode, set it anyway
    display.setFullWindow();
    // here we use paged drawing, even if the processor has enough RAM for full buffer
    // so this can be used with any supported processor board.
    // the cost in code overhead and execution time penalty is marginal
    // tell the graphics class to use paged drawing mode
    display.firstPage();
    do {
        // this part of code is executed multiple times, as many as needed,
        // in case of full buffer it is executed once
        // IMPORTANT: each iteration needs to draw the same, to avoid strange effects
        // use a copy of values that might change, don't read e.g. from analog or pins in the loop!
        display.fillScreen(GxEPD_WHITE); // set the background to white (fill the buffer with value for white)
        display.setCursor(x, y); // set the postition to start printing text
        display.print(text); // print some text
        // end of part executed multiple times
    }
    // tell the graphics class to transfer the buffer content (page) to the controller buffer
    // the graphics class will command the controller to refresh to the screen when the last page has been transferred
    // returns true if more pages need be drawn and transferred
    // returns false if the last page has been transferred and the screen refreshed for panels without fast partial update
    // returns false for panels with fast partial update when the controller buffer has been written once more, to make the differential buffers equal
    // (for full buffered with fast partial update the (full) buffer is just transferred again, and false returned)
    while (display.nextPage());
    //Debug("helloWorld done\n");
}

void helloFullScreenPartialMode() {
    //Debug("helloFullScreenPartialMode\n");
    const char fullscreen[] = "full screen update";
    const char fpm[] = "fast partial mode";
    const char spm[] = "slow partial mode";
    const char npm[] = "no partial mode";
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    const char* updatemode;
    if (display.epd2.hasFastPartialUpdate) {
        updatemode = fpm;
    } else if (display.epd2.hasPartialUpdate) {
        updatemode = spm;
    } else {
        updatemode = npm;
    }
    // do this outside of the loop
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    // center update text
    display.getTextBounds(fullscreen, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t utx = ((display.width() - tbw) / 2) - tbx;
    uint16_t uty = ((display.height() / 4) - tbh / 2) - tby;
    // center update mode
    display.getTextBounds(updatemode, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t umx = ((display.width() - tbw) / 2) - tbx;
    uint16_t umy = ((display.height() * 3 / 4) - tbh / 2) - tby;
    // center HelloWorld
    display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t hwx = ((display.width() - tbw) / 2) - tbx;
    uint16_t hwy = ((display.height() - tbh) / 2) - tby;
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(hwx, hwy);
        display.print(HelloWorld);
        display.setCursor(utx, uty);
        display.print(fullscreen);
        display.setCursor(umx, umy);
        display.print(updatemode);
    } while (display.nextPage());
    //Debug("helloFullScreenPartialMode done\n");
}

void helloArduino() {
    //Debugln("helloArduino");
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    // align with centered HelloWorld
    display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    // height might be different
    display.getTextBounds(HelloArduino, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t y = ((display.height() / 4) - tbh / 2) - tby; // y is base line!
    // make the window big enough to cover (overwrite) descenders of previous text
    uint16_t wh = FreeMonoBold9pt7b.yAdvance;
    uint16_t wy = (display.height() / 4) - wh / 2;
    display.setPartialWindow(0, wy, display.width(), wh);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        //display.drawRect(x, y - tbh, tbw, tbh, GxEPD_BLACK);
        display.setCursor(x, y);
        display.print(HelloArduino);
    } while (display.nextPage());
    delay(1000);
    Debug("helloArduino done\n");
}

void helloEpaper() {
    //Debug("helloEpaper\n");
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    // align with centered HelloWorld
    display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    // height might be different
    display.getTextBounds(HelloEpaper, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t y = (display.height() * 3 / 4) + tbh / 2; // y is base line!
    // make the window big enough to cover (overwrite) descenders of previous text
    uint16_t wh = FreeMonoBold9pt7b.yAdvance;
    uint16_t wy = (display.height() * 3 / 4) - wh / 2;
    display.setPartialWindow(0, wy, display.width(), wh);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(HelloEpaper);
    } while (display.nextPage());
    //Debug("helloEpaper done\n");
}

void helloValue(double v, char * format) {
    Debug("helloValue\n");
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
    char valueString[20];
    snprintf(valueString, sizeof(valueString), format, v);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(valueString, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = (display.height() * 3 / 4) + tbh / 2; // y is base line!
    // show what happens, if we use the bounding box for partial window
    uint16_t wx = (display.width() - tbw) / 2;
    uint16_t wy = (display.height() * 3 / 4) - tbh / 2;
    display.setPartialWindow(wx, wy, tbw, tbh);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(valueString);
    } while (display.nextPage());
    delay(2000);
    // make the partial window big enough to cover the previous text
    uint16_t ww = tbw; // remember window width
    display.getTextBounds(HelloEpaper, 0, 0, &tbx, &tby, &tbw, &tbh);
    // adjust, because HelloEpaper was aligned, not centered (could calculate this to be precise)
    ww = std::max(ww, uint16_t(tbw + 12)); // 12 seems ok
    wx = (display.width() - tbw) / 2;
    // make the window big enough to cover (overwrite) descenders of previous text
    uint16_t wh = FreeMonoBold9pt7b.yAdvance;
    wy = (display.height() * 3 / 4) - wh / 2;
    display.setPartialWindow(wx, wy, ww, wh);
    // alternately use the whole width for partial window
    //display.setPartialWindow(0, wy, display.width(), wh);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(valueString);
    } while (display.nextPage());
    //Debug("helloValue done\n");
}

void deepSleepTest() {
    //Debug("deepSleepTest\n");
    const char hibernating[] = "hibernating ...";
    const char wokeup[] = "woke up";
    const char from[] = "from deep sleep";
    const char again[] = "again";
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    // center text
    display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(hibernating);
    } while (display.nextPage());
    display.hibernate();
    delay(5000);
    display.getTextBounds(wokeup, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t wx = (display.width() - tbw) / 2;
    uint16_t wy = (display.height() / 3) + tbh / 2; // y is base line!
    display.getTextBounds(from, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t fx = (display.width() - tbw) / 2;
    uint16_t fy = (display.height() * 2 / 3) + tbh / 2; // y is base line!
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(wx, wy);
        display.print(wokeup);
        display.setCursor(fx, fy);
        display.print(from);
    } while (display.nextPage());
    delay(5000);
    display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t hx = (display.width() - tbw) / 2;
    uint16_t hy = (display.height() / 3) + tbh / 2; // y is base line!
    display.getTextBounds(again, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t ax = (display.width() - tbw) / 2;
    uint16_t ay = (display.height() * 2 / 3) + tbh / 2; // y is base line!
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(hx, hy);
        display.print(hibernating);
        display.setCursor(ax, ay);
        display.print(again);
    } while (display.nextPage());
    display.hibernate();
    //Debug("deepSleepTest done\n");
}

void showBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial) {
    //Debug("showBox\n");
    display.setRotation(1);
    if (partial) {
        display.setPartialWindow(x, y, w, h);
    } else {
        display.setFullWindow();
    }
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.fillRect(x, y, w, h, GxEPD_BLACK);
    } while (display.nextPage());
    //Debug("showBox done\n");
}

void drawCornerTest() {
    display.setFullWindow();
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    for (uint16_t r = 0; r <= 4; r++) {
        display.setRotation(r);
        display.firstPage();
        do {
            display.fillScreen(GxEPD_WHITE);
            display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
            display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
            display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
            display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
            display.setCursor(display.width() / 2, display.height() / 2);
            display.print(display.getRotation());
        } while (display.nextPage());
        delay(2000);
    }
}

void drawFont(const char name[], const GFXfont* f) {
    //display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(f);
    display.setCursor(0, 0);
    display.println();
    display.println(name);
    display.println(" !\"#$%&'()*+,-./");
    display.println("0123456789:;<=>?");
    display.println("@ABCDEFGHIJKLMNO");
    display.println("PQRSTUVWXYZ[\\]^_");
    if (display.epd2.hasColor) {
        display.setTextColor(GxEPD_RED);
    }
    display.println("`abcdefghijklmno");
    display.println("pqrstuvwxyz{|}~ ");
}

void showFont(const char name[], const GFXfont* f) {
    display.setFullWindow();
    display.setRotation(0);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do {
        drawFont(name, f);
    } while (display.nextPage());
}

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()
// showPartialUpdate() purposely uses values that are not multiples of 8 to test this

void showPartialUpdate() {
    // some useful background
    helloWorld();
    // use asymmetric values for test
    uint16_t box_x = 10;
    uint16_t box_y = 15;
    uint16_t box_w = 70;
    uint16_t box_h = 20;
    uint16_t cursor_y = box_y + box_h - 6;
    float value = 13.95;
    uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    // show where the update box is
    for (uint16_t r = 0; r < 4; r++) {
        display.setRotation(r);
        display.setPartialWindow(box_x, box_y, box_w, box_h);
        display.firstPage();
        do {
            display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
            //display.fillScreen(GxEPD_BLACK);
        } while (display.nextPage());
        delay(2000);
        display.firstPage();
        do {
            display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        } while (display.nextPage());
        delay(1000);
    }
    //return;
    // show updates in the update box
    for (uint16_t r = 0; r < 4; r++) {
        display.setRotation(r);
        display.setPartialWindow(box_x, box_y, box_w, box_h);
        for (uint16_t i = 1; i <= 10; i += incr) {
            display.firstPage();
            do {
                display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
                display.setCursor(box_x, cursor_y);
                display.print(value * i, 2);
            } while (display.nextPage());
            delay(500);
        }
        delay(1000);
        display.firstPage();
        do {
            display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        } while (display.nextPage());
        delay(1000);
    }
}

struct bitmap_pair {
    const unsigned char* black;
    const unsigned char* red;
};


void drawBitmaps3c104x212() {
    bitmap_pair bitmap_pairs[] = {
        {Bitmap3c104x212_1_black, Bitmap3c104x212_1_red},
        {Bitmap3c104x212_2_black, Bitmap3c104x212_2_red},
        {WS_Bitmap3c104x212_black, WS_Bitmap3c104x212_red}
    };

    if (display.epd2.panel == GxEPD2::GDEW0213Z16) {
        for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++) {
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
                if (bitmap_pairs[i].red == WS_Bitmap3c104x212_red) {
                    display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
                } else
                    display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
            } while (display.nextPage());
            delay(2000);
        }
    }
}


#ifdef _GxBitmaps3c400x300_H_
void drawBitmaps3c400x300() {
    bitmap_pair bitmap_pairs[] = {
        {Bitmap3c400x300_1_black, Bitmap3c400x300_1_red},
        {Bitmap3c400x300_2_black, Bitmap3c400x300_2_red},
        {WS_Bitmap3c400x300_black, WS_Bitmap3c400x300_red}
    };

    if (display.epd2.panel == GxEPD2::GDEW042Z15) {
        for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++) {
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
                display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
            } while (display.nextPage());
            delay(2000);
        }
    }
}
#endif


void drawBitmaps() {
    display.setFullWindow();
    // 3-color
    drawBitmaps3c104x212();
#ifdef _GxBitmaps3c400x300_H_
    drawBitmaps3c400x300();
#endif
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
    Debug("setup\n");
    if (!display.init()) {
        Debug("Display init failed\n");
        return -1;
    }
    // first update should be full refresh
    helloWorld();
    delay(1000);
    // partial refresh mode can be used to full screen,
    // effective if display panel hasFastPartialUpdate
    helloFullScreenPartialMode();
    delay(1000);
    helloArduino();
    delay(1000);
    helloEpaper();
    delay(1000);
    //helloValue(123.9, 1);
    //delay(1000);
    showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
    delay(1000);
    drawBitmaps();
    if (display.epd2.hasPartialUpdate) {
        showPartialUpdate();
        delay(1000);
    } // else // on GDEW0154Z04 only full update available, doesn't look nice
    //drawCornerTest();
    //showBox(16, 16, 48, 32, false);
    //showBox(16, 56, 48, 32, true);
    display.powerOff();
    deepSleepTest();
    Debug("setup done\n");
    return 0;
}

void loop() {
}
