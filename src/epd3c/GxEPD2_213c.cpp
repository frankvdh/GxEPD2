// Display Library for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: these e-papers require 3.3V supply AND data lines!
//
// based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
// Controller: IL0373 : http://www.e-paper-display.com/download_detail/downloadsId=535.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

#include "GxEPD2_213c.h"

GxEPD2_213c::GxEPD2_213c(int8_t cs, int8_t dc, int8_t rst, int8_t busy) :
  GxEPD2_EPD(cs, dc, rst, busy, LOW, 20000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
}

void GxEPD2_213c::clearScreen(uint8_t value)
{
  clearScreen(value, 0xFF);
}

void GxEPD2_213c::clearScreen(uint8_t black_value, uint8_t color_value)
{
  _initial_write = false; // initial full screen buffer clean done
  _Init_Part();
  _writeCommand(0x91);	// Partial In (PTIN)
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x10);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(black_value);
  }
  _writeCommand(0x13);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(color_value);
  }
  _Update_Part();
  _writeCommand(0x92); // Partial Out (PTOUT)
}

void GxEPD2_213c::writeScreenBuffer(uint8_t value)
{
  writeScreenBuffer(value, 0xFF);
}

void GxEPD2_213c::writeScreenBuffer(uint8_t black_value, uint8_t color_value)
{
  _initial_write = false; // initial full screen buffer clean done
  _Init_Part();
  _writeCommand(0x91);	// Partial In (PTIN)
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x10);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(black_value);
  }
  _writeCommand(0x13);	// Data Start Transmission 2 (DTM2)
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++) {
    _writeData(color_value);
  }
  _writeCommand(0x92); // Partial Out (PTOUT)
}

void GxEPD2_213c::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(bitmap, NULL, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_213c::writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_initial_write) writeScreenBuffer(); // initial full screen buffer clean
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  int16_t wb = (w + 7) / 8; // width bytes, bitmaps are padded
  x -= x % 8; // byte boundary
  w = wb * 8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  _Init_Part();
  _writeCommand(0x91);	// Partial In (PTIN)
  _setPartialRamArea(x1, y1, w1, h1);
  _writeCommand(0x10);	// Data Start Transmission 1 (DTM1)
  for (int16_t i = 0; i < h1; i++) {
    for (int16_t j = 0; j < w1 / 8; j++) {
      uint8_t data = 0xFF;
      if (black)
      {
        // use wb, h of bitmap for index!
        int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
        data = pgm ?
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          pgm_read_byte(&black[idx])
#else
          black[idx]
#endif
        : black[idx];
        if (invert) data = ~data;
      }
      _writeData(data);
    }
  }
  _writeCommand(0x13);	// Data Start Transmission 2 (DTM2)
  for (int16_t i = 0; i < h1; i++) {
    for (int16_t j = 0; j < w1 / 8; j++) {
      uint8_t data = 0xFF;
      if (color)
      {
        // use wb, h of bitmap for index!
        int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
        data = pgm ?
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          pgm_read_byte(&color[idx])
#else
          color[idx]
#endif
        : color[idx];
        if (invert) data = ~data;
      }
      _writeData(data);
    }
  }
  _writeCommand(0x92); // Partial Out (PTOUT)
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}

void GxEPD2_213c::writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                 int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImagePart(bitmap, NULL, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_213c::writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                 int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_initial_write) writeScreenBuffer(); // initial full screen buffer clean
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
  if ((x_part < 0) || (x_part >= w_bitmap)) return;
  if ((y_part < 0) || (y_part >= h_bitmap)) return;
  int16_t wb_bitmap = (w_bitmap + 7) / 8; // width bytes, bitmaps are padded
  x_part &= 0xfff8; // byte boundary
  w = w_bitmap - x_part < w ? w_bitmap - x_part : w; // limit
  h = h_bitmap - y_part < h ? h_bitmap - y_part : h; // limit
  x &= 0xfff8; // byte boundary
  w = 8 * ((w + 7) / 8); // byte boundary, bitmaps are padded
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_using_partial_mode) _Init_Part();
  _writeCommand(0x91);	// Partial In (PTIN)
  _setPartialRamArea(x1, y1, w1, h1);
  _writeCommand(0x10);	// Data Start Transmission 1 (DTM1)
  for (int16_t i = 0; i < h1; i++) {
    for (int16_t j = 0; j < w1 / 8; j++) {
      uint8_t data;
      // use wb_bitmap, h_bitmap of bitmap for index!
      int16_t idx = mirror_y ? x_part / 8 + j + dx / 8 + ((h_bitmap - 1 - (y_part + i + dy))) * wb_bitmap : x_part / 8 + j + dx / 8 + (y_part + i + dy) * wb_bitmap;
      data = pgm ?
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        pgm_read_byte(&black[idx])
#else
        black[idx]
#endif
      : black[idx];
      if (invert) data = ~data;
      _writeData(data);
    }
  }
  _writeCommand(0x13);	// Data Start Transmission 2 (DTM2)
  for (int16_t i = 0; i < h1; i++) {
    for (int16_t j = 0; j < w1 / 8; j++) {
      uint8_t data = 0xFF;
      if (color)
      {
        // use wb_bitmap, h_bitmap of bitmap for index!
        int16_t idx = mirror_y ? x_part / 8 + j + dx / 8 + ((h_bitmap - 1 - (y_part + i + dy))) * wb_bitmap : x_part / 8 + j + dx / 8 + (y_part + i + dy) * wb_bitmap;
        data = pgm ?
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          pgm_read_byte(&color[idx])
#else
          color[idx]
#endif
        : color[idx];
        if (invert) data = ~data;
      }
      _writeData(data);
    }
  }
  _writeCommand(0x92); // Partial Out (PTOUT)
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}

void GxEPD2_213c::writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data1)
  {
    writeImage(data1, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_213c::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_213c::drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImagePart(bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_213c::drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(black, color, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_213c::drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImagePart(black, color, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_213c::drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeNative(data1, data2, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_213c::refresh(bool partial_update_mode)
{
  if (partial_update_mode) refresh(0, 0, WIDTH, HEIGHT);
  else _Update_Full();
}

void GxEPD2_213c::refresh(int16_t x, int16_t y, int16_t w, int16_t h) {
  x &= 0xfff8; // byte boundary
  w &= 0xfff8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  w1 -= x1 - x;
  h1 -= y1 - y;
  _Init_Part();
  if (usePartialUpdateWindow) _writeCommand(0x91); // Partial In (PTIN)
  _setPartialRamArea(x1, y1, w1, h1);
  _Update_Part();
  if (usePartialUpdateWindow) _writeCommand(0x92); // Partial Out (PTOUT)
}

void GxEPD2_213c::powerOff()
{
  _PowerOff();
}



/** \brief: After this, the chip is in the deep-sleep mode to save power.
 *  Exit deep sleep mode via a hardware reset.
 *  Use _reset() to awaken and use InitDisplay to initialize.
 */
void GxEPD2_213c::hibernate() {
  _PowerOff();
  if (_rst >= 0) {
    _writeCommand(0x07); // Deep Sleep (DSLP)
    _writeData(0xA5);    //  check code
    _hibernating = true;
  }
}

void GxEPD2_213c::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t xe = (x + w - 1) | 0x0007; // byte boundary inclusive (last byte)
  uint16_t ye = y + h - 1;
  x &= 0xFFF8; // byte boundary
  _writeCommand(0x90);	 	// Partial Window (PTL)
//  _writeData(x >> 8);	Not in Datasheet
  _writeData(x & 0xff);			//  HRST = Horizontal start bank (LSB)
//  _writeData(xe >> 8);	Not in Datasheet
  _writeData(xe & 0xff);			//  HRED = Horizontal end bank
  _writeData(y >> 8);		//  VRST = Vertical start (MSB)
  _writeData(y & 0xff);		//  VRST = Vertical start(LSB)
  _writeData(ye >> 8);		//	VRED = Vertical end (MSB)
  _writeData(ye & 0xff);	//	VRED = Vertical end (LSB)
  _writeData(0x01);			//  PT_SCAN Gates scan only outside of window
							//   don't see any difference between 1 & 0
}

void GxEPD2_213c::_PowerOn()
{
  if (!_power_is_on)
  {
    _writeCommand(0x04);
    _waitWhileBusy("_PowerOn", power_on_time);
  }
  _power_is_on = true;
}

void GxEPD2_213c::_PowerOff() {
  _writeCommand(0x02); // Power Off (POF)
  _waitWhileBusy("_PowerOff", power_off_time);
  _power_is_on = false;
}

void GxEPD2_213c::_InitDisplay()
{
  if (_hibernating) _reset();
  _writeCommand(0x06);  // Booster Soft Start (BTST)
  _writeData (0x17);	//	_PHA[7:0] Phase A soft start period 10ms, strength 3,  Min 6.58us off time
  _writeData (0x17);	//	_PHB[7:0] Phase B soft start period 10ms, strength 3,  Min 6.58us off time
  _writeData (0x17);	//	_PHC[5:0] Phase C soft start period 10ms, strength 3,  Min 6.58us off time
  //_writeCommand(0x04);
  //_waitWhileBusy("_Init_Display Power On");
  _writeCommand(0X00);	// Panel Setting (PSR)
  _writeData(0x8f);		//	RES[1:0] = 10 -> Display resolution 128x296
						//	REG      = 0  -> LUT from OTP
						//	KW/R     = 0  -> KWR mode (black/white/red)
						//	UD       = 1  -> Scan up
						//	SHL      = 1  -> Shift right
						//	SHD_N    = 1  -> Booster ON
						//	RST_N    = 1  -> NOP
  _writeCommand(0x50); 	// VCOM and Data Interval Setting (CDI)
  _writeData(0x77);    	//	VBD[1:0] = 01 -> Border colour = LUTW
						//	DDX[1:0] = 11 -> Data Polarity red & BW
						//	CDI[3:0] = 0111 -> Vcom and  Data Interval = 10d
						//WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

  _writeCommand(0x61); 	// Resolution Setting (TRES)
  _writeData (0x68);   	//	HRES = 104
  _writeData (0x00);
  _writeData (0xd4);   	//	VRES = 212
}

void GxEPD2_213c::_Init_Full()
{
  _InitDisplay();
  _PowerOn();
}

void GxEPD2_213c::_Init_Part()
{
  _InitDisplay();
  _writeCommand(0x50); 	// Vcom and Data interval Setting (CDI)
  _writeData(0x77);    	//  VBB[1:0] = 01 = ? border data,
						//  DDX[1:0] = 11 = Red+Black data polarity,
						//  CDI[3:0] = 0111 = 10 hsync
  //_writeData(0xF7);  	//  WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
  _PowerOn();
}

void GxEPD2_213c::_Update_Full() {
  _writeCommand(0x12); 	// Display Refresh (DRF)
  _waitWhileBusy("_Update_Full", full_refresh_time);
}

void GxEPD2_213c::_Update_Part() {
  _writeCommand(0x12); 	// Display Refresh (DRF)
  _waitWhileBusy("_Update_Part", partial_refresh_time);
}


/** \brief read the OTP device
 *
 * \param buffer uint8_t* buffer to write data to
 * \param numBytes uint16_t number of bytes to read... one greater than the OTP size to allow for the first dummy
 * \return void
 *
 */
void GxEPD2_213c::readOtp(uint8_t *buffer, uint16_t numBytes) {
  _writeCommand(0xa2);  // Read OTP Data (ROTP)
  _writeData(buffer, numBytes);   // write dummy values, read data
}

