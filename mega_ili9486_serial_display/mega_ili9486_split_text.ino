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
  uint8_t cmdBytes[8];    // Bytes to send to FPGA
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

void setup() {
  Serial.begin(115200);      // USB/PC
  Serial1.begin(fpga1Baud);  // FPGA1
  Serial2.begin(fpga2Baud);  // FPGA2
  Serial3.begin(fpga3Baud);  // FPGA3

  uint16_t ID = tft.readID();
  Serial.print(F("LCD ID: 0x"));
  Serial.println(ID, HEX);

  tft.begin(ID);
  tft.setRotation(0);
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

    // ========== ANALOG INPUT COMMANDS ==========
    } else if (c.startsWith("#ANALOGREAD ")) {
      handleAnalogRead(c.substring(12));

    } else if (c == "#ANALOGREADALL") {
      handleAnalogReadAll();

    } else if (c.startsWith("#ANALOGREF ")) {
      handleAnalogRef(c.substring(11));

    } else if (c.startsWith("#ANALOGAVG ")) {
      handleAnalogAvg(c.substring(11));

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
  Serial.println();
  Serial.println(F("ANALOG INPUT (A8-A15):"));
  Serial.println(F("  #ANALOGREAD <pin> - Read A8-A15"));
  Serial.println(F("  #ANALOGREADALL - Read all 8 pins"));
  Serial.println(F("  #ANALOGREF <DEFAULT|INTERNAL>"));
  Serial.println(F("  #ANALOGAVG <1-16> - Set averaging"));
  Serial.println(F("  Note: EXTERNAL ref N/A (AREF used by LCD)"));
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

  // UP button - sends bytes: 0x55 0x50 ("UP")
  buttons[0].x = 5;
  buttons[0].y = btnY;
  buttons[0].w = btnWidth;
  buttons[0].h = btnHeight;
  buttons[0].label = "UP";
  buttons[0].cmdBytes[0] = 0x55;  // 'U'
  buttons[0].cmdBytes[1] = 0x50;  // 'P'
  buttons[0].cmdLen = 2;
  buttons[0].color = 0x07E0;  // Green

  // DOWN button - sends bytes: 0x44 0x4E ("DN")
  buttons[1].x = 5 + btnWidth + 3;
  buttons[1].y = btnY;
  buttons[1].w = btnWidth;
  buttons[1].h = btnHeight;
  buttons[1].label = "DN";
  buttons[1].cmdBytes[0] = 0x44;  // 'D'
  buttons[1].cmdBytes[1] = 0x4E;  // 'N'
  buttons[1].cmdLen = 2;
  buttons[1].color = 0x07E0;  // Green

  // LEFT button - sends bytes: 0x4C 0x54 ("LT")
  buttons[2].x = 5 + (btnWidth + 3) * 2;
  buttons[2].y = btnY;
  buttons[2].w = btnWidth;
  buttons[2].h = btnHeight;
  buttons[2].label = "LT";
  buttons[2].cmdBytes[0] = 0x4C;  // 'L'
  buttons[2].cmdBytes[1] = 0x54;  // 'T'
  buttons[2].cmdLen = 2;
  buttons[2].color = 0x07E0;  // Green

  // RIGHT button - sends bytes: 0x52 0x54 ("RT")
  buttons[3].x = 5 + (btnWidth + 3) * 3;
  buttons[3].y = btnY;
  buttons[3].w = btnWidth;
  buttons[3].h = btnHeight;
  buttons[3].label = "RT";
  buttons[3].cmdBytes[0] = 0x52;  // 'R'
  buttons[3].cmdBytes[1] = 0x54;  // 'T'
  buttons[3].cmdLen = 2;
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
