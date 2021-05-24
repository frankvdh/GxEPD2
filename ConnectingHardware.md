# GxEPD2
## Raspberry Pi Display Library for SPI E-Paper Displays

- With full Graphics and Text support (using a version of Adafruit_GFX)
- For Arduino devices (incling ESP), see the original GxEPD2 library at https://github.com/ZinggJM/GxEPD2

- For SPI e-paper displays from Dalian Good Display 
- and SPI e-paper boards from Waveshare

### important note :
- the Good-Display panels are for 3.3V supply and 3.3V data lines
- the actual Waveshare display boards now have level converters and series regulator, safe for 5V
- see https://github.com/WiringPi/WiringPi for information about wiringPi and the SPI interface

## mapping suggestions

#### mapping suggestion from Waveshare SPI e-Paper to Raspberry Pi
- BUSY -> 24, RST -> 17, DC -> 25, CS -> 8, GND -> GND, 3.3V -> 3.3V
- This is the standard setup, and works with displays integrated into a HAT (e.g. 250x122, 2.13inch E-Ink display HAT) and the WaveShare Universal e-Paper Raw Panel Driver HAT
