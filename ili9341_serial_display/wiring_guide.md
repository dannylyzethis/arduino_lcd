# ILI9341 LCD to Arduino Uno Wiring Guide

## Pin Connections

### SPI Connections (Required):
```
ILI9341 Pin    ->  Arduino Uno Pin
----------------------------------------
VCC            ->  5V (or 3.3V if your display requires it)
GND            ->  GND
CS (Chip Select) -> Pin 10
RESET/RST      ->  Pin 8  
DC/RS          ->  Pin 9
SDI/MOSI       ->  Pin 11
SCK            ->  Pin 13
LED            ->  3.3V (for backlight)
SDO/MISO       ->  Pin 12 (optional, for reading from display)
```

## Important Notes:

1. **Voltage**: Most ILI9341 modules have onboard voltage regulators and level shifters, so they can work with 5V Arduino Uno. Check your specific module documentation.

2. **Backlight**: The LED pin controls the backlight. Connect to 3.3V or 5V through a resistor (100-330 ohms) if needed.

3. **SD Card** (if your module has one): The SD card shares the SPI bus but needs a separate CS pin (you can use pin 4 for SD_CS).

## Required Libraries:

Install these through Arduino IDE Library Manager:
- Adafruit GFX Library
- Adafruit ILI9341

## Installation Steps:

1. Open Arduino IDE
2. Go to Tools -> Manage Libraries
3. Search for "Adafruit GFX" and install
4. Search for "Adafruit ILI9341" and install
5. Connect your wiring as shown above
6. Upload the sketch
7. Open Serial Monitor (9600 baud)
8. Type #HELP to see available commands