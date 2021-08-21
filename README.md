# GxEPD2 forked from https://github.com/ZinggJM/GxEPD2
## Arduino / Raspberry Pi Display Library for SPI E-Paper Displays

### Notes
- See https://github.com/ZinggJM/GxEPD2 for documentation on supported e-paper devices
- This fork extends GxEPD2 to work on a Raspberry Pi
- Uses the wiringPi library for access to pins
- SPI interface to the e-paper device 
- Includes a simple version of the AdaFruit GFX library
- This fork is of v1.3.7

### important note :
- the display panels are for 3.3V supply and 3.3V data lines
- never connect data lines directly to 5V Arduino data pins, use e.g. 4k7/10k resistor divider
- series resistor only is not enough for reliable operation (back-feed effect through protection diodes)
- 4k7/10k resistor divider may not work with flat cable extensions or Waveshare 4.2 board, use level converter then
- do not forget to connect GND
- the actual Waveshare display boards now have level converters and series regulator, safe for 5V
- use 3k3 pull-down on SS for ESP8266 for boards with level converters
- note that 7.5" e-paper displays don't work reliable if fed from 3.3V Arduino pin
- note that Waveshare bords with "clever" reset circuit may need shortened reset pulse
- use `init(115200, true, 2, false)` for Waveshare boards with "clever" reset circuit
- note that Waveshare bords with "clever" reset circuit need 1k pull-up on RST on ESP8266, or different pin

### Paged Drawing, Picture Loop
 - This library uses paged drawing to limit RAM use and cope with missing single pixel update support
 - buffer size can be selected in the application by template parameter page_height, see GxEPD2_Example
 - Paged drawing is implemented as picture loop, like in U8G2 (Oliver Kraus)
 - see https://github.com/olikraus/u8glib/wiki/tpictureloop
 - Paged drawing is also available using drawPaged() and drawCallback(), like in GxEPD
- ` // GxEPD style paged drawing; drawCallback() is called as many times as needed `
- ` void drawPaged(void (*drawCallback)(const void*), const void* pv) `
- paged drawing is done using Adafruit_GFX methods inside picture loop or drawCallback

### Full Screen Buffer Support
 - full screen buffer is selected by setting template parameter page_height to display height
 - drawing to full screen buffer is done using Adafruit_GFX methods without picture loop or drawCallback
 - and then calling method display()

### Low Level Bitmap Drawing Support
 - bitmap drawing support to the controller memory and screen is available:
 - either through the template class instance methods that forward calls to the base display class
 - or directly using an instance of a base display class and calling its methods directly

### Supporting Arduino Forum Topics:

- Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
- Good Display ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

### Note on documentation
- GxEPD2 uses Adafruit_GFX for Graphics and Text support, which is well documented there
- GxEPD2 uses meaningful method names, and has some comments in the header files
- consult the header files GxEPD2_BW.h, GxEPD2_3C.h and GxEPD2_GFX.h
- for the concept of paged drawing and picture loop see: 
- https://github.com/olikraus/u8glib/wiki/tpictureloop

### Supported SPI e-paper panels from Good Display:
- GDEW0102T4     1.02" b/w
- GDEP015OC1     1.54" b/w
- GDEH0154D67    1.54" b/w, replacement for GDEP015OC1
- GDEW0154T8     1.54" b/w 152x152
- GDEW0154M09    1.54" b/w 200x200
- GDEW0154M10    1.54" b/w 152x152 DES
- GDEW0154Z04    1.54" b/w/r 200x200
- GDEH0154Z90    1.54" b/w/r, replacement for GDEW0154Z04
- GDE0213B1      2.13" b/w
- GDEH0213B72    2.13" b/w, replacement for GDE0213B1
- GDEH0213B73    2.13" b/w, new replacement for GDE0213B1, GDEH0213B72
- GDEM0213B74    2.13" b/w
- GDEW0213I5F    2.13" b/w flexible
- GDEW0213T5D    2.13" b/w
- GDEW0213M21    2.13" b/w DES
- GDEW0213Z16    2.13" b/w/r
- GDEW0213Z19    2.13" b/w/r
- GDEH029A1      2.9" b/w
- GDEW029T5      2.9" b/w
- GDEW029T5D     2.9" b/w
- GDEM029T94     2.9" b/w
- GDEW029M06     2.9" b/w DES
- GDEW029Z10     2.9" b/w/r
- GDEW029Z13     2.9" b/w/r
- GDEM029C90     2.9" b/w/y
- GDEW026T0      2.6" b/w
- GDEW026M01     2.6" b/w DES
- GDEW027C44     2.7" b/w/r
- GDEW027W3      2.7" b/w
- GDEW0371W7     3.7" b/w
- GDEW042T2      4.2" b/w
- GDEW042M01     4.2" b/w DES
- GDEW042Z15     4.2" b/w/r
- ACeP565        5.65" Waveshare 5.65" 7-color e-paper display 600x448
- GDEW0583T7     5.83" b/w
- GDEW0583T8     5.83" b/w 648x460
- GDEW075T8      7.5" b/w
- GDEW075T7      7.5" b/w 800x480
- GDEW075Z09     7.5" b/w/r
- GDEW075Z08     7.5" b/w/r 800x480
- GDEH075Z90     7.5" b/w/r 880x528
- GDEH116T91    11.6" b/w 960x640
- GDEW1248T3    12.48 b/w 1304x984
#### Supported SPI e-paper panels & boards from Waveshare: compare with Good Display, same panel
#### other supported panels
- ED060SCT        6" grey levels, on Waveshare e-Paper IT8951 Driver HAT
- ED060KC1        6" grey levels, 1448x1072, on Waveshare e-Paper IT8951 Driver HAT
- ED078KC2        7.8" grey levels, 1872x1404, on Waveshare e-Paper IT8951 Driver HAT

### I can and will only support e-paper panels I have!
- promotion panels from suppliers are welcome, to add support to GxEPD2
- donation panels from users are welcome, to add support to GxEPD2

### Version 1.3.7
- added support for GDEW0102T4 1.02" b/w 80x128 panel or Waveshare 1.02" b/w board
- the GDEW0102T4 panel has a different connector than the other SPI panels
- the GDEW0102T4 panel can be used with the DESPI-C102 connection module
- https://buy-lcd.com/products/connection-board-demo-kit-for-102-inch-e-ink-display-despi-c102
- my test have been done with the Waveshare 1.02" b/w board
#### Version 1.3.6
- added support for Waveshare 2.9" b/w V2, driver class GxEPD2_290_T94_V2
- Waveshare 2.9" b/w V2 uses a GDEM029T94 variant without partial update wft in OTP
- driver class GxEPD2_290_T94_V2 uses partial update wft written to registers
- added NOTE for RST pull-up on ESP8266 with "clever" reset circuit, or alternate pin
#### Version 1.3.5
- added support for GDEM0213B74 122x250 b/w e-paper panel
- added support for ED078KC2, 7.8" grey levels, 1872x1404, on Waveshare e-Paper IT8951 Driver HAT
#### Version 1.3.4
- added support for GDEH116T91 960x640 b/w e-paper panel
- GDEH116T91 has only full screen refresh, no wavetable for differential refresh yet
- added support for processor Arduino Nano RP2040 Connect to the examples
- added general fast b/w refresh for capable 3-color displays GDEW0213Z19, GDEW029Z13
- added example GxEPD2x_FastBlackWhiteOnColor.ino for GDEW0213Z19, GDEW029Z13
- evaluation of other fast b/w capable 3-color panels may follow
#### Version 1.3.3
- added b/w differential refresh method to GDEW0213Z19
- added b/w differential refresh method to GDEW029Z13
- up to 100 b/w fast refreshes have been possible (resulting in slightly reddish background)
- added experimental example GxEPD2x_MixedTest for proof of concept
- GxEPD2x_MixedTest needs enough RAM for 2 GFXCanvas1 objects
- general 3-color mixed refresh in GxEPD2 is in evaluation, for capable panels
