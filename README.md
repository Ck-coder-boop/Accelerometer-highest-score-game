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

## Files
- main.c → main code
- Game module sheet.kicad_sch → KiCad schematic
- Screenshot 2026-05-04 221243.png → schematic image

## Schematic File
[Download schematic](Game%20module%20sheet.kicad_sch)

## Notes
Audio output was investigated using an I2S amplifier but was not included in the final implementation due to timing limitations.
