# Motion-Based Reaction Game (STM32)

## Overview
This project implements a motion-controlled reaction game using an STM32 microcontroller.  
The user responds to directional prompts by tilting the device, with performance tracked through a scoring and high-score system.

The system integrates multiple peripherals including an accelerometer, display, RGB LED, and push button, with additional features such as timing constraints, input filtering, and low-power idle behaviour.

---

## Circuit Schematic
![Circuit schematic](Screenshot%202026-05-04%20221243.png)

---

## Hardware Components
- **STM32 Nucleo-L432KC** – Main controller handling logic and timing  
- **Accelerometer (I2C)** – Detects motion and tilt direction  
- **Push Button** – Starts the game and wakes the system  
- **RGB LED** – Indicates system state (idle, active, result)  
- **TFT Display (SPI)** – Displays instructions and score  
- **MAX98357A Audio Amplifier (Experimental)** – Attempted audio output  

---

## Documentation
- 📄 [Project Report](Embedded%20Systems%20Project%202.pdf)  
- 📝 [Rough Weekly Notes](Rough_notes.txt)  
- 🎥 [Demonstration Video](https://youtu.be/MFnngxhUEXM)  

---

## Source Code

### Core Application
- [main.c](main.c) – Main game logic, timing, and state control  

### Peripheral Drivers
- [display.c](display.c) / [display.h](display.h) – TFT display control (text, graphics)  
- [i2c.c](i2c.c) / [i2c.h](i2c.h) – I2C communication (accelerometer interface)  
- [spi.c](spi.c) / [spi.h](spi.h) – SPI communication (display interface)  
- [eeng1030_lib.c](eeng1030_lib.c) / [eeng1030_lib.h](eeng1030_lib.h) – Utility functions (GPIO, timing, delays)  

### Data Files
- [sound_array.h](sound_array.h) – Stored audio sample data (experimental playback)  

---

## Hardware Files
- 📐 [Schematic (PDF)](Schematicpdf.pdf)  
- 🖼️ [Breadboard Setup Image](Screenshot%202026-05-04%20221243.png)  

---

## System Features
- Motion-based input using accelerometer  
- Direction detection (left, right, forward, back)  
- Score and high-score tracking  
- Timeout-based gameplay  
- Neutral-zone filtering to prevent repeated inputs  
- RGB LED state indication  
- Low-power idle mode using CPU sleep (WFI)  
- Experimental audio playback using I2S  

---

## Notes
Audio output was investigated using a MAX98357A I2S amplifier.  
Although signal activity was observed, reliable playback was not achieved due to strict timing requirements for I2S communication when implemented in software. As a result, audio functionality was treated as experimental and was not included in the final system operation.

---

## Repository Structure (Summary)
- `main.c` → Game logic and control  
- Peripheral files → Hardware interfacing  
- `sound_array.h` → Audio data  
- Report + notes → Documentation  
- Schematic + image → Hardware design  
