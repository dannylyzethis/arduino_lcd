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
 * GPIO System (Pins 22-45 on side connector):
 * - 24 configurable I/O pins available
 * - Modes: INPUT, INPUT_PULLUP, OUTPUT
 * - Event-driven: RISING, FALLING, or BOTH edge detection
 * - Register-based control: Set all 24 pins with 24-bit value (0x000000-0xFFFFFF)
 * - Events displayed on LCD bottom screen and logged to serial
 * Commands:
 *   #GPIOMODE 22 OUT     - Set pin 22 as output
 *   #GPIOWRITE 22 1      - Write HIGH to pin 22
 *   #GPIOREAD 23         - Read pin 23
 *   #GPIOEVENT 24 RISING - Trigger event on pin 24 rising edge
 *   #GPIOREG             - Show 24-bit register (all pins)
 *   #GPIOSET 0xFFFFFF    - Set register high on all 24 pins
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

// ========== LIBRARY INCLUDES ==========
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include "MenuSystem.h"
#include "MenuConfig.h"

// ========== MODULE INCLUDES ==========
#include "sam_config.h"
#include "sam_globals.h"
#include "sam_display.h"
#include "sam_touch.h"
#include "sam_gpio.h"
#include "sam_analog.h"
#include "sam_i2c.h"
#include "sam_spi.h"
#include "sam_eeprom.h"
#include "sam_logging.h"
#include "sam_fpga_serial.h"
#include "sam_waveform.h"
#include "sam_rules.h"
#include "sam_macros.h"
#include "sam_hooks.h"
#include "sam_failsafe.h"
#include "sam_commands.h"

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------
void setup() {
  // Capture/reset cause then disable watchdog to avoid boot loops.
  bootResetCause = MCUSR;
  MCUSR = 0;
  wdt_disable();
  wdtEnabled = false;

  Serial.begin(115200);      // USB/PC
  serialRxEnableMs = millis() + 250;

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

  // Apply saved/default view mode geometry
  applyViewMode();

  tft.fillScreen(0);
  drawDivider();
  topPosY = 0;
  bottomPosY = (viewMode == VIEW_FULL) ? 0 : bottomMinY;

  cmd.reserve(300);       // Mega has plenty of RAM
  fpgaBuffer.reserve(200);

  // Initialize touch buttons
  initButtons();

  // Initialize menu system
  menuManager = new MenuManager(&tft);
  initMenus();
  menuManager->begin(&mainMenu);
  menuManager->setPosition(0, 0, screenW, screenH);
  menuManager->setColors(0x0000, 0xFFFF, 0x07E0, 0x07FF, 0x39E7);

  // Initialize GPIO system
  initGPIO();

  // Initialize I2C (Wire library)
  Wire.begin();

  // Initialize event hooks defaults
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    gpioHooks[i].enabled = false;
    gpioHooks[i].macroId = 0;
    gpioHooks[i].modeMask = 0x03;
    gpioHooks[i].cooldownMs = 300;
    gpioHooks[i].lastTriggerMs = 0;
  }
  for (uint8_t i = 0; i < 8; i++) {
    thresholdHooks[i].enabled = false;
    thresholdHooks[i].macroId = 0;
    thresholdHooks[i].modeMask = 0x03;
    thresholdHooks[i].cooldownMs = 300;
    thresholdHooks[i].lastTriggerMs = 0;
  }

  for (uint8_t i = 0; i < MAX_RULES; i++) {
    smartRules[i].enabled = false;
    smartRules[i].srcType = RULE_SRC_ANALOG;
    smartRules[i].srcIndex = 0;
    smartRules[i].op = RULE_OP_GT;
    smartRules[i].value = 0;
    smartRules[i].holdMs = 0;
    smartRules[i].action = RULE_ACT_LOG;
    smartRules[i].actionArg = 0;
    smartRules[i].cooldownMs = 1000;
    smartRules[i].sinceMs = 0;
    smartRules[i].lastFireMs = 0;
  }

  // Startup message
  tft.setTextColor(0x07FF);  // Cyan
  tft.setTextSize(2);
  tft.println(F("SAM"));
  tft.setTextColor(0x07E0);  // Green
  tft.println(F("ILI9486/9488"));
  tft.setTextColor(0xFFE0);  // Yellow
  tft.println(F("Smart Arduino Monitor"));
  tft.setTextColor(0xFFFF);  // White
  tft.println(F("Text-Based"));
  tft.println(F("Touch Buttons"));
  tft.println();

  topPosY = tft.getCursorY();

  Serial.println(F("=== SAM Smart Arduino Monitor ==="));
  Serial.print(F("Firmware: v"));
  Serial.println(F(SAM_FIRMWARE_VERSION));
  Serial.print(F("ResetCause: "));
  printResetCause();
  Serial.println(F("Command Console"));
  Serial.print(F("Resolution: "));
  Serial.print(screenW);
  Serial.print(F("x"));
  Serial.println(screenH);
  Serial.println(F("Type #HELP for commands"));
  Serial.println(F("#SHOWBTNS for touch buttons"));
  Serial.println(F("READY"));
  setWhy("READY");
}

// ---------------------------------------------------------------------------
// loop
// ---------------------------------------------------------------------------
void loop() {
  // Process menu touch input (has priority when active)
  if (menuActive) {
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      // Map touch coordinates to screen coordinates
      int16_t px, py;
      switch (tft.getRotation()) {
        case 0:
          px = map(p.y, TS_MINY, TS_MAXY, 0, screenW);
          py = map(p.x, TS_MINX, TS_MAXX, 0, screenH);
          break;
        case 1:
          px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
          py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
          break;
        case 2:
          px = map(p.y, TS_MAXY, TS_MINY, 0, screenW);
          py = map(p.x, TS_MAXX, TS_MINX, 0, screenH);
          break;
        case 3:
          px = map(p.x, TS_MAXX, TS_MINX, 0, screenW);
          py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
          break;
      }

      // Handle menu touch
      menuManager->handleTouch(px, py);
      delay(200);  // Debounce
    }
    if (wdtEnabled) wdt_reset();
    return;  // Skip other processing when menu is active
  }

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

  // Update statistics
  updateStats();
  updateWatch();
  if (linkIndicatorsEnabled) {
    drawLinkIndicators();
  }

  // Check threshold alerts
  static unsigned long lastThresholdCheck = 0;
  if (millis() - lastThresholdCheck >= 100) {  // Check every 100ms
    lastThresholdCheck = millis();
    checkThresholds();
  }

  evaluateRules();
  evaluateEscalation();
  evaluateFailsafe();

  if (cmdReady) {
    processCmd(cmd);
    cmd = "";
    cmdReady = false;
  }

  // Check all FPGA serials
  checkFPGASerial(Serial1, 1);
  checkFPGASerial(Serial2, 2);
  checkFPGASerial(Serial3, 3);
  if (wdtEnabled) wdt_reset();
}

// ---------------------------------------------------------------------------
// serialEvent — Arduino IDE serial ISR hook (must live in the main sketch)
// ---------------------------------------------------------------------------
void serialEvent() {
  while (Serial.available()) {
    if (millis() < serialRxEnableMs) {
      (void)Serial.read();
      cmd = "";
      continue;
    }

    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (cmd.length() > 0) {
        int hashPos = cmd.indexOf('#');
        int passPos = cmd.indexOf(">>>");
        int startPos = -1;
        if (hashPos >= 0 && passPos >= 0) startPos = min(hashPos, passPos);
        else if (hashPos >= 0) startPos = hashPos;
        else if (passPos >= 0) startPos = passPos;

        if (startPos > 0) {
          cmd = cmd.substring(startPos);
        }
        // startPos < 0 means no # or >>> — pass plain text through to showText
      }
      if (cmd.length() > 0) cmdReady = true;
    } else {
      if (cmd.length() < 290) {
        cmd += c;
      }
    }
  }
}
