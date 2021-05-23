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
*   Now xStart and xStart can control the position of the picture normally,
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

#include <stdio.h>

#include "BMPfile.h"
#include "GxEPD2.h"
#include "Debug.h"

/*Bitmap file header   14bit*/
typedef struct BMP_FILE_HEADER {
    uint16_t bType;        //File identifier
    uint32_t bSize;      //The size of the file
    uint16_t bReserved1;   //Reserved value, must be set to 0
    uint16_t bReserved2;   //Reserved value, must be set to 0
    uint32_t bOffset;    //The offset from the beginning of the file header to the beginning of the image data bit
} __attribute__ ((packed)) BMPFILEHEADER;    // 14bit

/*Bitmap information header  40bit*/
typedef struct BMP_INFO {
    uint32_t biInfoSize;      //The size of the header
    uint32_t biWidth;         //The width of the image
    uint32_t biHeight;        //The height of the image
    uint16_t biPlanes;          //The number of planes in the image
    uint16_t biBitCount;        //The number of bits per pixel
    uint32_t biCompression;   //Compression type
    uint32_t bimpImageSize;   //The size of the image, in bytes
    uint32_t biXPelsPerMeter; //Horizontal resolution
    uint32_t biYPelsPerMeter; //Vertical resolution
    uint32_t biClrUsed;       //The number of colors used
    uint32_t biClrImportant;  //The number of important colors
} __attribute__ ((packed)) BMPINFOHEADER;

/*Color table: palette */
typedef struct RGB_QUAD {
    uint8_t rgbBlue;               //Blue intensity
    uint8_t rgbGreen;              //Green strength
    uint8_t rgbRed;                //Red intensity
    uint8_t rgbReversed;           //Reserved value
} __attribute__ ((packed)) BMPRGBQUAD;

/*************************************************************************

Read a monochrome (single-bit) bitmap from a .BMP file to a specified location in the display buffer
This does not actually update the display itself

Params: path -- path to file
        buffer: buffer to read to (assumed to be the same format as the bitmap -- bit 7 of byte = first bit
        xStart, yStart: coordinates of the top-left corner of the image (may be negative)
        bufWidth, bufHeight: Width & Height of display, for clipping
        invert: Invert image colours (0xff = invert, 0x00 = don't change)
        mirrorY: Invert the image in the Y axis
*************************************************************************/

bool readBmp_Mono(const char *path, uint8_t *buffer, int16_t xStart, int16_t yStart, uint16_t bufWidth, uint16_t bufHeight, uint8_t invert, bool mirrorY) {
    // Binary file open
    FILE *fp;
    if((fp = fopen(path, "rb")) == NULL) {
        Debug("Cann't open the file!\n");
        return false;
    }

    BMPFILEHEADER bmpFileHeader;
    BMPINFOHEADER bmpInfoHeader;
    // Set the file pointer from the beginning
    fseek(fp, 0, SEEK_SET);
    fread(&bmpFileHeader, sizeof(BMPFILEHEADER), 1, fp);    //sizeof(BMPFILEHEADER) must be 14
    fread(&bmpInfoHeader, sizeof(BMPINFOHEADER), 1, fp);    //sizeof(BMPFILEHEADER) must be 50
    Debug("pixel = %d * %d\r\n", bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);

    // Determine if it is a monochrome bitmap
    if(bmpInfoHeader.biBitCount != 1) {
        Debug("%s is not a monochrome bitmap!\n", path);
        fclose(fp);
        return false;
    }

    uint16_t Image_Width_Byte = (bmpInfoHeader.biWidth % 8 == 0)? (bmpInfoHeader.biWidth / 8): (bmpInfoHeader.biWidth / 8 + 1);
    uint16_t bufWidthByte = (bufWidth % 8 == 0)? (bufWidth / 8): (bufWidth / 8 + 1);
    uint16_t Bmp_Width_Byte = (Image_Width_Byte % 4 == 0) ? Image_Width_Byte: ((Image_Width_Byte / 4 + 1) * 4);

    // Determine black and white based on the palette
    uint16_t bmprgbquadsize = 1 << bmpInfoHeader.biBitCount;// 2^1 = 2
    BMPRGBQUAD bmprgbquad[bmprgbquadsize];        //palette
    fread(&bmprgbquad, sizeof(BMPRGBQUAD), bmprgbquadsize, fp);

    // Read image data into the cache
    uint8_t Rdata;
    fseek(fp, bmpFileHeader.bOffset, SEEK_SET);
    for(uint16_t y = 0; y < bmpInfoHeader.biHeight; y++) {//Total display column
        for(uint16_t x = 0; x < Bmp_Width_Byte; x++) {//Show a line in the line
            if(fread((char *)&Rdata, 1, 1, fp) != 1) {
                Debug("read failed: %s\n", path);
                fclose(fp);
                return false;
            }
            if(x*8 + xStart >= 0 && y + yStart >= 0 && x + xStart < bufWidthByte && x * 8 + xStart < bufWidth  && y + yStart < bufHeight) { //bmp
                buffer[x + xStart + ((mirrorY ? y : bmpInfoHeader.biHeight - y - 1) + yStart) * bufWidthByte] =  Rdata ^ invert;
                // Debug("rdata = %d\r\n", Rdata);
            }
        }
    }
    fclose(fp);
    return true;
}

/*************************************************************************

Read a gray-scale (4-bit) bitmap from a .BMP file to a specified location in the display buffer.
This does not actually update the display itself
UNTESTED!!!!!
Params: path -- path to file
        buffer: buffer to read to (assumed to be the same format as the bitmap -- bit 7 of byte = first bit
        xStart, yStart: coordinates of the top-left corner of the image (may be negative)
        bufWidth, bufHeight: Width & Height of display, for clipping
        invert: Invert image colours (0xff = invert, 0x00 = don't change)
        mirrorY: Invert the image in the Y axis

*************************************************************************/
bool readBmp_4Gray(const char *path, uint8_t *buffer, uint16_t xStart, uint16_t yStart, uint16_t bufWidth, uint16_t bufHeight, uint8_t invert, bool mirrorY) {
    // Binary file open
    FILE *fp;
    if((fp = fopen(path, "rb")) == NULL) {
        Debug("Cann't open the file!\n");
        return false;
    }

    BMPFILEHEADER bmpFileHeader;
    BMPINFOHEADER bmpInfoHeader;
    // Set the file pointer from the beginning
    fseek(fp, 0, SEEK_SET);
    fread(&bmpFileHeader, sizeof(BMPFILEHEADER), 1, fp);    //sizeof(BMPFILEHEADER) must be 14
    fread(&bmpInfoHeader, sizeof(BMPINFOHEADER), 1, fp);    //sizeof(BMPFILEHEADER) must be 50
    Debug("pixel = %d * %d\r\n", bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);

    // Determine if it is a monochrome bitmap
    if(bmpInfoHeader.biBitCount != 4) {
        Debug("%s is not a 4-bit bitmap!\n", path);
        return false;
    }

    uint16_t Image_Width_Byte = (bmpInfoHeader.biWidth % 4 == 0)? (bmpInfoHeader.biWidth / 4): (bmpInfoHeader.biWidth / 4 + 1);
    uint16_t bufWidthByte = (bufWidth % 2 == 0)? (bufWidth / 2): (bufWidth / 2 + 1);
    uint16_t Bmp_Width_Byte = (Image_Width_Byte % 2 == 0) ? Image_Width_Byte: ((Image_Width_Byte / 2 + 1) * 4);

    // Read image data into the buffer
    uint8_t Rdata;
    fseek(fp, bmpFileHeader.bOffset, SEEK_SET);

    for(uint16_t y = 0; y < bmpInfoHeader.biHeight; y++) {//Total display column
        for(uint16_t x = 0; x < Bmp_Width_Byte; x++) {//Show a line in the line
            if(fread((char *)&Rdata, 1, 1, fp) != 1) {
                Debug("Read failed in %s\n", path);
                fclose(fp);
                return false;
            }
            if(x * 2 + xStart >= 0 && y + yStart >= 0 && x + xStart < bufWidthByte && x * 2 + xStart < bufWidth  && y + yStart < bufHeight) { //bmp
                buffer[x + xStart + ((mirrorY ? y : bmpInfoHeader.biHeight - y - 1) + yStart) * bufWidthByte] =  Rdata ^ invert;
                // Debug("rdata = %d\r\n", Rdata);
            }
        }
    }
    fclose(fp);
    return true;
}


/*************************************************************************

Read a RGB (24-bit) bitmap from a .BMP file to a specified location in the display buffer.
This does not actually update the display itself
UNTESTED!!!!!
Params: path -- path to file
        buffer: buffer to read to (assumed to be 1 uint16 per pixel, containing
        xStart, yStart: coordinates of the top-left corner of the image (may be negative)
        bufWidth, bufHeight: Width & Height of display, for clipping
        mirrorY: Invert the image in the Y axis

*************************************************************************/
bool readBmp_RGB_7Color(const char *path, uint16_t *buffer, uint16_t xStart, uint16_t yStart, uint16_t bufWidth, uint16_t bufHeight, bool mirrorY) {
    // Binary file open
    FILE *fp;
    if((fp = fopen(path, "rb")) == NULL) {
        Debug("Cann't open the file!\n");
        return false;
    }

    BMPFILEHEADER bmpFileHeader;
    BMPINFOHEADER bmpInfoHeader;
    // Set the file pointer from the beginning
    fseek(fp, 0, SEEK_SET);
    fread(&bmpFileHeader, sizeof(BMPFILEHEADER), 1, fp);    //sizeof(BMPFILEHEADER) must be 14
    fread(&bmpInfoHeader, sizeof(BMPINFOHEADER), 1, fp);    //sizeof(BMPFILEHEADER) must be 50
    Debug("pixel = %d * %d\r\n", bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);

    // Determine if it is a monochrome bitmap
    if(bmpInfoHeader.biBitCount != 24) {
        Debug("%s is not a 4-bit bitmap!\n", path);
        return false;
    }

    // Read image data into the cache
    uint8_t Rdata[3];
    fseek(fp, bmpFileHeader.bOffset, SEEK_SET);

    for(uint16_t y = 0; y < bmpInfoHeader.biHeight; y++) {
        for(uint16_t x = 0; x < bmpInfoHeader.biWidth ; x++) {
            if(fread((char *)Rdata, 3, 1, fp) != 1) {
                fclose(fp);
                return false;
                break;
            }
            uint16_t colour;
            if(Rdata[0] == 0 && Rdata[1] == 0 && Rdata[2] == 0) {
                colour =  GxEPD_BLACK;
            } else if(Rdata[0] == 255 && Rdata[1] == 255 && Rdata[2] == 255) {
                colour =  GxEPD_WHITE;
            } else if(Rdata[0] == 0 && Rdata[1] == 255 && Rdata[2] == 0) {
                colour = GxEPD_GREEN;
            } else if(Rdata[0] == 255 && Rdata[1] == 0 && Rdata[2] == 0) {
                colour =  GxEPD_BLUE;
            } else if(Rdata[0] == 0 && Rdata[1] == 0 && Rdata[2] == 255) {
                colour =  GxEPD_RED;
            } else if(Rdata[0] == 0 && Rdata[1] == 255 && Rdata[2] == 255) {
                colour =  GxEPD_YELLOW;
            } else if(Rdata[0] == 0 && Rdata[1] == 128 && Rdata[2] == 255) {
                colour =  GxEPD_ORANGE;
            }
            if(x + xStart >= 0 && y + yStart >= 0 && x + xStart < bufWidth  && y + yStart < bufHeight)
                buffer[x + xStart + ((mirrorY ? y : bmpInfoHeader.biHeight - y - 1) + yStart) * bufWidth] =  colour;
        }
    }
    fclose(fp);
    return true;
}


