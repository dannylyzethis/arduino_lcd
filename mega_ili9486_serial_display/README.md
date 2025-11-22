# Arduino Mega 2560 + ILI9486 Serial Display Controller

Enhanced serial display controller for Arduino Mega 2560 with ILI9486 320x480 LCD display.

## Hardware Requirements

- **Arduino Mega 2560** (8KB RAM, 256KB Flash)
- **ILI9486 Display** (320x480 pixels)
- **USB Cable** for serial communication

## Features

### Enhanced Over Uno Version

This version takes advantage of the Mega's superior specifications:

- **Higher Baud Rate**: 115200 vs 9600 (faster communication)
- **Larger Buffers**: 500-byte command buffer vs 100-byte
- **Extended Features**: More drawing primitives and commands
- **Better Memory Management**: Real-time RAM monitoring
- **Extended Color Palette**: 12+ named colors vs 8

### Display Features

- **Text Display**: Variable size (1-10), custom positioning, auto-wrapping
- **Shapes**: Rectangles, circles, triangles (outline and filled)
- **Lines**: Standard, horizontal, vertical, and pixel drawing
- **Progress Bars**: Visual progress indicators
- **Color Support**: 16-bit RGB565 with named color palette
- **Screen Control**: Clear, rotation (0-3), reset
- **Auto-Scroll**: Automatic screen clearing when full

## Wiring

The ILI9486 typically uses an 8-bit parallel interface with the Arduino Mega:

```
ILI9486 Pin  ->  Arduino Mega Pin
----------------------------------
VCC          ->  5V
GND          ->  GND
LCD_RST      ->  A4
LCD_CS       ->  A3
LCD_RS/DC    ->  A2
LCD_WR       ->  A1
LCD_RD       ->  A0
LCD_D0       ->  22
LCD_D1       ->  23
LCD_D2       ->  24
LCD_D3       ->  25
LCD_D4       ->  26
LCD_D5       ->  27
LCD_D6       ->  28
LCD_D7       ->  29
```

*Note: Pin assignments may vary by shield/breakout board. Check your specific hardware.*

## Installation

1. Install required libraries via Arduino IDE Library Manager:
   - Adafruit GFX Library
   - MCUFRIEND_kbv

2. Upload `mega_ili9486_serial_display.ino` to your Arduino Mega 2560

3. Open Serial Monitor at **115200 baud**

## Command Reference

### Text Commands

| Command | Description | Example |
|---------|-------------|---------|
| `<text>` | Display text on screen | `Hello World` |
| `#SIZE <1-10>` | Set text size | `#SIZE 3` |
| `#POS <x> <y>` | Set cursor position | `#POS 50 100` |

### Color Commands

| Command | Description | Example |
|---------|-------------|---------|
| `#COLOR <name>` | Set text/draw color | `#COLOR RED` |
| `#BGCOLOR <name\|NONE>` | Set text background | `#BGCOLOR BLUE` |
| `#FCOLOR <name>` | Set fill color for shapes | `#FCOLOR GREEN` |

**Available Colors:**
- Basic: `RED`, `GREEN`, `BLUE`, `YELLOW`, `CYAN`, `MAGENTA`, `WHITE`, `BLACK`
- Extended: `ORANGE`, `PINK`, `PURPLE`, `NAVY`, `DARKGREEN`, `DARKCYAN`, `MAROON`, `OLIVE`, `LIGHTGREY`, `DARKGREY`, `GREENYELLOW`, `BROWN`

### Drawing Commands

#### Rectangles
- `#RECT <x> <y> <w> <h>` - Draw rectangle outline
- `#FRECT <x> <y> <w> <h>` - Draw filled rectangle
- `#RRECT <x> <y> <w> <h> <r>` - Draw rounded rectangle
- `#FRRECT <x> <y> <w> <h> <r>` - Draw filled rounded rectangle

#### Circles
- `#CIRCLE <x> <y> <radius>` - Draw circle outline
- `#FCIRCLE <x> <y> <radius>` - Draw filled circle

#### Triangles
- `#TRI <x1> <y1> <x2> <y2> <x3> <y3>` - Draw triangle outline
- `#FTRI <x1> <y1> <x2> <y2> <x3> <y3>` - Draw filled triangle

#### Lines & Pixels
- `#LINE <x1> <y1> <x2> <y2>` - Draw line
- `#HLINE <x> <y> <length>` - Draw horizontal line (fast)
- `#VLINE <x> <y> <length>` - Draw vertical line (fast)
- `#PIXEL <x> <y>` - Draw single pixel

#### Special
- `#PROG <x> <y> <w> <h> <percent>` - Draw progress bar (0-100%)

### Screen Commands

| Command | Description | Example |
|---------|-------------|---------|
| `#CLEAR` or `#CLR` | Clear entire screen | `#CLEAR` |
| `#ROT <0-3>` | Set rotation (0=portrait, 1=landscape, etc.) | `#ROT 1` |
| `#RESET` | Reset all settings to defaults | `#RESET` |

### Settings Commands

| Command | Description | Example |
|---------|-------------|---------|
| `#SCROLL <ON\|OFF>` | Enable/disable auto-scroll | `#SCROLL ON` |
| `#ECHO <ON\|OFF>` | Enable/disable command echo | `#ECHO OFF` |

### Information Commands

| Command | Description |
|---------|-------------|
| `#INFO` | Display system information (resolution, colors, RAM, etc.) |
| `#ID` | Returns device identifier: `COM LCD MEGA ILI9486` |
| `#HELP` | Display complete command list |

### Test Commands

| Command | Description |
|---------|-------------|
| `#TEST` | Run full display test (colors, shapes, text) |
| `#TESTWRAP` | Test text wrapping functionality |
| `#TESTCOLORS` | Display color palette |
| `#TESTSHAPES` | Draw various shapes |

## Usage Examples

### Basic Text Display
```
#CLEAR
#SIZE 3
#COLOR GREEN
Hello from Arduino Mega!
#SIZE 2
#COLOR YELLOW
ILI9486 320x480 Display
```

### Drawing Shapes
```
#CLEAR
#COLOR RED
#RECT 10 10 100 80
#FCOLOR BLUE
#FCIRCLE 200 150 50
#COLOR YELLOW
#TRI 50 200 150 200 100 120
```

### Progress Bar Example
```
#CLEAR
#SIZE 2
Loading...
#COLOR GREEN
#FCOLOR GREEN
#PROG 10 50 300 30 0
#PROG 10 50 300 30 25
#PROG 10 50 300 30 50
#PROG 10 50 300 30 75
#PROG 10 50 300 30 100
```

### Multi-Color Text
```
#CLEAR
#SIZE 3
#COLOR RED
RED TEXT
#COLOR GREEN
GREEN TEXT
#COLOR BLUE
BLUE TEXT
```

## Technical Specifications

### Display
- **Controller**: ILI9486
- **Resolution**: 320x480 pixels
- **Color Depth**: 16-bit RGB565 (65,536 colors)
- **Interface**: 8-bit parallel

### Arduino Mega 2560
- **RAM**: 8KB SRAM
- **Flash**: 256KB
- **Serial Baud**: 115200
- **Command Buffer**: 500 bytes

### Performance
- **Free RAM**: ~7KB available during operation (check with `#INFO`)
- **Text Sizes**: 1-10 (6x8 base font scaled)
- **Max Chars/Line**: Varies by text size (e.g., 53 chars @ size 1, 26 @ size 2)

## Differences from Uno Version

| Feature | Uno (ILI9341) | Mega (ILI9486) |
|---------|---------------|----------------|
| Display Size | 240x320 | 320x480 |
| Baud Rate | 9600 | 115200 |
| Buffer Size | 100 bytes | 500 bytes |
| Text Sizes | 1-5 | 1-10 |
| Colors | 8 basic | 12+ extended |
| Drawing Commands | Basic | Extended (triangles, rounded rects, etc.) |
| RAM Monitoring | No | Yes (#INFO) |
| Fill Color | Uses text color | Separate fill color |
| Auto-Scroll | Always on | Configurable |
| Command Echo | Always on | Configurable |

## Troubleshooting

### Display shows white screen
- Check wiring connections
- Verify ILI9486 is detected (should show at startup)
- Try different rotation: `#ROT 0`, `#ROT 1`, etc.

### No serial response
- Check baud rate is set to 115200
- Verify USB connection
- Try resetting Arduino

### Text wrapping incorrectly
- Run `#TESTWRAP` to verify
- Check `#INFO` for current settings
- Try `#RESET` to restore defaults

### Colors look wrong
- Run `#TESTCOLORS` to verify palette
- Some displays may have different color order
- Try adjusting in MCUFRIEND_kbv library if needed

## Future Enhancement Ideas

- [ ] Bitmap image display
- [ ] Touch screen support
- [ ] SD card integration for graphics
- [ ] Animation sequences
- [ ] Multiple serial port support
- [ ] JSON command protocol
- [ ] Font selection
- [ ] Screen capture to SD

## Version History

- **v2.0** - Initial Mega + ILI9486 version
  - 320x480 resolution support
  - Extended command set
  - Improved memory management
  - Higher baud rate
  - Extended color palette

## License

MIT License - Free for personal and commercial use

## Credits

- Based on Adafruit GFX Library
- Uses MCUFRIEND_kbv for display control
- Enhanced for Arduino Mega 2560
