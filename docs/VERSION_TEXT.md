# S.A.M. Text-Based FPGA Version Documentation

**Branch**: TBD (commit before register system)
**Status**: Split-screen with text commands
**Memory Usage**: ~32KB flash (~99%)
**Startup Message**: `READY`

## Overview

The Text-Based FPGA version introduces split-screen functionality and FPGA communication capabilities. The display is divided into two sections: the top half for serial commands and text display, and the bottom half for real-time FPGA monitoring. This version includes touch button support for interactive FPGA communication.

## Features

- ✅ Split-screen display (top: commands, bottom: FPGA)
- ✅ FPGA serial communication (pins 12/13 via ICSP)
- ✅ Touch button interface (4 programmable buttons)
- ✅ FPGA response parsing (temperature, status, counter, raw data)
- ✅ FPGA passthrough mode (>>>)
- ✅ Text-based commands for display control
- ✅ Configurable FPGA baud rate
- ✅ Screen rotation with touch remapping
- ✅ Automatic text wrapping per section
- ⚠️ Memory constrained (~99% usage)

## Hardware Setup

### Required Components
- Arduino Uno R3
- ILI9341 TFT LCD Shield with resistive touchscreen
- FPGA development board (optional)
- USB cable for serial connection

### Pin Connections

**LCD Shield**: Connects via Arduino headers (8-bit parallel mode)

**Touchscreen**:
- YP: A3
- XM: A2
- YM: D9
- XP: D8

**FPGA Serial (SoftwareSerial)**:
- FPGA RX ← Arduino D13 (SCK on ICSP header)
- FPGA TX → Arduino D12 (MISO on ICSP header)
- GND: Common ground between Arduino and FPGA

**Important**: Using ICSP header pins allows the LCD shield to remain installed while connecting to FPGA.

## Installation

1. Install Arduino IDE and required libraries:
   - Adafruit_GFX
   - MCUFRIEND_kbv
   - TouchScreen

2. Clone repository:
```bash
git clone https://github.com/dannylyzethis/arduino_lcd.git
cd arduino_lcd
git checkout text-based
```

3. Connect FPGA to Arduino pins 12/13 (if using FPGA features)

4. Upload `ili9341_serial_display/ili9341_serial_display.ino` to Arduino Uno

5. Open serial monitor at 9600 baud

## Serial Communication

**PC Serial (USB)**: 9600 baud (fixed)
**FPGA Serial (Pins 12/13)**: 9600 baud (default, configurable)

### Connecting
```bash
# Linux/Mac
screen /dev/ttyUSB0 9600

# Windows - Use Arduino IDE Serial Monitor
# Set baud rate to 9600, line ending to "Newline"
```

## Screen Layout

```
┌─────────────────────────┐
│   TOP SECTION           │  Text: White, Size 2
│   Serial Commands       │  Background: Black
│   Display Area          │  Auto-scrolling
├─────────────────────────┤  ← Divider line
│   BOTTOM SECTION        │  Text: Green, Size 1
│   FPGA Monitor          │  Background: Black
│   Live Data             │  Auto-scrolling
└─────────────────────────┘
     (Touch Buttons)          Shown when #SHOWBTNS
```

**Layout Details**:
- **Top Section**: 50% of screen (0 to 158 pixels Y)
- **Divider Line**: White horizontal line at Y=160
- **Bottom Section**: 50% of screen (162 to 320 pixels Y)
- **Touch Buttons**: Overlay on bottom section when visible

## Command Reference

### Display Control Commands

#### Clear Top Section
```
#CLR
```
Clears the top section of the display and resets cursor.

#### Set Top Text Color
```
#COLOR <hex_color>
```
Sets text color for top section (RGB565 format).

**Examples**:
```
#COLOR FFFF    # White (default)
#COLOR F800    # Red
#COLOR 07E0    # Green
#COLOR 001F    # Blue
```

#### Set Top Background Color
```
#BGCOLOR <hex_color>
```
Enables opaque text mode for top section.

#### Set Bottom Text Color
```
#BOTCOLOR <hex_color>
```
Sets text color for bottom section (default: green 07E0).

#### Set Top Text Size
```
#SIZE <1-5>
```
Sets text size for top section (default: 2).

#### Set Bottom Text Size
```
#BOTSIZE <1-5>
```
Sets text size for bottom section (default: 1).

#### Rotate Screen
```
#ROT<0-3>
```
Rotates display and remaps touch coordinates accordingly.

- `#ROT0` - Portrait (240x320)
- `#ROT1` - Landscape (320x240)
- `#ROT2` - Portrait inverted (240x320)
- `#ROT3` - Landscape inverted (320x240)

### FPGA Communication Commands

#### Set FPGA Baud Rate
```
#FPGABAUD <baud_rate>
```
Changes FPGA serial communication speed.

**Examples**:
```
#FPGABAUD 9600      # Default
#FPGABAUD 115200    # High speed
#FPGABAUD 57600
```

**Note**: Change takes effect immediately. Ensure FPGA matches this baud rate.

#### Send Bytes to FPGA
```
#FPGABYTES <hex_bytes>
```
Sends raw hex bytes to FPGA.

**Examples**:
```
#FPGABYTES 54 45 4D 50           # Send "TEMP" (ASCII)
#FPGABYTES 01 02 03 04            # Send byte sequence
#FPGABYTES 0x48 0x45 0x4C 0x4C    # 0x prefix optional
```

**Format**:
- Space-separated hex values
- Each value 00-FF
- Prefix 0x optional
- Case insensitive

#### FPGA Passthrough Mode
```
>>>your_text_here
```
Sends text directly to FPGA with newline termination.

**Examples**:
```
>>>HELLO                # Sends "HELLO\n" to FPGA
>>>GET_STATUS           # Sends "GET_STATUS\n"
>>>SET TEMP 25          # Sends "SET TEMP 25\n"
```

**Note**: FPGA responses appear in bottom section automatically.

### Touch Button Commands

#### Show Touch Buttons
```
#SHOWBTNS
```
Displays four touch buttons at bottom of screen (T, S, C, D).

**Button Functions**:
- **T (Red)**: Send "TEMP" to FPGA, parse 2-byte temperature response
- **S (Green)**: Send "STATUS" to FPGA, parse 1-byte status response
- **C (Yellow)**: Send "COUNTER" to FPGA, parse 2-byte counter response
- **D (Blue)**: Send "DATA" to FPGA, parse 4-byte raw data response

#### Hide Touch Buttons
```
#HIDEBTNS
```
Hides touch buttons and restores full bottom section for FPGA display.

## Touch Button Protocol

### Button T (Temperature)
**Sends**: `54 45 4D 50` (ASCII "TEMP")
**Expects**: 2 bytes (signed 16-bit, big-endian)
**Display Format**: Temperature in Celsius (divide by 10)

**Example Response**:
```
FPGA sends: 0x00 0xE6
Display shows: 23.0C  (0x00E6 = 230 → 23.0°C)
```

### Button S (Status)
**Sends**: `53 54 41 54 55 53` (ASCII "STATUS")
**Expects**: 1 byte status code
**Display Format**: Status value in hex

**Example Response**:
```
FPGA sends: 0x05
Display shows: S:05
```

### Button C (Counter)
**Sends**: `43 4F 55 4E 54 45 52` (ASCII "COUNTER")
**Expects**: 2 bytes (unsigned 16-bit, big-endian)
**Display Format**: Decimal counter value

**Example Response**:
```
FPGA sends: 0x12 0x34
Display shows: C:4660  (0x1234 = 4660)
```

### Button D (Data)
**Sends**: `44 41 54 41` (ASCII "DATA")
**Expects**: 4 bytes raw data
**Display Format**: Hex bytes separated by spaces

**Example Response**:
```
FPGA sends: 0xAA 0xBB 0xCC 0xDD
Display shows: AA BB CC DD
```

### Response Timeout
Buttons wait 500ms for FPGA response. If no response received, displays "?".

## Usage Examples

### Example 1: FPGA Temperature Monitoring
```
#SHOWBTNS                # Show touch buttons
<Touch T button>         # Send temperature request
# Bottom displays: 22.5C

<Touch T button again>   # Check again
# Bottom displays: 23.1C
```

### Example 2: Custom FPGA Commands
```
#FPGABYTES 47 45 54 5F 44 41 54 41    # Send "GET_DATA"
# Wait for FPGA response in bottom section

>>>SET_LED 1             # Send via passthrough
# FPGA receives "SET_LED 1\n"
```

### Example 3: Split-Screen Status Display
```
#CLR
#SIZE 2
System Monitor
#SIZE 1
CPU: Active
Memory: OK
# Top section shows system info

# Bottom section shows live FPGA messages
# FPGA continuously sends temperature updates
```

### Example 4: High-Speed FPGA Communication
```
#FPGABAUD 115200         # Switch to high speed
>>>START_STREAM          # Request continuous data
# FPGA sends rapid updates shown in bottom section
```

## FPGA Integration Guide

### FPGA Serial Setup

Your FPGA serial module should:
1. Accept 8-bit UART data (default 9600 baud)
2. Respond to button commands within 500ms
3. Use correct byte format for each response type
4. Optionally send continuous updates (displayed in bottom section)

### Example FPGA Temperature Response (Verilog)

```verilog
// When receiving "TEMP" (0x54 0x45 0x4D 0x50)
reg [15:0] temperature;  // In tenths of degree C
temperature = 16'd235;   // 23.5°C

// Send response (big-endian)
uart_tx(temperature[15:8]);  // Send high byte first
uart_tx(temperature[7:0]);   // Send low byte
```

### Example FPGA Continuous Updates

```verilog
// Send periodic status messages
always @(posedge clk_1hz) begin
  uart_tx_string("Temp: 25.5C\n");
  // Arduino displays in bottom section
end
```

## Text Wrapping and Scrolling

### Top Section
- Text wraps based on text size and screen width
- Auto-scrolls when reaching divider line
- Clears and starts from top when full

### Bottom Section
- Text wraps independently from top section
- Auto-scrolls when reaching bottom of screen (or button area)
- Clears and restarts from divider when full
- Adjusts scroll area when buttons visible

**Characters Per Line** (Portrait 240 width):
- Size 1: 40 chars (bottom default)
- Size 2: 20 chars (top default)
- Size 3: 13 chars
- Size 4: 10 chars
- Size 5: 8 chars

## Memory Constraints

This version uses **~99% of flash memory** (31,900+ bytes of 32,256 bytes).

**Limitations**:
- Very little room for additional features
- Graphics commands removed to save space
- String buffers limited to:
  - Command buffer: 100 characters
  - FPGA buffer: 80 characters

**RAM Usage**: ~1,900 bytes of 2,048 bytes

## Troubleshooting

### FPGA not responding
- Verify pin connections (D12/D13)
- Check FPGA baud rate matches (#FPGABAUD)
- Ensure common ground between Arduino and FPGA
- Test with #FPGABYTES to send known sequence
- Check FPGA serial module is running

### Touch buttons not working
- Ensure buttons are visible (#SHOWBTNS)
- Calibrate touch if needed (adjust TS_MIN/MAX values)
- Try different screen rotations
- Check touchscreen connections (A2, A3, D8, D9)
- Press firmly in center of button

### Bottom section not showing FPGA data
- Verify FPGA is sending newline-terminated strings
- Check that data contains printable characters
- Use Serial Monitor to verify data is arriving (echoed)
- Try #HIDEBTNS to ensure buttons not covering text

### Memory overflow errors
- This version is at memory limit
- Cannot add significant features without removing others
- Consider upgrading to register-based version for more room

### Display corruption after rotation
- Screen and buttons automatically adjust to rotation
- Touch coordinates remap for all 4 orientations
- If issues persist, try #HIDEBTNS and #SHOWBTNS after rotation

## Performance Notes

- Touch detection: 200ms debounce time
- Button response timeout: 500ms
- FPGA serial: Buffered, non-blocking reads
- Text display: Instant for both sections
- Screen updates: ~50-100ms for section clear

## Upgrading to Register-Based Version

The register-based version provides:
- **Better memory efficiency** (95% vs 99%)
- **Register-based architecture** (no code changes for parameter adjustments)
- **FPGA framing modes** (length-prefix, termination, both)
- **8 user FPGA registers** (32-bit parameter storage)
- **Comprehensive help system**

See main [README.md](../README.md) for upgrade instructions.

## Technical Specifications

**Display**:
- Controller: ILI9341
- Resolution: 240x320 pixels
- Split: 50/50 with divider line
- Top section: Y 0-158 (159 pixels)
- Bottom section: Y 162-320 (158 pixels)

**Serial Communication**:
- PC Serial: Hardware UART via USB, 9600 baud
- FPGA Serial: SoftwareSerial on pins 12/13, configurable baud

**Touchscreen**:
- Type: 4-wire resistive
- Pressure range: 10-1000
- Resistance: 300 ohms
- Calibration: TS_MINX/MAXX/MINY/MAXY defines

**Memory**:
- Flash: 31,900 / 32,256 bytes (~99%)
- RAM: ~1,900 / 2,048 bytes (~93%)

**Touch Buttons**:
- Count: 4 (T, S, C, D)
- Height: 40 pixels
- Width: ~57 pixels each (with spacing)
- Response timeout: 500ms
- Debounce: 200ms

---

[← Back to Main Documentation](../README.md)
