// Raspberry Pi SPI interface for e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires the wiringPi library. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Author: Frank van der Hulst
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

#include <unistd.h>

#include "RPI_SPI.h"
#include "Debug.h"

#define _cs 8
#define _dc 25
#define _rst 17
#define _busy 24

RPI_SPI::RPI_SPI()
{
    //ctor
}

RPI_SPI SPI;

SPISettings::SPISettings(uint32_t speed, uint8_t bitorder, uint8_t mode) :
    _speed(speed), _bitorder(bitorder), _mode(mode)
{
}

#ifdef WIN32
#else
static int checkRaspbian(void) {
    int i;
    int fd;
    char value_str[20];
    fd = open("/etc/issue", O_RDONLY);
    Debug("Current environment: ");
    while(1) {
        if (fd < 0) {
            Debug( "Open failed: '/etc/issue'\n");
            return -1;
        }
        for(i=0;; i++) {
            if (read(fd, &value_str[i], 1) < 0) {
                Debug( "Failed to read value from '/etc/issue'!\n");
                return -1;
            }
            if(value_str[i] == ' ') {
                Debug("\n");
                break;
            }
            Debug("%c",value_str[i]);
        }
        break;
    }

    if(i<5) {
        Debug("Short string from '/etc/issue'\r\n");
    } else {
        char RPI_System[10]   = {"Raspbian"};
        for(i=0; i<6; i++) {
            if(RPI_System[i]!= value_str[i]) {
                Debug("Raspbian not found: %6.0s\n", value_str);
                return -1;
            }
        }
    }
    return 0;
}

/**
 * Set GPIO pin mode
**/

static void setPinMode(uint16_t Pin, uint16_t Mode) {
    if(Mode == 0 || Mode == INPUT) {
        pinMode(Pin, INPUT);
        pullUpDnControl(Pin, PUD_UP);
    } else {
        pinMode(Pin, OUTPUT);
    }
}
#endif // WIN32

static bool setupDone = false;

/******************************************************************************
function:	Initialize the library and set the pins, SPI protocol
parameter:
Info:
******************************************************************************/
int RPI_SPI::begin(void) {
    Debug("SPI begin\n");
    if (setupDone) return 0;
    setupDone = true;
#ifdef WIN32
#else
    if(checkRaspbian() < 0) {
        return 1;
    }
    //if(wiringPiSetup() < 0)//use wiringpi Pin n

    if(wiringPiSetupGpio() < 0) { //use wiringPi Pin number table
        Debug("set wiringPi lib failed	!!! \r\n");
        return 1;
    } else {
        Debug("set wiringPi lib success !!! \r\n");
    }

    // GPIO Config
    setPinMode(_rst, OUTPUT);
    setPinMode(_dc, OUTPUT);
    setPinMode(_cs, OUTPUT);
    setPinMode(_busy, INPUT);

    digitalWrite(_cs, HIGH);

    wiringPiSPISetup(0,10000000);
    // wiringPiSPISetupMode(0, 32000000, 0);
#endif // WIN32
    Debug("SPI begin complete\n");
    return 0;
}

/******************************************************************************
function:	Module exits, closes SPI and wiringPi library
parameter:
Info:
******************************************************************************/
void RPI_SPI::end(void) {
    Debug("SPI end\n");
    digitalWrite(_cs, 0);
    digitalWrite(_dc, 0);
    digitalWrite(_rst, 0);
}

void RPI_SPI::beginTransaction(const SPISettings &_spi_settings) {
    Debug("SPI beginTransaction\n");
}

void RPI_SPI::endTransaction(void) {
    Debug("SPI endTransaction\n");
}
