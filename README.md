# Motion-Based Reaction Game (STM32)

## Overview
This project implements a motion-controlled reaction game using an STM32 microcontroller.

## Circuit Schematic

![Circuit schematic](Screenshot%202026-05-04%20221243.png)

## Hardware
- STM32 Nucleo-L432KC
- Accelerometer (I2C)
- Push button
- RGB LED
- TFT Display
- MAX98357A I2S Audio Amplifier (experimental)

## Project Report

[Download Full Report (PDF)](Embedded%20Systems%20Project%202.pdf)

### Source Code
- [main.c](main.c)
- [display.c](display.c)
- [display.h](display.h)
- [i2c.c](i2c.c)
- [i2c.h](i2c.h)
- [spi.c](spi.c)
- [spi.h](spi.h)
- [eeng1030_lib.c](eeng1030_lib.c)
- [eeng1030_lib.h](eeng1030_lib.h)
- [sound_array.h](sound_array.h)

## Files
- main.c → main code
- Screenshot 2026-05-04 221243.png → schematic image
- 
## Schematic Download
[Download schematic](Schematicpdf.pdf)

## Notes
Audio output was investigated using an I2S amplifier but was not included in the final implementation due to timing limitations.
