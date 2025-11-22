# S.A.M. (Serial Arduino Monitor)

**S**erial **A**rduino **M**onitor - A versatile ILI9341 LCD display controller with FPGA integration for Arduino Uno R3.

## Project Overview

S.A.M. is a comprehensive serial display system built for Arduino Uno R3 with an ILI9341 TFT LCD shield. The project evolved through multiple iterations to provide flexible display control, FPGA communication, and efficient memory management within the Arduino's 32KB flash memory constraint.

This project is dedicated to the memory of Sammy.

## Hardware Requirements

- **Arduino Uno R3** (ATmega328P - 32KB Flash, 2KB RAM)
- **ILI9341 TFT LCD Shield** (240x320 pixels, 8-bit parallel)
- **Resistive Touchscreen** (integrated with shield)
- **FPGA Board** (optional, for advanced features)
  - FPGA RX connected to Arduino pin 12
  - FPGA TX connected to Arduino pin 13
  - Accessible via ICSP header

## Software Dependencies

- Arduino IDE 1.8.x or higher
- **Adafruit_GFX** library
- **MCUFRIEND_kbv** library
- **TouchScreen** library (for touch-enabled versions)

## Project Versions

S.A.M. comes in three distinct versions, each building upon the previous with additional features:

### 1. Original Version (Branch: `main`)
**Status**: Basic single-screen display
**Features**:
- Simple serial text display
- Text color and size control
- Basic graphics primitives (rectangles, circles, lines)
- Screen rotation support
- No FPGA support
- No split-screen functionality

**Best for**: Simple serial terminal display applications

[📖 View Original Version Documentation](docs/VERSION_ORIGINAL.md)

---

### 2. Text-Based FPGA Version (Branch: TBD)
**Status**: Split-screen with text commands
**Startup Message**: `READY`
**Features**:
- Split-screen design (top: commands, bottom: FPGA monitoring)
- FPGA serial communication (pins 12/13)
- Touch button interface (T/S/C/D buttons)
- Response parsing for FPGA data (temperature, status, counter, raw)
- Text-based commands (#COLOR, #SIZE, #BGCOLOR, etc.)
- FPGA passthrough mode (>>>)
- Configurable FPGA baud rate

**Best for**: Interactive FPGA monitoring with manual command control

[📖 View Text-Based Version Documentation](docs/VERSION_TEXT.md)

---

### 3. Register-Based Version (Branch: `claude/register-based-commands-01BRB8ZApEM6KTx3Srfihhgh`)
**Status**: Memory-optimized register system
**Startup Message**: `S.A.M. REG-v1.0`
**Features**:
- All features from text-based version
- **16x 32-bit register architecture** for parameter control
- **Memory optimized**: 95% usage (30,734 / 32,256 bytes)
- FPGA framing modes (Raw, Length-Prefix, Terminated, Both)
- 8 user-definable FPGA registers (R08-R0F)
- Dynamic baud rate switching without reboot
- Comprehensive help system with examples
- Register-based hex command interface (#W, #R)

**Best for**: Production FPGA applications requiring flexible configuration

[📖 View Register-Based Version Documentation](docs/VERSION_REGISTER.md)

## Quick Start

### Installation

1. Clone this repository:
```bash
git clone https://github.com/dannylyzethis/arduino_lcd.git
cd arduino_lcd
```

2. Choose your version:
```bash
# For original version
git checkout main

# For text-based version
git checkout text-based

# For register-based version
git checkout claude/register-based-commands-01BRB8ZApEM6KTx3Srfihhgh
```

3. Install required libraries via Arduino IDE Library Manager:
   - Adafruit GFX Library
   - MCUFRIEND_kbv
   - TouchScreen

4. Open `ili9341_serial_display/ili9341_serial_display.ino` in Arduino IDE

5. Select **Tools → Board → Arduino Uno**

6. Upload to your Arduino

### Basic Usage

Connect to the Arduino via serial at **9600 baud**:

```bash
# Linux/Mac
screen /dev/ttyUSB0 9600

# Windows (Arduino IDE Serial Monitor)
Tools → Serial Monitor (set to 9600 baud)
```

**Original Version**: Type text directly, use #commands for control

**Text-Based Version**: Same as original, plus >>> for FPGA passthrough

**Register-Based Version**: Use #W/#R for register control, comprehensive help via #HELP

## Memory Usage Comparison

| Version | Flash Usage | RAM Usage | Available Space |
|---------|-------------|-----------|-----------------|
| Original | ~85% | ~1,800 bytes | ~5KB flash |
| Text-Based | ~99% | ~1,900 bytes | ~300 bytes flash |
| Register-Based | ~95% | ~1,850 bytes | ~1,500 bytes flash |

The register-based version achieves better memory efficiency than text-based despite more features by eliminating redundant variables and optimizing string storage.

## Version Selection Guide

**Choose Original Version if you need**:
- Simple serial display only
- No FPGA communication
- Maximum simplicity
- Room for custom additions

**Choose Text-Based Version if you need**:
- FPGA monitoring
- Touch button interface
- Human-readable commands
- Quick prototyping with FPGA

**Choose Register-Based Version if you need**:
- Production FPGA applications
- Parameter storage without code changes
- Dynamic configuration switching
- Memory-efficient operation
- Automated control systems
- Multiple framing protocols

## Pin Connections

### LCD Shield (8-bit Parallel Mode)
- LCD data/control pins: Connected via shield headers
- SD card: Not used (disabled to save memory)

### Touchscreen (Resistive)
- YP: A3
- XM: A2
- YM: D9
- XP: D8

### FPGA Serial (SoftwareSerial)
- FPGA RX ← Arduino D13 (SCK on ICSP header)
- FPGA TX → Arduino D12 (MISO on ICSP header)
- GND: Common ground required

**Note**: Using ICSP header allows shield to remain installed

## Contributing

This project is a personal tribute and learning exercise. Feel free to fork and adapt for your own needs.

## License

Open source - use freely with attribution.

## Acknowledgments

- Built with Adafruit GFX and MCUFRIEND libraries
- Created in memory of Sammy
- Developed by Danny Lyze

## Support

For issues or questions:
1. Check version-specific documentation in `/docs`
2. Review example commands in help text (#HELP)
3. Verify hardware connections match pin definitions

---

**Repository Renaming Note**: To rename this repository to "S.A.M" on GitHub:
1. Go to repository Settings
2. Scroll to "Repository name"
3. Change to: `S.A.M` or `serial-arduino-monitor`
4. Click "Rename"

The repository URL will automatically redirect from the old name to the new one.
