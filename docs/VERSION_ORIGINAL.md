# S.A.M. Original Version Documentation

**Branch**: `main`
**Status**: Basic single-screen display
**Memory Usage**: ~27KB flash (~85%)
**Startup Message**: `ILI9341 Ready`

## Overview

The original version of S.A.M. is a straightforward serial-controlled LCD display. It provides basic text rendering with color control, graphics primitives, and screen rotation. This version does not include FPGA support or split-screen functionality.

## Features

- ✅ Serial text display (9600 baud)
- ✅ Text color and background color control
- ✅ Text size adjustment (1-5)
- ✅ Manual cursor positioning
- ✅ Automatic text wrapping
- ✅ Screen rotation (0-3 for 90° increments)
- ✅ Graphics primitives (rectangles, circles, lines, filled shapes)
- ✅ Progress bar visualization
- ❌ No FPGA communication
- ❌ No touchscreen support
- ❌ No split-screen

## Hardware Setup

### Required Components
- Arduino Uno R3
- ILI9341 TFT LCD Shield (240x320 pixels)
- USB cable for serial connection

### Connections
The LCD shield connects directly to Arduino headers. No additional wiring required.

## Installation

1. Install Arduino IDE and required libraries:
   - Adafruit_GFX
   - MCUFRIEND_kbv

2. Clone repository and checkout main branch:
```bash
git clone https://github.com/dannylyzethis/arduino_lcd.git
cd arduino_lcd
git checkout main
```

3. Open `ili9341_serial_display/ili9341_serial_display.ino`

4. Upload to Arduino Uno

## Serial Communication

**Baud Rate**: 9600
**Line Ending**: Newline (NL)
**Command Prefix**: `#` for commands

### Connecting
```bash
# Linux/Mac
screen /dev/ttyUSB0 9600

# Windows - Use Arduino IDE Serial Monitor
# Set baud rate to 9600, line ending to "Newline"
```

## Command Reference

### Display Commands

#### Clear Screen
```
#CLR
```
or
```
#CLEAR
```
Clears the entire display and resets cursor to (0,0).

#### Set Text Color
```
#COLOR <hex_color>
```
Sets the text color using RGB565 format.

**Examples**:
```
#COLOR FFFF    # White
#COLOR F800    # Red
#COLOR 07E0    # Green
#COLOR 001F    # Blue
#COLOR FFE0    # Yellow
#COLOR F81F    # Magenta
#COLOR 07FF    # Cyan
```

**Common Colors**:
| Color | Hex Value |
|-------|-----------|
| White | FFFF |
| Black | 0000 |
| Red | F800 |
| Green | 07E0 |
| Blue | 001F |
| Yellow | FFE0 |
| Cyan | 07FF |
| Magenta | F81F |

#### Set Background Color
```
#BGCOLOR <hex_color>
```
Enables opaque text mode with specified background color.

**Example**:
```
#BGCOLOR 0000    # Black background
#COLOR FFFF      # White text
Hello World      # Displays white text on black background
```

#### Set Text Size
```
#SIZE <1-5>
```
Sets text size multiplier (1=smallest, 5=largest).

**Example**:
```
#SIZE 1    # 6x8 pixels per character
#SIZE 2    # 12x16 pixels per character (default)
#SIZE 3    # 18x24 pixels per character
```

#### Set Cursor Position
```
#POS <x> <y>
```
Moves the cursor to specified pixel coordinates.

**Example**:
```
#POS 0 0      # Top-left corner
#POS 120 160  # Center of screen (240x320)
```

#### Rotate Screen
```
#ROT<0-3>
```
Rotates the display orientation.

**Rotations**:
- `#ROT0` - Portrait (240x320)
- `#ROT1` - Landscape (320x240)
- `#ROT2` - Portrait inverted (240x320)
- `#ROT3` - Landscape inverted (320x240)

#### Display Information
```
#INFO
```
Prints display status to serial monitor:
- Screen resolution
- Current cursor position
- Text size
- Characters per line
- Background color setting

### Graphics Commands

#### Draw Rectangle
```
#RECT <x> <y> <width> <height> <color>
```
Draws a rectangle outline.

**Example**:
```
#RECT 10 10 100 50 FFFF    # White rectangle at (10,10), 100x50 pixels
```

#### Draw Filled Rectangle
```
#FILL <x> <y> <width> <height> <color>
```
Draws a filled rectangle.

**Example**:
```
#FILL 0 0 240 50 F800    # Red filled bar at top
```

#### Draw Circle
```
#CIRCLE <x> <y> <radius> <color>
```
Draws a circle outline.

**Example**:
```
#CIRCLE 120 160 50 07E0    # Green circle, center (120,160), radius 50
```

#### Draw Line
```
#LINE <x1> <y1> <x2> <y2> <color>
```
Draws a line between two points.

**Example**:
```
#LINE 0 0 239 319 FFFF    # White diagonal line corner to corner
```

#### Draw Progress Bar
```
#PROG <x> <y> <width> <height> <percentage> <fg_color> <bg_color>
```
Draws a progress bar with specified percentage fill.

**Example**:
```
#PROG 10 100 220 30 75 07E0 F800    # 75% green/red bar
```

### Text Display

Any text sent without the `#` prefix is displayed directly on screen.

**Example**:
```
Hello World
Temperature: 25.5C
System Status: OK
```

## Usage Examples

### Example 1: Status Display
```
#CLR
#SIZE 2
#COLOR FFFF
System Monitor
#SIZE 1
#COLOR 07E0
CPU: 45%
Memory: 1234 KB
Status: Running
```

### Example 2: Temperature Gauge
```
#CLR
#COLOR FFFF
#SIZE 2
Temperature Monitor
#PROG 10 50 220 30 65 F800 0000
#SIZE 1
#POS 10 90
Current: 22.5C
```

### Example 3: Colored Messages
```
#CLR
#COLOR F800
ERROR: Connection lost
#COLOR FFE0
WARNING: Low battery
#COLOR 07E0
INFO: System ready
```

## Screen Layout

```
┌─────────────────────────┐
│                         │  240 pixels wide
│                         │
│    Text and Graphics    │  320 pixels tall
│      Display Area       │
│                         │
│                         │
│                         │
└─────────────────────────┘
```

**Coordinate System**:
- Origin (0,0) is top-left corner
- X increases to the right (0-239)
- Y increases downward (0-319)
- Text wraps automatically at screen edge

## Text Wrapping Behavior

The display automatically wraps text based on:
- Current text size
- Screen width
- Available characters per line

**Characters per line**:
- Size 1: 40 characters
- Size 2: 20 characters (default)
- Size 3: 13 characters
- Size 4: 10 characters
- Size 5: 8 characters

When text exceeds the available width, it automatically wraps to the next line. When text reaches the bottom of the screen, the display clears and starts from the top.

## Memory Constraints

Arduino Uno R3 has limited resources:
- **Flash**: 32KB (program storage) - ~85% used
- **RAM**: 2KB (runtime memory) - ~1800 bytes used

**Limitations**:
- Command buffer limited to 100 characters
- Complex graphics may use significant flash memory
- Long strings should be sent in segments if needed

## Troubleshooting

### Display shows garbage or wrong colors
- Check that LCD shield is properly seated
- Try different rotation modes (#ROT0, #ROT1, etc.)
- Verify ILI9341 controller is detected (check serial output at startup)

### Text not appearing
- Verify serial baud rate is 9600
- Check that text color differs from background
- Try #CLR to clear screen
- Ensure cursor position is within screen bounds

### Serial connection issues
- Verify correct COM port selected
- Check USB cable connection
- Reset Arduino and reconnect
- Ensure no other program is using the serial port

### Display dimensions wrong after rotation
- Dimensions update automatically on rotation
- Use #INFO to verify current resolution
- Re-run positioning commands after rotation

## Performance Notes

- Graphics commands execute immediately
- Text rendering is fast for normal sizes
- Large filled rectangles may take ~100ms
- Screen clear takes ~50-100ms
- No noticeable delay for text display

## Upgrading to Advanced Versions

If you need FPGA communication or split-screen functionality:

**→ Text-Based Version**: Adds FPGA support, touch buttons, split screen
**→ Register-Based Version**: Adds register system, memory optimization, framing modes

See main [README.md](../README.md) for version comparison and upgrade paths.

## Serial Protocol Details

**Format**: ASCII text, newline-terminated
**Encoding**: UTF-8 compatible (printable ASCII recommended)
**Command Format**: `#COMMAND parameters`
**Response**: Commands return acknowledgment to serial monitor

**Example Session**:
```
→ #CLR
← CLR

→ #COLOR F800
← Color:F800

→ #SIZE 2
← Size:2

→ Hello World
← 1L: Hello World
```

## Code Customization

The original version is designed to be simple and modifiable:

**To add custom commands**: Edit `processCmd()` function
**To change default colors**: Modify initialization in `setup()`
**To adjust text behavior**: Modify `showText()` function
**To add graphics**: Use Adafruit_GFX drawing functions

## Technical Specifications

**Display**:
- Controller: ILI9341
- Resolution: 240x320 pixels
- Color Depth: RGB565 (16-bit, 65K colors)
- Interface: 8-bit parallel

**Arduino**:
- Board: Arduino Uno R3
- MCU: ATmega328P
- Clock: 16 MHz
- Flash: 32KB
- RAM: 2KB
- EEPROM: 1KB (unused)

**Serial**:
- Hardware UART (USB)
- Baud: 9600
- Data bits: 8
- Parity: None
- Stop bits: 1

---

[← Back to Main Documentation](../README.md)
