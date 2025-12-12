# Arduino Mega ILI9486 Split Screen Display - Complete Command Reference

## Table of Contents
1. [System Overview](#system-overview)
2. [Display Commands](#display-commands)
3. [FPGA Serial Interface](#fpga-serial-interface)
4. [I2C Communication](#i2c-communication)
5. [SPI Communication](#spi-communication)
6. [EEPROM Storage](#eeprom-storage)
7. [Analog Input](#analog-input)
8. [GPIO Control](#gpio-control)
9. [Touch Interface](#touch-interface)
10. [Drawing Commands](#drawing-commands)
11. [Color Reference](#color-reference)
12. [Technical Specifications](#technical-specifications)

---

## System Overview

**Device ID:** COM LCD MEGA ILI9486 SPLIT TEXT
**Display:** ILI9486/ILI9488 TFT LCD
**Resolution:** 320x480 pixels
**Controller:** Arduino Mega 2560
**Communication:** Serial (USB), 9600 baud default

### Split Screen Layout
- **Top Screen:** Main display area for user text and graphics
- **Bottom Screen:** Status/log area for FPGA communication and system messages

---

## Display Commands

### Basic Text Display

#### Clear Screen
```
#CLR              Clear top screen only
#CLRBOT           Clear bottom screen only
#CLRALL           Clear both screens
```

**Response:** `OK:CLR`, `OK:CLRBOT`, or `OK:CLRALL`

#### Text Size
```
#SIZE <1-10>      Set top screen text size (1-10)
#BOTSIZE <1-5>    Set bottom screen text size (1-5)
```

**Examples:**
```
#SIZE 3           Set top text to size 3
#BOTSIZE 2        Set bottom text to size 2
```

**Response:** `OK:SIZE <n>` or `OK:BOTSIZE <n>`

#### Text Color
```
#COLOR <name>     Set text color
#BOTCOLOR <name>  Set bottom text color
#BGCOLOR <name>   Set background color for text
#BGCOLOR NONE     Disable background (transparent text)
```

**Examples:**
```
#COLOR RED        Set text to red
#BGCOLOR BLACK    Set black background
#BGCOLOR NONE     Transparent text
```

**Response:** `OK:COLOR <name>`, `OK:BGCOLOR <name>`, or `OK:BG_NONE`

#### Text Position
```
#POS <x> <y>      Set cursor position (top screen)
```

**Examples:**
```
#POS 0 0          Top-left corner
#POS 160 240      Center of screen (portrait mode)
```

**Response:** `OK:POS`

**Coordinate System:**
- Origin (0,0) is top-left corner
- X increases to the right (0-319 in portrait)
- Y increases downward (0-479 in portrait)

#### Screen Rotation
```
#ROT <0-3>        Set screen rotation
```

**Rotation Values:**
- `0` - Portrait (320x480)
- `1` - Landscape (480x320)
- `2` - Portrait inverted (320x480)
- `3` - Landscape inverted (480x320)

**Response:** `OK:ROT <n>`
**Note:** Clears screen and resets cursor to (0,0)

---

## FPGA Serial Interface

### Three Independent FPGA Serial Ports

**Port Mapping:**
- **FPGA1:** Serial1 (TX1=18, RX1=19)
- **FPGA2:** Serial2 (TX2=16, RX2=17)
- **FPGA3:** Serial3 (TX3=14, RX3=15)

### FPGA Selection
```
#FPGASEL <1|2|3>  Select active FPGA port
#FPGASEL ?        Query current selection
```

**Examples:**
```
#FPGASEL 1        Select FPGA1
#FPGASEL ?        Returns: FPGA1_ACTIVE
```

### FPGA Communication

#### Passthrough Mode
```
>>>text           Send text directly to selected FPGA
```

**Examples:**
```
>>>HELLO          Sends "HELLO" + termination byte
>>>READ REG 0x10  Send command to FPGA
```

#### Direct Commands
```
#FPGASEND <text>  Send text to FPGA
#FPGAPING         Send "PING" to FPGA
#FPGABYTES <hex>  Send hex bytes to FPGA
```

**Examples:**
```
#FPGASEND HELLO FPGA
#FPGAPING
#FPGABYTES 0x41 0x42 0x43    Send ABC
#FPGABYTES 41 42 43          Send ABC (hex without 0x)
```

**Response:** `OK:SENT`, `OK:PING`

#### Read from FPGA
```
#FPGAREAD <bytes> [timeout_ms]     Read N bytes from FPGA
#FPGAQUERY <hex> <expect> [timeout] Send hex, read response
```

**Examples:**
```
#FPGAREAD 10 1000           Read 10 bytes, 1 second timeout
#FPGAQUERY 0x10 0x20 4 500  Send 0x10 0x20, expect 4 bytes
```

**Response:** Displays received bytes according to parse mode

### FPGA Configuration

#### Baud Rate
```
#FPGA1BAUD <300-115200>   Set FPGA1 baud rate
#FPGA2BAUD <300-115200>   Set FPGA2 baud rate
#FPGA3BAUD <300-115200>   Set FPGA3 baud rate
#FPGA1BAUD ?              Query current baud rate
```

**Common Baud Rates:** 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200

**Examples:**
```
#FPGA1BAUD 115200
#FPGA2BAUD 9600
#FPGA1BAUD ?              Returns: FPGA1_BAUD=115200
```

#### Stop Bits
```
#FPGA1STOP <1|2>   Set stop bits (1 or 2)
#FPGA2STOP <1|2>
#FPGA3STOP <1|2>
#FPGA1STOP ?       Query stop bits
```

**Examples:**
```
#FPGA1STOP 1      One stop bit
#FPGA2STOP 2      Two stop bits
```

#### RX Mode
```
#FPGA1RX <TEXT|BINARY>   Set receive mode
#FPGA2RX <TEXT|BINARY>
#FPGA3RX <TEXT|BINARY>
#FPGA1RX ?               Query RX mode
```

**Modes:**
- `TEXT` - Display as ASCII text on bottom screen
- `BINARY` - Display as hex values

**Examples:**
```
#FPGA1RX TEXT
#FPGA2RX BINARY
```

#### Parse Mode (Display Format)
```
#FPGA1PARSE <mode>   Set display format for received data
#FPGA2PARSE <mode>
#FPGA3PARSE <mode>
#FPGA1PARSE ?        Query current parse mode
```

**Parse Modes:**
- `NONE` - Hex only: `0x41`
- `ASCII` - Show printable chars: `'A'` or `[41]` for non-printable
- `DEC` - Decimal: `65`
- `MIXED` - Hex + ASCII: `0x41='A'`

**Examples:**
```
#FPGA1PARSE MIXED     Show hex and ASCII
#FPGA2PARSE DEC       Show decimal values
```

#### Termination Byte
```
#TERM <mode>      Set TX termination byte
#TERM ?           Query current termination
```

**Modes:**
- `NONE` - No termination byte
- `LF` - Line feed (0x0A)
- `CR` - Carriage return (0x0D)
- `CRLF` - CR + LF (0x0D 0x0A)
- `0xFF` - Custom byte (hex format)
- `255` - Custom byte (decimal format)

**Examples:**
```
#TERM LF          Send LF after each message
#TERM 0x00        Send NULL terminator
#TERM NONE        No termination
```

### FPGA Status
```
#STATUS           Display all FPGA settings
```

**Shows:**
- Selected FPGA port
- Baud rates for all ports
- Stop bits configuration
- RX mode (TEXT/BINARY)
- Parse modes
- Termination byte setting

---

## I2C Communication

### Hardware Pins
- **SDA:** Pin 20
- **SCL:** Pin 21
- **Pull-ups:** Required (typically 4.7kΩ)

### I2C Address Range
- **Valid addresses:** 0x00 to 0x7F (7-bit addressing)
- **Common devices:**
  - 0x50-0x57: EEPROMs (AT24C series)
  - 0x68: RTC (DS1307, DS3231)
  - 0x76-0x77: BME280/BMP280 sensors
  - 0x3C-0x3D: OLED displays (SSD1306)
  - 0x48-0x4F: ADCs (ADS1115)

### I2C Commands

#### Scan I2C Bus
```
#I2CSCAN          Scan for I2C devices (0x00-0x7F)
```

**Example Output:**
```
Scanning I2C bus (0x00-0x7F)...
0x3C 0x50 0x68
3 device(s)
```

**Response:** Lists all found device addresses in hex

#### Read from I2C Device
```
#I2CREAD <addr> <reg> <bytes>    Read from register
```

**Parameters:**
- `addr` - Device address (hex or decimal)
- `reg` - Register address (hex or decimal)
- `bytes` - Number of bytes to read (1-32)

**Examples:**
```
#I2CREAD 0x68 0x00 7        Read 7 bytes from RTC register 0x00
#I2CREAD 104 0 7            Same (104 decimal = 0x68 hex)
#I2CREAD 0x50 0x0100 16     Read 16 bytes from EEPROM address 0x0100
```

**Response:** `I2C[0xAD:0xRG]=0xDA 0xTA...` (addr:reg=data)

#### Write to I2C Device
```
#I2CWRITE <addr> <reg> <data...>    Write to register
#I2CWRITE <addr> <data...>          Write without register
```

**Parameters:**
- `addr` - Device address (hex or decimal)
- `reg` - Register address (optional, hex or decimal)
- `data` - Data bytes (hex or decimal, space-separated)

**Examples:**
```
#I2CWRITE 0x68 0x00 0x12 0x34        Write to register
#I2CWRITE 0x50 0x00 0x48 0x45 0x4C   Write HEL to EEPROM
#I2CWRITE 0x3C 0x40 65 66 67         Write ABC (decimal)
```

**Response:** `I2C_WR[0xAD:0xRG] N bytes` (N = data bytes written)

---

## SPI Communication

### Hardware Pins
- **MOSI:** Pin 51 (Master Out Slave In)
- **MISO:** Pin 50 (Master In Slave Out)
- **SCK:** Pin 52 (Serial Clock)
- **CS:** User-defined (Chip Select)

### SPI Commands

#### Initialize SPI
```
#SPIBEGIN <cs_pin>    Initialize SPI with CS pin
```

**Parameters:**
- `cs_pin` - Chip select pin number (any digital pin)

**Examples:**
```
#SPIBEGIN 53          Use pin 53 for CS
#SPIBEGIN 10          Use pin 10 for CS
```

**Response:** `SPI_INIT(CS=<pin>)`
**Note:** CS pin is set to OUTPUT and HIGH (inactive)

#### End SPI
```
#SPIEND              Disable SPI
```

**Response:** `SPI ended`

#### Transfer Data
```
#SPITRANSFER <byte...>    Send and receive bytes
```

**Parameters:**
- `byte` - Data bytes (hex or decimal, space-separated)

**Examples:**
```
#SPITRANSFER 0x9F                      Read ID command
#SPITRANSFER 0x03 0x00 0x00 0x00       Read from address 0
#SPITRANSFER 65 66 67                  Send ABC (decimal)
```

**Response:**
```
SPI_TX[3]=0x03 0x00 0x00
SPI_RX[3]=0xAB 0xCD 0xEF
```

**Note:**
- CS goes LOW before transfer, HIGH after
- SPI is full-duplex: sends and receives simultaneously

#### Configure SPI Settings
```
#SPISETTINGS <speed> <mode>    Configure SPI parameters
```

**Parameters:**
- `speed` - Clock frequency in Hz (100 to 16000000)
- `mode` - SPI mode (0-3)

**SPI Modes:**
| Mode | Clock Polarity (CPOL) | Clock Phase (CPHA) | Description |
|------|----------------------|-------------------|-------------|
| 0    | 0 (idle low)        | 0 (leading edge)  | Most common |
| 1    | 0 (idle low)        | 1 (trailing edge) |             |
| 2    | 1 (idle high)       | 0 (leading edge)  |             |
| 3    | 1 (idle high)       | 1 (trailing edge) |             |

**Common Speed Values:**
- `4000000` - 4 MHz (safe for most devices)
- `8000000` - 8 MHz (fast devices)
- `1000000` - 1 MHz (slow devices)
- `16000000` - 16 MHz (maximum, use with caution)

**Examples:**
```
#SPISETTINGS 4000000 0       4 MHz, Mode 0
#SPISETTINGS 1000000 3       1 MHz, Mode 3
```

**Response:** `SPI_CFG(4000000 Hz, Mode 0)`

### Typical SPI Device Examples

#### Flash Memory (W25Q series)
```
#SPIBEGIN 53
#SPISETTINGS 4000000 0
#SPITRANSFER 0x9F              Read JEDEC ID
#SPITRANSFER 0x03 0x00 0x00 0x00    Read from address 0
```

#### SD Card (SPI mode)
```
#SPIBEGIN 53
#SPISETTINGS 250000 0          Start slow for initialization
#SPISETTINGS 4000000 0         Speed up after init
```

---

## EEPROM Storage

### Internal EEPROM Specifications
- **Size:** 4096 bytes (4 KB)
- **Address Range:** 0x0000 to 0x0FFF (0 to 4095)
- **Technology:** Non-volatile memory
- **Write Endurance:** ~100,000 cycles per byte
- **Data Retention:** 20+ years

### EEPROM Commands

#### Read Single Byte
```
#EEPROMREAD <addr>    Read one byte from address
```

**Parameters:**
- `addr` - Address (0-4095, hex or decimal)

**Examples:**
```
#EEPROMREAD 0          Read from address 0
#EEPROMREAD 0x100      Read from address 256
#EEPROMREAD 4095       Read from last address
```

**Response:** `EEPROM[0x<ADDR>]=0x<DATA> (<decimal>)`

#### Write Single Byte
```
#EEPROMWRITE <addr> <value>    Write one byte to address
```

**Parameters:**
- `addr` - Address (0-4095, hex or decimal)
- `value` - Byte value (0-255, hex or decimal)

**Examples:**
```
#EEPROMWRITE 0 0xFF            Write 0xFF to address 0
#EEPROMWRITE 100 65            Write 'A' (65) to address 100
#EEPROMWRITE 0x200 0x42        Write 0x42 to address 512
```

**Response:** `EEPROM[0x<ADDR>]=0x<DATA> (<decimal>)`
**Note:** Write takes ~3.3ms to complete

#### Dump Memory Range
```
#EEPROMDUMP <start> <end>    Display memory contents
```

**Parameters:**
- `start` - Start address (0-4095)
- `end` - End address (0-4095)

**Examples:**
```
#EEPROMDUMP 0 255              Dump first 256 bytes
#EEPROMDUMP 0x100 0x1FF        Dump 256 bytes from 0x100
#EEPROMDUMP 0 4095             Dump entire EEPROM
```

**Example Output:**
```
EEPROM Dump:
Addr  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
------+------------------------------------------------
0x000 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
0x010 | 48 65 6C 6C 6F 00 FF FF FF FF FF FF FF FF FF FF
```

#### Clear All EEPROM
```
#EEPROMCLEAR          Erase entire EEPROM (set to 0xFF)
```

**Response:** `Clearing EEPROM (4096 bytes)...` then `EEPROM cleared (all 0xFF)`
**Note:** Takes ~13 seconds to complete (all 4096 bytes)
**Warning:** This operation is irreversible!

### EEPROM Usage Examples

#### Store Configuration
```
#EEPROMWRITE 0 0xAA            Magic byte (config valid marker)
#EEPROMWRITE 1 115             Baud rate setting
#EEPROMWRITE 2 3               Text size
```

#### Store Text String
```
#EEPROMWRITE 100 72            'H'
#EEPROMWRITE 101 101           'e'
#EEPROMWRITE 102 108           'l'
#EEPROMWRITE 103 108           'l'
#EEPROMWRITE 104 111           'o'
#EEPROMWRITE 105 0             NULL terminator
```

---

## Analog Input

### Hardware Specifications
- **Available Pins:** A8-A15 (8 analog inputs)
- **Resolution:** 10-bit (0-1023 raw value)
- **Voltage References:**
  - DEFAULT: 5.0V
  - INTERNAL: 1.1V (precise internal reference)
  - EXTERNAL: Not available (AREF pin used by LCD shield)

### Pin Mapping
| Pin | Arduino Name | Physical Pin |
|-----|--------------|--------------|
| 8   | A8          | PK0         |
| 9   | A9          | PK1         |
| 10  | A10         | PK2         |
| 11  | A11         | PK3         |
| 12  | A12         | PK4         |
| 13  | A13         | PK5         |
| 14  | A14         | PK6         |
| 15  | A15         | PK7         |

### Analog Commands

#### Read Single Pin
```
#ANALOGREAD <pin>     Read analog value from pin
```

**Parameters:**
- `pin` - Pin number (8-15 for A8-A15)

**Examples:**
```
#ANALOGREAD 8         Read A8
#ANALOGREAD 15        Read A15
```

**Response:** `A<PIN>=<raw> (<voltage>V)`

**Example Output:**
```
A8=512 (2.50V)        With DEFAULT reference
A8=931 (1.00V)        With INTERNAL reference
```

#### Read All Pins
```
#ANALOGREADALL        Read all 8 analog pins
```

**Example Output:**
```
Analog Inputs (A8-A15):
Ref: DEFAULT (5V)
A8 =512 (2.50V)
A9 =0 (0.00V)
A10=1023 (5.00V)
A11=256 (1.25V)
A12=768 (3.75V)
A13=128 (0.63V)
A14=896 (4.38V)
A15=64 (0.31V)
```

#### Set Voltage Reference
```
#ANALOGREF <mode>     Set voltage reference
#ANALOGREF ?          Query current reference
```

**Modes:**
- `DEFAULT` - 5.0V reference (default)
- `INTERNAL` - 1.1V internal reference
- `EXTERNAL` - Not available (returns error)

**Examples:**
```
#ANALOGREF DEFAULT    Use 5V reference
#ANALOGREF INTERNAL   Use 1.1V reference
#ANALOGREF ?          Query current setting
```

**Response:**
```
Analog reference: DEFAULT (5V)
Analog reference: INTERNAL (1.1V)
WARNING: Max input 1.1V!
```

**IMPORTANT NOTES:**
- When using INTERNAL (1.1V) reference, **DO NOT** apply more than 1.1V to analog pins or permanent damage may occur!
- EXTERNAL reference is not available because the AREF pin is used by the LCD shield
- Allow 100µs settling time after changing reference (handled automatically)

#### Set Averaging
```
#ANALOGAVG <samples>     Set number of samples to average
#ANALOGAVG ?             Query current averaging
```

**Parameters:**
- `samples` - Number of samples (1-16)

**Examples:**
```
#ANALOGAVG 1          No averaging (fastest)
#ANALOGAVG 8          Average 8 samples (good noise reduction)
#ANALOGAVG 16         Average 16 samples (best noise reduction)
#ANALOGAVG ?          Query current setting
```

**Response:** `Analog averaging: <N> samples`

**Performance Notes:**
- Each sample takes ~100µs
- 16 samples adds ~1.6ms delay
- Averaging reduces noise significantly

### Voltage Calculations

#### DEFAULT Reference (5V)
```
Voltage = (raw_value × 5.0) / 1023
```

**Examples:**
- Raw 0 = 0.00V
- Raw 512 = 2.50V
- Raw 1023 = 5.00V

#### INTERNAL Reference (1.1V)
```
Voltage = (raw_value × 1.1) / 1023
```

**Examples:**
- Raw 0 = 0.00V
- Raw 512 = 0.55V
- Raw 1023 = 1.10V

### Analog Input Applications

#### Read Temperature Sensor (LM35, 10mV/°C)
```
#ANALOGREF DEFAULT
#ANALOGAVG 8
#ANALOGREAD 8
```
Calculate temperature: `Temp(°C) = Voltage × 100`

#### Read Potentiometer
```
#ANALOGREF DEFAULT
#ANALOGAVG 4
#ANALOGREAD 9
```
Position: `0-1023 = 0-100%`

#### Read Low Voltage Sensor (<1.1V)
```
#ANALOGREF INTERNAL
#ANALOGAVG 16
#ANALOGREAD 10
```
Higher precision for signals under 1.1V

---

## GPIO Control

### GPIO Pin Specifications
- **Available Pins:** 22-29 (8 digital I/O pins)
- **Voltage:** 5V logic (compatible with 3.3V devices via level shifter)
- **Current:** 40mA max per pin
- **Pull-up:** ~20kΩ internal pull-up available

### GPIO Pin Mapping
| Pin | Port | Bit | Function |
|-----|------|-----|----------|
| 22  | PA0  | 0   | GPIO     |
| 23  | PA1  | 1   | GPIO     |
| 24  | PA2  | 2   | GPIO     |
| 25  | PA3  | 3   | GPIO     |
| 26  | PA4  | 4   | GPIO     |
| 27  | PA5  | 5   | GPIO     |
| 28  | PA6  | 6   | GPIO     |
| 29  | PA7  | 7   | GPIO     |

**Note:** All 8 pins share Port A on ATmega2560

### GPIO Commands

#### Set Pin Mode
```
#GPIOMODE <pin> <mode>    Configure pin direction
```

**Modes:**
- `IN` - Input without pull-up
- `INPU` - Input with pull-up (20kΩ to VCC)
- `OUT` - Output

**Parameters:**
- `pin` - Pin number (22-29)

**Examples:**
```
#GPIOMODE 22 OUT          Set pin 22 as output
#GPIOMODE 23 IN           Set pin 23 as input
#GPIOMODE 24 INPU         Set pin 24 as input with pull-up
```

**Response:** `GPIO<PIN>_<MODE>`

#### Write Digital Output
```
#GPIOWRITE <pin> <value>    Set output state
```

**Parameters:**
- `pin` - Pin number (22-29, must be configured as OUTPUT)
- `value` - State: `0`/`LOW` or `1`/`HIGH`

**Examples:**
```
#GPIOWRITE 22 1           Set pin 22 HIGH (5V)
#GPIOWRITE 22 0           Set pin 22 LOW (0V)
#GPIOWRITE 23 HIGH        Set pin 23 HIGH
#GPIOWRITE 23 LOW         Set pin 23 LOW
```

**Response:** `GPIO<PIN>=<state>`

#### Read Digital Input
```
#GPIOREAD <pin>          Read single pin state
#GPIOREADALL             Read all 8 pins
```

**Examples:**
```
#GPIOREAD 22             Read pin 22
#GPIOREADALL             Read pins 22-29
```

**Response:**
```
GPIO22=1                 Single pin
GPIO[22-29]=0b10110011   All pins (binary)
GPIO[22-29]=179          All pins (decimal)
GPIO[22-29]=0xB3         All pins (hex)
```

#### Configure Pin Events
```
#GPIOEVENT <pin> <mode>    Set interrupt event type
```

**Event Modes:**
- `NONE` - No event detection
- `RISING` - Trigger on LOW-to-HIGH transition
- `FALLING` - Trigger on HIGH-to-LOW transition
- `BOTH` - Trigger on any change

**Examples:**
```
#GPIOEVENT 22 RISING      Detect rising edge on pin 22
#GPIOEVENT 23 FALLING     Detect falling edge on pin 23
#GPIOEVENT 24 BOTH        Detect any change on pin 24
#GPIOEVENT 25 NONE        Disable events on pin 25
```

**Response:** `GPIO<PIN>_EV_<MODE>`

**Event Output Format:**
When an event is detected, the system sends:
```
GPIO<PIN>_EVENT <state>
```

**Example:**
```
GPIO22_EVENT 1           Pin 22 went HIGH
GPIO23_EVENT 0           Pin 23 went LOW
```

#### Register Operations
```
#GPIOREG                 Read 8-bit port register
#GPIOSET <value>         Write 8-bit port register
```

**Parameters:**
- `value` - 8-bit value (0-255, hex or decimal)

**Examples:**
```
#GPIOREG                 Read all 8 pins as single byte
#GPIOSET 0xFF            Set all pins HIGH
#GPIOSET 0x00            Set all pins LOW
#GPIOSET 0xAA            Set pattern 10101010
#GPIOSET 170             Set pattern 10101010 (decimal)
```

**Response:**
```
GPIO_REG=0b10110011      Binary representation
GPIO_REG=179             Decimal value
GPIO_REG=0xB3            Hex value
```

**Bit Mapping:**
- Bit 0 = Pin 22
- Bit 1 = Pin 23
- ...
- Bit 7 = Pin 29

### GPIO Usage Examples

#### LED Control
```
#GPIOMODE 22 OUT
#GPIOWRITE 22 1          LED ON
#GPIOWRITE 22 0          LED OFF
```

#### Button Input (with pull-up)
```
#GPIOMODE 23 INPU        Enable internal pull-up
#GPIOREAD 23             Read button (0=pressed, 1=not pressed)
```

#### Detect Button Press Event
```
#GPIOMODE 23 INPU
#GPIOEVENT 23 FALLING    Trigger when button pressed
```

#### Parallel Output (8-bit data)
```
#GPIOMODE 22 OUT
#GPIOMODE 23 OUT
... (configure all pins)
#GPIOSET 0x42            Output byte 0x42 (01000010)
```

---

## Touch Interface

### Touch Screen Specifications
- **Type:** Resistive 4-wire touch screen
- **Resolution:** Based on ADC (mapped to display coordinates)
- **Calibration:** Required for accurate touch detection
- **Pressure Detection:** Supported

### Touch Calibration Values
**Default Calibration (Portrait Mode):**
```
MIN_X: 120
MAX_X: 900
MIN_Y: 120
MAX_Y: 940
```

**Coordinate Mapping:**
- Raw touch coordinates (120-900, 120-940) → Screen coordinates (0-319, 0-479)

### Touch Commands

#### Show/Hide Touch Buttons
```
#SHOWBTNS            Display touch buttons on screen
#HIDEBTNS            Hide touch buttons
```

**Button Layout (Portrait):**
```
┌──────────────────────┐
│                      │
│   Main Display       │
│                      │
│                      │
├──────────────────────┤
│ UP  │ DN  │ LT  │ RT │
└──────────────────────┘
```

**Button Coordinates (Portrait 320x480):**
- UP button: X=0-79, Y=400-479
- DN button: X=80-159, Y=400-479
- LT button: X=160-239, Y=400-479
- RT button: X=240-319, Y=400-479

**Button Output:**
When a button is pressed, sends byte array:
- UP: `0x55 0x50`
- DN: `0x44 0x4E`
- LT: `0x4C 0x54`
- RT: `0x52 0x54`

**Response:** `OK:SHOWBTNS` or `OK:HIDEBTNS`

#### Touch Calibration
```
#TOUCHCAL ?                              Query current calibration
#TOUCHCAL <minx> <maxx> <miny> <maxy>   Set calibration values
```

**Parameters:**
- `minx` - Minimum raw X value (typically 100-200)
- `maxx` - Maximum raw X value (typically 800-950)
- `miny` - Minimum raw Y value (typically 100-200)
- `maxy` - Maximum raw Y value (typically 800-950)

**Examples:**
```
#TOUCHCAL ?                      Show current calibration
#TOUCHCAL 120 900 120 940       Set calibration values
```

**Response:**
```
Touch Calibration:
MIN_X=120, MAX_X=900
MIN_Y=120, MAX_Y=940
```

#### Touch Test Mode
```
#TOUCHTEST           Toggle touch test mode
```

**Features:**
- Displays raw touch coordinates (ADC values)
- Shows mapped screen coordinates
- Indicates pressure/touch detection
- Useful for calibration

**Example Output:**
```
Touch screen to see raw coordinates
[TOUCH] Raw=(456,678) Map=(160,320) P=250 - TOUCH DETECTED!
[TOUCH] Raw=(120,120) Map=(0,0) P=50 - NO TOUCH
```

**Usage:**
1. Run `#TOUCHTEST` to enable
2. Touch screen corners and edges
3. Note the raw values for calibration
4. Run `#TOUCHTEST` again to disable

### Touch Calibration Procedure

1. **Enable test mode:**
   ```
   #TOUCHTEST
   ```

2. **Touch top-left corner:**
   Note the raw X and Y values (these are your MIN values)

3. **Touch bottom-right corner:**
   Note the raw X and Y values (these are your MAX values)

4. **Set calibration:**
   ```
   #TOUCHCAL <minx> <maxx> <miny> <maxy>
   ```

5. **Disable test mode:**
   ```
   #TOUCHTEST
   ```

6. **Test buttons:**
   ```
   #SHOWBTNS
   ```
   Touch each button to verify accuracy

---

## Drawing Commands

### Shape Drawing

#### Rectangle (Outline)
```
#RECT <x> <y> <w> <h>    Draw rectangle outline
```

**Parameters:**
- `x` - X coordinate of top-left corner
- `y` - Y coordinate of top-left corner
- `w` - Width in pixels
- `h` - Height in pixels

**Examples:**
```
#RECT 10 10 100 50      Draw 100×50 rectangle at (10,10)
#RECT 0 0 320 480       Draw border around entire screen
```

**Response:** `OK:RECT`

#### Filled Rectangle
```
#FILL <x> <y> <w> <h>    Draw filled rectangle
```

**Examples:**
```
#FILL 50 50 200 100     Draw solid 200×100 rectangle
#COLOR RED
#FILL 0 0 320 480       Fill entire screen with red
```

**Response:** `OK:FILL`

#### Circle (Outline)
```
#CIRCLE <x> <y> <r>      Draw circle outline
```

**Parameters:**
- `x` - X coordinate of center
- `y` - Y coordinate of center
- `r` - Radius in pixels

**Examples:**
```
#CIRCLE 160 240 50      Draw circle at screen center, radius 50
#CIRCLE 50 50 30        Draw circle at (50,50), radius 30
```

**Response:** `OK:CIRCLE`

#### Line
```
#LINE <x1> <y1> <x2> <y2>    Draw line
```

**Parameters:**
- `x1`, `y1` - Start point coordinates
- `x2`, `y2` - End point coordinates

**Examples:**
```
#LINE 0 0 319 479       Draw diagonal line
#LINE 0 240 319 240     Draw horizontal line at middle
#LINE 160 0 160 479     Draw vertical line at center
```

**Response:** `OK:LINE`

#### Progress Bar
```
#PROG <x> <y> <w> <h> <%>    Draw progress bar
```

**Parameters:**
- `x`, `y` - Position of top-left corner
- `w`, `h` - Width and height of bar
- `%` - Fill percentage (0-100)

**Examples:**
```
#PROG 10 400 300 30 50      Draw 50% filled progress bar
#PROG 10 400 300 30 0       Empty progress bar
#PROG 10 400 300 30 100     Full progress bar
```

**Response:** `OK:PROG <percentage>%`

**Visual:**
```
┌────────────────────────────┐
│████████████████            │  50%
└────────────────────────────┘
```

### Drawing Examples

#### Create Dashboard
```
#CLR
#COLOR CYAN
#RECT 5 5 310 470           Border
#COLOR YELLOW
#FILL 10 10 300 30          Title bar
#COLOR WHITE
#POS 100 15
Title Text
#COLOR GREEN
#PROG 10 400 300 30 75      75% progress
```

#### Draw Graph Axes
```
#CLR
#COLOR WHITE
#LINE 30 430 30 50          Y-axis
#LINE 30 430 300 430        X-axis
#COLOR RED
#CIRCLE 30 430 3            Origin point
```

---

## Color Reference

### Standard Colors

| Color Name | RGB565 Hex | R   | G   | B   | Description |
|-----------|-----------|-----|-----|-----|-------------|
| BLACK     | 0x0000    | 0   | 0   | 0   | Black       |
| WHITE     | 0xFFFF    | 255 | 255 | 255 | White       |
| RED       | 0xF800    | 255 | 0   | 0   | Pure red    |
| GREEN     | 0x07E0    | 0   | 255 | 0   | Pure green  |
| BLUE      | 0x001F    | 0   | 0   | 255 | Pure blue   |
| CYAN      | 0x07FF    | 0   | 255 | 255 | Cyan        |
| MAGENTA   | 0xF81F    | 255 | 0   | 255 | Magenta     |
| YELLOW    | 0xFFE0    | 255 | 255 | 0   | Yellow      |
| ORANGE    | 0xFD20    | 255 | 165 | 0   | Orange      |
| PINK      | 0xF81F    | 255 | 0   | 255 | Pink/Magenta|
| PURPLE    | 0x780F    | 128 | 0   | 128 | Purple      |
| NAVY      | 0x000F    | 0   | 0   | 128 | Navy blue   |

### Color Format: RGB565

The display uses RGB565 color encoding:
- **5 bits** for Red (0-31)
- **6 bits** for Green (0-63)
- **5 bits** for Blue (0-31)
- Total: 16 bits (65,536 colors)

**Bit Layout:**
```
15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
R  R  R  R  R  G  G  G  G  G  G  B  B  B  B  B
```

### Color Usage Examples
```
#COLOR RED                  Set text to red
#BGCOLOR BLACK              Black background
#COLOR CYAN
#FILL 0 0 320 480          Fill screen with cyan
```

---

## Technical Specifications

### Display Hardware
- **Controller:** ILI9486 / ILI9488
- **Resolution:** 320×480 pixels (portrait), 480×320 (landscape)
- **Color Depth:** 16-bit RGB565 (65,536 colors)
- **Interface:** 8-bit parallel (faster than SPI)
- **Touch:** Optional 4-wire resistive (when installed)

### Arduino Mega Connections

#### Serial Ports
| Port    | TX Pin | RX Pin | Usage              |
|---------|--------|--------|--------------------|
| Serial  | 1      | 0      | USB (programming)  |
| Serial1 | 18     | 19     | FPGA1              |
| Serial2 | 16     | 17     | FPGA2              |
| Serial3 | 14     | 15     | FPGA3              |

#### I2C
| Signal | Pin | Notes                  |
|--------|-----|------------------------|
| SDA    | 20  | Requires pull-up (4.7kΩ)|
| SCL    | 21  | Requires pull-up (4.7kΩ)|

#### SPI
| Signal | Pin | Direction |
|--------|-----|-----------|
| MOSI   | 51  | Master→Slave |
| MISO   | 50  | Slave→Master |
| SCK    | 52  | Clock |
| CS     | User| Chip Select (any pin) |

#### Analog Input
| Pin  | Arduino | Max Voltage (DEFAULT) | Max Voltage (INTERNAL) |
|------|---------|-----------------------|------------------------|
| A8   | PK0     | 5V                    | 1.1V                  |
| A9   | PK1     | 5V                    | 1.1V                  |
| A10  | PK2     | 5V                    | 1.1V                  |
| A11  | PK3     | 5V                    | 1.1V                  |
| A12  | PK4     | 5V                    | 1.1V                  |
| A13  | PK5     | 5V                    | 1.1V                  |
| A14  | PK6     | 5V                    | 1.1V                  |
| A15  | PK7     | 5V                    | 1.1V                  |

#### GPIO
| Pin Range | Port | Count | Max Current |
|-----------|------|-------|-------------|
| 22-29     | PA   | 8     | 40mA/pin    |

### Memory Usage
- **Flash:** ~60KB (program code)
- **RAM:** ~4KB (variables, buffers)
- **EEPROM:** 4KB (non-volatile storage)

### Performance
- **Serial Baud Rates:** 300 to 115200 bps
- **I2C Clock:** 100kHz standard mode
- **SPI Clock:** 100Hz to 16MHz
- **ADC Conversion:** ~100µs per sample
- **Touch Scan Rate:** ~10Hz when enabled

### Power Requirements
- **Voltage:** 5V via USB or external
- **Current:** 200-400mA (typical, depends on display backlight)
- **Display Backlight:** ~100mA

---

## Quick Command Reference

### Most Common Commands
```
Text display:         <text>               Display text
Clear screen:         #CLR                 Clear display
Text size:            #SIZE 3              Set text size
Text color:           #COLOR RED           Set color
Position:             #POS 0 0             Set cursor position

FPGA passthrough:     >>>text              Send to FPGA
FPGA send:            #FPGASEND text       Send text
FPGA read:            #FPGAREAD 10 1000    Read 10 bytes

I2C scan:             #I2CSCAN             Find I2C devices
I2C read:             #I2CREAD 0x68 0 7    Read 7 bytes
I2C write:            #I2CWRITE 0x68 0 0x12

SPI init:             #SPIBEGIN 53         Start SPI
SPI transfer:         #SPITRANSFER 0x9F    Send/receive

EEPROM read:          #EEPROMREAD 0        Read address 0
EEPROM write:         #EEPROMWRITE 0 0xFF  Write to address 0
EEPROM dump:          #EEPROMDUMP 0 255    View memory

Analog read:          #ANALOGREAD 8        Read A8
Analog read all:      #ANALOGREADALL       Read A8-A15
Analog reference:     #ANALOGREF INTERNAL  1.1V reference

GPIO setup:           #GPIOMODE 22 OUT     Set pin mode
GPIO write:           #GPIOWRITE 22 1      Set pin HIGH
GPIO read:            #GPIOREAD 22         Read pin state

Touch buttons:        #SHOWBTNS            Show buttons
Touch calibration:    #TOUCHCAL ?          Show cal values

Help:                 #HELP                Full command list
Status:               #STATUS              Show all settings
Device ID:            #ID                  Show device info
```

---

## Error Messages

### Common Errors
| Error Message | Meaning | Solution |
|--------------|---------|----------|
| ERR:FORMAT ... | Invalid command syntax | Check command format |
| ERR:INVALID_PARAMS | Parameter out of range | Verify parameter values |
| ERR:INVALID_ADDR | Invalid I2C address | Use 0x00-0x7F range |
| ERR:PIN_RANGE | Pin number invalid | Check valid pin ranges |
| ERR:SPI_NOT_INIT | SPI not initialized | Run #SPIBEGIN first |
| ERR:SPEED_RANGE | SPI speed invalid | Use 100-16000000 Hz |
| ERR:MODE_RANGE | SPI mode invalid | Use 0-3 |
| ERR:NOT_OUTPUT | GPIO write on input pin | Set pin to OUTPUT mode |
| ERR:PIN_22_29_ONLY | GPIO pin out of range | Use pins 22-29 only |
| ERR:EXTERNAL_NOT_AVAILABLE | AREF not available | Use DEFAULT or INTERNAL |

### Tips
- Commands are case-insensitive (except color names)
- Add `?` to query current setting for most commands
- Hex values can be entered with or without `0x` prefix
- Use `#HELP` to see all available commands
- Use `#STATUS` to see all current settings

---

## Appendix A: Common I2C Device Addresses

### Sensors
- **BMP180/BMP280:** 0x76 or 0x77 (pressure/temperature)
- **BME280:** 0x76 or 0x77 (pressure/temperature/humidity)
- **MPU6050:** 0x68 or 0x69 (gyro/accelerometer)
- **AHT20:** 0x38 (temperature/humidity)
- **SHT31:** 0x44 or 0x45 (temperature/humidity)

### Displays
- **SSD1306:** 0x3C or 0x3D (128×64 OLED)
- **SH1106:** 0x3C or 0x3D (128×64 OLED)
- **LCD1602 (PCF8574):** 0x27 or 0x3F (I2C backpack)

### Memory
- **AT24C32:** 0x50 (4KB EEPROM)
- **AT24C256:** 0x50 (32KB EEPROM)
- **24LC256:** 0x50-0x57 (configurable)

### Real-Time Clocks
- **DS1307:** 0x68 (RTC)
- **DS3231:** 0x68 (precision RTC)
- **PCF8523:** 0x68 (RTC)

### ADCs/DACs
- **ADS1115:** 0x48-0x4B (16-bit ADC)
- **MCP4725:** 0x60-0x67 (12-bit DAC)

### I/O Expanders
- **PCF8574:** 0x20-0x27 (8-bit I/O)
- **MCP23017:** 0x20-0x27 (16-bit I/O)

---

## Appendix B: Typical Use Cases

### 1. Data Logger
```
#I2CSCAN                              Find sensors
#I2CREAD 0x76 0xF7 3                 Read BME280
#ANALOGREADALL                        Read analog inputs
#EEPROMWRITE <addr> <data>           Store data
```

### 2. FPGA Development Interface
```
#FPGASEL 1                           Select FPGA1
#FPGA1BAUD 115200                    Set baud rate
#FPGA1PARSE MIXED                    Show hex+ASCII
>>>CONFIG 0x1234                     Send command
#FPGAREAD 16 2000                    Read response
```

### 3. Process Monitoring
```
#ANALOGREF DEFAULT                   5V range
#ANALOGAVG 8                         Reduce noise
#ANALOGREADALL                       Read all sensors
#PROG 10 400 300 30 75              Show progress bar
```

### 4. GPIO-Based Control Panel
```
#GPIOMODE 22 OUT                     LED output
#GPIOMODE 23 INPU                    Button input
#GPIOEVENT 23 FALLING                Detect press
#GPIOWRITE 22 1                      Turn on LED
```

### 5. SPI Flash Programming
```
#SPIBEGIN 53                         Init with CS=53
#SPISETTINGS 4000000 0               4MHz, Mode 0
#SPITRANSFER 0x9F                    Read JEDEC ID
#SPITRANSFER 0x03 0x00 0x00 0x00    Read from 0x000000
```

---

## Support and Documentation

### Getting Help
- Send `#HELP` for command list
- Send `#STATUS` for current settings
- Send `#ID` for device identification

### Version Information
- **Firmware:** MEGA Split Screen ILI9486
- **Features:** Split display, FPGA serial, I2C, SPI, EEPROM, Analog, GPIO, Touch
- **Command Set:** Text-based serial protocol

### Notes
- All commands start with `#` except plain text and FPGA passthrough (`>>>`)
- Commands are not case-sensitive (except color names in some contexts)
- Serial communication at 9600 baud (default)
- Command buffer: 200 characters maximum
- Response format: Human-readable text with status codes

---

*Document Version: 1.0*
*Last Updated: 2025-12-12*
*Compatible with: mega_ili9486_split_text firmware*
