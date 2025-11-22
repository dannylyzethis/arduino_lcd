# Arduino Mega 2560 + ILI9486 Serial Display Controller

**Enhanced Edition v2.3** - Professional serial display controller for Arduino Mega 2560 with ILI9486 320x480 LCD display.

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

**Basic Features:**
- **Text Display**: Variable size (1-10), custom positioning, auto-wrapping, alignment (LEFT/CENTER/RIGHT)
- **Shapes**: Rectangles, circles, triangles (outline and filled), rounded rectangles
- **Lines**: Standard, horizontal, vertical, and pixel drawing
- **Progress Bars**: Visual progress indicators
- **Color Support**: 16-bit RGB565 with 12+ named colors
- **Screen Control**: Clear, rotation (0-3), invert, reset
- **Auto-Scroll**: Automatic screen clearing when full

**Advanced Features (v2.1-v2.3):**
- **Scrolling Marquee**: Smooth horizontal text scrolling with variable speed
- **Line Graphs**: Plot up to 100 data points with auto-scaling
- **Bar Charts**: Display up to 20 bars with auto-scaling
- **Bitmap Display**: Monochrome bitmap rendering from hex data
- **Text Boxes**: Bordered text boxes with auto-centering
- **Grid Drawing**: Configurable grid patterns for graph backgrounds
- **Animation Frames** (NEW v2.3): Store and play sprite animations with frame sequences
- **Frame Playback**: Configurable speed and looping support

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
| `#ALIGN <LEFT\|CENTER\|RIGHT>` | Set text alignment | `#ALIGN CENTER` |

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
- `#GRID <x> <y> <w> <h> <spacing>` - Draw grid with specified spacing

### Advanced Features (NEW in v2.1)

#### Scrolling Marquee
- **Command**: `#MARQUEE <y> <speed> <text>`
- **Description**: Scrolls text horizontally across the screen
- **Parameters**:
  - `y`: Vertical position
  - `speed`: Pixels to move per frame (1-100)
  - `text`: Text to scroll
- **Notes**: Send any serial input to stop the marquee
- **Example**: `#MARQUEE 100 5 Breaking News: Arduino is awesome!`

#### Line Graph Plotting
- **Command**: `#GRAPH <x> <y> <w> <h> <val1,val2,val3,...>`
- **Description**: Plots a line graph with auto-scaling
- **Parameters**:
  - `x, y`: Top-left corner
  - `w, h`: Width and height of graph
  - `values`: Comma-separated data points (up to 100)
- **Features**: Auto-scales to min/max, draws border, smooth lines
- **Example**: `#GRAPH 10 50 300 150 10,25,18,35,42,38,50,45`

#### Bar Chart
- **Command**: `#BAR <x> <y> <w> <h> <val1,val2,val3,...>`
- **Description**: Displays bar chart with auto-scaling
- **Parameters**:
  - `x, y`: Top-left corner
  - `w, h`: Width and height of chart
  - `values`: Comma-separated values (up to 20)
- **Features**: Auto-scales to maximum value, equal-width bars
- **Example**: `#BAR 10 250 300 100 15,30,22,45,35,28`

#### Monochrome Bitmap Display
- **Command**: `#BITMAP <x> <y> <w> <h> <hexdata>`
- **Description**: Displays 1-bit monochrome bitmap
- **Parameters**:
  - `x, y`: Top-left corner
  - `w, h`: Width and height in pixels
  - `hexdata`: Hex string (1 bit per pixel, MSB first)
- **Example**: `#BITMAP 50 50 8 8 FF818181818181FF` (draws a box)
- **Notes**:
  - Each byte represents 8 pixels
  - 1 = foreground (textColor), 0 = background (bgColor if opaque)
  - Data is row-major order

#### Text Box with Border
- **Command**: `#TEXTBOX <x> <y> <w> <h> <text>`
- **Description**: Draws bordered box with centered text
- **Parameters**:
  - `x, y`: Top-left corner
  - `w, h`: Width and height
  - `text`: Text to display (centered)
- **Example**: `#TEXTBOX 50 200 200 50 STATUS: OK`

### Animation Frame Commands (NEW in v2.3)

#### Define Animation Frame
- **Command**: `#FRAMEDEF <id> <w> <h> <hexdata>`
- **Description**: Define a monochrome bitmap frame for animation
- **Parameters**:
  - `id`: Frame ID (0-9)
  - `w, h`: Width and height in pixels (max 128x128)
  - `hexdata`: Hex string bitmap data (1 bit per pixel, MSB first)
- **Memory**: Max 10 frames, up to 512 bytes each
- **Example**: `#FRAMEDEF 0 16 16 FFFFC003C003C003C003C003C003C003C003C003C003C003C003C003C003FFFF`
- **Notes**: Frames are stored in RAM using dynamic allocation

#### Play Animation
- **Command**: `#ANIMATE <x> <y> <frames> <delay> [loop]`
- **Description**: Play sequence of frames as animation
- **Parameters**:
  - `x, y`: Display position
  - `frames`: Comma-separated frame IDs (e.g., `0,1,2,3`)
  - `delay`: Delay between frames in milliseconds (10-5000)
  - `loop`: Optional - use `LOOP` or `1` for continuous playback
- **Example**: `#ANIMATE 100 200 0,1,2,3,2,1 100 LOOP`
- **Notes**:
  - Send any character to serial to stop animation
  - Uses current text color for foreground
  - Respects background color if opaque text enabled

#### Clear Frames
- **Command**: `#FRAMECLEAR [id]`
- **Description**: Clear one or all animation frames
- **Parameters**:
  - `id`: (Optional) Frame ID to clear. If omitted, clears all frames
- **Examples**:
  - `#FRAMECLEAR` - Clear all frames
  - `#FRAMECLEAR 3` - Clear only frame 3

#### Stop Animation
- **Command**: `#FRAMESTOP`
- **Description**: Stop currently playing animation
- **Example**: `#FRAMESTOP`

#### Frame Information
- **Command**: `#FRAMEINFO`
- **Description**: Display frame memory usage and defined frames
- **Example**: `#FRAMEINFO`
- **Output**: Shows all defined frames with dimensions and memory usage

### Screen Commands

| Command | Description | Example |
|---------|-------------|---------|
| `#CLEAR` or `#CLR` | Clear entire screen | `#CLEAR` |
| `#ROT <0-3>` | Set rotation (0=portrait, 1=landscape, etc.) | `#ROT 1` |
| `#INVERT <ON\|OFF>` | Invert display colors | `#INVERT ON` |
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

### Text Alignment (NEW in v2.1)
```
#CLEAR
#SIZE 2
#ALIGN LEFT
Left aligned text
#ALIGN CENTER
Centered text!
#ALIGN RIGHT
Right aligned
#ALIGN LEFT
```

### Scrolling Marquee (NEW in v2.1)
```
#CLEAR
#SIZE 3
#COLOR CYAN
#MARQUEE 100 5 Welcome to Arduino Mega Display System!
(Send any character to stop scrolling)
```

### Sensor Data Visualization (NEW in v2.1)
```
#CLEAR
#SIZE 2
Temperature Over Time:
#COLOR YELLOW
#FCOLOR YELLOW
#GRAPH 10 50 300 150 20,22,21,23,25,24,26,28,27,25,23
#POS 0 220
Humidity Levels:
#COLOR CYAN
#FCOLOR CYAN
#BAR 10 260 300 100 45,52,48,65,72,68,75
```

### Status Display with Text Boxes (NEW in v2.1)
```
#CLEAR
#COLOR GREEN
#TEXTBOX 10 10 140 40 CPU: 45°C
#COLOR BLUE
#TEXTBOX 170 10 140 40 RAM: 6.5KB
#COLOR YELLOW
#TEXTBOX 10 70 140 40 Uptime: 1h
#COLOR RED
#TEXTBOX 170 70 140 40 Load: 23%
```

### Custom Icon Display (NEW in v2.1)
```
#CLEAR
#SIZE 1
#POS 10 10
Smiley Face:
#COLOR YELLOW
#BITMAP 80 10 16 16 03C00FF01FF83FFC7C3EF81FF81FF81FF81F7C3E3FFC1FF80FF003C0
```

### Sprite Animation Example (NEW in v2.3)
```
#CLEAR
#SIZE 2
#COLOR CYAN
Animated Sprite Demo

# Define 4 frames of a simple bouncing ball animation (8x8 pixels)
#FRAMEDEF 0 8 8 3C7EFFFF7E3C0000
#FRAMEDEF 1 8 8 003C7EFF7E3C0000
#FRAMEDEF 2 8 8 00003C7EFF7E3C00
#FRAMEDEF 3 8 8 0000003C7EFF7E3C

# Play animation at position (150, 200) with 150ms delay per frame, looping
#ANIMATE 150 200 0,1,2,3,2,1 150 LOOP

# (Send any character to stop)
# Stop the animation
#FRAMESTOP

# Check frame memory usage
#FRAMEINFO
```

### Walking Character Animation (NEW in v2.3)
```
#CLEAR
#SIZE 2
Walking Animation:

# Define walk cycle frames (16x16 stick figure)
#FRAMEDEF 0 16 16 0000018003C007E00FF00FF00FF007E003C001800180018003C007E00E700C300
#FRAMEDEF 1 16 16 0000018003C007E00FF00FF00FF007E003C0018001800C700E7007E003C00180
#FRAMEDEF 2 16 16 0000018003C007E00FF00FF00FF007E003C001800180018003C007E00E700C300
#FRAMEDEF 3 16 16 0000018003C007E00FF00FF00FF007E003C001800E700C70018001800180018

# Animate the walk cycle
#COLOR GREEN
#ANIMATE 50 100 0,1,2,3 200 LOOP
```

### Loading Spinner Animation (NEW in v2.3)
```
#CLEAR
#POS 0 200
#SIZE 2
#ALIGN CENTER
Loading...

# Define 8-frame spinner (12x12 pixels)
#FRAMEDEF 0 12 12 0E01F03F07F0FF0FF01E00000000000000
#FRAMEDEF 1 12 12 00001E00F01FF0FF07F03F01F00E000000
#FRAMEDEF 2 12 12 0000000000001E00F01FF0FF07F03F01F0
#FRAMEDEF 3 12 12 1F03F07F0FF01E00000000000000E01F0
#FRAMEDEF 4 12 12 F01F03F01E00000000000000E01F07F0FF0
#FRAMEDEF 5 12 12 0FF07F03F01E0000000000000000000E01F0
#FRAMEDEF 6 12 12 1F00E00000000000000E01F03F07F0FF0
#FRAMEDEF 7 12 12 0FF0FF07F03F01F00E000000000000000000

# Show spinner at center of screen
#COLOR CYAN
#ANIMATE 154 150 0,1,2,3,4,5,6,7 100 LOOP
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

- [ ] Touch screen support
- [ ] SD card integration for graphics/fonts
- [ ] Multiple serial port support
- [ ] JSON command protocol for structured data
- [ ] Custom font loading
- [ ] Screen capture to SD card
- [x] Animation frame sequences *(Implemented in v2.3)*
- [ ] Real-time clock display
- [ ] Weather icon library
- [ ] Pre-loaded sprite libraries
- [ ] Frame compression for larger sprites

## Version History

- **v2.3** - Animation Frame System (Current)
  - ✅ Animation frame storage (up to 10 frames)
  - ✅ Frame definition with dynamic memory allocation
  - ✅ Animation playback with configurable speed
  - ✅ Loop support for continuous animation
  - ✅ Frame memory management and monitoring
  - ✅ Stop/clear frame commands
  - Sprite animation support
  - Enhanced documentation with animation examples

- **v2.2** - Bug Fixes and New Visualization Features
  - ✅ Fixed text alignment underflow bugs
  - ✅ Fixed wrapped text alignment
  - ✅ Analog gauge/meter displays
  - ✅ Level meters (horizontal/vertical)
  - ✅ Arc drawing for pie charts
  - Enhanced reliability

- **v2.1** - Enhanced Edition with Advanced Features
  - ✅ Text alignment (LEFT, CENTER, RIGHT)
  - ✅ Scrolling marquee text
  - ✅ Line graph plotting (up to 100 points)
  - ✅ Bar chart visualization (up to 20 bars)
  - ✅ Monochrome bitmap display
  - ✅ Text boxes with borders
  - ✅ Grid drawing
  - ✅ Display inversion mode
  - Bug fixes and optimizations
  - Enhanced documentation

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
