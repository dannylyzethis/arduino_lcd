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
11. [Display Widgets](#display-widgets)
12. [Configuration System](#configuration-system)
13. [Data Logging System](#data-logging-system)
14. [Waveform Generator (PWM)](#waveform-generator-pwm)
15. [Pulse/Frequency Measurement](#pulsefrequency-measurement)
16. [Color Reference](#color-reference)
17. [Technical Specifications](#technical-specifications)

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

### Feature Summary
- **Display:** Text output, colors, rotation, touch buttons, graphical widgets
- **FPGA Interface:** 3x serial ports with configurable baud/termination/parsing
- **Peripherals:** I2C, SPI, GPIO (8 pins), Analog input (8 channels)
- **Storage:** 4KB EEPROM with protection zones and configuration system
- **Data Logging:** Background logging to EEPROM with circular buffer
- **Signal Generation:** PWM waveform generator (square/triangle/sine)
- **Measurement:** Pulse width and frequency measurement
- **Configuration:** Persistent settings for rotation, FPGA, custom button commands

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
- `speed` - Clock frequency in Hz (100 to 8000000)
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
- `8000000` - 8 MHz (maximum - Mega 16MHz/2)
- `1000000` - 1 MHz (slow devices)
- `250000` - 250 kHz (very slow/initialization)

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

#### Configure Write Protection
```
#EEPROMPROTECT <zone> <start> <end>    Set protection zone
#EEPROMPROTECT <zone> OFF              Disable protection zone
#EEPROMPROTECT?                        Show all protection zones
```

**Parameters:**
- `zone` - Protection zone number (0-3, supports 4 zones)
- `start` - Start address of protected range
- `end` - End address of protected range

**Examples:**
```
#EEPROMPROTECT 0 0 99          Protect addresses 0-99 (config area)
#EEPROMPROTECT 1 500 999       Protect addresses 500-999
#EEPROMPROTECT 1 OFF           Disable protection zone 1
#EEPROMPROTECT?                Query all protection zones
```

**Response Examples:**
```
Zone 0: 0x00 - 0x63           (Zone 0 protected: addresses 0-99)
ERR:PROTECTED_ADDR 50         (Attempt to write to protected address)
Zone 1 DISABLED               (Zone disabled successfully)
```

**Notes:**
- Zone 0 (addresses 0-99) is protected by default for config storage
- Up to 4 independent protection zones can be configured
- Protected addresses cannot be written via #EEPROMWRITE
- Protection zones prevent accidental data corruption
- Useful for reserving areas for calibration data, settings, etc.

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
- **SPI Clock:** 100Hz to 8MHz (Mega CLK/2)
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

Display widgets:      #GAUGE 100 100 50 75 100    Draw gauge
                      #BARGRAPH 10 10 300 100 50 75 80    Bar graph
                      #TREND 10 10 300 100 512 520 515    Trend line
                      #XYPLOT 10 10 300 200 0,0 10,5 20,15 GRID    XY plot

Configuration:        #CONFIGSAVE          Save settings
                      #CONFIGLOAD          Load settings
                      #BTNCONFIG UP 3 0xAA 0xBB 0xCC    Custom button
                      #FPGAWRITE 100 0xFF 0xAA    Write FPGA zone
                      #FPGAREAD 100 10     Read FPGA zone

Data logging:         #LOGSTART 1000 0     Log A8 every 1s
                      #LOGREAD 20          Read 20 entries
                      #LOGZONE 2000 4095   Set log zone

Waveform gen:         #WAVEGEN 9 SINE 1000    1kHz sine wave
Frequency:            #FREQMON 2 1000      Monitor frequency

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
| ERR:SPEED_RANGE | SPI speed invalid | Use 100-8000000 Hz |
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

## Display Widgets

### Overview
Advanced visualization widgets for creating dynamic dashboards and monitoring interfaces.

### Widget Commands

#### Circular Gauge
```
#GAUGE <x> <y> <radius> <value> <max>    Draw analog-style gauge
```

**Parameters:**
- `x`, `y` - Center position of gauge
- `radius` - Radius in pixels
- `value` - Current value to display
- `max` - Maximum value (full scale)

**Examples:**
```
#GAUGE 100 100 60 75 100        75% gauge at (100,100), radius 60
#GAUGE 160 150 80 3500 5000     RPM gauge
#COLOR GREEN
#GAUGE 80 120 50 45 100         Green 45% gauge
```

**Visual Features:**
- 270° sweep (bottom-left to bottom-right)
- Tick marks every 30°
- Red needle pointer
- Value displayed at bottom

#### Bar Graph
```
#BARGRAPH <x> <y> <w> <h> <val1> <val2> ...    Draw multi-bar graph
```

**Parameters:**
- `x`, `y` - Top-left corner
- `w`, `h` - Width and height
- `val1, val2, ...` - Values for each bar (up to 16 bars)

**Examples:**
```
#BARGRAPH 10 50 300 100 25 50 75 100      4 bars
#BARGRAPH 10 50 300 150 10 20 15 25 30    5 bars showing trend
```

**Visual Features:**
- Auto-scaling to maximum value
- Multi-color bars (green, cyan, yellow pattern)
- Bordered box
- Bars drawn from bottom up

#### Large Numeric Display
```
#NUMBOX <x> <y> <value>    Display large number
```

**Parameters:**
- `x`, `y` - Top-left corner
- `value` - Number or text to display (can include decimals, units)

**Examples:**
```
#NUMBOX 50 100 25.6           Display "25.6"
#NUMBOX 10 50 3.14V           Display "3.14V"
#COLOR RED
#NUMBOX 100 200 ERROR         Display "ERROR" in red
```

**Visual Features:**
- Extra-large text (4x size)
- Uses current text color
- Supports any text/numbers

#### Trend/Line Graph
```
#TREND <x> <y> <w> <h> <val1> <val2> ...    Draw line graph
```

**Parameters:**
- `x`, `y` - Top-left corner
- `w`, `h` - Width and height
- `val1, val2, ...` - Data points to plot (up to 46 points)

**Examples:**
```
#TREND 10 100 300 150 10 15 12 18 22 25 20      Plot 7 points
#TREND 10 50 300 100 100 105 103 108 112 110    Temperature trend
```

**Visual Features:**
- Auto-scaling to min/max values
- Center reference line (gray)
- Connected line plot
- Data point markers (circles)
- Bordered box

#### XY Plot/Graph
```
#XYPLOT <x> <y> <w> <h> <x1,y1> <x2,y2> ...    Draw XY scatter/line plot
```

**Parameters:**
- `x`, `y` - Top-left corner
- `w`, `h` - Width and height
- `x1,y1` - First data point (X,Y coordinates)
- `x2,y2` - Second data point
- `...` - Additional points (up to 50 XY pairs)

**Options:**
- `STYLE POINTS` - Show only data points
- `STYLE LINES` - Show only connecting lines
- `STYLE BOTH` - Show both points and lines (default)
- `GRID` - Display grid lines

**Examples:**
```
# Simple XY plot (auto-scaled)
#XYPLOT 10 10 300 200 0,0 10,5 20,15 30,25 40,20

# Plot with grid and points only
#XYPLOT 10 10 300 200 0,0 50,100 100,150 150,120 STYLE POINTS GRID

# Frequency response plot
#XYPLOT 10 10 300 150 10,-20 100,-15 1000,-10 10000,-25 STYLE LINES

# Calibration curve
#XYPLOT 10 10 300 200 0,0 256,1.1 512,2.2 768,3.3 1023,4.4 GRID

# Lissajous figure
#XYPLOT 50 50 200 200 0,0 50,86 86,50 100,0 86,-50 50,-86 STYLE BOTH
```

**Visual Features:**
- Auto-scaling to fit all data points (with 10% margin)
- Axis labels showing X and Y min/max values
- Optional grid lines (5x5 dotted pattern)
- Three plot styles: points, lines, or both
- Supports negative values
- Perfect for: oscilloscope traces, calibration curves, frequency responses, I-V curves, position tracking

**Use Cases:**
- **Sensor Calibration:** Plot ADC reading vs actual voltage
- **Frequency Response:** Plot frequency vs gain (Bode plot)
- **I-V Characteristics:** Plot current vs voltage for components
- **Position Tracking:** Plot X-Y coordinates for robot paths
- **Oscilloscope Traces:** Plot time-domain waveforms
- **Lissajous Patterns:** Phase relationship visualization

### Widget Usage Examples

#### Real-Time Dashboard
```
#CLR
#COLOR WHITE
#NUMBOX 10 10 98.6F          Temperature reading
#COLOR CYAN
#GAUGE 200 80 60 45 100      CPU usage gauge
#COLOR GREEN
#BARGRAPH 10 180 300 100 50 60 55 70 65    5-minute load average
```

#### Sensor Monitoring
```
#ANALOGREADALL                Read all sensors
#COLOR YELLOW
#TREND 10 10 300 150 512 520 518 525 530    Plot A8 history
#COLOR WHITE
#NUMBOX 50 200 2.55V         Display voltage
```

---

## Configuration System

### Overview
Persistent storage of system settings, FPGA parameters, and custom button configurations in EEPROM. Settings are automatically loaded on power-up and can be saved/restored at any time.

### Configurable Settings

**Display Settings:**
- Screen rotation (0-3)

**FPGA Settings:**
- Baud rates for Serial1, Serial2, Serial3
- Termination bytes for each FPGA port
- Stop bits configuration
- RX and parse modes

**Button Settings:**
- Custom byte sequences for UP, DOWN, LEFT, RIGHT buttons
- Configurable length (1-15 bytes per button)
- Allows custom control commands

### Configuration Commands

#### Save Configuration
```
#CONFIGSAVE    Save current settings to EEPROM
```

**What gets saved:**
- Display rotation
- All FPGA serial port settings
- Custom button byte sequences

**Example:**
```
#ROT 1                   Change rotation
#FPGA1BAUD 115200        Change FPGA1 baud rate
#BTNCONFIG UP 3 0xAA 0xBB 0xCC    Custom button bytes
#CONFIGSAVE              Save all settings
```

#### Load Configuration
```
#CONFIGLOAD    Load settings from EEPROM
```

**Response:**
```
CONFIG_LOADED
Rotation: 1
```

**Note:** Configuration is automatically loaded on power-up/reset

#### Reset to Defaults
```
#CONFIGRESET    Reset all settings to factory defaults
```

**Defaults:**
- Rotation: 0
- FPGA selected: 1
- Button UP: 0x55 0x50 ("UP")
- Button DOWN: 0x44 0x4E ("DN")
- Button LEFT: 0x4C 0x54 ("LT")
- Button RIGHT: 0x52 0x54 ("RT")

#### Show Configuration
```
#CONFIGSHOW    Display all current settings
```

**Example Output:**
```
=== CONFIGURATION ===
Rotation: 1
FPGA Selected: 1
FPGA1 Baud: 115200
FPGA2 Baud: 9600
FPGA3 Baud: 9600
Button Configs:
  UP [2]: 0x55 0x50
  DOWN [2]: 0x44 0x4E
  LEFT [2]: 0x4C 0x54
  RIGHT [2]: 0x52 0x54
```

### Button Configuration

#### Configure Button Bytes
```
#BTNCONFIG <button> <length> <byte1> [byte2] ...    Set button command
#BTNCONFIG <button> ?                                Query button config
```

**Parameters:**
- `button` - Button name: UP, DOWN, LEFT, or RIGHT
- `length` - Number of bytes to send (1-15)
- `byte1-15` - Hex or decimal byte values

**Examples:**
```
# Single byte command
#BTNCONFIG UP 1 0xFF

# Multi-byte command (3 bytes)
#BTNCONFIG DOWN 3 0xAA 0xBB 0xCC

# Long command (8 bytes)
#BTNCONFIG LEFT 8 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08

# Query button configuration
#BTNCONFIG UP ?
# Response: UP [3]: 0xAA 0xBB 0xCC
```

**Notes:**
- Changes take effect immediately
- Use #CONFIGSAVE to make changes persistent
- Buttons send bytes to currently selected FPGA port
- Termination byte is appended automatically

### FPGA EEPROM Zone

Dedicated 200-byte zone (addresses 100-299) for FPGA data exchange.

#### Write to FPGA Zone
```
#FPGAWRITE <addr> <byte1> [byte2] ...    Write to FPGA zone
```

**Parameters:**
- `addr` - Address (100-299)
- `byte1-N` - Data bytes to write

**Examples:**
```
#FPGAWRITE 100 0xFF 0xAA 0xBB    Write 3 bytes starting at 100
#FPGAWRITE 150 65 66 67          Write ASCII "ABC" at 150
```

#### Read from FPGA Zone
```
#FPGAREAD <addr> [count]    Read from FPGA zone
```

**Parameters:**
- `addr` - Address (100-299)
- `count` - Number of bytes to read (default: 1)

**Examples:**
```
#FPGAREAD 100                Read 1 byte from address 100
#FPGAREAD 100 10             Read 10 bytes from address 100
```

**Response:**
```
FPGA[0x64]: 0xFF 0xAA 0xBB 0xCC 0xDD ...
```

### Configuration Usage Examples

#### Power-On Configuration
```
# On first boot
#CONFIGSHOW              Check current settings
# Response: CONFIG_RESET (using defaults)

# Configure as needed
#ROT 1
#FPGA1BAUD 115200
#BTNCONFIG UP 3 0x01 0x02 0x03

# Save configuration
#CONFIGSAVE

# On next power-up, settings are automatically restored
# [CONFIG] Loaded from EEPROM
```

#### Custom Control Panel
```
# Configure buttons for specific FPGA commands
#BTNCONFIG UP 4 0x10 0x00 0xFF 0xAA       Start command
#BTNCONFIG DOWN 4 0x10 0x01 0xFF 0xAA     Stop command
#BTNCONFIG LEFT 3 0x20 0x00 0x01          Mode 1
#BTNCONFIG RIGHT 3 0x20 0x00 0x02         Mode 2

# Save button configuration
#CONFIGSAVE

# Test buttons
#SHOWBTNS
# Touch buttons now send custom commands
```

#### FPGA Data Exchange
```
# Arduino writes configuration to FPGA zone
#FPGAWRITE 100 0x01 0x02 0x03 0x04

# FPGA reads from zone via serial commands
# (FPGA sends: #FPGAREAD 100 4)

# FPGA writes results to zone
# (FPGA sends: #FPGAWRITE 150 0xAA 0xBB)

# Arduino reads results
#FPGAREAD 150 2
# Response: FPGA[0x96]: 0xAA 0xBB
```

---

## Data Logging System

### Overview
Automatic data logging to internal EEPROM with circular buffer management. Stores timestamped sensor readings or GPIO states for later analysis.

### EEPROM Memory Map
- **Addresses 0-99:** Configuration zone (protected by default)
  - System settings (rotation, FPGA baud rates, termination bytes)
  - Button byte sequences (customizable, up to 15 bytes each)
- **Addresses 100-299:** FPGA dedicated zone (200 bytes, protected)
  - Reserved for FPGA data exchange
  - Access via #FPGAWRITE and #FPGAREAD commands
- **Addresses 300-499:** User settings zone (200 bytes, protected)
  - Reserved for user-defined data
- **Addresses 500-1999:** Free space (1500 bytes, unprotected)
- **Addresses 2000-4095:** Data logging zone (3096 bytes, circular buffer)
  - Default log zone (configurable via #LOGZONE)
  - Log Entry Size: 8 bytes per entry
  - Maximum Entries: ~387 entries (with default zone)

### Log Entry Format
Each entry contains:
- **Timestamp** (4 bytes) - milliseconds since startup
- **Value** (2 bytes) - sensor reading or GPIO state
- **Source** (1 byte) - data source identifier
- **Flags** (1 byte) - reserved for future use

### Data Sources
| Source ID | Description |
|-----------|-------------|
| 0-7       | Analog pins A8-A15 |
| 15        | GPIO register (all 8 pins) |

### Logging Commands

#### Start Logging
```
#LOGSTART <interval> [source]    Start data logging
```

**Parameters:**
- `interval` - Time between samples in milliseconds (minimum 10ms)
- `source` - Data source to log (0-7 for A8-A15, 15 for GPIO)

**Examples:**
```
#LOGSTART 1000 0         Log A8 every 1 second
#LOGSTART 500 2          Log A10 every 500ms
#LOGSTART 100 15         Log GPIO register every 100ms
#LOGSTART 2000           Use previous source, 2 second interval
```

**Response:** `LOG_START: <interval>ms, source=<source>`

#### Stop Logging
```
#LOGSTOP                 Stop data logging
```

**Response:**
```
LOG_STOPPED
Entries logged: 145
```

#### Read Log Data
```
#LOGREAD [count]         Read log entries
```

**Parameters:**
- `count` - Number of entries to display (default 10)

**Example Output:**
```
=== LOG DATA (10 entries) ===
Time(ms)   Value  Source
1234       512    A8
2234       518    A8
3234       520    A8
4234       515    A8
...
```

#### Clear Log
```
#LOGCLEAR                Erase all logged data
```

**Response:** `LOG_CLEARED`

#### Configure Source
```
#LOGCONFIG <source>      Set data source without starting
```

**Examples:**
```
#LOGCONFIG 0             Set to log A8
#LOGCONFIG 15            Set to log GPIO register
```

#### Check Status
```
#LOGSTATUS               Display logging status
```

**Example Output:**
```
=== LOG STATUS ===
Active: YES
Interval: 1000ms
Source: A8
Entries: 145
Next addr: 0x05B4
```

#### Configure Log Zone
```
#LOGZONE <start> <end>       Set logging address range
#LOGZONE?                    Query current log zone
```

**Parameters:**
- `start` - Start address for logging (must not overlap protected zones)
- `end` - End address for logging

**Examples:**
```
#LOGZONE 1000 4095           Use addresses 1000-4095 (default)
#LOGZONE 2000 3999           Use addresses 2000-3999 (2000 bytes)
#LOGZONE?                    Query current zone
```

**Response Examples:**
```
LOG_ZONE: 0x3E8 - 0xFFF      (Zone configured: 1000-4095)
Capacity: 387 entries        (Maximum entries in this zone)
ERR:OVERLAPS_ZONE 0          (Zone overlaps with protected area)
ERR:START >= END             (Invalid range)
```

**Notes:**
- Default zone: 1000-4095 (3096 bytes, ~387 entries)
- Logging zone cannot overlap with protected EEPROM zones
- Changing zone resets write position and entry count
- Use smaller zones to reserve EEPROM for other data
- Calculate capacity: (end - start + 1) / 8 entries

### Logging Usage Examples

#### Temperature Data Logger
```
#ANALOGREF DEFAULT       Use 5V reference
#ANALOGAVG 4             Average 4 samples
#LOGCONFIG 0             Set to log A8
#LOGSTART 5000 0         Log every 5 seconds
... wait ...
#LOGSTOP                 Stop logging
#LOGREAD 20              View last 20 readings
```

#### GPIO Event Logger
```
#GPIOMODE 22 INPU        Configure input with pullup
#LOGSTART 100 15         Log GPIO state every 100ms
... activity occurs ...
#LOGSTOP
#LOGREAD 50              View GPIO state changes
```

#### Continuous Monitoring
```
#LOGSTART 60000 1        Log A9 every minute
... runs indefinitely ...
#LOGSTATUS               Check status anytime
#LOGREAD                 View recent 10 entries
```

#### EEPROM Zone Configuration Example
```
# Set up protection zones for dedicated storage
#EEPROMPROTECT 0 0 99             Protect config area (default)
#EEPROMPROTECT 1 100 299          Protect calibration data
#EEPROMPROTECT 2 300 499          Protect user settings

# Configure logging to use specific zone
#LOGZONE 2000 4095                Set log zone (2000-4095)
#LOGZONE?                         Verify configuration
# Response: LOG_ZONE: 0x7D0 - 0xFFF
# Response: Capacity: 262 entries

# Now addresses 500-1999 remain free for other uses
#EEPROMWRITE 500 0xAA             Write to unprotected area
#EEPROMWRITE 50 0xFF              ERROR: Protected zone 0

# Start logging in configured zone
#LOGSTART 1000 0                  Log A8 every second
```

### Important Notes
- **Circular Buffer:** When full, oldest data is overwritten
- **Power Loss:** Data persists in EEPROM after power cycle
- **EEPROM Wear:** 100,000 write cycles per byte - consider interval vs. lifetime
- **Background Operation:** Logging continues during other operations
- **Timestamp:** Uses millis() - resets on power cycle
- **Zone Protection:** Log zone cannot overlap with EEPROM protection zones
- **Default Protection:** Addresses 0-99 are protected by default to prevent config corruption

---

## Waveform Generator (PWM)

### Overview
Generate analog waveforms using PWM (Pulse Width Modulation) for testing, signal simulation, and control applications.

### PWM Pin Support
**Arduino Mega 2560 PWM Pins:**
- **2-13** (12 pins)
- **44-46** (3 pins)

**Total:** 15 PWM-capable pins

### Waveform Types

#### Square Wave
- 50% duty cycle
- Clean digital transitions
- Ideal for clock signals, digital testing

#### Triangle Wave
- Linear rise and fall
- Smooth transitions
- Good for sweep testing, motor ramping

#### Sine Wave
- Smooth sinusoidal output (32-step LUT approximation)
- Low harmonic distortion
- Ideal for audio, sensor testing

### Waveform Commands

#### Generate Waveform
```
#WAVEGEN <pin> <type> <freq>    Start waveform generation
```

**Parameters:**
- `pin` - PWM pin number (2-13, 44-46)
- `type` - Waveform type: `SQUARE`, `TRIANGLE` (or `TRI`), `SINE` (or `SIN`), or numeric (0/1/2)
- `freq` - Frequency in Hz (1-10000)

**Examples:**
```
#WAVEGEN 3 SQUARE 1000          1kHz square wave on pin 3
#WAVEGEN 5 SINE 440             440Hz sine wave (A4 note)
#WAVEGEN 6 TRIANGLE 100         100Hz triangle wave
#WAVEGEN 9 TRI 2000             2kHz triangle wave
#WAVEGEN 11 0 5000              5kHz square (type 0)
```

**Response:**
```
WAVE_START: Pin=3, Type=SQUARE, Freq=1000Hz
```

**Error Messages:**
```
ERR:PIN_NO_PWM                  Pin doesn't support PWM
PWM pins: 2-13, 44-46           Valid pins listed
ERR:FORMAT #WAVEGEN...          Invalid command format
```

#### Stop Waveform
```
#WAVESTOP                       Stop waveform generation
```

**Response:** `WAVE_STOPPED`

**Note:** Sets output pin to LOW (0V)

### Waveform Specifications

#### Frequency Range
- **Minimum:** 1 Hz
- **Maximum:** 10,000 Hz (10 kHz)
- **Resolution:** 1 Hz steps

#### Output Voltage
- **Logic High:** 5V (typical)
- **Logic Low:** 0V
- **PWM Resolution:** 8-bit (256 steps)

#### Waveform Accuracy
- **Square:** Exact 50% duty cycle
- **Triangle:** 256-step linear approximation
- **Sine:** 32-point lookup table (good quality)

### Waveform Usage Examples

#### Audio Tone Generation
```
#WAVEGEN 3 SINE 440         A4 note (440 Hz)
#WAVEGEN 3 SINE 523         C5 note
#WAVEGEN 3 SINE 880         A5 note
#WAVESTOP                   Silence
```

**Note:** Add low-pass filter and amplifier for speaker output

#### Function Generator
```
#WAVEGEN 5 SQUARE 1000      1kHz square wave
... test circuit ...
#WAVEGEN 5 TRIANGLE 1000    Switch to triangle
... test circuit ...
#WAVEGEN 5 SINE 1000        Switch to sine
... test circuit ...
#WAVESTOP                   Done
```

#### Motor Speed Control
```
#WAVEGEN 6 TRIANGLE 10      Slow ramp (10 Hz)
... motor ramps slowly ...
#WAVEGEN 6 SQUARE 100       Faster switching (100 Hz)
... motor runs faster ...
#WAVESTOP                   Stop motor
```

#### Clock Signal
```
#WAVEGEN 2 SQUARE 4000      4kHz clock signal
... runs continuously ...
#WAVESTOP                   Stop clock
```

### PWM Filtering

For analog output, add RC low-pass filter:

```
Arduino Pin ----[R]----+---- Output
                       |
                      [C]
                       |
                      GND

Recommended values:
R = 1kΩ
C = 10µF (cutoff ≈ 16Hz)
```

### Important Notes
- **One Waveform at a Time:** Stop current waveform before starting new one on different pin
- **Pin Conflicts:** Don't use PWM pins already in use (LCD uses pins 8, 9)
- **Load Current:** Maximum 40mA per pin - use transistor/MOSFET for higher loads
- **Background Operation:** Waveform continues during other operations
- **Frequency Limits:** Very high frequencies (>5kHz) may have reduced accuracy

### Common Applications

| Application | Type | Frequency | Notes |
|------------|------|-----------|-------|
| LED dimming | Square | 500-2000 Hz | Flicker-free |
| Servo control | Square | 50 Hz | Need specific pulse width |
| Audio test | Sine | 20-20000 Hz | Add filter |
| Motor control | Triangle | 10-100 Hz | Smooth acceleration |
| Clock signal | Square | 1-10000 Hz | Digital logic |
| Signal simulation | Any | Variable | Testing sensors |

---

## Pulse/Frequency Measurement

### Overview
Measure pulse widths, count pulses, and monitor frequencies for sensors, encoders, and signal analysis.

### Available Pins
- **All digital pins** for pulse measurement
- **Interrupt pins (2, 3, 18-21)** for continuous frequency monitoring

### Measurement Commands

#### Measure Pulse Width
```
#PULSEIN <pin> <state> <timeout>    Measure single pulse duration
```

**Parameters:**
- `pin` - Any digital pin number
- `state` - Pulse state to measure: `HIGH`, `LOW`, `1`, or `0`
- `timeout` - Maximum wait time in microseconds (max 3,000,000)

**Examples:**
```
#PULSEIN 30 HIGH 1000000       Measure HIGH pulse, 1 second timeout
#PULSEIN 31 LOW 500000         Measure LOW pulse, 500ms timeout
#PULSEIN 32 1 2000000          Measure HIGH pulse, 2 second timeout
```

**Response:**
```
PULSE_IN: Pin=30, State=HIGH, Duration=1234us
NOTE: Timeout or no pulse detected
```

**Measurement:**
- Returns pulse duration in microseconds (µs)
- Blocks until pulse detected or timeout
- Returns 0 if timeout occurs

#### Count Pulses (One-Shot)
```
#FREQCOUNT <pin> <duration>    Count pulses over time period
```

**Parameters:**
- `pin` - Any digital pin number
- `duration` - Measurement window in milliseconds (10-10000)

**Examples:**
```
#FREQCOUNT 30 1000         Count pulses for 1 second
#FREQCOUNT 31 5000         Count pulses for 5 seconds
#FREQCOUNT 32 100          Count pulses for 100ms
```

**Response:**
```
FREQ_COUNT: Pin=30, Pulses=1234, Duration=1000ms, Freq=1234.00Hz
```

**Features:**
- Counts rising edges (LOW to HIGH transitions)
- Calculates frequency in Hz
- Blocks during measurement
- Good for one-time measurements

#### Continuous Frequency Monitoring
```
#FREQMON <pin> <duration>      Start continuous monitoring
#FREQSTOP                      Stop monitoring
```

**Parameters:**
- `pin` - Interrupt-capable pin (2, 3, 18, 19, 20, 21)
- `duration` - Measurement window in milliseconds (100-10000)

**Examples:**
```
#FREQMON 2 1000            Monitor pin 2, update every 1 second
#FREQMON 3 500             Monitor pin 3, update every 500ms
#FREQSTOP                  Stop monitoring
```

**Response:**
```
FREQ_MON_START: Pin=2, Window=1000ms
FREQ: 1234.56Hz (1234 pulses)
FREQ: 1235.12Hz (1235 pulses)
...
FREQ_MON_STOPPED
```

**Features:**
- Uses hardware interrupts (non-blocking)
- Automatic periodic updates
- High accuracy for continuous signals
- Background operation

### Interrupt Pin Mapping

| Pin | Interrupt | Notes |
|-----|-----------|-------|
| 2   | INT0      | Available |
| 3   | INT1      | Available |
| 18  | INT5      | TX1 (Serial1) |
| 19  | INT4      | RX1 (Serial1) |
| 20  | INT3      | SDA (I2C) |
| 21  | INT2      | SCL (I2C) |

**Note:** Pins 18-21 may conflict with Serial1 or I2C if those features are in use.

### Measurement Specifications

#### Pulse Width (PULSEIN)
- **Resolution:** 1 microsecond
- **Range:** 10µs to 3,000,000µs (3 seconds)
- **Accuracy:** ±1µs typical
- **Blocking:** Yes

#### Frequency Count (FREQCOUNT)
- **Resolution:** 0.01 Hz
- **Range:** 0.1 Hz to 100 kHz (depending on duration)
- **Min Duration:** 10ms
- **Max Duration:** 10 seconds
- **Blocking:** Yes

#### Frequency Monitor (FREQMON)
- **Resolution:** 0.01 Hz
- **Range:** 10 Hz to 50 kHz
- **Update Rate:** 100ms to 10 seconds
- **Blocking:** No (uses interrupts)

### Usage Examples

#### Measure Ultrasonic Sensor (HC-SR04)
```
#GPIOMODE 30 OUT           Trigger pin
#GPIOMODE 31 IN            Echo pin
#GPIOWRITE 30 1            Send trigger pulse
#GPIOWRITE 30 0
#PULSEIN 31 HIGH 30000     Measure echo (30ms timeout)
```

**Distance calculation:**
- Duration in µs / 58 = Distance in cm
- Example: 1160µs / 58 = 20cm

#### Measure Motor RPM (with encoder)
```
#FREQCOUNT 2 1000          Count pulses for 1 second
```

**RPM calculation:**
- If encoder has 20 pulses per revolution:
- RPM = (Frequency × 60) / 20
- Example: 100 Hz → (100 × 60) / 20 = 300 RPM

#### Monitor Tachometer Signal
```
#FREQMON 3 1000            Update every second
... motor running ...
#FREQSTOP                  Stop when done
```

**Output:**
```
FREQ: 166.67Hz (167 pulses)    10,000 RPM (with 1 pulse/rev)
FREQ: 83.33Hz (83 pulses)      5,000 RPM
```

#### Measure PWM Signal
```
#PULSEIN 30 HIGH 10000     Measure HIGH time
#PULSEIN 30 LOW 10000      Measure LOW time
```

**Duty cycle calculation:**
- Duty % = (HIGH time / (HIGH time + LOW time)) × 100
- Example: HIGH=800µs, LOW=200µs → 80% duty cycle

#### Frequency Counter for Signal Generator
```
#FREQMON 2 1000            Continuous monitoring
... adjust signal generator ...
... read frequency updates ...
#FREQSTOP
```

#### Measure AC Power Frequency
```
#FREQCOUNT 30 2000         Count for 2 seconds (high accuracy)
```

**Expected:** ~50Hz (Europe) or ~60Hz (North America)

### Timing Accuracy

#### Factors Affecting Accuracy

**PULSEIN:**
- Interrupt overhead: ~5µs
- Clock precision: 16 MHz crystal
- Best for: >100µs pulses

**FREQCOUNT:**
- Polling speed: ~1µs per loop
- Best for: <10 kHz signals
- Accuracy improves with longer duration

**FREQMON:**
- Interrupt-driven: very accurate
- No polling overhead
- Best for: continuous signals >10 Hz

### Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| ERR:TIMEOUT_MAX_3000000 | Timeout too large | Use ≤3,000,000µs |
| ERR:DURATION_RANGE | Invalid duration | Use 10-10000ms (FREQCOUNT) or 100-10000ms (FREQMON) |
| ERR:PIN_NO_INTERRUPT | Pin doesn't support interrupts | Use pins 2, 3, 18-21 for FREQMON |
| ERR:NOT_ACTIVE | FREQSTOP called without active monitoring | Start FREQMON first |

### Important Notes

- **PULSEIN blocks:** Code waits for pulse or timeout
- **FREQCOUNT blocks:** Code waits for full duration
- **FREQMON is non-blocking:** Uses interrupts, runs in background
- **Pin conflicts:** Don't monitor pins used by other features
- **Signal levels:** Requires 5V logic levels or level shifting for 3.3V
- **Debouncing:** Mechanical switches may need debouncing
- **Maximum frequency:** Limited by interrupt latency (~50 kHz practical limit)

### Applications

| Application | Command | Notes |
|------------|---------|-------|
| Ultrasonic distance | PULSEIN | Measure echo pulse |
| Servo position feedback | PULSEIN | Measure pulse width |
| Motor RPM | FREQCOUNT or FREQMON | Encoder pulses |
| Tachometer | FREQMON | Continuous speed monitoring |
| Frequency counter | FREQCOUNT | One-time measurement |
| PWM analysis | PULSEIN | Measure duty cycle |
| IR remote decode | PULSEIN | Measure pulse patterns |
| Flow meter | FREQMON | Continuous flow rate |
| Wheel speed sensor | FREQMON | Vehicle speed |
| Signal generator test | FREQCOUNT | Verify output frequency |

---

## Support and Documentation

### Getting Help
- Send `#HELP` for command list
- Send `#STATUS` for current settings
- Send `#ID` for device identification

### Version Information
- **Firmware:** MEGA Split Screen ILI9486 v2.0
- **Features:** Split display, FPGA serial, I2C, SPI, EEPROM, Analog, GPIO, Touch, Widgets, Data Logging, Waveform Gen, Pulse/Freq Measurement
- **Command Set:** Text-based serial protocol
- **New in v2.0:** Display widgets, Data logging system, PWM waveform generator, Pulse/frequency measurement

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
