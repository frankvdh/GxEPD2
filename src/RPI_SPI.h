// Raspberry Pi SPI interface for e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires the wiringPi library. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Author: Frank van der Hulst
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

#ifndef RPI_SPI_H
#define RPI_SPI_H

#include <iostream>
#include <stdio.h>
#include <fcntl.h>

#include "Debug.h"

#define MSBFIRST 0
#define SPI_MODE0 0

#include <wiringPi.h>
#include <wiringPiSPI.h>


class SPISettings {
public:
    SPISettings(uint32_t speed, uint8_t bitorder, uint8_t mode);
private:
    uint32_t _speed;
    uint8_t _bitorder;
    uint8_t _mode;
};

class RPI_SPI
{
    public:
    RPI_SPI();
    void _writeCommand(uint8_t c);
    void _writeData(uint8_t d);
    void _writeData(const uint8_t* data, uint16_t n);
    void _writeCommandData(const uint8_t* pCommandData, uint8_t datalen);
    void _beginTransfer();
    static inline uint8_t transfer(uint8_t value) {
        wiringPiSPIDataRW(0, &value, 1);
        return value;
    };
    void _endTransfer();
    int begin(void);
    void end(void);

    void beginTransaction(const SPISettings &_spi_settings);
    void endTransaction(void);
};
extern RPI_SPI SPI;
#endif // RPI_SPI_H
