/*****************************************************************************
* | File      	:   GUI_BMPfile.h
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* |	This version:   V2.2
* | Date        :   2020-07-08
* | Info        :
* -----------------------------------------------------------------------------
* V2.2(2020-07-08):
* 1.Add GUI_ReadBmp_RGB_7Color()
* V2.1(2019-10-10):
* 1.Add GUI_ReadBmp_4Gray()
* V2.0(2018-11-12):
* 1.Change file name: GUI_BMP.h -> GUI_BMPfile.h
* 2.fix: GUI_ReadBmp()
*   Now Xstart and Xstart can control the position of the picture normally,
*   and support the display of images of any size. If it is larger than
*   the actual display range, it will not be displayed.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef __GUI_BMPFILE_H
#define __GUI_BMPFILE_H

#include <stdint.h>
#include "GxEPD2_EPD.h"
class BMPfile
{
  public:
static bool readBmpMono(const char *path, uint8_t *buffer, int16_t dispXBytes, int16_t dispY, uint16_t dispWidth, uint16_t dispHeight, GxEPD2_EPD::writeMode mode, bool mirrorY);
static bool readBmp4Gray(const char *path, uint8_t *buffer, int16_t xStart, int16_t yStart, uint16_t bufWidth, uint16_t bufHeight, uint8_t invert = 0, bool mirrorY = false);
static bool readBmpRgb7Color(const char *path, uint16_t *buffer, int16_t xStart, int16_t yStart, uint16_t bufWidth, uint16_t bufHeight, bool invert = false, bool mirrorY = false);
};

#endif
