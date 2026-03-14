# Serial Communication Guide (Legacy Uno / ILI9341)

This document is for the **legacy Uno + ILI9341** workflow in `ili9341_serial_display/`.

For the current primary branch workflow (**Mega + ILI9486/9488**), use:

- `README.md` at repository root
- `mega_ili9486_serial_display/README.md`

---

# Legacy Serial Communication Guide - No Python Required!

## Method 1: Arduino Serial Monitor (Easiest)
1. Upload the sketch to your Arduino
2. Open Tools -> Serial Monitor in Arduino IDE
3. Set baud rate to 9600
4. Type commands and press Enter
5. Examples:
   - Type: `Hello World` (displays text)
   - Type: `#CLEAR` (clears screen)
   - Type: `#COLOR RED` (changes text color)
   - Type: `#HELP` (shows all commands)

## Method 2: PuTTY (Windows/Linux)
1. Download PuTTY from putty.org
2. Select "Serial" connection type
3. Enter your COM port (e.g., COM3)
4. Set Speed to 9600
5. Click Open
6. Start typing commands!

## Method 3: Windows Command Line
Using PowerShell:
```powershell
# Open port
$port = new-Object System.IO.Ports.SerialPort COM3,9600,None,8,one
$port.Open()

# Send commands
$port.WriteLine("Hello LCD!")
$port.WriteLine("#CLEAR")
$port.WriteLine("#COLOR GREEN")

# Close when done
$port.Close()
```

## Method 4: Linux/Mac Terminal
```bash
# Configure serial port
stty -F /dev/ttyACM0 9600 cs8 -cstopb -parenb

# Send commands
echo "Hello LCD!" > /dev/ttyACM0
echo "#CLEAR" > /dev/ttyACM0
echo "#COLOR BLUE" > /dev/ttyACM0

# Or use screen for interactive session
screen /dev/ttyACM0 9600
# (Press Ctrl-A, K to exit screen)
```

## Method 5: CoolTerm (Cross-platform)
1. Download CoolTerm (free serial terminal)
2. Click Options
3. Select your serial port
4. Set baudrate to 9600
5. Click Connect
6. Type in the terminal window

## Method 6: Tera Term (Windows)
1. Download Tera Term
2. Select Serial connection
3. Choose your COM port
4. Setup -> Serial Port -> 9600 baud
5. Start typing commands

## Method 7: Create Batch Files (Windows)
Create send_to_lcd.bat:
```batch
@echo off
mode COM3:9600,n,8,1
echo %1 > COM3
```

Usage:
```
send_to_lcd.bat "Hello World"
send_to_lcd.bat "#CLEAR"
```

## Available Commands
- **Text**: Just type any text and press Enter
- **#CLEAR** - Clear the screen
- **#COLOR <color>** - Change text color (RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE)
- **#SIZE <1-5>** - Change text size
- **#POS <x> <y>** - Set cursor position
- **#RECT <x> <y> <width> <height>** - Draw rectangle
- **#CIRCLE <x> <y> <radius>** - Draw circle
- **#LINE <x1> <y1> <x2> <y2>** - Draw line
- **#INFO** - Show current settings
- **#HELP** - Show help

## Mega Branch Note
- The Mega firmware uses `115200` baud and a much larger command set (`#STATUS`, `#MENU`, `#FPGASEL`, `#TOUCHCAL`, GPIO/I2C/SPI/EEPROM/Analog commands).
- Refer to root `README.md` for the up-to-date command reference.

## Tips
- Always use 9600 baud rate (or change it in both sketch and terminal)
- Press Enter after each command
- Commands starting with # are special commands
- Regular text is displayed as-is
- The display will echo responses back to the serial terminal
