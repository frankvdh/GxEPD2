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

//#define DEBUG
#include "Debug.h"
#include "BMPfile.h"
#include "GxEPD2.h"

/*Bitmap file header   14 bytes */
typedef struct {
    uint16_t type;      //File identifier
    uint32_t size;      //The size of the file
    uint16_t reserved1; //Reserved value, must be set to 0
    uint16_t reserved2; //Reserved value, must be set to 0
    uint32_t offset;    // Ofset from beginning of file to beginning of image data
} __attribute__ ((packed)) HEADER;    // 14bit

/*Bitmap information header  40bit*/
typedef struct {
    uint32_t infoSize;          //The size of the header
    uint32_t width;             //The width of the image
    uint32_t height;            //The height of the image
    uint16_t planes;            //The number of planes in the image
    uint16_t bitCount;          //The number of bits per pixel
    uint32_t compression;       //Compression type
    uint32_t imageSize;         //The size of the image, in bytes
    uint32_t xResolution;       //Horizontal resolution
    uint32_t yResolution;       //Vertical resolution
    uint32_t colours;           //The number of colors used
    uint32_t coloursImportant;  //The number of important colors
} __attribute__ ((packed)) INFO;

/*Color table: palette */
typedef struct {
    uint8_t blue;               //Blue intensity
    uint8_t green;              //Green strength
    uint8_t red;                //Red intensity
    uint8_t reversed;           //Reserved value
} __attribute__ ((packed)) RGBQUAD;

/*************************************************************************

Read a monochrome (single-bit) bitmap from a .BMP file to a specified location in the display buffer
This does not actually update the display itself

Params: path -- path to file
        info: BMP INFO structure to return width asnd height
Returns: pointer to data
*************************************************************************/

bool BMPfile::readBmpMono(const char *path, uint8_t *buffer, int16_t dispXBytes, int16_t dispY, uint16_t dispWidth, uint16_t dispHeight, BMPfile::readMode mode, bool mirrorY) {
    Debug("readBmpMono %s, %d, %d, %d, %d\n", path, dispXBytes, dispY, dispWidth, dispHeight);
    FILE *fp;
    if((fp = fopen(path, "rb")) == NULL) {
        Debug("Can't open %s\n", path);
        return false;
    }

    HEADER header;
    INFO info;

    fseek(fp, 0, SEEK_SET);
    fread(&header, sizeof(header), 1, fp);    //sizeof(header) must be 14
    fread(&info, sizeof(INFO), 1, fp);    //sizeof(info) must be 50
    Debug("%d * %d, bit count = %d, compression = %d\n", info.width, info.height, info.bitCount, info.compression);

    // Determine if it is a monochrome bitmap
    if(info.bitCount != 1) {
        Debug("%s is not a monochrome bitmap!\n", path);
        fclose(fp);
        return false;
    }

    // Determine if it is a compressed bitmap
    if(info.compression != 0) {
        Debug("%s is a compressed bitmap!\n", path);
        fclose(fp);
        return false;
    }


    // Ignore palette
//    uint16_t rgbQuadSize = 1 << info.bitCount;// 2^1 = 2
//    RGBQUAD rgbQuad[rgbQuadSize];        //palette
//    fread(&rgbQuad, sizeof(RGBQUAD), rgbQuadSize, fp);

    uint16_t widthBytes = (info.width + 7) >> 3;
    if (dispY < 0) {
        fseek(fp, widthBytes * -dispY, SEEK_CUR);
        info.height += dispY;
        dispY = 0;
    }
    if (dispY + info.height > dispHeight) info.height = dispHeight - dispY;

    uint16_t dispWidthBytes = (dispWidth + 7) >> 3;
    uint16_t readBytes = (dispXBytes + widthBytes > dispWidth) ? (dispWidth - dispXBytes) : widthBytes;

    // Read image data into the buffer
    fseek(fp, header.offset, SEEK_SET);
    Debug("dispXBytes = %0x\n", dispXBytes);
    for(uint16_t y = 0; y < info.height; y++) { //Total display column
        Debug("\n%3d: ", y + dispY);
        if (dispXBytes < 0) {
            fseek(fp, -dispXBytes, SEEK_CUR);
        }
        for (uint16_t x = 0; x < readBytes; x++) {
            uint8_t data = 0xa5;
            if (fread(&data, 1, 1, fp) != 1) {
                Debug("read failed: %s\n", path);
                fclose(fp);
                return false;
            }
             switch (mode) {
            case BMPfile::OVERWRITE: buffer[(dispY + (mirrorY ? info.height - y - 1: y)) * dispWidthBytes + dispXBytes + x] = data; break;
            case BMPfile::INVERT: buffer[(dispY + (mirrorY ? info.height - y - 1: y)) * dispWidthBytes + dispXBytes + x] = ~data; break;
            case BMPfile::XOR: buffer[(dispY + (mirrorY ? info.height - y - 1: y)) * dispWidthBytes + dispXBytes + x] ^= data; break;
            case BMPfile::INVERT_XOR: buffer[(dispY + (mirrorY ? info.height - y - 1: y)) * dispWidthBytes + dispXBytes + x] ^= ~data; break;
            }
            Debug("%02x/%02x ", before, buffer[(y + dispY) * dispWidthBytes + dispXBytes + x]);
        }
        if (readBytes < widthBytes)
            fseek(fp, widthBytes - readBytes, SEEK_CUR);
        if (widthBytes & 0x03)
            fseek(fp, 4 - (widthBytes & 0x03), SEEK_CUR);
     }
    Debug("\n\n");
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
bool BMPfile::readBmp4Gray(const char *path, uint8_t *buffer, int16_t xStart, int16_t yStart, uint16_t bufWidth, uint16_t bufHeight, uint8_t invert, bool mirrorY) {
    // Binary file open
    FILE *fp;
    if((fp = fopen(path, "rb")) == NULL) {
        Debug("Can't open %s\n", path);
        return false;
    }

    HEADER header;           //bmp

    INFO info;
    // Set the file pointer from the beginning
    fseek(fp, 0, SEEK_SET);
    fread(&header, sizeof(header), 1, fp);    //sizeof(header) must be 14
    fread(&info, sizeof(INFO), 1, fp);    //sizeof(header) must be 50
    Debug("pixel = %d * %d\r\n", info.width, info.height);

    // Determine if it is a monochrome bitmap
    if(info.bitCount != 4) {
        Debug("%s is not a 4-bit bitmap!\n", path);
        return false;
    }

    uint16_t Image_Width_Byte = (info.width % 4 == 0)? (info.width / 4): (info.width / 4 + 1);
    uint16_t bufWidthByte = (bufWidth % 2 == 0)? (bufWidth / 2): (bufWidth / 2 + 1);
    uint16_t Bmp_Width_Byte = (Image_Width_Byte % 2 == 0) ? Image_Width_Byte: ((Image_Width_Byte / 2 + 1) * 4);

    // Read image data into the buffer
    uint8_t Rdata;
    fseek(fp, header.offset, SEEK_SET);

    for(uint16_t y = 0; y < info.height; y++) {//Total display column
        for(uint16_t x = 0; x < Bmp_Width_Byte; x++) {//Show a line in the line
            if(fread((char *)&Rdata, 1, 1, fp) != 1) {
                Debug("Read failed in %s\n", path);
                fclose(fp);
                return false;
            }
            if(x * 2 + xStart >= 0 && y + yStart >= 0 && x + xStart < bufWidthByte && x * 2 + xStart < bufWidth  && y + yStart < bufHeight) { //bmp
                buffer[x + xStart + ((mirrorY ? y : info.height - y - 1) + yStart) * bufWidthByte] =  Rdata ^ invert;
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
bool BMPfile::readBmpRgb7Color(const char *path, uint16_t *buffer, int16_t xStart, int16_t yStart, uint16_t bufWidth, uint16_t bufHeight, bool invert, bool mirrorY) {
    // Binary file open
    FILE *fp;
    if((fp = fopen(path, "rb")) == NULL) {
        Debug("Can't open %s\n", path);
        return false;
    }

    HEADER header;
    INFO info;
    // Set the file pointer from the beginning
    fseek(fp, 0, SEEK_SET);
    fread(&header, sizeof(header), 1, fp);    //sizeof(header) must be 14
    fread(&info, sizeof(INFO), 1, fp);    //sizeof(header) must be 50
    Debug("pixel = %d * %d\r\n", info.width, info.height);

    // Determine if it is a monochrome bitmap
    if(info.bitCount != 24) {
        Debug("%s is not a 4-bit bitmap!\n", path);
        return false;
    }

    // Read image data into the cache
    uint8_t data[3];
    fseek(fp, header.offset, SEEK_SET);

    for(uint16_t y = 0; y < info.height; y++) {
        for(uint16_t x = 0; x < info.width ; x++) {
            if(fread((char *)data, 3, 1, fp) != 1) {
                fclose(fp);
                return false;
                break;
            }
            uint16_t colour;
            if(data[0] == 0 && data[1] == 0 && data[2] == 0) {
                colour =  GxEPD_BLACK;
            } else if(data[0] == 255 && data[1] == 255 && data[2] == 255) {
                colour =  GxEPD_WHITE;
            } else if(data[0] == 0 && data[1] == 255 && data[2] == 0) {
                colour = GxEPD_GREEN;
            } else if(data[0] == 255 && data[1] == 0 && data[2] == 0) {
                colour =  GxEPD_BLUE;
            } else if(data[0] == 0 && data[1] == 0 && data[2] == 255) {
                colour =  GxEPD_RED;
            } else if(data[0] == 0 && data[1] == 255 && data[2] == 255) {
                colour =  GxEPD_YELLOW;
            } else if(data[0] == 0 && data[1] == 128 && data[2] == 255) {
                colour =  GxEPD_ORANGE;
            }
            if(x + xStart >= 0 && y + yStart >= 0 && x + xStart < bufWidth  && y + yStart < bufHeight)
                buffer[x + xStart + ((mirrorY ? y : info.height - y - 1) + yStart) * bufWidth] =  colour;
        }
    }
    fclose(fp);
    return true;
}


