# Motion-Based Reaction Game (STM32)

## Overview
This project implements an embedded motion-based reaction game using an STM32 microcontroller. The user must respond to directional prompts using tilt input from an accelerometer.

## Circuit Schematic

![Circuit schematic](hardware/schematic.png)

## Hardware
- STM32 Nucleo-L432KC
- Accelerometer (I2C)
- Push button
- RGB LED
- TFT Display
- MAX98357A I2S Audio Amplifier (experimental)

## Files
- `src/` → main code
- `hardware/` → schematic + images

[Download KiCad schematic](hardware/Game%20module%20sheet.kicad_sch)
