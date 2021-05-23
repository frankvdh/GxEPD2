# GxEPD2 forked from https://github.com/ZinggJM/GxEPD2
## Arduino / Raspberry Pi Display Library for SPI E-Paper Displays

### Notes
- See https://github.com/ZinggJM/GxEPD2 for documentation on supported e-paper devices
- This fork extends GxEPD2 to work on a Raspberry Pi
- Uses the wiringPi library for access to pins
- SPI interface to the e-paper device 
- Includes a simple version of the AdaFruit GFX library
- This fork is of v1.3.3

### Version 1.3.3
- added b/w differential refresh method to GDEW0213Z19
- added b/w differential refresh method to GDEW029Z13
- up to 100 b/w fast refreshes have been possible (resulting in slightly reddish background)
- added experimental example GxEPD2x_MixedTest for proof of concept
- GxEPD2x_MixedTest needs enough RAM for 2 GFXCanvas1 objects
- general 3-color mixed refresh in GxEPD2 is in evaluation, for capable panels
