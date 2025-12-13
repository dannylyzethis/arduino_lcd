/*
 * ILI9486/9488 Split Screen MEGA - FULL FEATURED EDITION
 * Arduino Mega 2560 + ILI9486/9488 (320x480)
 * Top=Commands, Bottom=FPGA/Serial Devices
 * Hardware Serial1/2/3 for FPGA (easier than registers!)
 *
 * Serial Ports (with hardware stop bit control, RX modes, and parsing):
 * - Serial:  USB/PC (115200 baud)
 * - Serial1: FPGA1 (TX1=18, RX1=19) - Default, 1 stop bit, TEXT mode, NONE parse
 * - Serial2: FPGA2 (TX2=16, RX2=17) - 1 stop bit, TEXT mode, NONE parse
 * - Serial3: FPGA3 (TX3=14, RX3=15) - 1 stop bit, TEXT mode, NONE parse
 * Configure:
 *   #FPGA1STOP 2        - Set 2 hardware stop bits
 *   #FPGA1RX BINARY     - Receive bytes as hex (for binary protocols)
 *   #FPGA1RX TEXT       - Receive as text (default)
 *   #FPGA1PARSE ASCII   - Parse bytes as ASCII with [XX] for non-printable
 *   #FPGA1PARSE DEC     - Display bytes as decimal numbers
 *   #FPGA1PARSE MIXED   - Show both hex and ASCII: "0x41='A'"
 *   #STATUS             - Show all current settings
 * Query any setting with ?: #FPGA1BAUD ?, #FPGA1RX ?, #TERM ?, etc.
 *
 * Touch Buttons: 4 buttons send byte arrays to FPGA (requires touch screen)
 * - UP:    0x55 0x50 ("UP")
 * - DOWN:  0x44 0x4E ("DN")
 * - LEFT:  0x4C 0x54 ("LT")
 * - RIGHT: 0x52 0x54 ("RT")
 * - Shows TX bytes on LCD bottom screen: "TX: 0x55 0x50"
 * - Works correctly in all screen rotations (0-3)
 * - Button positions automatically recalculate on rotation change
 * Customize in initButtons() function (search for "Initialize button layout")
 *
 * Touch Calibration:
 * - Adjustable calibration values for accurate touch detection
 * - Default: MINX=120, MAXX=900, MINY=70, MAXY=920
 * Commands:
 *   #TOUCHCAL ?                     - Show current calibration
 *   #TOUCHCAL 120 900 70 920        - Set calibration values
 *   #TOUCHTEST                      - Toggle test mode (shows raw & mapped coordinates)
 * Use #TOUCHTEST to find corner values, then set with #TOUCHCAL
 *
 * Termination Control (applies to ALL serial transmissions):
 * - Configurable termination: NONE, LF (\n), CR (\r), CRLF (\r\n), or CUSTOM byte
 * - Use #TERM command: #TERM LF, #TERM CRLF, #TERM 0xFF, #TERM 255, etc.
 * - Default: LF (\n)
 * - Custom byte mode for stop bits: #TERM 0xFF or #TERM FF
 * - Query: #TERM ?
 * - Applies to: >>> bypass, touch buttons, #FPGASEND, #FPGABYTES, #FPGAPING
 * - Ensures proper framing for all FPGA/embedded communication
 *
 * GPIO System (Pins 22-29 on side connector):
 * - 8 configurable I/O pins available
 * - Modes: INPUT, INPUT_PULLUP, OUTPUT
 * - Event-driven: RISING, FALLING, or BOTH edge detection
 * - Register-based control: Set all 8 pins with single byte (0-255)
 * - Events displayed on LCD bottom screen and logged to serial
 * Commands:
 *   #GPIOMODE 22 OUT     - Set pin 22 as output
 *   #GPIOWRITE 22 1      - Write HIGH to pin 22
 *   #GPIOREAD 23         - Read pin 23
 *   #GPIOEVENT 24 RISING - Trigger event on pin 24 rising edge
 *   #GPIOREG             - Show 8-bit register (all pins)
 *   #GPIOSET 255         - Set register to 0xFF (all HIGH)
 *
 * I2C System (Wire library - SDA=20, SCL=21):
 * - Communicate with I2C devices (sensors, EEPROMs, etc.)
 * - Full master mode support
 * Commands:
 *   #I2CSCAN                        - Scan I2C bus for devices (0x00-0x7F)
 *   #I2CREAD <addr> <reg> <bytes>   - Read from I2C device
 *   #I2CWRITE <addr> <reg> <data>   - Write to I2C register
 *   #I2CWRITE <addr> <data...>      - Write bytes to I2C device
 * Example: #I2CREAD 0x50 0x00 16  - Read 16 bytes from EEPROM at 0x50
 *
 * SPI System (Hardware SPI - MOSI=51, MISO=50, SCK=52):
 * - Communicate with SPI devices (sensors, SD cards, displays, etc.)
 * - Configurable CS pin, speed, and mode
 * Commands:
 *   #SPIBEGIN <cs_pin>              - Initialize SPI with chip select pin
 *   #SPIEND                         - End SPI communication
 *   #SPITRANSFER <byte...>          - Transfer bytes (shows response)
 *   #SPISETTINGS <speed> <mode>     - Set clock (Hz) and mode (0-3)
 * Example: #SPIBEGIN 53, #SPITRANSFER 0x9F 0x00 0x00  - Read chip ID
 *
 * EEPROM System (4KB internal EEPROM):
 * - Read/Write persistent data (survives power loss)
 * - 4096 bytes available (addresses 0-4095)
 * Commands:
 *   #EEPROMREAD <addr>              - Read single byte
 *   #EEPROMWRITE <addr> <value>     - Write single byte
 *   #EEPROMDUMP <start> <end>       - Dump address range (hex view)
 *   #EEPROMCLEAR                    - Clear entire EEPROM (write 0xFF)
 * Example: #EEPROMWRITE 0 42, #EEPROMREAD 0  - Store and read value
 *
 * Analog Input System (8 pins: A8-A15):
 * - Read analog voltages (0-5V with 10-bit resolution: 0-1023)
 * - Reference voltage control (DEFAULT=5V, INTERNAL=1.1V)
 * - Averaging for noise reduction
 * - NOTE: EXTERNAL reference not available (AREF used by LCD shield)
 * Commands:
 *   #ANALOGREAD <pin>               - Read single pin (A8-A15)
 *   #ANALOGREADALL                  - Read all 8 pins at once
 *   #ANALOGREF <DEFAULT|INTERNAL>   - Set voltage reference
 *   #ANALOGAVG <samples>            - Set averaging (1-16 samples)
 * Example: #ANALOGREAD A8, #ANALOGREAD 8  - Read pin A8
 * Resolution: 10-bit (0-1023), 0=0V, 1023=Vref
 * With DEFAULT ref (5V): each count = 4.88mV
 * With INTERNAL ref (1.1V): each count = 1.07mV (high precision!)
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Wire.h>      // I2C library
#include <SPI.h>       // SPI library
#include <EEPROM.h>    // Internal EEPROM library

MCUFRIEND_kbv tft;

// Touchscreen pins (standard MCUFRIEND configuration for Mega)
#define YP A3  // Y+ is on Analog3
#define XM A2  // X- is on Analog2
#define YM 9   // Y- is on Digital9
#define XP 8   // X+ is on Digital8

// Touchscreen calibration (adjust these for your specific screen)
// Now adjustable via #TOUCHCAL command
int16_t TS_MINX = 120;
int16_t TS_MAXX = 900;
int16_t TS_MINY = 70;
int16_t TS_MAXY = 920;

#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Touch test mode
bool touchTestMode = false;

#define BASE_CHAR_W 6
#define BASE_CHAR_H 8

// Screen layout
uint16_t screenW = 320;
uint16_t screenH = 480;
uint16_t dividerY;
uint16_t topMaxY;
uint16_t bottomMinY;
uint16_t bottomMaxY;

// Top screen (commands)
uint16_t topPosX = 0;
uint16_t topPosY = 0;
uint16_t topTextColor = 0xFFFF;  // White
uint16_t topBgColor = 0x0000;    // Black
bool topOpaqueText = false;
uint8_t topTextSize = 2;

// Bottom screen (FPGA data)
uint16_t bottomPosX = 0;
uint16_t bottomPosY = 0;
uint16_t bottomTextColor = 0x07E0;  // Green
uint16_t bottomBgColor = 0x0000;    // Black
bool bottomOpaqueText = false;
uint8_t bottomTextSize = 1;

// Command handling
String cmd = "";
bool cmdReady = false;
String fpgaBuffer = "";

// FPGA settings
uint8_t activeFpga = 1;  // 1, 2, or 3
unsigned long fpga1Baud = 9600;
unsigned long fpga2Baud = 9600;
unsigned long fpga3Baud = 9600;
uint8_t fpga1StopBits = 1;  // 1 or 2 stop bits
uint8_t fpga2StopBits = 1;
uint8_t fpga3StopBits = 1;

// FPGA reception modes
enum FPGARxMode {
  FPGA_RX_TEXT,    // Parse as text (printable chars, line endings)
  FPGA_RX_BINARY   // Raw bytes displayed as hex
};

FPGARxMode fpga1RxMode = FPGA_RX_TEXT;
FPGARxMode fpga2RxMode = FPGA_RX_TEXT;
FPGARxMode fpga3RxMode = FPGA_RX_TEXT;

// FPGA response parsing modes (for displaying binary data as formatted text)
enum FPGAParseMode {
  PARSE_NONE,      // No special parsing (default hex display)
  PARSE_ASCII,     // Show as ASCII characters with [XX] for non-printable
  PARSE_DEC,       // Show as decimal numbers
  PARSE_MIXED      // Show both hex and ASCII: "0x41 'A'"
};

FPGAParseMode fpga1ParseMode = PARSE_NONE;
FPGAParseMode fpga2ParseMode = PARSE_NONE;
FPGAParseMode fpga3ParseMode = PARSE_NONE;

// Termination character modes
enum TermMode {
  TERM_NONE,   // No termination
  TERM_LF,     // \n (0x0A)
  TERM_CR,     // \r (0x0D)
  TERM_CRLF,   // \r\n (0x0D 0x0A)
  TERM_CUSTOM  // Custom byte value
};

TermMode bypassTerm = TERM_LF;  // Default for >>> prefix
uint8_t customTermByte = 0xFF;  // Custom termination byte (default 0xFF)

// Button structure for touch interface
struct Button {
  uint16_t x, y, w, h;
  const char* label;
  uint8_t cmdBytes[15];   // Bytes to send to FPGA (up to 15 bytes)
  uint8_t cmdLen;         // Number of bytes in command
  uint16_t color;
};

// Button layout - 4 direction buttons at bottom
#define NUM_BUTTONS 4
Button buttons[NUM_BUTTONS];
bool buttonsVisible = false;
uint16_t buttonTextY;  // Y position where button area starts

// Touch debounce
unsigned long lastTouch = 0;
#define TOUCH_DEBOUNCE 200  // milliseconds

// ========== GPIO SYSTEM ==========
// GPIO pins available on Mega side connector (pins 22-29)
// These are free and not used by LCD/Touch/Serial
#define NUM_GPIO 8
const uint8_t gpioPins[NUM_GPIO] = {22, 23, 24, 25, 26, 27, 28, 29};

// GPIO modes
enum GPIOMode {
  GPIO_INPUT,
  GPIO_INPUT_PULLUP,
  GPIO_OUTPUT
};

// GPIO state tracking
GPIOMode gpioModes[NUM_GPIO];
uint8_t gpioStates[NUM_GPIO];      // Current state (for inputs) or output value
uint8_t gpioPrevStates[NUM_GPIO];  // Previous state for change detection
bool gpioEventEnable[NUM_GPIO];    // Event reporting enabled per pin
bool gpioEventRising[NUM_GPIO];    // Report rising edge
bool gpioEventFalling[NUM_GPIO];   // Report falling edge

// GPIO event queue (simple ring buffer)
#define GPIO_EVENT_QUEUE_SIZE 16
struct GPIOEvent {
  uint8_t pin;
  uint8_t newState;
  bool rising;  // true=rising, false=falling
};
GPIOEvent gpioEventQueue[GPIO_EVENT_QUEUE_SIZE];
uint8_t gpioEventHead = 0;
uint8_t gpioEventTail = 0;

// SPI configuration
int8_t spiCSPin = -1;             // Chip select pin (-1 = not initialized)
uint32_t spiSpeed = 1000000;      // Default 1 MHz
uint8_t spiMode = SPI_MODE0;      // Default mode 0

// Analog input configuration
const uint8_t analogPins[8] = {A8, A9, A10, A11, A12, A13, A14, A15};
uint8_t analogAvgSamples = 1;     // Number of samples to average (1-16)

// Analog reference mode tracking (using our own enum to avoid conflicts)
enum AnalogRefMode {
  AREF_DEFAULT = 0,
  AREF_INTERNAL = 1
};
AnalogRefMode analogRefMode = AREF_DEFAULT;  // Default to 5V reference

// ========== DATA LOGGING SYSTEM ==========
// Data logging to EEPROM with circular buffer
// Default EEPROM layout: [0-99] = config/reserved, [1000-4095] = log data
// Configurable and write-protected
uint16_t logStartAddr = 2000;    // Configurable log start address (default: 2000-4095)
uint16_t logEndAddr = 4095;      // Configurable log end address
#define LOG_ENTRY_SIZE 8  // Timestamp(4) + Value(2) + Source(1) + Flags(1)

bool loggingActive = false;
unsigned long logInterval = 1000;  // milliseconds
unsigned long lastLogTime = 0;
uint16_t logWriteAddr = 2000;
uint16_t logEntryCount = 0;
uint8_t logSource = 0;  // 0=A8, 1=A9, ... 15=GPIO_REG

// EEPROM Write Protection Zones (up to 4 protected ranges)
#define MAX_PROTECT_ZONES 4
struct ProtectZone {
  uint16_t startAddr;
  uint16_t endAddr;
  bool enabled;
};

ProtectZone protectZones[MAX_PROTECT_ZONES] = {
  {0, 99, true},      // Zone 0: Config area (0-99) - protected by default
  {100, 299, true},   // Zone 1: FPGA dedicated space (200 bytes)
  {300, 499, true},   // Zone 2: User settings (200 bytes)
  {0, 0, false}       // Zone 3: User-defined
};

// Check if address is write-protected
bool isProtected(uint16_t addr) {
  for (uint8_t i = 0; i < MAX_PROTECT_ZONES; i++) {
    if (protectZones[i].enabled) {
      if (addr >= protectZones[i].startAddr && addr <= protectZones[i].endAddr) {
        return true;
      }
    }
  }
  return false;
}

// ========== CONFIGURATION SYSTEM ==========
// EEPROM Memory Map:
// 0x00-0x63 (0-99):     Configuration zone (protected)
// 0x64-0x12B (100-299): FPGA zone (200 bytes, protected)
// 0x12C-0x1F3 (300-499): User zone (200 bytes, protected)
// 0x1F4-0x7CF (500-1999): Free space (1500 bytes)
// 0x7D0-0xFFF (2000-4095): Data logging zone (3096 bytes)

#define CONFIG_MAGIC 0xA5
#define CONFIG_VERSION 1

// EEPROM addresses for configuration
#define ADDR_MAGIC 0
#define ADDR_VERSION 1
#define ADDR_ROTATION 2
#define ADDR_FPGA_SEL 3
#define ADDR_FPGA1_BAUD 4         // 3 bytes (4,5,6)
#define ADDR_FPGA2_BAUD 7         // 3 bytes (7,8,9)
#define ADDR_FPGA3_BAUD 10        // 3 bytes (10,11,12)
#define ADDR_FPGA1_TERM 13
#define ADDR_FPGA2_TERM 14
#define ADDR_FPGA3_TERM 15
#define ADDR_FPGA1_STOP 16
#define ADDR_FPGA2_STOP 17
#define ADDR_FPGA3_STOP 18
#define ADDR_FPGA1_RX 19
#define ADDR_FPGA2_RX 20
#define ADDR_FPGA3_RX 21
#define ADDR_FPGA1_PARSE 22
#define ADDR_FPGA2_PARSE 23
#define ADDR_FPGA3_PARSE 24
// 25-31: Reserved
#define ADDR_BTN_UP 32      // 16 bytes: [0]=length, [1-15]=data
#define ADDR_BTN_DOWN 48    // 16 bytes
#define ADDR_BTN_LEFT 64    // 16 bytes
#define ADDR_BTN_RIGHT 80   // 16 bytes
// 96-99: Reserved

#define ADDR_FPGA_ZONE 100    // FPGA dedicated space (200 bytes)
#define ADDR_USER_ZONE 300    // User settings (200 bytes)

// Button configuration structure
struct ButtonConfig {
  uint8_t length;      // Number of bytes to send (0-15)
  uint8_t data[15];    // Button data bytes
};

ButtonConfig btnUp    = {2, {0x55, 0x50}};  // Default: "UP"
ButtonConfig btnDown  = {2, {0x44, 0x4E}};  // Default: "DN"
ButtonConfig btnLeft  = {2, {0x4C, 0x54}};  // Default: "LT"
ButtonConfig btnRight = {2, {0x52, 0x54}};  // Default: "RT"

// Save configuration to EEPROM
void saveConfig() {
  // Magic byte and version
  EEPROM.write(ADDR_MAGIC, CONFIG_MAGIC);
  EEPROM.write(ADDR_VERSION, CONFIG_VERSION);

  // Display settings
  EEPROM.write(ADDR_ROTATION, tft.getRotation());

  // FPGA settings
  EEPROM.write(ADDR_FPGA_SEL, activeFpga);

  // Store baud rates as 3-byte values (max 16777215)
  EEPROM.write(ADDR_FPGA1_BAUD, (fpga1Baud >> 16) & 0xFF);
  EEPROM.write(ADDR_FPGA1_BAUD + 1, (fpga1Baud >> 8) & 0xFF);
  EEPROM.write(ADDR_FPGA1_BAUD + 2, fpga1Baud & 0xFF);

  EEPROM.write(ADDR_FPGA2_BAUD, (fpga2Baud >> 16) & 0xFF);
  EEPROM.write(ADDR_FPGA2_BAUD + 1, (fpga2Baud >> 8) & 0xFF);
  EEPROM.write(ADDR_FPGA2_BAUD + 2, fpga2Baud & 0xFF);

  EEPROM.write(ADDR_FPGA3_BAUD, (fpga3Baud >> 16) & 0xFF);
  EEPROM.write(ADDR_FPGA3_BAUD + 1, (fpga3Baud >> 8) & 0xFF);
  EEPROM.write(ADDR_FPGA3_BAUD + 2, fpga3Baud & 0xFF);

  EEPROM.write(ADDR_FPGA1_TERM, (uint8_t)bypassTerm);  // Store term mode
  EEPROM.write(ADDR_FPGA2_TERM, customTermByte);  // Store custom term byte
  EEPROM.write(ADDR_FPGA3_TERM, 0);  // Reserved

  EEPROM.write(ADDR_FPGA1_STOP, fpga1StopBits);
  EEPROM.write(ADDR_FPGA2_STOP, fpga2StopBits);
  EEPROM.write(ADDR_FPGA3_STOP, fpga3StopBits);

  EEPROM.write(ADDR_FPGA1_RX, (uint8_t)fpga1RxMode);
  EEPROM.write(ADDR_FPGA2_RX, (uint8_t)fpga2RxMode);
  EEPROM.write(ADDR_FPGA3_RX, (uint8_t)fpga3RxMode);

  EEPROM.write(ADDR_FPGA1_PARSE, (uint8_t)fpga1ParseMode);
  EEPROM.write(ADDR_FPGA2_PARSE, (uint8_t)fpga2ParseMode);
  EEPROM.write(ADDR_FPGA3_PARSE, (uint8_t)fpga3ParseMode);

  // Button configurations
  EEPROM.write(ADDR_BTN_UP, btnUp.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_UP + 1 + i, btnUp.data[i]);
  }

  EEPROM.write(ADDR_BTN_DOWN, btnDown.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_DOWN + 1 + i, btnDown.data[i]);
  }

  EEPROM.write(ADDR_BTN_LEFT, btnLeft.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_LEFT + 1 + i, btnLeft.data[i]);
  }

  EEPROM.write(ADDR_BTN_RIGHT, btnRight.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_RIGHT + 1 + i, btnRight.data[i]);
  }
}

// Load configuration from EEPROM
bool loadConfig() {
  // Check magic byte
  if (EEPROM.read(ADDR_MAGIC) != CONFIG_MAGIC) {
    return false;  // No valid config
  }

  // Check version
  uint8_t ver = EEPROM.read(ADDR_VERSION);
  if (ver != CONFIG_VERSION) {
    return false;  // Version mismatch
  }

  // Load display settings (will be applied after tft.begin)
  uint8_t rot = EEPROM.read(ADDR_ROTATION);
  if (rot > 3) rot = 0;
  // Note: rotation applied in setup() after tft.begin()

  // Load FPGA settings
  activeFpga = EEPROM.read(ADDR_FPGA_SEL);
  if (activeFpga < 1 || activeFpga > 3) activeFpga = 1;

  // Load baud rates (3 bytes each)
  fpga1Baud = ((unsigned long)EEPROM.read(ADDR_FPGA1_BAUD) << 16) |
              ((unsigned long)EEPROM.read(ADDR_FPGA1_BAUD + 1) << 8) |
              EEPROM.read(ADDR_FPGA1_BAUD + 2);
  if (fpga1Baud == 0 || fpga1Baud > 115200) fpga1Baud = 9600;

  fpga2Baud = ((unsigned long)EEPROM.read(ADDR_FPGA2_BAUD) << 16) |
              ((unsigned long)EEPROM.read(ADDR_FPGA2_BAUD + 1) << 8) |
              EEPROM.read(ADDR_FPGA2_BAUD + 2);
  if (fpga2Baud == 0 || fpga2Baud > 115200) fpga2Baud = 9600;

  fpga3Baud = ((unsigned long)EEPROM.read(ADDR_FPGA3_BAUD) << 16) |
              ((unsigned long)EEPROM.read(ADDR_FPGA3_BAUD + 1) << 8) |
              EEPROM.read(ADDR_FPGA3_BAUD + 2);
  if (fpga3Baud == 0 || fpga3Baud > 115200) fpga3Baud = 9600;

  // Load termination settings
  bypassTerm = (TermMode)EEPROM.read(ADDR_FPGA1_TERM);
  customTermByte = EEPROM.read(ADDR_FPGA2_TERM);

  // Load stop bits
  fpga1StopBits = EEPROM.read(ADDR_FPGA1_STOP);
  fpga2StopBits = EEPROM.read(ADDR_FPGA2_STOP);
  fpga3StopBits = EEPROM.read(ADDR_FPGA3_STOP);
  if (fpga1StopBits < 1 || fpga1StopBits > 2) fpga1StopBits = 1;
  if (fpga2StopBits < 1 || fpga2StopBits > 2) fpga2StopBits = 1;
  if (fpga3StopBits < 1 || fpga3StopBits > 2) fpga3StopBits = 1;

  // Load RX modes
  fpga1RxMode = (FPGARxMode)EEPROM.read(ADDR_FPGA1_RX);
  fpga2RxMode = (FPGARxMode)EEPROM.read(ADDR_FPGA2_RX);
  fpga3RxMode = (FPGARxMode)EEPROM.read(ADDR_FPGA3_RX);

  // Load parse modes
  fpga1ParseMode = (FPGAParseMode)EEPROM.read(ADDR_FPGA1_PARSE);
  fpga2ParseMode = (FPGAParseMode)EEPROM.read(ADDR_FPGA2_PARSE);
  fpga3ParseMode = (FPGAParseMode)EEPROM.read(ADDR_FPGA3_PARSE);

  // Load button configurations
  btnUp.length = EEPROM.read(ADDR_BTN_UP);
  if (btnUp.length > 15) btnUp.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnUp.data[i] = EEPROM.read(ADDR_BTN_UP + 1 + i);
  }

  btnDown.length = EEPROM.read(ADDR_BTN_DOWN);
  if (btnDown.length > 15) btnDown.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnDown.data[i] = EEPROM.read(ADDR_BTN_DOWN + 1 + i);
  }

  btnLeft.length = EEPROM.read(ADDR_BTN_LEFT);
  if (btnLeft.length > 15) btnLeft.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnLeft.data[i] = EEPROM.read(ADDR_BTN_LEFT + 1 + i);
  }

  btnRight.length = EEPROM.read(ADDR_BTN_RIGHT);
  if (btnRight.length > 15) btnRight.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnRight.data[i] = EEPROM.read(ADDR_BTN_RIGHT + 1 + i);
  }

  return true;
}

// ========== WAVEFORM GENERATOR ==========
// PWM-based waveform generation
int8_t wavePin = -1;          // Pin for wave output (-1 = disabled)
uint8_t waveType = 0;         // 0=square, 1=triangle, 2=sine
uint16_t waveFreq = 1000;     // Hz
uint8_t waveDuty = 128;       // 0-255 for PWM
bool waveActive = false;
unsigned long waveLastUpdate = 0;
uint8_t wavePhase = 0;

// Sine lookup table (32 values for memory efficiency)
const uint8_t PROGMEM sineLUT[32] = {
  128, 152, 176, 198, 217, 233, 245, 253,
  255, 253, 245, 233, 217, 198, 176, 152,
  128, 103,  79,  57,  38,  22,  10,   2,
    0,   2,  10,  22,  38,  57,  79, 103
};

// ========== PULSE/FREQUENCY MEASUREMENT ==========
// Continuous frequency monitoring
int8_t freqMonPin = -1;           // Pin for frequency monitoring (-1 = disabled)
unsigned long freqMonDuration = 1000;  // Measurement window (ms)
unsigned long freqMonLastUpdate = 0;
volatile unsigned long pulseCount = 0;
bool freqMonActive = false;

// Interrupt handler for pulse counting
void pulseISR() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);      // USB/PC

  // Load configuration from EEPROM if available
  uint8_t savedRotation = 0;
  bool configLoaded = loadConfig();
  if (configLoaded) {
    Serial.println(F("[CONFIG] Loaded from EEPROM"));
    savedRotation = EEPROM.read(ADDR_ROTATION);  // Get saved rotation
    if (savedRotation > 3) savedRotation = 0;
  } else {
    Serial.println(F("[CONFIG] Using defaults (no saved config)"));
  }

  // Apply FPGA baud rates from config
  Serial1.begin(fpga1Baud);  // FPGA1
  Serial2.begin(fpga2Baud);  // FPGA2
  Serial3.begin(fpga3Baud);  // FPGA3

  uint16_t ID = tft.readID();
  Serial.print(F("LCD ID: 0x"));
  Serial.println(ID, HEX);

  tft.begin(ID);
  tft.setRotation(savedRotation);  // Apply rotation from config
  screenW = tft.width();
  screenH = tft.height();

  // Split screen 50/50
  dividerY = screenH / 2;
  topMaxY = dividerY - 2;
  bottomMinY = dividerY + 2;
  bottomMaxY = screenH;

  tft.fillScreen(0);
  drawDivider();
  topPosY = 0;
  bottomPosY = bottomMinY;

  cmd.reserve(300);       // Mega has plenty of RAM
  fpgaBuffer.reserve(200);

  // Initialize touch buttons
  initButtons();

  // Initialize GPIO system
  initGPIO();

  // Initialize I2C (Wire library)
  Wire.begin();

  // Startup message
  tft.setTextColor(0x07FF);  // Cyan
  tft.setTextSize(2);
  tft.println(F("MEGA 2560"));
  tft.setTextColor(0x07E0);  // Green
  tft.println(F("ILI9486/9488"));
  tft.setTextColor(0xFFE0);  // Yellow
  tft.println(F("Split Screen"));
  tft.setTextColor(0xFFFF);  // White
  tft.println(F("Text-Based"));
  tft.println(F("Touch Buttons"));
  tft.println();

  topPosY = tft.getCursorY();

  Serial.println(F("=== MEGA Split Screen ==="));
  Serial.println(F("Text-Based Commands"));
  Serial.print(F("Resolution: "));
  Serial.print(screenW);
  Serial.print(F("x"));
  Serial.println(screenH);
  Serial.println(F("Type #HELP for commands"));
  Serial.println(F("#SHOWBTNS for touch buttons"));
  Serial.println(F("READY"));
}

void loop() {
  // Process touch input
  if (buttonsVisible) {
    checkTouch();
  }

  // Poll GPIO for changes (event-driven)
  pollGPIO();

  // Process GPIO events
  processGPIOEvents();

  // Handle data logging
  if (loggingActive) {
    updateDataLogger();
  }

  // Handle waveform generation
  if (waveActive) {
    updateWaveform();
  }

  // Handle frequency monitoring
  if (freqMonActive) {
    updateFreqMonitor();
  }

  if (cmdReady) {
    processCmd(cmd);
    cmd = "";
    cmdReady = false;
  }

  // Check all FPGA serials
  checkFPGASerial(Serial1, 1);
  checkFPGASerial(Serial2, 2);
  checkFPGASerial(Serial3, 3);
}

void checkFPGASerial(HardwareSerial& serial, uint8_t id) {
  // Get RX mode and parse mode for this serial port
  FPGARxMode rxMode;
  FPGAParseMode parseMode;
  switch (id) {
    case 1: rxMode = fpga1RxMode; parseMode = fpga1ParseMode; break;
    case 2: rxMode = fpga2RxMode; parseMode = fpga2ParseMode; break;
    case 3: rxMode = fpga3RxMode; parseMode = fpga3ParseMode; break;
    default: rxMode = FPGA_RX_TEXT; parseMode = PARSE_NONE; break;
  }

  while (serial.available()) {
    uint8_t byteVal = serial.read();

    if (rxMode == FPGA_RX_BINARY) {
      // Binary mode: display based on parse mode
      String byteStr = "";

      switch (parseMode) {
        case PARSE_ASCII:
          // Show as ASCII with [XX] for non-printable
          if (isPrintable(byteVal)) {
            byteStr = String((char)byteVal);
            Serial.write(byteVal);
          } else {
            byteStr = "[";
            if (byteVal < 16) byteStr += "0";
            byteStr += String(byteVal, HEX);
            byteStr += "]";
            Serial.print(byteStr);
          }
          break;

        case PARSE_DEC:
          // Show as decimal
          byteStr = String(byteVal);
          Serial.print(byteVal);
          Serial.print(F(" "));
          break;

        case PARSE_MIXED:
          // Show both hex and ASCII: "0x41='A'"
          byteStr = "0x";
          if (byteVal < 16) byteStr += "0";
          byteStr += String(byteVal, HEX);
          if (isPrintable(byteVal)) {
            byteStr += "='";
            byteStr += (char)byteVal;
            byteStr += "'";
          }
          Serial.print(byteStr);
          Serial.print(F(" "));
          break;

        case PARSE_NONE:
        default:
          // Default hex display
          byteStr = "0x";
          if (byteVal < 16) byteStr += "0";
          byteStr += String(byteVal, HEX);
          Serial.print(F("0x"));
          if (byteVal < 16) Serial.print(F("0"));
          Serial.print(byteVal, HEX);
          Serial.print(F(" "));
          break;
      }

      // Add to buffer for LCD display
      if (fpgaBuffer.length() < 160) {
        if (fpgaBuffer.length() > 0 && parseMode != PARSE_ASCII) {
          fpgaBuffer += " ";
        }
        fpgaBuffer += byteStr;
      } else {
        // Buffer full, display and start new line
        showTextBottom(fpgaBuffer);
        fpgaBuffer = byteStr;
      }

      // Display every 10 bytes or on pause
      static uint8_t byteCount = 0;
      byteCount++;
      if (byteCount >= 10) {
        if (fpgaBuffer.length() > 0) {
          showTextBottom(fpgaBuffer);
          fpgaBuffer = "";
        }
        byteCount = 0;
      }

    } else {
      // Text mode: parse as characters
      char c = (char)byteVal;
      Serial.write(c);  // Echo to PC

      if (c == '\n' || c == '\r') {
        if (fpgaBuffer.length() > 0) {
          showTextBottom(fpgaBuffer);
          fpgaBuffer = "";
        }
      } else if (isPrintable(c)) {
        if (fpgaBuffer.length() < 180) {
          fpgaBuffer += c;
        } else {
          showTextBottom(fpgaBuffer);
          fpgaBuffer = "";
          fpgaBuffer += c;
        }
      }
    }
  }
}

void serialEvent() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      cmdReady = true;
    } else if (c != '\r') {
      if (cmd.length() < 290) {
        cmd += c;
      }
    }
  }
}

void drawDivider() {
  tft.drawLine(0, dividerY, screenW, dividerY, 0xFFFF);
}

uint16_t getLines(const String& txt) {
  uint16_t len = txt.length();
  if (len == 0) return 1;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * topTextSize);
  if (charsPerLine == 0) return 1;
  return (len + charsPerLine - 1) / charsPerLine;
}

uint16_t getLinesBottom(const String& txt) {
  uint16_t len = txt.length();
  if (len == 0) return 1;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * bottomTextSize);
  if (charsPerLine == 0) return 1;
  return (len + charsPerLine - 1) / charsPerLine;
}

void showText(const String& txt) {
  uint16_t lines = getLines(txt);
  uint16_t lineH = BASE_CHAR_H * topTextSize;
  uint16_t needH = lines * lineH;

  if (topPosY + needH > topMaxY) {
    tft.fillRect(0, 0, screenW, topMaxY, 0x0000);
    topPosY = 0;
  }

  tft.setTextSize(topTextSize);
  if (topOpaqueText) {
    tft.setTextColor(topTextColor, topBgColor);
  } else {
    tft.setTextColor(topTextColor);
  }
  tft.setCursor(topPosX, topPosY);

  uint16_t charsPerLine = screenW / (BASE_CHAR_W * topTextSize);
  if (charsPerLine == 0) charsPerLine = 1;

  if (txt.length() > charsPerLine) {
    int start = 0;
    while (start < txt.length()) {
      int end = min(start + charsPerLine, (int)txt.length());
      tft.print(txt.substring(start, end));
      start = end;
      if (start < txt.length()) {
        topPosY += lineH;
        if (topPosY >= topMaxY) break;
        tft.setCursor(0, topPosY);
      }
    }
    tft.println();
  } else {
    tft.println(txt);
  }

  topPosY = tft.getCursorY();
  if (topPosY >= topMaxY) topPosY = 0;
  topPosX = 0;
}

void showTextBottom(const String& txt) {
  uint16_t lines = getLinesBottom(txt);
  uint16_t lineH = BASE_CHAR_H * bottomTextSize;
  uint16_t needH = lines * lineH;

  if (bottomPosY + needH > bottomMaxY) {
    tft.fillRect(0, bottomMinY, screenW, bottomMaxY - bottomMinY, bottomBgColor);
    bottomPosY = bottomMinY;
  }

  tft.setTextSize(bottomTextSize);
  if (bottomOpaqueText) {
    tft.setTextColor(bottomTextColor, bottomBgColor);
  } else {
    tft.setTextColor(bottomTextColor);
  }
  tft.setCursor(bottomPosX, bottomPosY);

  uint16_t charsPerLine = screenW / (BASE_CHAR_W * bottomTextSize);
  if (charsPerLine == 0) charsPerLine = 1;

  if (txt.length() > charsPerLine) {
    int start = 0;
    while (start < txt.length()) {
      int end = min(start + charsPerLine, (int)txt.length());
      tft.print(txt.substring(start, end));
      start = end;
      if (start < txt.length()) {
        bottomPosY += lineH;
        if (bottomPosY >= bottomMaxY) break;
        tft.setCursor(0, bottomPosY);
      }
    }
    tft.println();
  } else {
    tft.println(txt);
  }

  bottomPosY = tft.getCursorY();
  if (bottomPosY >= bottomMaxY) bottomPosY = bottomMinY;
  bottomPosX = 0;
}

HardwareSerial& getFPGA() {
  switch (activeFpga) {
    case 2: return Serial2;
    case 3: return Serial3;
    default: return Serial1;
  }
}

// Apply termination character to serial stream
void applyTermination(HardwareSerial& serial) {
  switch (bypassTerm) {
    case TERM_LF:
      serial.write(0x0A);  // \n
      break;
    case TERM_CR:
      serial.write(0x0D);  // \r
      break;
    case TERM_CRLF:
      serial.write(0x0D);  // \r
      serial.write(0x0A);  // \n
      break;
    case TERM_CUSTOM:
      serial.write(customTermByte);  // Custom stop byte
      break;
    case TERM_NONE:
      // No termination
      break;
  }
}

// Read response bytes from FPGA with timeout
void readFPGAResponse(uint8_t numBytes, uint16_t timeout) {
  HardwareSerial& fpga = getFPGA();
  uint8_t buffer[64];
  uint8_t bytesRead = 0;
  unsigned long startTime = millis();

  // Get RX mode
  FPGARxMode rxMode;
  switch (activeFpga) {
    case 1: rxMode = fpga1RxMode; break;
    case 2: rxMode = fpga2RxMode; break;
    case 3: rxMode = fpga3RxMode; break;
    default: rxMode = FPGA_RX_TEXT; break;
  }

  Serial.print(F("[FPGA"));
  Serial.print(activeFpga);
  Serial.print(F(" READ "));
  Serial.print(numBytes);
  Serial.print(F(" bytes] "));

  // Read bytes with timeout
  while (bytesRead < numBytes && (millis() - startTime) < timeout) {
    if (fpga.available()) {
      buffer[bytesRead++] = fpga.read();
    }
  }

  if (bytesRead == 0) {
    Serial.println(F("TIMEOUT"));
    showTextBottom("FPGA: TIMEOUT");
    return;
  }

  // Display based on RX mode
  if (rxMode == FPGA_RX_BINARY) {
    // Binary mode: show as hex
    String hexStr = "";
    for (uint8_t i = 0; i < bytesRead; i++) {
      if (i > 0) {
        Serial.print(F(" "));
        hexStr += " ";
      }
      Serial.print(F("0x"));
      if (buffer[i] < 16) Serial.print(F("0"));
      Serial.print(buffer[i], HEX);

      hexStr += "0x";
      if (buffer[i] < 16) hexStr += "0";
      hexStr += String(buffer[i], HEX);
    }
    Serial.println();
    showTextBottom(hexStr);

  } else {
    // Text mode: show as characters
    String textStr = "";
    for (uint8_t i = 0; i < bytesRead; i++) {
      char c = (char)buffer[i];
      Serial.write(c);
      if (isPrintable(c)) {
        textStr += c;
      } else if (c == '\n' || c == '\r') {
        // Skip line endings in display
      } else {
        // Non-printable: show as hex
        textStr += "[0x";
        if (buffer[i] < 16) textStr += "0";
        textStr += String(buffer[i], HEX);
        textStr += "]";
      }
    }
    Serial.println();
    showTextBottom(textStr);
  }

  Serial.print(F("("));
  Serial.print(bytesRead);
  Serial.print(F("/"));
  Serial.print(numBytes);
  Serial.println(F(" bytes)"));
}

// Set UART hardware stop bits (1 or 2) - ATmega2560 specific
// Directly manipulates UCSRnC registers
void setSerialStopBits(uint8_t serialNum, uint8_t stopBits) {
  if (stopBits != 1 && stopBits != 2) return;

  // USBS bit (bit 3) in UCSRnC register
  // 0 = 1 stop bit, 1 = 2 stop bits
  bool twoStopBits = (stopBits == 2);

  switch (serialNum) {
    case 1:  // Serial1 - UCSR1C
      if (twoStopBits) {
        UCSR1C |= (1 << USBS1);   // Set bit 3: 2 stop bits
      } else {
        UCSR1C &= ~(1 << USBS1);  // Clear bit 3: 1 stop bit
      }
      fpga1StopBits = stopBits;
      break;

    case 2:  // Serial2 - UCSR2C
      if (twoStopBits) {
        UCSR2C |= (1 << USBS2);   // Set bit 3: 2 stop bits
      } else {
        UCSR2C &= ~(1 << USBS2);  // Clear bit 3: 1 stop bit
      }
      fpga2StopBits = stopBits;
      break;

    case 3:  // Serial3 - UCSR3C
      if (twoStopBits) {
        UCSR3C |= (1 << USBS3);   // Set bit 3: 2 stop bits
      } else {
        UCSR3C &= ~(1 << USBS3);  // Clear bit 3: 1 stop bit
      }
      fpga3StopBits = stopBits;
      break;
  }
}

void processCmd(String c) {
  c.trim();
  if (c.length() == 0) return;

  // FPGA passthrough with termination control
  if (c.startsWith(">>>")) {
    String data = c.substring(3);
    data.trim();
    HardwareSerial& fpga = getFPGA();

    // Send data
    fpga.print(data);

    // Apply termination
    applyTermination(fpga);

    Serial.print(F("OK:TX_S"));
    Serial.println(activeFpga);
    return;
  }

  if (c.startsWith("#")) {
    c.toUpperCase();

    if (c == "#CLEAR" || c == "#CLR") {
      tft.fillRect(0, 0, screenW, topMaxY, 0x0000);
      topPosX = topPosY = 0;
      drawDivider();
      Serial.println(F("OK:CLR"));

    } else if (c.startsWith("#COLOR ")) {
      String col = c.substring(7);
      setCol(col);

    } else if (c.startsWith("#BGCOLOR ")) {
      String col = c.substring(9);
      setBgCol(col);

    } else if (c.startsWith("#SIZE ")) {
      int s = c.substring(6).toInt();
      if (s >= 1 && s <= 10) {  // Mega can handle larger
        topTextSize = s;
        tft.setTextSize(s);
        Serial.print(F("OK:SIZE_"));
        Serial.println(s);
      }

    } else if (c.startsWith("#POS ")) {
      int sp = c.indexOf(' ', 5);
      if (sp > 0) {
        topPosX = c.substring(5, sp).toInt();
        topPosY = c.substring(sp + 1).toInt();
        tft.setCursor(topPosX, topPosY);
        Serial.println(F("OK:POS"));
      }

    } else if (c.startsWith("#RECT ")) {
      parseRect(c.substring(6));

    } else if (c.startsWith("#CIRCLE ")) {
      parseCirc(c.substring(8));

    } else if (c.startsWith("#LINE ")) {
      parseLine(c.substring(6));

    } else if (c.startsWith("#FILL ")) {
      parseFill(c.substring(6));

    } else if (c.startsWith("#PROG ")) {
      parseProg(c.substring(6));

    } else if (c.startsWith("#ROT")) {
      int r = c.substring(4).toInt();
      if (r >= 0 && r <= 3) {
        tft.setRotation(r);
        screenW = tft.width();
        screenH = tft.height();
        dividerY = screenH / 2;
        topMaxY = dividerY - 2;
        bottomMinY = dividerY + 2;
        bottomMaxY = screenH;
        tft.fillScreen(0);
        topPosX = topPosY = 0;
        bottomPosX = 0;
        bottomPosY = bottomMinY;
        drawDivider();

        // Recalculate button positions and redraw if visible
        if (buttonsVisible) {
          initButtons();  // Recalculate button positions for new rotation
          for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
            drawButton(i);
          }
        }

        Serial.print(F("OK:ROT_"));
        Serial.println(r);
      }

    } else if (c == "#CLRBOT") {
      tft.fillRect(0, bottomMinY, screenW, bottomMaxY - bottomMinY, bottomBgColor);
      bottomPosX = 0;
      bottomPosY = bottomMinY;
      drawDivider();
      Serial.println(F("OK:CLRBOT"));

    } else if (c == "#CLRALL") {
      tft.fillScreen(0);
      topPosX = topPosY = 0;
      bottomPosX = 0;
      bottomPosY = bottomMinY;
      drawDivider();
      Serial.println(F("OK:CLRALL"));

    } else if (c.startsWith("#BOTSIZE ")) {
      int s = c.substring(9).toInt();
      if (s >= 1 && s <= 5) {
        bottomTextSize = s;
        Serial.print(F("OK:BOTSIZE_"));
        Serial.println(s);
      }

    } else if (c.startsWith("#BOTCOLOR ")) {
      String col = c.substring(10);
      setBotCol(col);

    } else if (c == "#STATUS") {
      // Show all current settings
      Serial.println(F("=== STATUS ==="));
      Serial.print(F("Active FPGA: "));
      Serial.println(activeFpga);

      Serial.print(F("FPGA1: "));
      Serial.print(fpga1Baud);
      Serial.print(F(" baud, "));
      Serial.print(fpga1StopBits);
      Serial.print(F(" stop, "));
      Serial.print(fpga1RxMode == FPGA_RX_TEXT ? F("TEXT") : F("BINARY"));
      Serial.print(F(", Parse="));
      switch (fpga1ParseMode) {
        case PARSE_NONE: Serial.println(F("NONE")); break;
        case PARSE_ASCII: Serial.println(F("ASCII")); break;
        case PARSE_DEC: Serial.println(F("DEC")); break;
        case PARSE_MIXED: Serial.println(F("MIXED")); break;
      }

      Serial.print(F("FPGA2: "));
      Serial.print(fpga2Baud);
      Serial.print(F(" baud, "));
      Serial.print(fpga2StopBits);
      Serial.print(F(" stop, "));
      Serial.print(fpga2RxMode == FPGA_RX_TEXT ? F("TEXT") : F("BINARY"));
      Serial.print(F(", Parse="));
      switch (fpga2ParseMode) {
        case PARSE_NONE: Serial.println(F("NONE")); break;
        case PARSE_ASCII: Serial.println(F("ASCII")); break;
        case PARSE_DEC: Serial.println(F("DEC")); break;
        case PARSE_MIXED: Serial.println(F("MIXED")); break;
      }

      Serial.print(F("FPGA3: "));
      Serial.print(fpga3Baud);
      Serial.print(F(" baud, "));
      Serial.print(fpga3StopBits);
      Serial.print(F(" stop, "));
      Serial.print(fpga3RxMode == FPGA_RX_TEXT ? F("TEXT") : F("BINARY"));
      Serial.print(F(", Parse="));
      switch (fpga3ParseMode) {
        case PARSE_NONE: Serial.println(F("NONE")); break;
        case PARSE_ASCII: Serial.println(F("ASCII")); break;
        case PARSE_DEC: Serial.println(F("DEC")); break;
        case PARSE_MIXED: Serial.println(F("MIXED")); break;
      }

      Serial.print(F("Termination: "));
      switch (bypassTerm) {
        case TERM_NONE: Serial.println(F("NONE")); break;
        case TERM_LF: Serial.println(F("LF")); break;
        case TERM_CR: Serial.println(F("CR")); break;
        case TERM_CRLF: Serial.println(F("CRLF")); break;
        case TERM_CUSTOM:
          Serial.print(F("CUSTOM 0x"));
          if (customTermByte < 16) Serial.print(F("0"));
          Serial.println(customTermByte, HEX);
          break;
      }
      Serial.print(F("Rotation: "));
      Serial.println(tft.getRotation());
      Serial.print(F("Buttons: "));
      Serial.println(buttonsVisible ? F("VISIBLE") : F("HIDDEN"));
      Serial.println(F("============="));

    } else if (c.startsWith("#FPGASEL")) {
      String param = c.substring(8);
      param.trim();
      if (param == "?") {
        // Query current selection
        Serial.print(F("FPGASEL="));
        Serial.println(activeFpga);
      } else {
        uint8_t sel = param.toInt();
        if (sel >= 1 && sel <= 3) {
          activeFpga = sel;
          Serial.print(F("OK:FPGA"));
          Serial.println(sel);
        } else {
          Serial.println(F("ERR:USE_1_2_3"));
        }
      }

    } else if (c.startsWith("#FPGA1BAUD")) {
      String param = c.substring(10);
      param.trim();
      if (param == "?") {
        // Query current baud rate
        Serial.print(F("FPGA1BAUD="));
        Serial.println(fpga1Baud);
      } else {
        unsigned long baud = param.toInt();
        if (baud >= 300 && baud <= 115200) {
          fpga1Baud = baud;
          Serial1.end();
          Serial1.begin(fpga1Baud);
          Serial.print(F("OK:FPGA1_"));
          Serial.println(baud);
        } else {
          Serial.println(F("ERR:BAUD_300_115200"));
        }
      }

    } else if (c.startsWith("#FPGA2BAUD")) {
      String param = c.substring(10);
      param.trim();
      if (param == "?") {
        // Query current baud rate
        Serial.print(F("FPGA2BAUD="));
        Serial.println(fpga2Baud);
      } else {
        unsigned long baud = param.toInt();
        if (baud >= 300 && baud <= 115200) {
          fpga2Baud = baud;
          Serial2.end();
          Serial2.begin(fpga2Baud);
          Serial.print(F("OK:FPGA2_"));
          Serial.println(baud);
        } else {
          Serial.println(F("ERR:BAUD_300_115200"));
        }
      }

    } else if (c.startsWith("#FPGA3BAUD")) {
      String param = c.substring(10);
      param.trim();
      if (param == "?") {
        // Query current baud rate
        Serial.print(F("FPGA3BAUD="));
        Serial.println(fpga3Baud);
      } else {
        unsigned long baud = param.toInt();
        if (baud >= 300 && baud <= 115200) {
          fpga3Baud = baud;
          Serial3.end();
          Serial3.begin(fpga3Baud);
          Serial.print(F("OK:FPGA3_"));
          Serial.println(baud);
        } else {
          Serial.println(F("ERR:BAUD_300_115200"));
        }
      }

    } else if (c.startsWith("#FPGA1STOP")) {
      String param = c.substring(10);
      param.trim();
      if (param == "?") {
        // Query current stop bits
        Serial.print(F("FPGA1STOP="));
        Serial.println(fpga1StopBits);
      } else {
        uint8_t stopBits = param.toInt();
        if (stopBits == 1 || stopBits == 2) {
          setSerialStopBits(1, stopBits);
          Serial.print(F("OK:FPGA1_STOP_"));
          Serial.println(stopBits);
        } else {
          Serial.println(F("ERR:USE_1_OR_2"));
        }
      }

    } else if (c.startsWith("#FPGA2STOP")) {
      String param = c.substring(10);
      param.trim();
      if (param == "?") {
        // Query current stop bits
        Serial.print(F("FPGA2STOP="));
        Serial.println(fpga2StopBits);
      } else {
        uint8_t stopBits = param.toInt();
        if (stopBits == 1 || stopBits == 2) {
          setSerialStopBits(2, stopBits);
          Serial.print(F("OK:FPGA2_STOP_"));
          Serial.println(stopBits);
        } else {
          Serial.println(F("ERR:USE_1_OR_2"));
        }
      }

    } else if (c.startsWith("#FPGA3STOP")) {
      String param = c.substring(10);
      param.trim();
      if (param == "?") {
        // Query current stop bits
        Serial.print(F("FPGA3STOP="));
        Serial.println(fpga3StopBits);
      } else {
        uint8_t stopBits = param.toInt();
        if (stopBits == 1 || stopBits == 2) {
          setSerialStopBits(3, stopBits);
          Serial.print(F("OK:FPGA3_STOP_"));
          Serial.println(stopBits);
        } else {
          Serial.println(F("ERR:USE_1_OR_2"));
        }
      }

    } else if (c.startsWith("#FPGA1RX")) {
      String param = c.substring(8);
      param.trim();
      if (param == "?") {
        // Query current RX mode
        Serial.print(F("FPGA1RX="));
        Serial.println(fpga1RxMode == FPGA_RX_TEXT ? F("TEXT") : F("BINARY"));
      } else {
        if (param == "TEXT") {
          fpga1RxMode = FPGA_RX_TEXT;
          Serial.println(F("OK:FPGA1_RX_TEXT"));
        } else if (param == "BINARY" || param == "BIN") {
          fpga1RxMode = FPGA_RX_BINARY;
          Serial.println(F("OK:FPGA1_RX_BINARY"));
        } else {
          Serial.println(F("ERR:USE_TEXT_or_BINARY"));
        }
      }

    } else if (c.startsWith("#FPGA2RX")) {
      String param = c.substring(8);
      param.trim();
      if (param == "?") {
        // Query current RX mode
        Serial.print(F("FPGA2RX="));
        Serial.println(fpga2RxMode == FPGA_RX_TEXT ? F("TEXT") : F("BINARY"));
      } else {
        if (param == "TEXT") {
          fpga2RxMode = FPGA_RX_TEXT;
          Serial.println(F("OK:FPGA2_RX_TEXT"));
        } else if (param == "BINARY" || param == "BIN") {
          fpga2RxMode = FPGA_RX_BINARY;
          Serial.println(F("OK:FPGA2_RX_BINARY"));
        } else {
          Serial.println(F("ERR:USE_TEXT_or_BINARY"));
        }
      }

    } else if (c.startsWith("#FPGA3RX")) {
      String param = c.substring(8);
      param.trim();
      if (param == "?") {
        // Query current RX mode
        Serial.print(F("FPGA3RX="));
        Serial.println(fpga3RxMode == FPGA_RX_TEXT ? F("TEXT") : F("BINARY"));
      } else {
        if (param == "TEXT") {
          fpga3RxMode = FPGA_RX_TEXT;
          Serial.println(F("OK:FPGA3_RX_TEXT"));
        } else if (param == "BINARY" || param == "BIN") {
          fpga3RxMode = FPGA_RX_BINARY;
          Serial.println(F("OK:FPGA3_RX_BINARY"));
        } else {
          Serial.println(F("ERR:USE_TEXT_or_BINARY"));
        }
      }

    } else if (c.startsWith("#FPGASEND ")) {
      String data = c.substring(10);
      HardwareSerial& fpga = getFPGA();
      fpga.print(data);
      applyTermination(fpga);
      Serial.println(F("OK:SENT"));

    } else if (c == "#FPGAPING") {
      HardwareSerial& fpga = getFPGA();
      fpga.print(F("PING"));
      applyTermination(fpga);
      Serial.println(F("OK:PING"));

    } else if (c.startsWith("#FPGABYTES ")) {
      String hexData = c.substring(11);
      hexData.trim();
      int sent = sendHexBytes(hexData);
      Serial.print(F("OK:SENT_"));
      Serial.print(sent);
      Serial.println(F(" bytes"));

      // Show TX on LCD bottom
      showTextBottom("TX: " + hexData);

    } else if (c.startsWith("#FPGAREAD ")) {
      // #FPGAREAD <num_bytes> [timeout_ms]
      // Read expected number of bytes from FPGA with optional timeout
      int sp = c.indexOf(' ', 10);
      uint8_t numBytes = c.substring(10, sp > 0 ? sp : c.length()).toInt();
      uint16_t timeout = 1000;  // Default 1 second

      if (sp > 0) {
        timeout = c.substring(sp + 1).toInt();
        if (timeout == 0) timeout = 1000;
      }

      if (numBytes > 0 && numBytes <= 64) {
        readFPGAResponse(numBytes, timeout);
      } else {
        Serial.println(F("ERR:BYTES_1_TO_64"));
      }

    } else if (c.startsWith("#FPGAQUERY ")) {
      // #FPGAQUERY <send_hex> <expect_bytes> [timeout]
      // Send bytes, then read response
      // Example: #FPGAQUERY 52 45 47 4 1000  (send "REG", expect 4 bytes, 1s timeout)
      String rest = c.substring(11);
      rest.trim();

      // Find how many bytes to expect (last number before optional timeout)
      int lastSpace = rest.lastIndexOf(' ');
      int secondLastSpace = rest.lastIndexOf(' ', lastSpace - 1);

      if (lastSpace > 0 && secondLastSpace > 0) {
        String hexData = rest.substring(0, secondLastSpace);
        uint8_t expectBytes = rest.substring(secondLastSpace + 1, lastSpace).toInt();
        uint16_t timeout = rest.substring(lastSpace + 1).toInt();

        if (timeout == 0) timeout = 1000;
        if (expectBytes > 0 && expectBytes <= 64) {
          // Send query bytes
          sendHexBytes(hexData);
          delay(10);  // Brief delay for FPGA processing
          // Read response
          readFPGAResponse(expectBytes, timeout);
        } else {
          Serial.println(F("ERR:EXPECT_1_TO_64"));
        }
      } else {
        Serial.println(F("ERR:FORMAT_QUERY_HEX_BYTES_TIMEOUT"));
      }

    } else if (c.startsWith("#FPGA1PARSE")) {
      String param = c.substring(11);
      param.trim();
      if (param == "?") {
        // Query current parse mode
        Serial.print(F("FPGA1PARSE="));
        switch (fpga1ParseMode) {
          case PARSE_NONE: Serial.println(F("NONE")); break;
          case PARSE_ASCII: Serial.println(F("ASCII")); break;
          case PARSE_DEC: Serial.println(F("DEC")); break;
          case PARSE_MIXED: Serial.println(F("MIXED")); break;
        }
      } else {
        if (param == "NONE") {
          fpga1ParseMode = PARSE_NONE;
          Serial.println(F("OK:FPGA1_PARSE_NONE"));
        } else if (param == "ASCII") {
          fpga1ParseMode = PARSE_ASCII;
          Serial.println(F("OK:FPGA1_PARSE_ASCII"));
        } else if (param == "DEC") {
          fpga1ParseMode = PARSE_DEC;
          Serial.println(F("OK:FPGA1_PARSE_DEC"));
        } else if (param == "MIXED") {
          fpga1ParseMode = PARSE_MIXED;
          Serial.println(F("OK:FPGA1_PARSE_MIXED"));
        } else {
          Serial.println(F("ERR:USE_NONE_ASCII_DEC_MIXED"));
        }
      }

    } else if (c.startsWith("#FPGA2PARSE")) {
      String param = c.substring(11);
      param.trim();
      if (param == "?") {
        // Query current parse mode
        Serial.print(F("FPGA2PARSE="));
        switch (fpga2ParseMode) {
          case PARSE_NONE: Serial.println(F("NONE")); break;
          case PARSE_ASCII: Serial.println(F("ASCII")); break;
          case PARSE_DEC: Serial.println(F("DEC")); break;
          case PARSE_MIXED: Serial.println(F("MIXED")); break;
        }
      } else {
        if (param == "NONE") {
          fpga2ParseMode = PARSE_NONE;
          Serial.println(F("OK:FPGA2_PARSE_NONE"));
        } else if (param == "ASCII") {
          fpga2ParseMode = PARSE_ASCII;
          Serial.println(F("OK:FPGA2_PARSE_ASCII"));
        } else if (param == "DEC") {
          fpga2ParseMode = PARSE_DEC;
          Serial.println(F("OK:FPGA2_PARSE_DEC"));
        } else if (param == "MIXED") {
          fpga2ParseMode = PARSE_MIXED;
          Serial.println(F("OK:FPGA2_PARSE_MIXED"));
        } else {
          Serial.println(F("ERR:USE_NONE_ASCII_DEC_MIXED"));
        }
      }

    } else if (c.startsWith("#FPGA3PARSE")) {
      String param = c.substring(11);
      param.trim();
      if (param == "?") {
        // Query current parse mode
        Serial.print(F("FPGA3PARSE="));
        switch (fpga3ParseMode) {
          case PARSE_NONE: Serial.println(F("NONE")); break;
          case PARSE_ASCII: Serial.println(F("ASCII")); break;
          case PARSE_DEC: Serial.println(F("DEC")); break;
          case PARSE_MIXED: Serial.println(F("MIXED")); break;
        }
      } else {
        if (param == "NONE") {
          fpga3ParseMode = PARSE_NONE;
          Serial.println(F("OK:FPGA3_PARSE_NONE"));
        } else if (param == "ASCII") {
          fpga3ParseMode = PARSE_ASCII;
          Serial.println(F("OK:FPGA3_PARSE_ASCII"));
        } else if (param == "DEC") {
          fpga3ParseMode = PARSE_DEC;
          Serial.println(F("OK:FPGA3_PARSE_DEC"));
        } else if (param == "MIXED") {
          fpga3ParseMode = PARSE_MIXED;
          Serial.println(F("OK:FPGA3_PARSE_MIXED"));
        } else {
          Serial.println(F("ERR:USE_NONE_ASCII_DEC_MIXED"));
        }
      }

    // ========== I2C COMMANDS ==========
    } else if (c == "#I2CSCAN") {
      i2cScan();

    } else if (c.startsWith("#I2CREAD ")) {
      handleI2CRead(c.substring(9));

    } else if (c.startsWith("#I2CWRITE ")) {
      handleI2CWrite(c.substring(10));

    // ========== SPI COMMANDS ==========
    } else if (c.startsWith("#SPIBEGIN ")) {
      handleSPIBegin(c.substring(10));

    } else if (c == "#SPIEND") {
      handleSPIEnd();

    } else if (c.startsWith("#SPITRANSFER ")) {
      handleSPITransfer(c.substring(13));

    } else if (c.startsWith("#SPISETTINGS ")) {
      handleSPISettings(c.substring(13));

    // ========== EEPROM COMMANDS ==========
    } else if (c.startsWith("#EEPROMREAD ")) {
      handleEEPROMRead(c.substring(12));

    } else if (c.startsWith("#EEPROMWRITE ")) {
      handleEEPROMWrite(c.substring(13));

    } else if (c.startsWith("#EEPROMDUMP ")) {
      handleEEPROMDump(c.substring(12));

    } else if (c == "#EEPROMCLEAR") {
      handleEEPROMClear();

    } else if (c.startsWith("#EEPROMPROTECT ")) {
      handleEEPROMProtect(c.substring(15));

    } else if (c == "#EEPROMPROTECT?") {
      handleEEPROMProtect("?");

    // ========== ANALOG INPUT COMMANDS ==========
    } else if (c.startsWith("#ANALOGREAD ")) {
      handleAnalogRead(c.substring(12));

    } else if (c == "#ANALOGREADALL") {
      handleAnalogReadAll();

    } else if (c.startsWith("#ANALOGREF ")) {
      handleAnalogRef(c.substring(11));

    } else if (c.startsWith("#ANALOGAVG ")) {
      handleAnalogAvg(c.substring(11));

    } else if (c.startsWith("#GAUGE ")) {
      handleGauge(c.substring(7));

    } else if (c.startsWith("#BARGRAPH ")) {
      handleBarGraph(c.substring(10));

    } else if (c.startsWith("#NUMBOX ")) {
      handleNumBox(c.substring(8));

    } else if (c.startsWith("#TREND ")) {
      handleTrend(c.substring(7));

    } else if (c.startsWith("#LOGSTART ")) {
      handleLogStart(c.substring(10));

    } else if (c == "#LOGSTOP") {
      handleLogStop();

    } else if (c.startsWith("#LOGREAD ")) {
      handleLogRead(c.substring(9));

    } else if (c == "#LOGREAD") {
      handleLogRead("");

    } else if (c == "#LOGCLEAR") {
      handleLogClear();

    } else if (c.startsWith("#LOGCONFIG ")) {
      handleLogConfig(c.substring(11));

    } else if (c == "#LOGSTATUS") {
      handleLogStatus();

    } else if (c.startsWith("#LOGZONE ")) {
      handleLogZone(c.substring(9));

    } else if (c == "#LOGZONE?") {
      handleLogZone("?");

    } else if (c.startsWith("#WAVEGEN ")) {
      handleWaveGen(c.substring(9));

    } else if (c == "#WAVESTOP") {
      handleWaveStop();

    } else if (c.startsWith("#PULSEIN ")) {
      handlePulseIn(c.substring(9));

    } else if (c.startsWith("#FREQCOUNT ")) {
      handleFreqCount(c.substring(11));

    } else if (c.startsWith("#FREQMON ")) {
      handleFreqMon(c.substring(9));

    } else if (c == "#FREQSTOP") {
      handleFreqStop();

    } else if (c == "#CONFIGSAVE") {
      handleConfigSave();

    } else if (c == "#CONFIGLOAD") {
      handleConfigLoad();

    } else if (c == "#CONFIGRESET") {
      handleConfigReset();

    } else if (c == "#CONFIGSHOW") {
      handleConfigShow();

    } else if (c.startsWith("#BTNCONFIG ")) {
      handleBtnConfig(c.substring(11));

    } else if (c.startsWith("#FPGAWRITE ")) {
      handleFPGAWrite(c.substring(11));

    } else if (c.startsWith("#FPGAREAD ")) {
      handleFPGARead(c.substring(10));

    } else if (c == "#HELP") {
      help();

    } else if (c == "#ID") {
      Serial.println(F("COM LCD MEGA ILI9486 SPLIT TEXT"));

    } else if (c.startsWith("#TERM")) {
      String param = c.substring(5);
      param.trim();
      if (param == "?") {
        // Query current termination mode
        Serial.print(F("TERM="));
        switch (bypassTerm) {
          case TERM_NONE: Serial.println(F("NONE")); break;
          case TERM_LF: Serial.println(F("LF")); break;
          case TERM_CR: Serial.println(F("CR")); break;
          case TERM_CRLF: Serial.println(F("CRLF")); break;
          case TERM_CUSTOM:
            Serial.print(F("CUSTOM 0x"));
            if (customTermByte < 16) Serial.print(F("0"));
            Serial.print(customTermByte, HEX);
            Serial.print(F(" ("));
            Serial.print(customTermByte);
            Serial.println(F(")"));
            break;
        }
      } else {
        String mode = param;
        if (mode == "NONE") {
          bypassTerm = TERM_NONE;
          Serial.println(F("OK:TERM_NONE"));
        } else if (mode == "LF" || mode == "\\N") {
          bypassTerm = TERM_LF;
          Serial.println(F("OK:TERM_LF"));
        } else if (mode == "CR" || mode == "\\R") {
          bypassTerm = TERM_CR;
          Serial.println(F("OK:TERM_CR"));
        } else if (mode == "CRLF" || mode == "\\R\\N") {
          bypassTerm = TERM_CRLF;
          Serial.println(F("OK:TERM_CRLF"));
        } else {
          // Try to parse as custom byte value (hex or decimal)
          long value = -1;
          if (mode.startsWith("0X") || mode.startsWith("0x")) {
            // Hex format: 0xFF or 0XFF
            value = strtol(mode.c_str() + 2, NULL, 16);
          } else if (mode.length() <= 2 &&
                     ((mode[0] >= '0' && mode[0] <= '9') ||
                      (mode[0] >= 'A' && mode[0] <= 'F') ||
                      (mode[0] >= 'a' && mode[0] <= 'f'))) {
            // Try hex without 0x prefix (FF, 0A, etc)
            value = strtol(mode.c_str(), NULL, 16);
          } else {
            // Try decimal (255, 10, etc)
            value = mode.toInt();
          }

          if (value >= 0 && value <= 255) {
            bypassTerm = TERM_CUSTOM;
            customTermByte = (uint8_t)value;
            Serial.print(F("OK:TERM_CUSTOM_0x"));
            if (customTermByte < 16) Serial.print(F("0"));
            Serial.print(customTermByte, HEX);
            Serial.print(F(" ("));
            Serial.print(customTermByte);
            Serial.println(F(")"));
          } else {
            Serial.println(F("ERR:USE_NONE_LF_CR_CRLF_or_BYTE"));
          }
        }
      }

    } else if (c == "#SHOWBTNS") {
      showButtons();
      Serial.println(F("OK:SHOWBTNS"));

    } else if (c == "#HIDEBTNS") {
      hideButtons();
      Serial.println(F("OK:HIDEBTNS"));

    } else if (c.startsWith("#TOUCHCAL")) {
      String param = c.substring(9);
      param.trim();
      if (param == "?") {
        // Query current calibration
        Serial.print(F("TOUCHCAL: MINX="));
        Serial.print(TS_MINX);
        Serial.print(F(" MAXX="));
        Serial.print(TS_MAXX);
        Serial.print(F(" MINY="));
        Serial.print(TS_MINY);
        Serial.print(F(" MAXY="));
        Serial.println(TS_MAXY);
      } else {
        // Parse calibration values: #TOUCHCAL <minx> <maxx> <miny> <maxy>
        int vals[4];
        int count = 0;
        int lastIdx = 0;
        for (int i = 0; i <= param.length() && count < 4; i++) {
          if (i == param.length() || param[i] == ' ') {
            if (i > lastIdx) {
              vals[count++] = param.substring(lastIdx, i).toInt();
            }
            lastIdx = i + 1;
          }
        }

        if (count == 4) {
          TS_MINX = vals[0];
          TS_MAXX = vals[1];
          TS_MINY = vals[2];
          TS_MAXY = vals[3];
          Serial.print(F("OK:TOUCHCAL "));
          Serial.print(TS_MINX);
          Serial.print(F(" "));
          Serial.print(TS_MAXX);
          Serial.print(F(" "));
          Serial.print(TS_MINY);
          Serial.print(F(" "));
          Serial.println(TS_MAXY);
        } else {
          Serial.println(F("ERR:USE_MINX_MAXX_MINY_MAXY"));
        }
      }

    } else if (c == "#TOUCHTEST") {
      touchTestMode = !touchTestMode;
      Serial.print(F("OK:TOUCHTEST_"));
      Serial.println(touchTestMode ? F("ON") : F("OFF"));
      if (touchTestMode) {
        Serial.println(F("Touch screen to see raw coordinates"));
        Serial.println(F("Use #TOUCHTEST again to disable"));
      }

    } else if (c.startsWith("#GPIOMODE ")) {
      // #GPIOMODE <pin> <IN|INPU|OUT>
      int sp = c.indexOf(' ', 10);
      if (sp > 0) {
        uint8_t pin = c.substring(10, sp).toInt();
        String mode = c.substring(sp + 1);
        mode.trim();
        setGPIOMode(pin, mode);
      }

    } else if (c.startsWith("#GPIOWRITE ")) {
      // #GPIOWRITE <pin> <0|1|HIGH|LOW>
      int sp = c.indexOf(' ', 11);
      if (sp > 0) {
        uint8_t pin = c.substring(11, sp).toInt();
        String val = c.substring(sp + 1);
        val.trim();
        writeGPIO(pin, val);
      }

    } else if (c.startsWith("#GPIOREAD ")) {
      // #GPIOREAD <pin>
      uint8_t pin = c.substring(10).toInt();
      readGPIO(pin);

    } else if (c == "#GPIOREADALL") {
      readAllGPIO();

    } else if (c.startsWith("#GPIOEVENT ")) {
      // #GPIOEVENT <pin> <NONE|RISING|FALLING|BOTH>
      int sp = c.indexOf(' ', 11);
      if (sp > 0) {
        uint8_t pin = c.substring(11, sp).toInt();
        String type = c.substring(sp + 1);
        type.trim();
        setGPIOEvent(pin, type);
      }

    } else if (c == "#GPIOREG") {
      // Show GPIO register (all pin states as 8-bit value)
      showGPIORegister();

    } else if (c.startsWith("#GPIOSET ")) {
      // #GPIOSET <0-255> - Set all 8 pins at once
      uint8_t regVal = c.substring(9).toInt();
      setGPIORegister(regVal);

    } else {
      Serial.print(F("ERR:UNKNOWN:"));
      Serial.println(c);
    }
  } else {
    showText(c);
  }
}

void setCol(String& n) {
  n.toUpperCase();
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    topTextColor = c;
    Serial.print(F("OK:COLOR_"));
    Serial.println(n);
  }
}

void setBgCol(String& n) {
  n.trim();
  n.toUpperCase();
  if (n == "NONE") {
    topOpaqueText = false;
    Serial.println(F("OK:BG_NONE"));
    return;
  }
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    topBgColor = c;
    topOpaqueText = true;
    Serial.print(F("OK:BGCOLOR_"));
    Serial.println(n);
  }
}

void setBotCol(String& n) {
  n.toUpperCase();
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    bottomTextColor = c;
    Serial.print(F("OK:BOTCOLOR_"));
    Serial.println(n);
  }
}

uint16_t getColorFromName(String& n) {
  if (n == "RED") return 0xF800;
  if (n == "GREEN") return 0x07E0;
  if (n == "BLUE") return 0x001F;
  if (n == "CYAN") return 0x07FF;
  if (n == "MAGENTA") return 0xF81F;
  if (n == "YELLOW") return 0xFFE0;
  if (n == "WHITE") return 0xFFFF;
  if (n == "BLACK") return 0x0000;
  if (n == "ORANGE") return 0xFD20;
  if (n == "PINK") return 0xFC9F;
  if (n == "PURPLE") return 0x8010;
  if (n == "NAVY") return 0x0010;
  return 0xFFFF;
}

int sendHexBytes(String hexStr) {
  uint8_t buffer[128];
  int byteCount = 0;
  int startIdx = 0;

  while (startIdx < hexStr.length() && byteCount < 128) {
    while (startIdx < hexStr.length() && hexStr[startIdx] == ' ') {
      startIdx++;
    }
    if (startIdx >= hexStr.length()) break;

    int endIdx = startIdx;
    while (endIdx < hexStr.length() && hexStr[endIdx] != ' ') {
      endIdx++;
    }

    String hexByte = hexStr.substring(startIdx, endIdx);
    hexByte.trim();

    if (hexByte.startsWith("0x") || hexByte.startsWith("0X")) {
      hexByte = hexByte.substring(2);
    }

    if (hexByte.length() > 0) {
      long value = strtol(hexByte.c_str(), NULL, 16);
      if (value >= 0 && value <= 255) {
        buffer[byteCount++] = (uint8_t)value;
      }
    }
    startIdx = endIdx;
  }

  HardwareSerial& fpga = getFPGA();
  for (int i = 0; i < byteCount; i++) {
    fpga.write(buffer[i]);
  }

  // Apply termination after sending all bytes
  applyTermination(fpga);

  return byteCount;
}

void parseRect(String p) {
  int v[4], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 4) {
    tft.drawRect(v[0], v[1], v[2], v[3], topTextColor);
    Serial.println(F("OK:RECT"));
  }
}

void parseFill(String p) {
  int v[4], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 4) {
    tft.fillRect(v[0], v[1], v[2], v[3], topTextColor);
    Serial.println(F("OK:FILL"));
  }
}

void parseCirc(String p) {
  int v[3], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 3; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 3) {
    tft.drawCircle(v[0], v[1], v[2], topTextColor);
    Serial.println(F("OK:CIRCLE"));
  }
}

void parseLine(String p) {
  int v[4], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 4) {
    tft.drawLine(v[0], v[1], v[2], v[3], topTextColor);
    Serial.println(F("OK:LINE"));
  }
}

void parseProg(String p) {
  int v[5], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 5; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 5) {
    int pct = constrain(v[4], 0, 100);
    int fw = (v[2] * pct) / 100;
    tft.drawRect(v[0], v[1], v[2], v[3], topTextColor);
    if (fw > 2) {
      tft.fillRect(v[0]+1, v[1]+1, fw-2, v[3]-2, topTextColor);
    }
    Serial.println(F("OK:PROG"));
  }
}

void help() {
  Serial.println(F("=== MEGA Split Screen Commands ==="));
  Serial.println(F("TEXT-BASED (Easy to Use!)"));
  Serial.println();
  Serial.println(F("TOP SCREEN:"));
  Serial.println(F("  #CLR - Clear top"));
  Serial.println(F("  #SIZE <1-10> - Text size"));
  Serial.println(F("  #COLOR <name> - Text color"));
  Serial.println(F("  #BGCOLOR <name|NONE>"));
  Serial.println(F("  #POS <x> <y> - Position"));
  Serial.println();
  Serial.println(F("BOTTOM SCREEN:"));
  Serial.println(F("  #CLRBOT - Clear bottom"));
  Serial.println(F("  #BOTSIZE <1-5>"));
  Serial.println(F("  #BOTCOLOR <name>"));
  Serial.println();
  Serial.println(F("FPGA SERIAL:"));
  Serial.println(F("  >>>text - Passthrough to FPGA"));
  Serial.println(F("  #STATUS - Show all settings"));
  Serial.println(F("  #FPGASEL <1|2|3|?> - Select FPGA"));
  Serial.println(F("  #FPGA1BAUD <baud|?>"));
  Serial.println(F("  #FPGA2BAUD <baud|?>"));
  Serial.println(F("  #FPGA3BAUD <baud|?>"));
  Serial.println(F("  #FPGA1STOP <1|2|?> - Hardware stop bits"));
  Serial.println(F("  #FPGA2STOP <1|2|?>"));
  Serial.println(F("  #FPGA3STOP <1|2|?>"));
  Serial.println(F("  #FPGA1RX <TEXT|BINARY|?> - RX mode"));
  Serial.println(F("  #FPGA2RX <TEXT|BINARY|?>"));
  Serial.println(F("  #FPGA3RX <TEXT|BINARY|?>"));
  Serial.println(F("  #FPGA1PARSE <NONE|ASCII|DEC|MIXED|?>"));
  Serial.println(F("    NONE: Hex (0x41), ASCII: 'A' or [41]"));
  Serial.println(F("    DEC: 65, MIXED: 0x41='A'"));
  Serial.println(F("  #FPGA2PARSE <mode|?>"));
  Serial.println(F("  #FPGA3PARSE <mode|?>"));
  Serial.println(F("  #FPGASEND <text>"));
  Serial.println(F("  #FPGAPING"));
  Serial.println(F("  #FPGABYTES <hex>"));
  Serial.println(F("  #FPGAREAD <bytes> [timeout]"));
  Serial.println(F("    Read N bytes from FPGA"));
  Serial.println(F("  #FPGAQUERY <hex> <expect> [timeout]"));
  Serial.println(F("    Send hex, read response"));
  Serial.println(F("  #TERM <NONE|LF|CR|CRLF|0xFF|255|?>"));
  Serial.println(F("    (TX termination byte)"));
  Serial.println();
  Serial.println(F("QUERY: Add ? to most commands to query"));
  Serial.println(F("  Example: #FPGA1BAUD ?"));
  Serial.println();
  Serial.println(F("DRAWING:"));
  Serial.println(F("  #RECT <x y w h>"));
  Serial.println(F("  #FILL <x y w h>"));
  Serial.println(F("  #CIRCLE <x y r>"));
  Serial.println(F("  #LINE <x1 y1 x2 y2>"));
  Serial.println(F("  #PROG <x y w h %>"));
  Serial.println();
  Serial.println(F("TOUCH BUTTONS:"));
  Serial.println(F("  #SHOWBTNS - Show touch buttons"));
  Serial.println(F("  #HIDEBTNS - Hide touch buttons"));
  Serial.println(F("  Buttons send byte arrays:"));
  Serial.println(F("    UP=0x55 0x50, DN=0x44 0x4E"));
  Serial.println(F("    LT=0x4C 0x54, RT=0x52 0x54"));
  Serial.println();
  Serial.println(F("TOUCH CALIBRATION:"));
  Serial.println(F("  #TOUCHCAL ? - Show current calibration"));
  Serial.println(F("  #TOUCHCAL <minx> <maxx> <miny> <maxy>"));
  Serial.println(F("  #TOUCHTEST - Toggle touch test mode"));
  Serial.println(F("    Shows raw & mapped coordinates"));
  Serial.println();
  Serial.println(F("GPIO (Pins 22-29):"));
  Serial.println(F("  #GPIOMODE <pin> <IN|INPU|OUT>"));
  Serial.println(F("  #GPIOWRITE <pin> <0|1>"));
  Serial.println(F("  #GPIOREAD <pin>"));
  Serial.println(F("  #GPIOREADALL - Read all pins"));
  Serial.println(F("  #GPIOEVENT <pin> <NONE|RISING|FALLING|BOTH>"));
  Serial.println(F("  #GPIOREG - Show 8-bit register"));
  Serial.println(F("  #GPIOSET <0-255> - Set register"));
  Serial.println();
  Serial.println(F("I2C (SDA=20, SCL=21):"));
  Serial.println(F("  #I2CSCAN - Scan for I2C devices"));
  Serial.println(F("  #I2CREAD <addr> <reg> <bytes>"));
  Serial.println(F("  #I2CWRITE <addr> <reg> <data...>"));
  Serial.println(F("  #I2CWRITE <addr> <data...>"));
  Serial.println();
  Serial.println(F("SPI (MOSI=51, MISO=50, SCK=52):"));
  Serial.println(F("  #SPIBEGIN <cs_pin> - Init SPI"));
  Serial.println(F("  #SPIEND - End SPI"));
  Serial.println(F("  #SPITRANSFER <byte...>"));
  Serial.println(F("  #SPISETTINGS <speed> <mode>"));
  Serial.println();
  Serial.println(F("EEPROM (4KB internal):"));
  Serial.println(F("  #EEPROMREAD <addr>"));
  Serial.println(F("  #EEPROMWRITE <addr> <value>"));
  Serial.println(F("  #EEPROMDUMP <start> <end>"));
  Serial.println(F("  #EEPROMCLEAR - Erase all"));
  Serial.println(F("  #EEPROMPROTECT <zone> <start> <end>"));
  Serial.println(F("  #EEPROMPROTECT <zone> OFF"));
  Serial.println(F("  #EEPROMPROTECT? - Show zones"));
  Serial.println();
  Serial.println(F("ANALOG INPUT (A8-A15):"));
  Serial.println(F("  #ANALOGREAD <pin> - Read A8-A15"));
  Serial.println(F("  #ANALOGREADALL - Read all 8 pins"));
  Serial.println(F("  #ANALOGREF <DEFAULT|INTERNAL>"));
  Serial.println(F("  #ANALOGAVG <1-16> - Set averaging"));
  Serial.println(F("  Note: EXTERNAL ref N/A (AREF used by LCD)"));
  Serial.println();
  Serial.println(F("DISPLAY WIDGETS:"));
  Serial.println(F("  #GAUGE <x y r val max>"));
  Serial.println(F("  #BARGRAPH <x y w h val1 val2...>"));
  Serial.println(F("  #NUMBOX <x y value>"));
  Serial.println(F("  #TREND <x y w h val1 val2...>"));
  Serial.println();
  Serial.println(F("DATA LOGGING (EEPROM):"));
  Serial.println(F("  #LOGSTART <ms> [source]"));
  Serial.println(F("  #LOGSTOP - Stop logging"));
  Serial.println(F("  #LOGREAD [count] - Read entries"));
  Serial.println(F("  #LOGCLEAR - Clear log"));
  Serial.println(F("  #LOGSTATUS - Show log status"));
  Serial.println(F("  #LOGCONFIG <source> - Set source"));
  Serial.println(F("    source: 0-7=A8-A15, 15=GPIO"));
  Serial.println(F("  #LOGZONE <start> <end> - Set zone"));
  Serial.println(F("  #LOGZONE? - Query log zone"));
  Serial.println();
  Serial.println(F("WAVEFORM GENERATOR (PWM):"));
  Serial.println(F("  #WAVEGEN <pin> <type> <freq>"));
  Serial.println(F("    type: SQUARE, TRIANGLE, SINE"));
  Serial.println(F("    freq: 1-10000 Hz"));
  Serial.println(F("    PWM pins: 2-13, 44-46"));
  Serial.println(F("  #WAVESTOP - Stop waveform"));
  Serial.println();
  Serial.println(F("PULSE/FREQUENCY MEASUREMENT:"));
  Serial.println(F("  #PULSEIN <pin> <state> <timeout>"));
  Serial.println(F("    state: HIGH or LOW"));
  Serial.println(F("    timeout: microseconds"));
  Serial.println(F("  #FREQCOUNT <pin> <duration>"));
  Serial.println(F("    duration: milliseconds"));
  Serial.println(F("  #FREQMON <pin> <duration>"));
  Serial.println(F("    Continuous monitoring"));
  Serial.println(F("  #FREQSTOP - Stop monitoring"));
  Serial.println();
  Serial.println(F("CONFIGURATION SYSTEM:"));
  Serial.println(F("  #CONFIGSAVE - Save settings to EEPROM"));
  Serial.println(F("  #CONFIGLOAD - Load settings from EEPROM"));
  Serial.println(F("  #CONFIGRESET - Reset to defaults"));
  Serial.println(F("  #CONFIGSHOW - Show all settings"));
  Serial.println(F("  Saves: rotation, FPGA baud/term, buttons"));
  Serial.println();
  Serial.println(F("BUTTON CONFIGURATION:"));
  Serial.println(F("  #BTNCONFIG <btn> <len> <bytes...>"));
  Serial.println(F("    btn: UP, DOWN, LEFT, RIGHT"));
  Serial.println(F("    len: 1-15 bytes"));
  Serial.println(F("  #BTNCONFIG <btn> ? - Query button"));
  Serial.println(F("  Example: #BTNCONFIG UP 3 0xAA 0xBB 0xCC"));
  Serial.println();
  Serial.println(F("FPGA EEPROM ZONE (100-299):"));
  Serial.println(F("  #FPGAWRITE <addr> <bytes...>"));
  Serial.println(F("  #FPGAREAD <addr> [count]"));
  Serial.println(F("  200 bytes for FPGA data exchange"));
  Serial.println();
  Serial.println(F("OTHER:"));
  Serial.println(F("  #CLRALL - Clear both screens"));
  Serial.println(F("  #ROT <0-3> - Rotation"));
  Serial.println(F("  #ID - Device ID"));
  Serial.println(F("  #HELP - This help"));
  Serial.println();
  Serial.println(F("COLORS: RED GREEN BLUE CYAN"));
  Serial.println(F("        MAGENTA YELLOW WHITE"));
  Serial.println(F("        BLACK ORANGE PINK"));
  Serial.println(F("        PURPLE NAVY"));
}

// ========== I2C FUNCTIONS ==========

void i2cScan() {
  Serial.println(F("Scanning I2C bus (0x00-0x7F)..."));
  uint8_t count = 0;

  for (uint8_t addr = 0; addr < 128; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("  Device found at 0x"));
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println(F("No I2C devices found"));
  } else {
    Serial.print(F("Found "));
    Serial.print(count);
    Serial.println(F(" device(s)"));
  }
}

void handleI2CRead(String params) {
  // Parse: <addr> <reg> <bytes>
  int addr, reg, numBytes;
  int sp1 = params.indexOf(' ');
  int sp2 = params.indexOf(' ', sp1 + 1);

  if (sp1 == -1 || sp2 == -1) {
    Serial.println(F("ERR:FORMAT #I2CREAD <addr> <reg> <bytes>"));
    return;
  }

  addr = parseHexOrDec(params.substring(0, sp1));
  reg = parseHexOrDec(params.substring(sp1 + 1, sp2));
  numBytes = params.substring(sp2 + 1).toInt();

  if (addr < 0 || addr > 127 || numBytes < 1 || numBytes > 32) {
    Serial.println(F("ERR:INVALID_PARAMS"));
    return;
  }

  // Write register address
  Wire.beginTransmission(addr);
  Wire.write(reg);
  uint8_t error = Wire.endTransmission(false); // Repeated start

  if (error != 0) {
    Serial.print(F("ERR:I2C_WRITE_FAIL:"));
    Serial.println(error);
    return;
  }

  // Read bytes
  Wire.requestFrom((uint8_t)addr, (uint8_t)numBytes);

  Serial.print(F("I2C[0x"));
  if (addr < 16) Serial.print("0");
  Serial.print(addr, HEX);
  Serial.print(F("] Reg 0x"));
  if (reg < 16) Serial.print("0");
  Serial.print(reg, HEX);
  Serial.print(F(": "));

  uint8_t bytesRead = 0;
  while (Wire.available() && bytesRead < numBytes) {
    uint8_t b = Wire.read();
    if (bytesRead > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (b < 16) Serial.print("0");
    Serial.print(b, HEX);
    bytesRead++;
  }
  Serial.println();
}

void handleI2CWrite(String params) {
  // Parse: <addr> <reg> <data...> or <addr> <data...>
  // First, parse all hex/dec numbers
  uint8_t values[32];
  uint8_t count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length(); i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        values[count++] = parseHexOrDec(token);
        if (count >= 32) break;
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #I2CWRITE <addr> <data...>"));
    return;
  }

  uint8_t addr = values[0];
  if (addr > 127) {
    Serial.println(F("ERR:INVALID_ADDR"));
    return;
  }

  // Write to I2C
  Wire.beginTransmission(addr);
  for (uint8_t i = 1; i < count; i++) {
    Wire.write(values[i]);
  }
  uint8_t error = Wire.endTransmission();

  if (error != 0) {
    Serial.print(F("ERR:I2C_WRITE_FAIL:"));
    Serial.println(error);
    return;
  }

  Serial.print(F("I2C[0x"));
  if (addr < 16) Serial.print("0");
  Serial.print(addr, HEX);
  Serial.print(F("] Wrote "));
  Serial.print(count - 1);
  Serial.println(F(" bytes"));
}

// ========== SPI FUNCTIONS ==========

void handleSPIBegin(String params) {
  int csPin = params.toInt();

  if (csPin < 0 || csPin > 53) {
    Serial.println(F("ERR:INVALID_PIN"));
    return;
  }

  spiCSPin = csPin;
  pinMode(spiCSPin, OUTPUT);
  digitalWrite(spiCSPin, HIGH);

  SPI.begin();
  SPI.beginTransaction(SPISettings(spiSpeed, MSBFIRST, spiMode));

  Serial.print(F("SPI initialized, CS="));
  Serial.println(spiCSPin);
}

void handleSPIEnd() {
  if (spiCSPin < 0) {
    Serial.println(F("ERR:SPI_NOT_INIT"));
    return;
  }

  SPI.endTransaction();
  SPI.end();
  spiCSPin = -1;

  Serial.println(F("SPI ended"));
}

void handleSPITransfer(String params) {
  if (spiCSPin < 0) {
    Serial.println(F("ERR:SPI_NOT_INIT (use #SPIBEGIN first)"));
    return;
  }

  // Parse bytes
  uint8_t txBytes[64];
  uint8_t rxBytes[64];
  uint8_t count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length(); i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        txBytes[count++] = parseHexOrDec(token);
        if (count >= 64) break;
      }
      lastPos = i + 1;
    }
  }

  if (count == 0) {
    Serial.println(F("ERR:NO_DATA"));
    return;
  }

  // Transfer bytes
  digitalWrite(spiCSPin, LOW);
  for (uint8_t i = 0; i < count; i++) {
    rxBytes[i] = SPI.transfer(txBytes[i]);
  }
  digitalWrite(spiCSPin, HIGH);

  // Print TX
  Serial.print(F("SPI TX: "));
  for (uint8_t i = 0; i < count; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (txBytes[i] < 16) Serial.print("0");
    Serial.print(txBytes[i], HEX);
  }
  Serial.println();

  // Print RX
  Serial.print(F("SPI RX: "));
  for (uint8_t i = 0; i < count; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (rxBytes[i] < 16) Serial.print("0");
    Serial.print(rxBytes[i], HEX);
  }
  Serial.println();
}

void handleSPISettings(String params) {
  // Parse: <speed> <mode>
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #SPISETTINGS <speed> <mode>"));
    return;
  }

  uint32_t speed = params.substring(0, sp).toInt();
  uint8_t mode = params.substring(sp + 1).toInt();

  if (speed < 100 || speed > 16000000) {
    Serial.println(F("ERR:SPEED_RANGE (100-16000000 Hz)"));
    return;
  }

  if (mode > 3) {
    Serial.println(F("ERR:MODE_RANGE (0-3)"));
    return;
  }

  spiSpeed = speed;
  spiMode = mode;

  // Update settings if SPI is active
  if (spiCSPin >= 0) {
    SPI.endTransaction();
    SPI.beginTransaction(SPISettings(spiSpeed, MSBFIRST, spiMode));
  }

  Serial.print(F("SPI settings: "));
  Serial.print(spiSpeed);
  Serial.print(F(" Hz, Mode "));
  Serial.println(spiMode);
}

// ========== EEPROM FUNCTIONS ==========

void handleEEPROMRead(String params) {
  int addr = parseHexOrDec(params);

  if (addr < 0 || addr >= EEPROM.length()) {
    Serial.print(F("ERR:ADDR_RANGE (0-"));
    Serial.print(EEPROM.length() - 1);
    Serial.println(F(")"));
    return;
  }

  uint8_t value = EEPROM.read(addr);

  Serial.print(F("EEPROM["));
  Serial.print(addr);
  Serial.print(F("] = 0x"));
  if (value < 16) Serial.print("0");
  Serial.print(value, HEX);
  Serial.print(F(" ("));
  Serial.print(value);
  Serial.println(F(")"));
}

void handleEEPROMWrite(String params) {
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMWRITE <addr> <value>"));
    return;
  }

  int addr = parseHexOrDec(params.substring(0, sp));
  uint8_t value = parseHexOrDec(params.substring(sp + 1));

  if (addr < 0 || addr >= EEPROM.length()) {
    Serial.print(F("ERR:ADDR_RANGE (0-"));
    Serial.print(EEPROM.length() - 1);
    Serial.println(F(")"));
    return;
  }

  // Check write protection
  if (isProtected(addr)) {
    Serial.print(F("ERR:PROTECTED_ADDR "));
    Serial.println(addr);
    return;
  }

  EEPROM.write(addr, value);

  Serial.print(F("EEPROM["));
  Serial.print(addr);
  Serial.print(F("] = 0x"));
  if (value < 16) Serial.print("0");
  Serial.println(value, HEX);
}

void handleEEPROMDump(String params) {
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMDUMP <start> <end>"));
    return;
  }

  int startAddr = parseHexOrDec(params.substring(0, sp));
  int endAddr = parseHexOrDec(params.substring(sp + 1));

  if (startAddr < 0 || endAddr >= EEPROM.length() || startAddr > endAddr) {
    Serial.println(F("ERR:INVALID_RANGE"));
    return;
  }

  Serial.println(F("EEPROM Dump:"));
  Serial.println(F("Addr  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
  Serial.println(F("------+------------------------------------------------"));

  for (int addr = startAddr; addr <= endAddr; addr += 16) {
    // Print address
    if (addr < 0x1000) Serial.print("0");
    if (addr < 0x100) Serial.print("0");
    if (addr < 0x10) Serial.print("0");
    Serial.print(addr, HEX);
    Serial.print(F("  | "));

    // Print 16 bytes
    for (int i = 0; i < 16; i++) {
      int a = addr + i;
      if (a <= endAddr) {
        uint8_t b = EEPROM.read(a);
        if (b < 16) Serial.print("0");
        Serial.print(b, HEX);
      } else {
        Serial.print(F("  "));
      }
      Serial.print(F(" "));
    }
    Serial.println();
  }
}

void handleEEPROMClear() {
  Serial.print(F("Clearing EEPROM ("));
  Serial.print(EEPROM.length());
  Serial.println(F(" bytes)..."));

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);
    if (i % 256 == 0) {
      Serial.print(F("."));
    }
  }

  Serial.println();
  Serial.println(F("EEPROM cleared (all 0xFF)"));
}

// Helper function to parse hex (0x prefix) or decimal numbers
int parseHexOrDec(String str) {
  str.trim();
  if (str.startsWith("0x") || str.startsWith("0X")) {
    return strtol(str.c_str() + 2, NULL, 16);
  } else {
    return str.toInt();
  }
}

// ========== ANALOG INPUT FUNCTIONS ==========

void handleAnalogRead(String params) {
  params.trim();
  params.toUpperCase();

  // Parse pin number - accept "A8" or "8" for A8
  int pinNum = -1;
  if (params.startsWith("A")) {
    pinNum = params.substring(1).toInt();
  } else {
    pinNum = params.toInt();
  }

  // Validate pin range (8-15 for A8-A15)
  if (pinNum < 8 || pinNum > 15) {
    Serial.println(F("ERR:PIN_RANGE (A8-A15 or 8-15)"));
    return;
  }

  // Get the actual analog pin (A8-A15)
  uint8_t analogPin = analogPins[pinNum - 8];

  // Read analog value with averaging
  uint32_t sum = 0;
  for (uint8_t i = 0; i < analogAvgSamples; i++) {
    sum += analogRead(analogPin);
    if (analogAvgSamples > 1) delay(1);  // Small delay between samples
  }
  uint16_t value = sum / analogAvgSamples;

  // Calculate voltage based on reference
  float voltage;
  if (analogRefMode == AREF_INTERNAL) {
    voltage = (value * 1.1) / 1023.0;  // 1.1V internal reference
  } else {
    voltage = (value * 5.0) / 1023.0;  // AREF_DEFAULT = 5V
  }

  // Print result
  Serial.print(F("A"));
  Serial.print(pinNum);
  Serial.print(F(" = "));
  Serial.print(value);
  Serial.print(F(" ("));
  Serial.print(voltage, 3);
  Serial.print(F("V)"));
  if (analogAvgSamples > 1) {
    Serial.print(F(" [avg "));
    Serial.print(analogAvgSamples);
    Serial.print(F("]"));
  }
  Serial.println();
}

void handleAnalogReadAll() {
  Serial.println(F("Analog Inputs (A8-A15):"));

  // Determine voltage reference string
  const char* refStr;
  float refVoltage;
  if (analogRefMode == AREF_INTERNAL) {
    refStr = "INTERNAL";
    refVoltage = 1.1;
  } else {
    refStr = "DEFAULT";
    refVoltage = 5.0;
  }

  Serial.print(F("  Reference: "));
  Serial.print(refStr);
  Serial.print(F(" ("));
  Serial.print(refVoltage, 1);
  Serial.print(F("V), Averaging: "));
  Serial.println(analogAvgSamples);

  // Read all 8 pins
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t pinNum = i + 8;  // A8-A15
    uint8_t analogPin = analogPins[i];

    // Read with averaging
    uint32_t sum = 0;
    for (uint8_t j = 0; j < analogAvgSamples; j++) {
      sum += analogRead(analogPin);
      if (analogAvgSamples > 1) delay(1);
    }
    uint16_t value = sum / analogAvgSamples;

    // Calculate voltage
    float voltage = (value * refVoltage) / 1023.0;

    // Print
    Serial.print(F("  A"));
    Serial.print(pinNum);
    Serial.print(F(": "));
    if (value < 10) Serial.print(F("   "));
    else if (value < 100) Serial.print(F("  "));
    else if (value < 1000) Serial.print(F(" "));
    Serial.print(value);
    Serial.print(F(" = "));
    Serial.print(voltage, 3);
    Serial.println(F("V"));
  }
}

void handleAnalogRef(String params) {
  params.trim();
  params.toUpperCase();

  if (params == "?") {
    // Query current reference
    Serial.print(F("ANALOGREF="));
    if (analogRefMode == AREF_INTERNAL) {
      Serial.println(F("INTERNAL (1.1V)"));
    } else {
      Serial.println(F("DEFAULT (5V)"));
    }
    return;
  }

  // Set reference
  if (params == "DEFAULT") {
    analogRefMode = AREF_DEFAULT;
    analogReference(DEFAULT);
    Serial.println(F("Analog reference: DEFAULT (5V)"));
  } else if (params == "INTERNAL") {
    analogRefMode = AREF_INTERNAL;
    analogReference(INTERNAL1V1);  // Mega uses INTERNAL1V1
    Serial.println(F("Analog reference: INTERNAL (1.1V)"));
    Serial.println(F("WARNING: Max input 1.1V!"));
  } else if (params == "EXTERNAL") {
    Serial.println(F("ERR:EXTERNAL_NOT_AVAILABLE"));
    Serial.println(F("AREF pin used by LCD shield"));
    Serial.println(F("Use DEFAULT or INTERNAL only"));
  } else {
    Serial.println(F("ERR:USE DEFAULT|INTERNAL"));
  }
}

void handleAnalogAvg(String params) {
  int samples = params.toInt();

  if (samples < 1 || samples > 16) {
    Serial.println(F("ERR:RANGE (1-16 samples)"));
    return;
  }

  analogAvgSamples = samples;

  Serial.print(F("Analog averaging: "));
  Serial.print(analogAvgSamples);
  Serial.println(F(" samples"));
  if (samples > 1) {
    Serial.println(F("Note: Adds ~1ms delay per sample"));
  }
}

// ========== BUTTON FUNCTIONS ==========

// Initialize button layout
void initButtons() {
  // Calculate button layout in bottom section
  // Reserve bottom 50 pixels for buttons (larger for 480px height)
  uint16_t btnHeight = 45;
  uint16_t btnWidth = (screenW - 15) / 4;  // 4 buttons with small gaps
  uint16_t btnY = screenH - btnHeight - 2;  // Use screenH, not bottomMaxY!
  buttonTextY = btnY - 2;  // Text area ends just above buttons

  // UP button - uses btnUp config
  buttons[0].x = 5;
  buttons[0].y = btnY;
  buttons[0].w = btnWidth;
  buttons[0].h = btnHeight;
  buttons[0].label = "UP";
  buttons[0].cmdLen = btnUp.length;
  for (uint8_t i = 0; i < btnUp.length && i < 15; i++) {
    buttons[0].cmdBytes[i] = btnUp.data[i];
  }
  buttons[0].color = 0x07E0;  // Green

  // DOWN button - uses btnDown config
  buttons[1].x = 5 + btnWidth + 3;
  buttons[1].y = btnY;
  buttons[1].w = btnWidth;
  buttons[1].h = btnHeight;
  buttons[1].label = "DN";
  buttons[1].cmdLen = btnDown.length;
  for (uint8_t i = 0; i < btnDown.length && i < 15; i++) {
    buttons[1].cmdBytes[i] = btnDown.data[i];
  }
  buttons[1].color = 0x07E0;  // Green

  // LEFT button - uses btnLeft config
  buttons[2].x = 5 + (btnWidth + 3) * 2;
  buttons[2].y = btnY;
  buttons[2].w = btnWidth;
  buttons[2].h = btnHeight;
  buttons[2].label = "LT";
  buttons[2].cmdLen = btnLeft.length;
  for (uint8_t i = 0; i < btnLeft.length && i < 15; i++) {
    buttons[2].cmdBytes[i] = btnLeft.data[i];
  }
  buttons[2].color = 0x07E0;  // Green

  // RIGHT button - uses btnRight config
  buttons[3].x = 5 + (btnWidth + 3) * 3;
  buttons[3].y = btnY;
  buttons[3].w = btnWidth;
  buttons[3].h = btnHeight;
  buttons[3].label = "RT";
  buttons[3].cmdLen = btnRight.length;
  for (uint8_t i = 0; i < btnRight.length && i < 15; i++) {
    buttons[3].cmdBytes[i] = btnRight.data[i];
  }
  buttons[3].color = 0x07E0;  // Green
}

// Draw a single button
void drawButton(uint8_t idx) {
  Button &btn = buttons[idx];

  // Draw button background
  tft.fillRect(btn.x, btn.y, btn.w, btn.h, btn.color);
  tft.drawRect(btn.x, btn.y, btn.w, btn.h, 0xFFFF);  // White border

  // Draw label centered
  tft.setTextColor(0x0000);  // Black text
  tft.setTextSize(2);

  // Calculate text position (centered)
  uint8_t labelLen = strlen(btn.label);
  int16_t textW = labelLen * 6 * 2;  // char width * size
  int16_t textH = 8 * 2;  // char height * size
  int16_t textX = btn.x + (btn.w - textW) / 2;
  int16_t textY = btn.y + (btn.h - textH) / 2;

  tft.setCursor(textX, textY);
  tft.print(btn.label);
}

// Show all buttons
void showButtons() {
  if (buttonsVisible) return;

  // Adjust bottom text area to make room for buttons
  bottomMaxY = buttonTextY;

  // Clear bottom section and redraw
  tft.fillRect(0, bottomMinY, screenW, screenH - bottomMinY, bottomBgColor);
  bottomPosX = 0;
  bottomPosY = bottomMinY;

  // Draw all buttons
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    drawButton(i);
  }

  buttonsVisible = true;
  drawDivider();

  // Debug: Show button positions for troubleshooting
  Serial.println(F("=== BUTTONS VISIBLE ==="));
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    Serial.print(F("BTN"));
    Serial.print(i);
    Serial.print(F(" ["));
    Serial.print(buttons[i].label);
    Serial.print(F("]: X="));
    Serial.print(buttons[i].x);
    Serial.print(F("-"));
    Serial.print(buttons[i].x + buttons[i].w);
    Serial.print(F(", Y="));
    Serial.print(buttons[i].y);
    Serial.print(F("-"));
    Serial.println(buttons[i].y + buttons[i].h);
  }
}

// Hide buttons
void hideButtons() {
  if (!buttonsVisible) return;

  // Restore full bottom area
  bottomMaxY = screenH;

  // Clear button area
  tft.fillRect(0, buttonTextY, screenW, screenH - buttonTextY, bottomBgColor);

  buttonsVisible = false;
  drawDivider();
}

// Check for touch and handle button presses
void checkTouch() {
  // Prevent too-frequent polling
  unsigned long now = millis();
  if (now - lastTouch < TOUCH_DEBOUNCE) return;

  // Get touch point
  TSPoint p = ts.getPoint();

  // Restore pins for LCD operation (touchscreen library changes pin modes)
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  digitalWrite(XM, HIGH);
  digitalWrite(YP, HIGH);

  // Debug: Show raw touch data (always, to see if touch is being read)
  Serial.print(F("[TOUCH] Raw: X="));
  Serial.print(p.x);
  Serial.print(F(" Y="));
  Serial.print(p.y);
  Serial.print(F(" Z="));
  Serial.print(p.z);
  Serial.print(F(" (Range: "));
  Serial.print(MINPRESSURE);
  Serial.print(F("-"));
  Serial.print(MAXPRESSURE);
  Serial.print(F(")"));

  // Check if touched
  if (p.z < MINPRESSURE || p.z > MAXPRESSURE) {
    Serial.println(F(" - NO TOUCH"));
    return;
  }
  Serial.println(F(" - TOUCH DETECTED!"));

  // Touch test mode - show raw coordinates
  if (touchTestMode) {
    lastTouch = now;
    Serial.print(F("[TOUCH] Raw: X="));
    Serial.print(p.x);
    Serial.print(F(" Y="));
    Serial.print(p.y);
    Serial.print(F(" Z="));
    Serial.print(p.z);

    // Also show mapped coordinates
    uint8_t rotation = tft.getRotation();
    int16_t px, py;
    switch (rotation) {
      case 0:  // Portrait
        px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
        py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
        break;
      case 1:  // Landscape
        px = map(p.y, TS_MINY, TS_MAXY, 0, screenW);
        py = map(p.x, TS_MAXX, TS_MINX, 0, screenH);
        break;
      case 2:  // Portrait inverted
        px = map(p.x, TS_MAXX, TS_MINX, 0, screenW);
        py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
        break;
      case 3:  // Landscape inverted
        px = map(p.y, TS_MAXY, TS_MINY, 0, screenW);
        py = map(p.x, TS_MINX, TS_MAXX, 0, screenH);
        break;
      default:
        px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
        py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
        break;
    }

    Serial.print(F(" | Mapped: X="));
    Serial.print(px);
    Serial.print(F(" Y="));
    Serial.print(py);
    Serial.print(F(" | Rot="));
    Serial.println(rotation);
    return;
  }

  // Map touch coordinates to screen coordinates based on rotation
  int16_t px, py;
  uint8_t rotation = tft.getRotation();

  switch (rotation) {
    case 0:  // Portrait
      px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
      py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
      break;
    case 1:  // Landscape
      px = map(p.y, TS_MINY, TS_MAXY, 0, screenW);
      py = map(p.x, TS_MAXX, TS_MINX, 0, screenH);
      break;
    case 2:  // Portrait inverted
      px = map(p.x, TS_MAXX, TS_MINX, 0, screenW);
      py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
      break;
    case 3:  // Landscape inverted
      px = map(p.y, TS_MAXY, TS_MINY, 0, screenW);
      py = map(p.x, TS_MINX, TS_MAXX, 0, screenH);
      break;
    default:
      px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
      py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
      break;
  }

  // Debug: Show mapped touch coordinates and calibration
  Serial.print(F("[TOUCH] Mapped: X="));
  Serial.print(px);
  Serial.print(F(" Y="));
  Serial.print(py);
  Serial.print(F(" | Rot="));
  Serial.print(rotation);
  Serial.print(F(" | Cal: X["));
  Serial.print(TS_MINX);
  Serial.print(F("-"));
  Serial.print(TS_MAXX);
  Serial.print(F("] Y["));
  Serial.print(TS_MINY);
  Serial.print(F("-"));
  Serial.print(TS_MAXY);
  Serial.println(F("]"));

  // Check if touch is within any button
  bool buttonHit = false;
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    Button &btn = buttons[i];

    // Debug: Show button check
    Serial.print(F("  Checking BTN"));
    Serial.print(i);
    Serial.print(F(" ["));
    Serial.print(btn.label);
    Serial.print(F("]: X="));
    Serial.print(btn.x);
    Serial.print(F("-"));
    Serial.print(btn.x + btn.w);
    Serial.print(F(", Y="));
    Serial.print(btn.y);
    Serial.print(F("-"));
    Serial.print(btn.y + btn.h);

    if (px >= btn.x && px < (btn.x + btn.w) &&
        py >= btn.y && py < (btn.y + btn.h)) {
      Serial.println(F(" --> HIT!"));
      buttonHit = true;

      // Button pressed!
      lastTouch = now;

      // Visual feedback - briefly invert button
      tft.fillRect(btn.x + 1, btn.y + 1, btn.w - 2, btn.h - 2, 0xFFFF);
      delay(50);
      drawButton(i);

      // Send byte array to selected FPGA
      HardwareSerial& fpga = getFPGA();
      for (uint8_t j = 0; j < btn.cmdLen; j++) {
        fpga.write(btn.cmdBytes[j]);
      }

      // Apply termination after button bytes
      applyTermination(fpga);

      // Log to serial (show bytes in hex)
      Serial.print(F("[BTN>FPGA"));
      Serial.print(activeFpga);
      Serial.print(F("] "));
      Serial.print(btn.label);
      Serial.print(F(" = "));

      // Build hex string for display
      String hexDisplay = "";
      for (uint8_t j = 0; j < btn.cmdLen; j++) {
        if (j > 0) {
          Serial.print(F(" "));
          hexDisplay += " ";
        }
        Serial.print(F("0x"));
        if (btn.cmdBytes[j] < 16) Serial.print(F("0"));
        Serial.print(btn.cmdBytes[j], HEX);

        hexDisplay += "0x";
        if (btn.cmdBytes[j] < 16) hexDisplay += "0";
        hexDisplay += String(btn.cmdBytes[j], HEX);
      }
      Serial.println();

      // Show on bottom screen what was SENT
      String msg = "TX: " + hexDisplay;
      showTextBottom(msg);

      break;  // Only process one button per touch
    } else {
      Serial.println(F(" - miss"));
    }
  }

  if (!buttonHit) {
    Serial.println(F("[TOUCH] No button was hit at these coordinates"));
  }
}

// ========== GPIO FUNCTIONS ==========

// Initialize GPIO system
void initGPIO() {
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    gpioModes[i] = GPIO_INPUT;
    pinMode(gpioPins[i], INPUT);
    gpioStates[i] = digitalRead(gpioPins[i]);
    gpioPrevStates[i] = gpioStates[i];
    gpioEventEnable[i] = false;
    gpioEventRising[i] = false;
    gpioEventFalling[i] = false;
  }
  Serial.println(F("GPIO: Pins 22-29 initialized"));
}

// Poll GPIO pins for changes (called in loop)
void pollGPIO() {
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    // Only poll input pins
    if (gpioModes[i] != GPIO_OUTPUT) {
      uint8_t newState = digitalRead(gpioPins[i]);

      if (newState != gpioPrevStates[i]) {
        // State changed!
        gpioStates[i] = newState;

        // Check if event reporting is enabled
        if (gpioEventEnable[i]) {
          bool rising = (newState == HIGH && gpioPrevStates[i] == LOW);
          bool falling = (newState == LOW && gpioPrevStates[i] == HIGH);

          // Queue event if configured
          if ((rising && gpioEventRising[i]) || (falling && gpioEventFalling[i])) {
            queueGPIOEvent(i, newState, rising);
          }
        }

        gpioPrevStates[i] = newState;
      }
    }
  }
}

// Queue a GPIO event
void queueGPIOEvent(uint8_t gpioIdx, uint8_t newState, bool rising) {
  uint8_t nextHead = (gpioEventHead + 1) % GPIO_EVENT_QUEUE_SIZE;
  if (nextHead != gpioEventTail) {  // Not full
    gpioEventQueue[gpioEventHead].pin = gpioIdx;
    gpioEventQueue[gpioEventHead].newState = newState;
    gpioEventQueue[gpioEventHead].rising = rising;
    gpioEventHead = nextHead;
  }
}

// Process GPIO events (called in loop)
void processGPIOEvents() {
  while (gpioEventTail != gpioEventHead) {
    GPIOEvent &evt = gpioEventQueue[gpioEventTail];

    // Display event on LCD
    String msg = "GPIO";
    msg += gpioPins[evt.pin];
    msg += evt.rising ? " RISE" : " FALL";
    showTextBottom(msg);

    // Log to serial
    Serial.print(F("[GPIO"));
    Serial.print(gpioPins[evt.pin]);
    Serial.print(F("] "));
    Serial.println(evt.rising ? F("RISING") : F("FALLING"));

    gpioEventTail = (gpioEventTail + 1) % GPIO_EVENT_QUEUE_SIZE;
  }
}

// Set GPIO mode
void setGPIOMode(uint8_t pin, String& mode) {
  // Find GPIO index
  int8_t idx = -1;
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioPins[i] == pin) {
      idx = i;
      break;
    }
  }

  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_29_ONLY"));
    return;
  }

  mode.trim();
  if (mode == "IN") {
    gpioModes[idx] = GPIO_INPUT;
    pinMode(pin, INPUT);
    gpioStates[idx] = digitalRead(pin);
    gpioPrevStates[idx] = gpioStates[idx];
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_IN"));
  } else if (mode == "INPU" || mode == "PULLUP") {
    gpioModes[idx] = GPIO_INPUT_PULLUP;
    pinMode(pin, INPUT_PULLUP);
    gpioStates[idx] = digitalRead(pin);
    gpioPrevStates[idx] = gpioStates[idx];
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_INPU"));
  } else if (mode == "OUT") {
    gpioModes[idx] = GPIO_OUTPUT;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, gpioStates[idx]);  // Maintain current state
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_OUT"));
  } else {
    Serial.println(F("ERR:USE_IN_INPU_OUT"));
  }
}

// Write GPIO output
void writeGPIO(uint8_t pin, String& val) {
  // Find GPIO index
  int8_t idx = -1;
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioPins[i] == pin) {
      idx = i;
      break;
    }
  }

  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_29_ONLY"));
    return;
  }

  if (gpioModes[idx] != GPIO_OUTPUT) {
    Serial.println(F("ERR:NOT_OUTPUT"));
    return;
  }

  val.trim();
  uint8_t state = 0;
  if (val == "1" || val == "HIGH" || val == "H") {
    state = HIGH;
  } else if (val == "0" || val == "LOW" || val == "L") {
    state = LOW;
  } else {
    Serial.println(F("ERR:USE_0_1_HIGH_LOW"));
    return;
  }

  digitalWrite(pin, state);
  gpioStates[idx] = state;
  Serial.print(F("OK:GPIO"));
  Serial.print(pin);
  Serial.print(F("="));
  Serial.println(state);
}

// Read GPIO input
void readGPIO(uint8_t pin) {
  // Find GPIO index
  int8_t idx = -1;
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioPins[i] == pin) {
      idx = i;
      break;
    }
  }

  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_29_ONLY"));
    return;
  }

  uint8_t state = digitalRead(pin);
  gpioStates[idx] = state;

  Serial.print(F("GPIO"));
  Serial.print(pin);
  Serial.print(F("="));
  Serial.println(state);
}

// Read all GPIO pins
void readAllGPIO() {
  Serial.print(F("GPIO: "));
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    uint8_t state = digitalRead(gpioPins[i]);
    gpioStates[i] = state;
    Serial.print(gpioPins[i]);
    Serial.print(F("="));
    Serial.print(state);
    if (i < NUM_GPIO - 1) Serial.print(F(" "));
  }
  Serial.println();
}

// Set GPIO event configuration
void setGPIOEvent(uint8_t pin, String& type) {
  // Find GPIO index
  int8_t idx = -1;
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioPins[i] == pin) {
      idx = i;
      break;
    }
  }

  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_29_ONLY"));
    return;
  }

  type.trim();
  if (type == "NONE") {
    gpioEventEnable[idx] = false;
    gpioEventRising[idx] = false;
    gpioEventFalling[idx] = false;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_NONE"));
  } else if (type == "RISING" || type == "RISE") {
    gpioEventEnable[idx] = true;
    gpioEventRising[idx] = true;
    gpioEventFalling[idx] = false;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_RISE"));
  } else if (type == "FALLING" || type == "FALL") {
    gpioEventEnable[idx] = true;
    gpioEventRising[idx] = false;
    gpioEventFalling[idx] = true;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_FALL"));
  } else if (type == "BOTH" || type == "CHANGE") {
    gpioEventEnable[idx] = true;
    gpioEventRising[idx] = true;
    gpioEventFalling[idx] = true;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_BOTH"));
  } else {
    Serial.println(F("ERR:USE_NONE_RISING_FALLING_BOTH"));
  }
}

// Show GPIO register (8 pins as 8-bit value)
void showGPIORegister() {
  uint8_t regVal = 0;
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioStates[i]) {
      regVal |= (1 << i);
    }
  }

  Serial.print(F("GPIOREG=0x"));
  if (regVal < 16) Serial.print(F("0"));
  Serial.print(regVal, HEX);
  Serial.print(F(" ("));
  Serial.print(regVal);
  Serial.print(F(") BIN="));
  for (int i = 7; i >= 0; i--) {
    Serial.print((regVal >> i) & 1);
  }
  Serial.println();
}

// Set GPIO register (8 pins at once, outputs only)
void setGPIORegister(uint8_t regVal) {
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioModes[i] == GPIO_OUTPUT) {
      uint8_t bitVal = (regVal >> i) & 1;
      digitalWrite(gpioPins[i], bitVal);
      gpioStates[i] = bitVal;
    }
  }

  Serial.print(F("OK:GPIOSET=0x"));
  if (regVal < 16) Serial.print(F("0"));
  Serial.println(regVal, HEX);
}

// ========== DISPLAY WIDGET FUNCTIONS ==========

// Draw circular gauge: #GAUGE <x> <y> <radius> <value> <max>
void handleGauge(String params) {
  int vals[5];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 5; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 4) {
    Serial.println(F("ERR:FORMAT #GAUGE <x> <y> <r> <val> [max]"));
    return;
  }

  int16_t cx = vals[0];
  int16_t cy = vals[1];
  int16_t r = vals[2];
  int16_t value = vals[3];
  int16_t maxVal = (count >= 5) ? vals[4] : 100;

  // Constrain value
  value = constrain(value, 0, maxVal);

  // Draw gauge background (arc from 135° to 45° = 270° total)
  // Draw outer circle
  tft.drawCircle(cx, cy, r, topTextColor);
  tft.drawCircle(cx, cy, r - 1, topTextColor);

  // Calculate angle (135° to 45°, clockwise)
  // 135° = bottom-left, 45° = bottom-right
  // Map value to angle
  int angle = map(value, 0, maxVal, 135, 405);  // 405 = 45 + 360
  if (angle >= 360) angle -= 360;

  // Draw tick marks (every 30°)
  for (int a = 135; a <= 405; a += 30) {
    int actualAngle = a >= 360 ? a - 360 : a;
    float rad = actualAngle * PI / 180.0;
    int16_t x1 = cx + (r - 5) * cos(rad);
    int16_t y1 = cy + (r - 5) * sin(rad);
    int16_t x2 = cx + r * cos(rad);
    int16_t y2 = cy + r * sin(rad);
    tft.drawLine(x1, y1, x2, y2, topTextColor);
  }

  // Draw needle
  float rad = angle * PI / 180.0;
  int16_t needleX = cx + (r - 10) * cos(rad);
  int16_t needleY = cy + (r - 10) * sin(rad);
  tft.drawLine(cx, cy, needleX, needleY, 0xF800);  // Red needle
  tft.fillCircle(cx, cy, 3, 0xF800);  // Red center

  // Draw value text
  tft.setTextSize(1);
  tft.setTextColor(topTextColor);
  tft.setCursor(cx - 12, cy + r - 15);
  tft.print(value);

  Serial.print(F("OK:GAUGE "));
  Serial.print(value);
  Serial.print(F("/"));
  Serial.println(maxVal);
}

// Draw bar graph: #BARGRAPH <x> <y> <w> <h> <val1> <val2> ...
void handleBarGraph(String params) {
  int vals[20];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 20; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 5) {
    Serial.println(F("ERR:FORMAT #BARGRAPH <x> <y> <w> <h> <val1> ..."));
    return;
  }

  int16_t x = vals[0];
  int16_t y = vals[1];
  int16_t w = vals[2];
  int16_t h = vals[3];
  int numBars = count - 4;

  // Find max value for scaling
  int maxVal = 0;
  for (int i = 0; i < numBars; i++) {
    if (vals[4 + i] > maxVal) maxVal = vals[4 + i];
  }
  if (maxVal == 0) maxVal = 100;

  // Draw border
  tft.drawRect(x, y, w, h, topTextColor);

  // Calculate bar width
  int barWidth = (w - (numBars + 1) * 2) / numBars;
  if (barWidth < 1) barWidth = 1;

  // Draw bars
  for (int i = 0; i < numBars; i++) {
    int barX = x + 2 + i * (barWidth + 2);
    int value = vals[4 + i];
    int barHeight = map(value, 0, maxVal, 0, h - 4);

    // Draw bar from bottom up
    uint16_t barColor = (i % 3 == 0) ? 0x07E0 : (i % 3 == 1) ? 0x07FF : 0xFFE0;
    tft.fillRect(barX, y + h - 2 - barHeight, barWidth, barHeight, barColor);
  }

  Serial.print(F("OK:BARGRAPH "));
  Serial.print(numBars);
  Serial.println(F(" bars"));
}

// Draw large numeric display: #NUMBOX <x> <y> <value> [decimals]
void handleNumBox(String params) {
  int vals[4];
  int count = 0;
  int lastPos = 0;
  String valStr = "";

  // Parse parameters
  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        if (count == 0) vals[count++] = params.substring(lastPos, i).toInt();  // x
        else if (count == 1) vals[count++] = params.substring(lastPos, i).toInt();  // y
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #NUMBOX <x> <y> <value> [decimals]"));
    return;
  }

  // Get value string (rest of params)
  valStr = params.substring(lastPos);
  valStr.trim();

  int16_t x = vals[0];
  int16_t y = vals[1];

  // Draw large number
  tft.setTextSize(4);
  tft.setTextColor(topTextColor, topBgColor);
  tft.setCursor(x, y);
  tft.print(valStr);

  Serial.print(F("OK:NUMBOX "));
  Serial.println(valStr);
}

// Draw trend/line graph: #TREND <x> <y> <w> <h> <val1> <val2> ...
void handleTrend(String params) {
  int vals[50];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 50; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 6) {
    Serial.println(F("ERR:FORMAT #TREND <x> <y> <w> <h> <val1> <val2> ..."));
    return;
  }

  int16_t x = vals[0];
  int16_t y = vals[1];
  int16_t w = vals[2];
  int16_t h = vals[3];
  int numPoints = count - 4;

  // Find min/max for scaling
  int minVal = vals[4];
  int maxVal = vals[4];
  for (int i = 1; i < numPoints; i++) {
    if (vals[4 + i] < minVal) minVal = vals[4 + i];
    if (vals[4 + i] > maxVal) maxVal = vals[4 + i];
  }
  if (maxVal == minVal) maxVal = minVal + 1;

  // Draw border and grid
  tft.drawRect(x, y, w, h, topTextColor);

  // Draw center line
  tft.drawLine(x, y + h / 2, x + w, y + h / 2, 0x39E7);  // Gray

  // Plot points
  int xStep = (w - 4) / (numPoints - 1);
  for (int i = 0; i < numPoints - 1; i++) {
    int x1 = x + 2 + i * xStep;
    int y1 = y + 2 + map(vals[4 + i], minVal, maxVal, h - 4, 0);
    int x2 = x + 2 + (i + 1) * xStep;
    int y2 = y + 2 + map(vals[4 + i + 1], minVal, maxVal, h - 4, 0);

    tft.drawLine(x1, y1, x2, y2, topTextColor);
    tft.fillCircle(x1, y1, 2, topTextColor);  // Data point marker
  }

  // Draw last point
  int xLast = x + 2 + (numPoints - 1) * xStep;
  int yLast = y + 2 + map(vals[4 + numPoints - 1], minVal, maxVal, h - 4, 0);
  tft.fillCircle(xLast, yLast, 2, topTextColor);

  Serial.print(F("OK:TREND "));
  Serial.print(numPoints);
  Serial.println(F(" points"));
}

// ========== DATA LOGGING FUNCTIONS ==========

void updateDataLogger() {
  unsigned long now = millis();

  if (now - lastLogTime >= logInterval) {
    lastLogTime = now;

    // Read value based on source
    uint16_t value = 0;
    if (logSource <= 7) {
      // Analog pin A8-A15
      value = analogRead(analogPins[logSource]);
    } else if (logSource == 15) {
      // GPIO register
      value = 0;
      for (uint8_t i = 0; i < NUM_GPIO; i++) {
        if (gpioModes[i] != GPIO_OUTPUT) {
          value |= (digitalRead(gpioPins[i]) << i);
        } else {
          value |= (gpioStates[i] << i);
        }
      }
    }

    // Write log entry to EEPROM
    // Format: [timestamp:4][value:2][source:1][flags:1]
    uint16_t addr = logWriteAddr;

    // Write timestamp (4 bytes)
    EEPROM.write(addr++, (now >> 24) & 0xFF);
    EEPROM.write(addr++, (now >> 16) & 0xFF);
    EEPROM.write(addr++, (now >> 8) & 0xFF);
    EEPROM.write(addr++, now & 0xFF);

    // Write value (2 bytes)
    EEPROM.write(addr++, (value >> 8) & 0xFF);
    EEPROM.write(addr++, value & 0xFF);

    // Write source (1 byte)
    EEPROM.write(addr++, logSource);

    // Write flags (1 byte) - reserved
    EEPROM.write(addr++, 0);

    // Update write address (circular buffer)
    logWriteAddr = addr;
    if (logWriteAddr > logEndAddr - LOG_ENTRY_SIZE) {
      logWriteAddr = logStartAddr;
    }

    logEntryCount++;
    if (logEntryCount > (logEndAddr - logStartAddr + 1) / LOG_ENTRY_SIZE) {
      logEntryCount = (logEndAddr - logStartAddr + 1) / LOG_ENTRY_SIZE;
    }
  }
}

void handleLogStart(String params) {
  // Parse: <interval> <source>
  // source: 0-7=A8-A15, 15=GPIO_REG
  int sp = params.indexOf(' ');

  if (sp == -1) {
    logInterval = params.toInt();
    // Keep current source
  } else {
    logInterval = params.substring(0, sp).toInt();
    logSource = params.substring(sp + 1).toInt();
  }

  if (logInterval < 10) logInterval = 10;  // Min 10ms
  if (logSource > 15) logSource = 0;

  // Initialize logging position
  logWriteAddr = logStartAddr;
  logEntryCount = 0;

  loggingActive = true;
  lastLogTime = millis();

  Serial.print(F("LOG_START: "));
  Serial.print(logInterval);
  Serial.print(F("ms, source="));
  if (logSource <= 7) {
    Serial.print(F("A"));
    Serial.println(logSource + 8);
  } else if (logSource == 15) {
    Serial.println(F("GPIO_REG"));
  } else {
    Serial.println(logSource);
  }
}

void handleLogStop() {
  loggingActive = false;
  Serial.println(F("LOG_STOPPED"));
  Serial.print(F("Entries logged: "));
  Serial.println(logEntryCount);
}

void handleLogRead(String params) {
  uint16_t numEntries = 10;  // Default
  if (params.length() > 0) {
    numEntries = params.toInt();
  }

  if (numEntries > logEntryCount) numEntries = logEntryCount;

  Serial.print(F("=== LOG DATA ("));
  Serial.print(numEntries);
  Serial.println(F(" entries) ==="));
  Serial.println(F("Time(ms)   Value  Source"));

  // Read from oldest to newest
  uint16_t readAddr = logWriteAddr - (numEntries * LOG_ENTRY_SIZE);
  if (readAddr < logStartAddr) {
    readAddr = logEndAddr - (logStartAddr - readAddr) + 1;
  }

  for (uint16_t i = 0; i < numEntries; i++) {
    // Read timestamp
    unsigned long timestamp = 0;
    timestamp |= ((unsigned long)EEPROM.read(readAddr++)) << 24;
    timestamp |= ((unsigned long)EEPROM.read(readAddr++)) << 16;
    timestamp |= ((unsigned long)EEPROM.read(readAddr++)) << 8;
    timestamp |= EEPROM.read(readAddr++);

    // Read value
    uint16_t value = 0;
    value |= ((uint16_t)EEPROM.read(readAddr++)) << 8;
    value |= EEPROM.read(readAddr++);

    // Read source
    uint8_t source = EEPROM.read(readAddr++);

    // Skip flags
    readAddr++;

    // Wrap address
    if (readAddr > logEndAddr) {
      readAddr = logStartAddr;
    }

    // Print entry
    Serial.print(timestamp);
    Serial.print(F("\t"));
    Serial.print(value);
    Serial.print(F("\t"));
    if (source <= 7) {
      Serial.print(F("A"));
      Serial.println(source + 8);
    } else if (source == 15) {
      Serial.println(F("GPIO"));
    } else {
      Serial.println(source);
    }
  }
}

void handleLogClear() {
  logWriteAddr = logStartAddr;
  logEntryCount = 0;
  Serial.println(F("LOG_CLEARED"));
}

void handleLogConfig(String params) {
  // #LOGCONFIG <source>
  logSource = params.toInt();
  if (logSource > 15) logSource = 0;

  Serial.print(F("LOG_SOURCE="));
  if (logSource <= 7) {
    Serial.print(F("A"));
    Serial.println(logSource + 8);
  } else if (logSource == 15) {
    Serial.println(F("GPIO_REG"));
  } else {
    Serial.println(logSource);
  }
}

void handleLogStatus() {
  Serial.println(F("=== LOG STATUS ==="));
  Serial.print(F("Active: "));
  Serial.println(loggingActive ? F("YES") : F("NO"));
  Serial.print(F("Interval: "));
  Serial.print(logInterval);
  Serial.println(F("ms"));
  Serial.print(F("Source: "));
  if (logSource <= 7) {
    Serial.print(F("A"));
    Serial.println(logSource + 8);
  } else if (logSource == 15) {
    Serial.println(F("GPIO_REG"));
  } else {
    Serial.println(logSource);
  }
  Serial.print(F("Entries: "));
  Serial.println(logEntryCount);
  Serial.print(F("Next addr: 0x"));
  Serial.println(logWriteAddr, HEX);
}

void handleLogZone(String params) {
  // #LOGZONE <start> <end> - Configure logging address range
  // #LOGZONE ? - Query current range

  if (params == "?") {
    Serial.print(F("LOG_ZONE: 0x"));
    Serial.print(logStartAddr, HEX);
    Serial.print(F(" - 0x"));
    Serial.println(logEndAddr, HEX);
    uint16_t capacity = (logEndAddr - logStartAddr + 1) / LOG_ENTRY_SIZE;
    Serial.print(F("Capacity: "));
    Serial.print(capacity);
    Serial.println(F(" entries"));
    return;
  }

  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #LOGZONE <start> <end>"));
    return;
  }

  uint16_t start = parseHexOrDec(params.substring(0, sp));
  uint16_t end = parseHexOrDec(params.substring(sp + 1));

  if (start >= EEPROM.length() || end >= EEPROM.length()) {
    Serial.println(F("ERR:ADDR_RANGE"));
    return;
  }

  if (start >= end) {
    Serial.println(F("ERR:START >= END"));
    return;
  }

  // Check if any protection zones overlap
  for (uint8_t i = 0; i < MAX_PROTECT_ZONES; i++) {
    if (protectZones[i].enabled) {
      if ((start >= protectZones[i].startAddr && start <= protectZones[i].endAddr) ||
          (end >= protectZones[i].startAddr && end <= protectZones[i].endAddr)) {
        Serial.print(F("ERR:OVERLAPS_ZONE "));
        Serial.println(i);
        return;
      }
    }
  }

  logStartAddr = start;
  logEndAddr = end;
  logWriteAddr = logStartAddr;
  logEntryCount = 0;

  Serial.print(F("LOG_ZONE: 0x"));
  Serial.print(logStartAddr, HEX);
  Serial.print(F(" - 0x"));
  Serial.println(logEndAddr, HEX);
}

void handleEEPROMProtect(String params) {
  // #EEPROMPROTECT <zone> <start> <end> - Configure protection zone
  // #EEPROMPROTECT <zone> OFF - Disable protection zone
  // #EEPROMPROTECT ? - Show all protection zones

  if (params == "?") {
    Serial.println(F("=== EEPROM PROTECTION ==="));
    for (uint8_t i = 0; i < MAX_PROTECT_ZONES; i++) {
      Serial.print(F("Zone "));
      Serial.print(i);
      Serial.print(F(": "));
      if (protectZones[i].enabled) {
        Serial.print(F("0x"));
        Serial.print(protectZones[i].startAddr, HEX);
        Serial.print(F(" - 0x"));
        Serial.println(protectZones[i].endAddr, HEX);
      } else {
        Serial.println(F("DISABLED"));
      }
    }
    return;
  }

  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMPROTECT <zone> <start> <end>"));
    return;
  }

  uint8_t zone = params.substring(0, sp).toInt();
  if (zone >= MAX_PROTECT_ZONES) {
    Serial.print(F("ERR:ZONE_RANGE (0-"));
    Serial.print(MAX_PROTECT_ZONES - 1);
    Serial.println(F(")"));
    return;
  }

  String rest = params.substring(sp + 1);
  rest.trim();

  if (rest.equalsIgnoreCase("OFF")) {
    protectZones[zone].enabled = false;
    Serial.print(F("Zone "));
    Serial.print(zone);
    Serial.println(F(" DISABLED"));
    return;
  }

  sp = rest.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMPROTECT <zone> <start> <end>"));
    return;
  }

  uint16_t start = parseHexOrDec(rest.substring(0, sp));
  uint16_t end = parseHexOrDec(rest.substring(sp + 1));

  if (start >= EEPROM.length() || end >= EEPROM.length()) {
    Serial.println(F("ERR:ADDR_RANGE"));
    return;
  }

  if (start > end) {
    Serial.println(F("ERR:START > END"));
    return;
  }

  protectZones[zone].startAddr = start;
  protectZones[zone].endAddr = end;
  protectZones[zone].enabled = true;

  Serial.print(F("Zone "));
  Serial.print(zone);
  Serial.print(F(": 0x"));
  Serial.print(start, HEX);
  Serial.print(F(" - 0x"));
  Serial.println(end, HEX);
}

// ========== CONFIGURATION COMMAND HANDLERS ==========

void handleConfigSave() {
  saveConfig();
  Serial.println(F("CONFIG_SAVED"));
}

void handleConfigLoad() {
  if (loadConfig()) {
    // Apply rotation
    uint8_t rot = EEPROM.read(ADDR_ROTATION);
    if (rot > 3) rot = 0;
    tft.setRotation(rot);

    // Update screen dimensions
    screenW = tft.width();
    screenH = tft.height();
    dividerY = screenH / 2;
    topMaxY = dividerY - 2;
    bottomMinY = dividerY + 2;
    bottomMaxY = screenH;

    // Reinitialize buttons with new config
    initButtons();

    Serial.println(F("CONFIG_LOADED"));
    Serial.print(F("Rotation: "));
    Serial.println(rot);
  } else {
    Serial.println(F("ERR:NO_CONFIG_FOUND"));
  }
}

void handleConfigReset() {
  // Reset display to defaults
  tft.setRotation(0);
  screenW = tft.width();
  screenH = tft.height();
  dividerY = screenH / 2;
  topMaxY = dividerY - 2;
  bottomMinY = dividerY + 2;
  bottomMaxY = screenH;

  // Reset FPGA defaults
  activeFpga = 1;
  fpga1Baud = 9600;
  fpga2Baud = 9600;
  fpga3Baud = 9600;

  // Reset button configs to defaults
  btnUp.length = 2;
  btnUp.data[0] = 0x55;
  btnUp.data[1] = 0x50;

  btnDown.length = 2;
  btnDown.data[0] = 0x44;
  btnDown.data[1] = 0x4E;

  btnLeft.length = 2;
  btnLeft.data[0] = 0x4C;
  btnLeft.data[1] = 0x54;

  btnRight.length = 2;
  btnRight.data[0] = 0x52;
  btnRight.data[1] = 0x54;

  // Reinitialize buttons
  initButtons();

  Serial.println(F("CONFIG_RESET"));
}

void handleConfigShow() {
  Serial.println(F("=== CONFIGURATION ==="));

  // Display settings
  Serial.print(F("Rotation: "));
  Serial.println(tft.getRotation());

  // FPGA settings
  Serial.print(F("FPGA Selected: "));
  Serial.println(activeFpga);
  Serial.print(F("FPGA1 Baud: "));
  Serial.println(fpga1Baud);
  Serial.print(F("FPGA2 Baud: "));
  Serial.println(fpga2Baud);
  Serial.print(F("FPGA3 Baud: "));
  Serial.println(fpga3Baud);

  // Button configurations
  Serial.println(F("Button Configs:"));
  Serial.print(F("  UP ["));
  Serial.print(btnUp.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnUp.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnUp.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnUp.data[i], HEX);
  }
  Serial.println();

  Serial.print(F("  DOWN ["));
  Serial.print(btnDown.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnDown.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnDown.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnDown.data[i], HEX);
  }
  Serial.println();

  Serial.print(F("  LEFT ["));
  Serial.print(btnLeft.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnLeft.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnLeft.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnLeft.data[i], HEX);
  }
  Serial.println();

  Serial.print(F("  RIGHT ["));
  Serial.print(btnRight.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnRight.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnRight.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnRight.data[i], HEX);
  }
  Serial.println();
}

void handleBtnConfig(String params) {
  // #BTNCONFIG <btn> <len> <bytes...>
  // #BTNCONFIG <btn> ?

  int sp1 = params.indexOf(' ');
  if (sp1 == -1) {
    Serial.println(F("ERR:FORMAT #BTNCONFIG <UP|DOWN|LEFT|RIGHT> <len> <bytes...>"));
    return;
  }

  String btnName = params.substring(0, sp1);
  btnName.toUpperCase();

  ButtonConfig* targetBtn = NULL;
  if (btnName == "UP") targetBtn = &btnUp;
  else if (btnName == "DOWN") targetBtn = &btnDown;
  else if (btnName == "LEFT") targetBtn = &btnLeft;
  else if (btnName == "RIGHT") targetBtn = &btnRight;
  else {
    Serial.println(F("ERR:UNKNOWN_BTN (use UP, DOWN, LEFT, RIGHT)"));
    return;
  }

  String rest = params.substring(sp1 + 1);
  rest.trim();

  // Query mode
  if (rest == "?") {
    Serial.print(btnName);
    Serial.print(F(" ["));
    Serial.print(targetBtn->length);
    Serial.print(F("]: "));
    for (uint8_t i = 0; i < targetBtn->length; i++) {
      if (i > 0) Serial.print(F(" "));
      Serial.print(F("0x"));
      if (targetBtn->data[i] < 16) Serial.print(F("0"));
      Serial.print(targetBtn->data[i], HEX);
    }
    Serial.println();
    return;
  }

  // Configure mode
  int sp2 = rest.indexOf(' ');
  if (sp2 == -1) {
    Serial.println(F("ERR:FORMAT #BTNCONFIG <btn> <len> <bytes...>"));
    return;
  }

  uint8_t len = rest.substring(0, sp2).toInt();
  if (len == 0 || len > 15) {
    Serial.println(F("ERR:LENGTH (1-15)"));
    return;
  }

  // Parse bytes
  String bytesStr = rest.substring(sp2 + 1);
  targetBtn->length = len;

  int byteIdx = 0;
  int pos = 0;
  while (pos < bytesStr.length() && byteIdx < len) {
    // Skip whitespace
    while (pos < bytesStr.length() && bytesStr.charAt(pos) == ' ') pos++;
    if (pos >= bytesStr.length()) break;

    // Find next space or end
    int nextSpace = bytesStr.indexOf(' ', pos);
    if (nextSpace == -1) nextSpace = bytesStr.length();

    String byteStr = bytesStr.substring(pos, nextSpace);
    targetBtn->data[byteIdx++] = parseHexOrDec(byteStr);

    pos = nextSpace + 1;
  }

  if (byteIdx < len) {
    Serial.println(F("ERR:NOT_ENOUGH_BYTES"));
    return;
  }

  // Reinitialize buttons
  initButtons();

  Serial.print(btnName);
  Serial.println(F(" CONFIGURED"));
}

void handleFPGAWrite(String params) {
  // #FPGAWRITE <addr> <value...>
  // Write to FPGA zone (100-299)

  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #FPGAWRITE <addr> <value...>"));
    return;
  }

  uint16_t addr = parseHexOrDec(params.substring(0, sp));

  // Validate FPGA zone
  if (addr < ADDR_FPGA_ZONE || addr >= ADDR_USER_ZONE) {
    Serial.print(F("ERR:FPGA_ZONE ("));
    Serial.print(ADDR_FPGA_ZONE);
    Serial.print(F("-"));
    Serial.print(ADDR_USER_ZONE - 1);
    Serial.println(F(")"));
    return;
  }

  // Parse bytes to write
  String bytesStr = params.substring(sp + 1);
  int pos = 0;
  uint16_t writeAddr = addr;
  uint8_t bytesWritten = 0;

  while (pos < bytesStr.length() && writeAddr < ADDR_USER_ZONE) {
    // Skip whitespace
    while (pos < bytesStr.length() && bytesStr.charAt(pos) == ' ') pos++;
    if (pos >= bytesStr.length()) break;

    // Find next space or end
    int nextSpace = bytesStr.indexOf(' ', pos);
    if (nextSpace == -1) nextSpace = bytesStr.length();

    String byteStr = bytesStr.substring(pos, nextSpace);
    uint8_t value = parseHexOrDec(byteStr);

    // Temporarily disable protection to write to FPGA zone
    protectZones[1].enabled = false;
    EEPROM.write(writeAddr++, value);
    protectZones[1].enabled = true;

    bytesWritten++;
    pos = nextSpace + 1;
  }

  Serial.print(F("FPGA_WRITE: "));
  Serial.print(bytesWritten);
  Serial.print(F(" bytes at 0x"));
  Serial.println(addr, HEX);
}

void handleFPGARead(String params) {
  // #FPGAREAD <addr> [count]
  // Read from FPGA zone (100-299)

  int sp = params.indexOf(' ');
  uint16_t addr;
  uint8_t count = 1;

  if (sp == -1) {
    addr = parseHexOrDec(params);
  } else {
    addr = parseHexOrDec(params.substring(0, sp));
    count = params.substring(sp + 1).toInt();
    if (count == 0) count = 1;
    if (count > 200) count = 200;
  }

  // Validate FPGA zone
  if (addr < ADDR_FPGA_ZONE || addr >= ADDR_USER_ZONE) {
    Serial.print(F("ERR:FPGA_ZONE ("));
    Serial.print(ADDR_FPGA_ZONE);
    Serial.print(F("-"));
    Serial.print(ADDR_USER_ZONE - 1);
    Serial.println(F(")"));
    return;
  }

  Serial.print(F("FPGA[0x"));
  Serial.print(addr, HEX);
  Serial.print(F("]: "));

  for (uint8_t i = 0; i < count && (addr + i) < ADDR_USER_ZONE; i++) {
    if (i > 0) Serial.print(F(" "));
    uint8_t value = EEPROM.read(addr + i);
    Serial.print(F("0x"));
    if (value < 16) Serial.print(F("0"));
    Serial.print(value, HEX);
  }
  Serial.println();
}

// ========== WAVEFORM GENERATOR FUNCTIONS ==========

void updateWaveform() {
  unsigned long now = micros();
  unsigned long period = 1000000UL / waveFreq;  // Period in microseconds

  // Update phase based on time
  if (now - waveLastUpdate >= period / 256) {
    waveLastUpdate = now;
    wavePhase++;

    uint8_t pwmValue = 0;

    switch (waveType) {
      case 0:  // Square wave
        pwmValue = (wavePhase < 128) ? 255 : 0;
        break;

      case 1:  // Triangle wave
        if (wavePhase < 128) {
          pwmValue = wavePhase * 2;
        } else {
          pwmValue = 255 - ((wavePhase - 128) * 2);
        }
        break;

      case 2:  // Sine wave (using lookup table)
        {
          uint8_t idx = (wavePhase * 32) / 256;
          pwmValue = pgm_read_byte(&sineLUT[idx]);
        }
        break;
    }

    analogWrite(wavePin, pwmValue);
  }
}

void handleWaveGen(String params) {
  // Parse: <pin> <type> <freq>
  // type: 0=square, 1=triangle, 2=sine
  int vals[3];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 3; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        token.toUpperCase();

        if (count == 1) {  // Wave type
          if (token == "SQUARE") vals[count++] = 0;
          else if (token == "TRIANGLE" || token == "TRI") vals[count++] = 1;
          else if (token == "SINE" || token == "SIN") vals[count++] = 2;
          else vals[count++] = token.toInt();
        } else {
          vals[count++] = token.toInt();
        }
      }
      lastPos = i + 1;
    }
  }

  if (count < 3) {
    Serial.println(F("ERR:FORMAT #WAVEGEN <pin> <type> <freq>"));
    Serial.println(F("  type: SQUARE, TRIANGLE, SINE (or 0,1,2)"));
    Serial.println(F("  freq: 1-10000 Hz"));
    return;
  }

  int8_t pin = vals[0];
  waveType = vals[1];
  waveFreq = vals[2];

  // Validate
  if (waveType > 2) waveType = 0;
  if (waveFreq < 1) waveFreq = 1;
  if (waveFreq > 10000) waveFreq = 10000;

  // Check if pin supports PWM (Mega PWM pins: 2-13, 44-46)
  if (!((pin >= 2 && pin <= 13) || (pin >= 44 && pin <= 46))) {
    Serial.println(F("ERR:PIN_NO_PWM"));
    Serial.println(F("PWM pins: 2-13, 44-46"));
    return;
  }

  wavePin = pin;
  pinMode(wavePin, OUTPUT);
  waveActive = true;
  wavePhase = 0;
  waveLastUpdate = micros();

  Serial.print(F("WAVE_START: Pin="));
  Serial.print(wavePin);
  Serial.print(F(", Type="));
  switch (waveType) {
    case 0: Serial.print(F("SQUARE")); break;
    case 1: Serial.print(F("TRIANGLE")); break;
    case 2: Serial.print(F("SINE")); break;
  }
  Serial.print(F(", Freq="));
  Serial.print(waveFreq);
  Serial.println(F("Hz"));
}

void handleWaveStop() {
  if (wavePin >= 0) {
    analogWrite(wavePin, 0);
    wavePin = -1;
  }
  waveActive = false;
  Serial.println(F("WAVE_STOPPED"));
}

// ========== PULSE/FREQUENCY MEASUREMENT FUNCTIONS ==========

// Measure pulse width using pulseIn()
void handlePulseIn(String params) {
  // Parse: <pin> <state> <timeout>
  int vals[3];
  int count = 0;
  int lastPos = 0;
  uint8_t state = HIGH;

  for (int i = 0; i <= params.length() && count < 3; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        token.toUpperCase();

        if (count == 1) {  // State parameter
          if (token == "HIGH" || token == "1") state = HIGH;
          else if (token == "LOW" || token == "0") state = LOW;
          else vals[count] = token.toInt();  // Allow numeric
          count++;
        } else {
          vals[count++] = token.toInt();
        }
      }
      lastPos = i + 1;
    }
  }

  if (count < 3) {
    Serial.println(F("ERR:FORMAT #PULSEIN <pin> <state> <timeout>"));
    Serial.println(F("  state: HIGH, LOW, 1, or 0"));
    Serial.println(F("  timeout: microseconds (max 3000000)"));
    return;
  }

  uint8_t pin = vals[0];
  unsigned long timeout = vals[2];

  if (timeout > 3000000UL) {
    Serial.println(F("ERR:TIMEOUT_MAX_3000000"));
    return;
  }

  // Configure pin as input
  pinMode(pin, INPUT);

  // Measure pulse
  unsigned long duration = pulseIn(pin, state, timeout);

  // Report result
  Serial.print(F("PULSE_IN: Pin="));
  Serial.print(pin);
  Serial.print(F(", State="));
  Serial.print(state == HIGH ? F("HIGH") : F("LOW"));
  Serial.print(F(", Duration="));
  Serial.print(duration);
  Serial.println(F("us"));

  if (duration == 0) {
    Serial.println(F("NOTE: Timeout or no pulse detected"));
  }
}

// Count pulses over a duration and calculate frequency
void handleFreqCount(String params) {
  // Parse: <pin> <duration>
  int vals[2];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #FREQCOUNT <pin> <duration>"));
    Serial.println(F("  duration: milliseconds (10-10000)"));
    return;
  }

  uint8_t pin = vals[0];
  unsigned long duration = vals[1];

  if (duration < 10 || duration > 10000) {
    Serial.println(F("ERR:DURATION_RANGE (10-10000 ms)"));
    return;
  }

  // Configure pin as input
  pinMode(pin, INPUT);

  // Count rising edges
  unsigned long startTime = millis();
  unsigned long count_pulses = 0;
  uint8_t lastState = digitalRead(pin);

  while (millis() - startTime < duration) {
    uint8_t currentState = digitalRead(pin);
    if (currentState == HIGH && lastState == LOW) {
      count_pulses++;
    }
    lastState = currentState;
  }

  // Calculate frequency
  float actualDuration = millis() - startTime;
  float frequency = (count_pulses * 1000.0) / actualDuration;

  // Report result
  Serial.print(F("FREQ_COUNT: Pin="));
  Serial.print(pin);
  Serial.print(F(", Pulses="));
  Serial.print(count_pulses);
  Serial.print(F(", Duration="));
  Serial.print((unsigned long)actualDuration);
  Serial.print(F("ms, Freq="));
  Serial.print(frequency, 2);
  Serial.println(F("Hz"));
}

// Start continuous frequency monitoring using interrupts
void handleFreqMon(String params) {
  // Parse: <pin> <duration>
  int vals[2];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #FREQMON <pin> <duration>"));
    Serial.println(F("  duration: measurement window (100-10000 ms)"));
    Serial.println(F("  Interrupt pins: 2, 3, 18-21"));
    return;
  }

  uint8_t pin = vals[0];
  freqMonDuration = vals[1];

  if (freqMonDuration < 100 || freqMonDuration > 10000) {
    Serial.println(F("ERR:DURATION_RANGE (100-10000 ms)"));
    return;
  }

  // Check if pin supports interrupts (Mega: 2, 3, 18, 19, 20, 21)
  uint8_t interruptNum = digitalPinToInterrupt(pin);
  if (interruptNum == NOT_AN_INTERRUPT) {
    Serial.println(F("ERR:PIN_NO_INTERRUPT"));
    Serial.println(F("Interrupt pins: 2, 3, 18, 19, 20, 21"));
    return;
  }

  // Stop any existing monitoring
  if (freqMonActive) {
    detachInterrupt(digitalPinToInterrupt(freqMonPin));
  }

  // Configure pin and interrupt
  freqMonPin = pin;
  pinMode(freqMonPin, INPUT);
  pulseCount = 0;
  freqMonLastUpdate = millis();
  freqMonActive = true;

  attachInterrupt(interruptNum, pulseISR, RISING);

  Serial.print(F("FREQ_MON_START: Pin="));
  Serial.print(freqMonPin);
  Serial.print(F(", Window="));
  Serial.print(freqMonDuration);
  Serial.println(F("ms"));
}

// Update frequency monitor (called from loop)
void updateFreqMonitor() {
  unsigned long now = millis();

  if (now - freqMonLastUpdate >= freqMonDuration) {
    // Calculate frequency
    float actualDuration = now - freqMonLastUpdate;
    float frequency = (pulseCount * 1000.0) / actualDuration;

    // Report
    Serial.print(F("FREQ: "));
    Serial.print(frequency, 2);
    Serial.print(F("Hz ("));
    Serial.print(pulseCount);
    Serial.println(F(" pulses)"));

    // Reset for next window
    pulseCount = 0;
    freqMonLastUpdate = now;
  }
}

// Stop frequency monitoring
void handleFreqStop() {
  if (freqMonActive && freqMonPin >= 0) {
    detachInterrupt(digitalPinToInterrupt(freqMonPin));
    freqMonPin = -1;
    freqMonActive = false;
    Serial.println(F("FREQ_MON_STOPPED"));
  } else {
    Serial.println(F("ERR:NOT_ACTIVE"));
  }
}
