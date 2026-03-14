#include "sam_commands.h"
#include "sam_globals.h"
#include "sam_display.h"
#include "sam_touch.h"
#include "sam_gpio.h"
#include "sam_analog.h"
#include "sam_waveform.h"
#include "sam_logging.h"
#include "sam_eeprom.h"
#include "sam_fpga_serial.h"
#include "sam_i2c.h"
#include "sam_spi.h"
#include "sam_rules.h"
#include "sam_failsafe.h"
#include "sam_macros.h"
#include "sam_hooks.h"
#include <EEPROM.h>
#include <avr/wdt.h>

// ---------------------------------------------------------------------------
// updateWatch — called from loop()
// ---------------------------------------------------------------------------
void updateWatch() {
  if (!watchEnabled) return;
  unsigned long now = millis();
  if (now - watchLastMs < watchIntervalMs) return;
  watchLastMs = now;

  String msg = "WATCH ";
  if (watchSource == "GPIO") {
    uint32_t regVal = 0;
    for (uint8_t i = 0; i < NUM_GPIO; i++) {
      if (digitalRead(gpioPins[i])) regVal |= (1UL << i);
    }
    char hx[9];
    snprintf(hx, sizeof(hx), "%06lX", regVal & 0xFFFFFFUL);
    msg += "GPIO=0x";
    msg += hx;
  } else {
    int idx = -1;
    if (watchSource.length() == 2 && watchSource[0] == 'A') idx = watchSource[1] - '8';
    if (watchSource.length() == 3 && watchSource[0] == 'A') idx = (watchSource[1] - '0') * 10 + (watchSource[2] - '0') - 8;
    if (idx >= 0 && idx < 8) {
      uint16_t v = analogRead(analogPins[idx]);
      msg += watchSource;
      msg += "=";
      msg += String(v);
    } else {
      msg += "INVALID_SRC";
    }
  }
  showTextBottom(msg);
}

// ---------------------------------------------------------------------------
// parseHexOrDec
// ---------------------------------------------------------------------------
long parseHexOrDec(String str) {
  str.trim();
  if (str.startsWith("0x") || str.startsWith("0X")) {
    return strtol(str.c_str() + 2, NULL, 16);
  } else {
    return str.toInt();
  }
}

// ---------------------------------------------------------------------------
// getFreeRAM
// ---------------------------------------------------------------------------
int getFreeRAM() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// ---------------------------------------------------------------------------
// handleMemInfo
// ---------------------------------------------------------------------------
void handleMemInfo() {
  int freeRAM = getFreeRAM();

  Serial.println(F("=== MEMORY INFO ==="));
  Serial.print(F("Free RAM: "));
  Serial.print(freeRAM);
  Serial.print(F(" bytes ("));
  Serial.print((freeRAM * 100) / 8192);
  Serial.println(F("%)"));

  Serial.print(F("EEPROM Total: "));
  Serial.println(F("4096 bytes"));

  Serial.print(F("EEPROM Config: "));
  Serial.println(F("0-99 (100 bytes)"));

  Serial.print(F("EEPROM FPGA Zone: "));
  Serial.println(F("100-299 (200 bytes)"));

  Serial.print(F("EEPROM User Zone: "));
  Serial.println(F("300-499 (200 bytes)"));

  Serial.print(F("EEPROM Free: "));
  Serial.println(F("500-1999 (1500 bytes)"));

  Serial.print(F("EEPROM Log Zone: "));
  Serial.print(logStartAddr);
  Serial.print(F("-"));
  Serial.print(logEndAddr);
  Serial.print(F(" ("));
  Serial.print(logEndAddr - logStartAddr + 1);
  Serial.println(F(" bytes)"));

  Serial.print(F("Log Entries: "));
  Serial.print(logEntryCount);
  Serial.print(F("/"));
  Serial.println((logEndAddr - logStartAddr + 1) / LOG_ENTRY_SIZE);

  Serial.println(F("=================="));
}

// ---------------------------------------------------------------------------
// handleUptime
// ---------------------------------------------------------------------------
void handleUptime() {
  unsigned long uptime = millis();
  unsigned long seconds = uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  Serial.print(F("UPTIME: "));
  if (days > 0) {
    Serial.print(days);
    Serial.print(F("d "));
  }
  if (hours > 0 || days > 0) {
    Serial.print(hours);
    Serial.print(F("h "));
  }
  if (minutes > 0 || hours > 0 || days > 0) {
    Serial.print(minutes);
    Serial.print(F("m "));
  }
  Serial.print(seconds);
  Serial.print(F("s ("));
  Serial.print(uptime);
  Serial.println(F("ms)"));
}

// ---------------------------------------------------------------------------
// handleBenchmark
// ---------------------------------------------------------------------------
void handleBenchmark() {
  Serial.println(F("=== BENCHMARK ==="));
  unsigned long start, duration;

  // Test 1: Integer math
  start = micros();
  volatile long result = 0;
  for (int i = 0; i < 10000; i++) {
    result += i * 3;
  }
  duration = micros() - start;
  Serial.print(F("Int Math (10k): "));
  Serial.print(duration);
  Serial.println(F("us"));

  // Test 2: Float math
  start = micros();
  volatile float fresult = 0.0;
  for (int i = 0; i < 1000; i++) {
    fresult += (float)i * 3.14159;
  }
  duration = micros() - start;
  Serial.print(F("Float Math (1k): "));
  Serial.print(duration);
  Serial.println(F("us"));

  // Test 3: Analog read
  start = micros();
  for (int i = 0; i < 100; i++) {
    volatile int val = analogRead(A8);
  }
  duration = micros() - start;
  Serial.print(F("Analog Read (100): "));
  Serial.print(duration);
  Serial.print(F("us ("));
  Serial.print(duration / 100);
  Serial.println(F("us/read)"));

  // Test 4: Digital write
  pinMode(22, OUTPUT);
  start = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWrite(22, i & 1);
  }
  duration = micros() - start;
  Serial.print(F("Digital Write (1k): "));
  Serial.print(duration);
  Serial.print(F("us ("));
  Serial.print(duration / 1000);
  Serial.println(F("us/write)"));

  // Test 5: Display pixel drawing
  start = millis();
  for (int i = 0; i < 1000; i++) {
    tft.drawPixel(100 + (i % 100), 100 + (i / 100), topTextColor);
  }
  duration = millis() - start;
  Serial.print(F("Display Pixels (1k): "));
  Serial.print(duration);
  Serial.println(F("ms"));

  // Test 6: Display text
  start = millis();
  tft.setTextSize(1);
  for (int i = 0; i < 10; i++) {
    tft.setCursor(10, 10);
    tft.print(F("Benchmark"));
  }
  duration = millis() - start;
  Serial.print(F("Display Text (10x): "));
  Serial.print(duration);
  Serial.println(F("ms"));

  // Test 7: EEPROM write
  start = millis();
  for (int i = 0; i < 100; i++) {
    EEPROM.write(500 + i, i & 0xFF);
  }
  duration = millis() - start;
  Serial.print(F("EEPROM Write (100): "));
  Serial.print(duration);
  Serial.print(F("ms ("));
  Serial.print(duration / 100.0, 2);
  Serial.println(F("ms/byte)"));

  // Test 8: EEPROM read
  start = micros();
  for (int i = 0; i < 100; i++) {
    volatile uint8_t val = EEPROM.read(500 + i);
  }
  duration = micros() - start;
  Serial.print(F("EEPROM Read (100): "));
  Serial.print(duration);
  Serial.print(F("us ("));
  Serial.print(duration / 100);
  Serial.println(F("us/byte)"));

  Serial.println(F("================="));
}

// ---------------------------------------------------------------------------
// help
// ---------------------------------------------------------------------------
void help() {
  Serial.println(F("=== SAM Smart Arduino Monitor Commands ==="));
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
  Serial.println(F("  #VIEW <SPLIT|FULL|?>"));
  Serial.println(F("  #ADDRMODE <ON|OFF|?>"));
  Serial.println(F("  #ADDR <byte|?>"));
  Serial.println(F("  #ADDRSEND <hex bytes>"));
  Serial.println(F("  #FRAME <ON|OFF|?>"));
  Serial.println(F("  #BRIDGE <ON|OFF|?>"));
  Serial.println(F("  #WATCH START <A8..A15|GPIO> <ms>"));
  Serial.println(F("  #WATCH STOP | #WATCH ?"));
  Serial.println(F("  #WDT <ON ms|OFF|?>"));
  Serial.println(F("  #LINK <ON|OFF|?|REPORT>"));
  Serial.println(F("  #LINKS <ON|OFF|?> (alias)"));
  Serial.println(F("  #PROFILE <SAFE|BALANCED|PERF|CUSTOM|?>"));
  Serial.println(F("  #SUMMARY"));
  Serial.println(F("  #WHY"));
  Serial.println(F("  #ESC <ON|OFF|?|RESET>"));
  Serial.println(F("  #ESC SET <ERR|TIMEOUT|RULE|ALL> <threshold> <window_ms> <LOG|ALERT|MACRO n>"));
  Serial.println(F("  #SAFE <ON|OFF|?|CLEAR>"));
  Serial.println(F("  #SAFE SET <threshold> <window_ms> <hold_ms>"));
  Serial.println(F("  #RULE ?|ON|OFF|LIST"));
  Serial.println(F("  #RULE ADD <expr> [FOR <ms>] THEN <MACRO id|LOG|ALERT>"));
  Serial.println(F("  #RULE CLEAR <id|ALL>"));
  Serial.println(F("  #HEALTH | #HEALTHRESET"));
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
  Serial.println(F("MENU SYSTEM:"));
  Serial.println(F("  #MENU - Show FPGA menu"));
  Serial.println(F("  #MENUHIDE - Hide menu"));
  Serial.println(F("  #MENUBACK - Go back one level"));
  Serial.println(F("  Touch menu items to navigate"));
  Serial.println(F("  Menu includes:"));
  Serial.println(F("    8 Control Registers (64-bit)"));
  Serial.println(F("    8 Status Registers (64-bit)"));
  Serial.println(F("    Temperature Sensors"));
  Serial.println(F("    Firmware Information"));
  Serial.println();
  Serial.println(F("TOUCH CALIBRATION:"));
  Serial.println(F("  #TOUCHCAL ? - Show current calibration"));
  Serial.println(F("  #TOUCHCAL <minx> <maxx> <miny> <maxy>"));
  Serial.println(F("  #TOUCHTEST - Toggle touch test mode"));
  Serial.println(F("    Shows raw & mapped coordinates"));
  Serial.println(F("  Touch top-right corner to toggle SPLIT/FULL"));
  Serial.println();
  Serial.println(F("GPIO (Pins 22-45):"));
  Serial.println(F("  #GPIOMODE <pin> <IN|INPU|OUT>"));
  Serial.println(F("  #GPIOWRITE <pin> <0|1>"));
  Serial.println(F("  #GPIOREAD <pin>"));
  Serial.println(F("  #GPIOREADALL - Read all pins"));
  Serial.println(F("  #GPIOEVENT <pin> <NONE|RISING|FALLING|BOTH>"));
  Serial.println(F("  #HOOK GPIO <pin> <RISE|FALL|BOTH|NONE> <macro> [cooldown]"));
  Serial.println(F("  #HOOK THR <0-7> <ENTER|EXIT|BOTH|NONE> <macro> [cooldown]"));
  Serial.println(F("  #HOOK LIST"));
  Serial.println(F("  #GPIOREG - Show 24-bit register"));
  Serial.println(F("  #GPIOSET <0x000000-0xFFFFFF> - Set register"));
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
  Serial.println(F("  #XYPLOT <x y w h x1,y1 x2,y2...>"));
  Serial.println(F("    Options: STYLE POINTS|LINES|BOTH GRID"));
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
  Serial.println(F("EVENT LOG (RAM):"));
  Serial.println(F("  #LOG ? - Event log status"));
  Serial.println(F("  #LOG LAST <n> [ALL|RULE|GPIO|THR|ERR|INFO]"));
  Serial.println(F("  #LOG CLEAR - Clear event log"));
  Serial.println();
  Serial.println(F("WAVEFORM GENERATOR (PWM):"));
  Serial.println(F("  #WAVEGEN <pin> <type> <freq>"));
  Serial.println(F("    type: SQUARE, TRIANGLE, SINE"));
  Serial.println(F("    freq: 1-10000 Hz"));
  Serial.println(F("    PWM pins: 2-13, 44-46"));
  Serial.println(F("  #WAVESTOP - Stop waveform"));
  Serial.println();
  Serial.println(F("PWM CONTROL:"));
  Serial.println(F("  #PWMSET <pin> <duty> - Set PWM output"));
  Serial.println(F("    duty: 0-255 (0=off, 255=full)"));
  Serial.println(F("  #PWMSTOP <pin> - Stop PWM on pin"));
  Serial.println(F("  #PWMFREQ <pin> <freq> - Set PWM frequency"));
  Serial.println(F("    freq: 31-31250 Hz (steps)"));
  Serial.println(F("    WARNING: Affects all pins on timer!"));
  Serial.println(F("    Pins 4,13 protected (Timer0/millis)"));
  Serial.println();
  Serial.println(F("SYSTEM INFO:"));
  Serial.println(F("  #MEMINFO - Show memory usage"));
  Serial.println(F("  #UPTIME - Show system uptime"));
  Serial.println(F("  #BENCHMARK - Run performance tests"));
  Serial.println();
  Serial.println(F("STATISTICS:"));
  Serial.println(F("  #STATSSTART <pin> - Start stats for A8-A15"));
  Serial.println(F("    pin: 0=A8, 1=A9, ... 7=A15"));
  Serial.println(F("  #STATSSTOP <pin> - Stop stats collection"));
  Serial.println(F("  #STATSSHOW <pin> - Show min/max/avg"));
  Serial.println(F("  #STATSRESET - Reset all statistics"));
  Serial.println();
  Serial.println(F("THRESHOLD ALERTS:"));
  Serial.println(F("  #THRESHOLD <pin> <low> <high>"));
  Serial.println(F("    Triggers alert when value outside range"));
  Serial.println(F("  #THRESHOLDCLEAR <pin> - Clear threshold"));
  Serial.println(F("  #THRESHOLDSTATUS - Show all thresholds"));
  Serial.println();
  Serial.println(F("SCROLLING TEXT:"));
  Serial.println(F("  #SCROLL <y> <text>"));
  Serial.println(F("    Horizontal scrolling text at Y position"));
  Serial.println();
  Serial.println(F("COMMAND MACROS:"));
  Serial.println(F("  #MACRODEF <id> <cmd1>;<cmd2>..."));
  Serial.println(F("    Define macro (id: 0-2, max 5 cmds)"));
  Serial.println(F("  #MACRORUN <id> - Run macro"));
  Serial.println(F("  #MACROLIST - List all macros"));
  Serial.println(F("  #MACROCLEAR <id> - Clear macro"));
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
  Serial.println(F("  #VERSION - Firmware version/build"));
  Serial.println(F("  #HELP - This help"));
  Serial.println();
  Serial.println(F("COLORS: RED GREEN BLUE CYAN"));
  Serial.println(F("        MAGENTA YELLOW WHITE"));
  Serial.println(F("        BLACK ORANGE PINK"));
  Serial.println(F("        PURPLE NAVY"));
}

// ---------------------------------------------------------------------------
// processCmd — main command dispatcher
// ---------------------------------------------------------------------------
void processCmd(String c) {
  c.trim();
  if (c.length() == 0) return;

  // FPGA passthrough with termination control
  if (c.startsWith(">>>")) {
    String data = c.substring(3);
    data.trim();
    HardwareSerial& fpga = getFPGA();
    uint8_t raw[FRAME_MAX_PAYLOAD];
    uint8_t len = (uint8_t)min((int)data.length(), (int)FRAME_MAX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++) raw[i] = (uint8_t)data[i];
    sendBytesToFPGA(fpga, activeFpga, raw, len, true);

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
      Serial.print(F("View: "));
      Serial.println(viewMode == VIEW_FULL ? F("FULL") : F("SPLIT"));
      Serial.print(F("ProtoAddr: "));
      Serial.println(protoAddrMode ? F("ON") : F("OFF"));
      Serial.print(F("ProtoFrame: "));
      Serial.println(protoFrameMode ? F("ON") : F("OFF"));
      Serial.print(F("Bridge: "));
      Serial.println(bridgeMode ? F("ON") : F("OFF"));
      Serial.print(F("LocalAddr: 0x"));
      if (localAddress < 16) Serial.print(F("0"));
      Serial.println(localAddress, HEX);
      Serial.print(F("Counters RX/TX F1: "));
      Serial.print(fpgaRxBytes[1]); Serial.print(F("/")); Serial.println(fpgaTxBytes[1]);
      Serial.print(F("Counters RX/TX F2: "));
      Serial.print(fpgaRxBytes[2]); Serial.print(F("/")); Serial.println(fpgaTxBytes[2]);
      Serial.print(F("Counters RX/TX F3: "));
      Serial.print(fpgaRxBytes[3]); Serial.print(F("/")); Serial.println(fpgaTxBytes[3]);
      Serial.print(F("Addr Local/Pass: "));
      Serial.print(addrLocalHandled); Serial.print(F("/")); Serial.println(addrPassthrough);
      Serial.print(F("Watch: "));
      Serial.print(watchEnabled ? F("ON " ) : F("OFF "));
      Serial.print(watchSource); Serial.print(F(" @")); Serial.println(watchIntervalMs);
      Serial.print(F("Links: ")); Serial.println(linkIndicatorsEnabled ? F("ON") : F("OFF"));
      Serial.print(F("Profile: ")); Serial.println(profileName(smartProfile));
      Serial.print(F("Escalation: ")); Serial.println(escalation.enabled ? F("ON") : F("OFF"));
      Serial.print(F("Esc Fires: ")); Serial.println(escalation.fireCount);
      Serial.print(F("Failsafe: "));
      if (failsafe.enabled) {
        Serial.print(F("ON "));
        Serial.println(failsafe.active ? F("(ACTIVE)") : F("(IDLE)"));
      } else {
        Serial.println(F("OFF"));
      }
      Serial.print(F("Safe Fires: ")); Serial.println(failsafe.fireCount);
      Serial.print(F("WDT: "));
      if (wdtEnabled) {
        Serial.print(F("ON @"));
        Serial.print(wdtTimeoutMs);
        Serial.println(F("ms"));
      } else {
        Serial.println(F("OFF"));
      }
      Serial.print(F("LastReset: "));
      printResetCause();
      Serial.println(F("============="));

    } else if (c.startsWith("#VIEW ")) {
      String param = c.substring(6);
      param.trim();
      if (param == "?") {
        Serial.print(F("VIEW="));
        Serial.println(viewMode == VIEW_FULL ? F("FULL") : F("SPLIT"));
      } else if (param == "FULL") {
        viewMode = VIEW_FULL;
        applyViewMode();
        tft.fillScreen(0x0000);
        drawDivider();
        saveConfig();
        Serial.println(F("OK:VIEW_FULL"));
      } else if (param == "SPLIT") {
        viewMode = VIEW_SPLIT;
        applyViewMode();
        tft.fillScreen(0x0000);
        drawDivider();
        saveConfig();
        Serial.println(F("OK:VIEW_SPLIT"));
      } else {
        Serial.println(F("ERR:USE_VIEW_FULL_SPLIT_?"));
      }

    } else if (c.startsWith("#ADDRMODE ")) {
      String param = c.substring(10);
      param.trim();
      if (param == "?") {
        Serial.print(F("ADDRMODE="));
        Serial.println(protoAddrMode ? F("ON") : F("OFF"));
      } else if (param == "ON") {
        protoAddrMode = true;
        saveConfig();
        Serial.println(F("OK:ADDRMODE_ON"));
      } else if (param == "OFF") {
        protoAddrMode = false;
        saveConfig();
        Serial.println(F("OK:ADDRMODE_OFF"));
      }

    } else if (c.startsWith("#ADDR ")) {
      String param = c.substring(6);
      param.trim();
      if (param == "?") {
        Serial.print(F("ADDR=0x"));
        if (localAddress < 16) Serial.print(F("0"));
        Serial.println(localAddress, HEX);
      } else {
        uint8_t v = (uint8_t)strtol(param.c_str(), NULL, 0);
        if (v == 0) {
          Serial.println(F("ERR:ADDR_1_255"));
        } else {
          localAddress = v;
          saveConfig();
          Serial.println(F("OK:ADDR"));
        }
      }

    } else if (c.startsWith("#FRAME ")) {
      String param = c.substring(7);
      param.trim();
      if (param == "?") {
        Serial.print(F("FRAME="));
        Serial.println(protoFrameMode ? F("ON") : F("OFF"));
      } else if (param == "ON") {
        protoFrameMode = true;
        saveConfig();
        Serial.println(F("OK:FRAME_ON"));
      } else if (param == "OFF") {
        protoFrameMode = false;
        for (uint8_t id = 1; id <= 3; id++) frameRx[id].stage = 0;
        saveConfig();
        Serial.println(F("OK:FRAME_OFF"));
      } else {
        Serial.println(F("ERR:USE_FRAME_ON_OFF_?"));
      }

    } else if (c.startsWith("#BRIDGE ")) {
      String param = c.substring(8);
      param.trim();
      if (param == "?") {
        Serial.print(F("BRIDGE="));
        Serial.println(bridgeMode ? F("ON") : F("OFF"));
      } else if (param == "ON") {
        bridgeMode = true;
        saveConfig();
        Serial.println(F("OK:BRIDGE_ON"));
      } else if (param == "OFF") {
        bridgeMode = false;
        saveConfig();
        Serial.println(F("OK:BRIDGE_OFF"));
      }

    } else if (c.startsWith("#ADDRSEND ")) {
      String hexData = c.substring(10);
      hexData.trim();
      handleAddrSend(hexData);

    } else if (c.startsWith("#WATCH ")) {
      String param = c.substring(7);
      param.trim();
      if (param == "?" || param == "STATUS") {
        Serial.print(F("WATCH="));
        Serial.println(watchEnabled ? F("ON") : F("OFF"));
        Serial.print(F("WATCHSRC="));
        Serial.println(watchSource);
        Serial.print(F("WATCHMS="));
        Serial.println(watchIntervalMs);
      } else if (param == "STOP") {
        watchEnabled = false;
        Serial.println(F("OK:WATCH_STOP"));
      } else if (param.startsWith("START ")) {
        int sp = param.indexOf(' ', 6);
        if (sp > 0) {
          watchSource = param.substring(6, sp);
          watchSource.toUpperCase();
          watchIntervalMs = max(100UL, (unsigned long)param.substring(sp + 1).toInt());
          watchEnabled = true;
          watchLastMs = 0;
          Serial.println(F("OK:WATCH_START"));
        } else {
          Serial.println(F("ERR:FORMAT #WATCH START <A8..A15|GPIO> <MS>"));
        }
      }

    } else if (c.startsWith("#WDT ")) {
      String param = c.substring(5);
      param.trim();
      param.toUpperCase();
      if (param == "?") {
        Serial.print(F("WDT="));
        if (wdtEnabled) {
          Serial.print(F("ON "));
          Serial.println(wdtTimeoutMs);
        } else {
          Serial.println(F("OFF"));
        }
      } else if (param == "OFF") {
        setWatchdog(false, 0);
        Serial.println(F("OK:WDT_OFF"));
      } else if (param.startsWith("ON ")) {
        uint16_t ms = (uint16_t)max(15, param.substring(3).toInt());
        setWatchdog(true, ms);
        Serial.print(F("OK:WDT_ON "));
        Serial.println(wdtTimeoutMs);
      } else {
        Serial.println(F("ERR:USE_WDT_ON_MS_OFF_?"));
      }

    } else if (c == "#SUMMARY") {
      printSmartSummary();

    } else if (c == "#WHY") {
      Serial.print(F("WHY["));
      Serial.print(lastWhyMs);
      Serial.print(F("ms] "));
      Serial.println(lastWhy);

    } else if (c.startsWith("#PROFILE ")) {
      String param = c.substring(9);
      param.trim();
      param.toUpperCase();
      if (param == "?" || param == "STATUS") {
        Serial.print(F("PROFILE="));
        Serial.println(profileName(smartProfile));
        Serial.println(F("Profiles: SAFE BALANCED PERF CUSTOM"));
      } else if (param == "SAFE") {
        applySmartProfile(PROFILE_SAFE);
        Serial.println(F("OK:PROFILE_SAFE"));
      } else if (param == "BALANCED") {
        applySmartProfile(PROFILE_BALANCED);
        Serial.println(F("OK:PROFILE_BALANCED"));
      } else if (param == "PERF") {
        applySmartProfile(PROFILE_PERF);
        Serial.println(F("OK:PROFILE_PERF"));
      } else if (param == "CUSTOM") {
        smartProfile = PROFILE_CUSTOM;
        Serial.println(F("OK:PROFILE_CUSTOM"));
      } else {
        Serial.println(F("ERR:USE_PROFILE_SAFE_BALANCED_PERF_CUSTOM_?"));
      }

    } else if (c.startsWith("#LINK ") || c.startsWith("#LINKS ")) {
      String param = c.startsWith("#LINKS ") ? c.substring(7) : c.substring(6);
      param.trim();
      param.toUpperCase();
      if (param == "?" || param == "STATUS" || param == "REPORT") {
        printLinkSummary();
      } else if (param == "ON") {
        linkIndicatorsEnabled = true;
        Serial.println(F("OK:LINK_ON"));
      } else if (param == "OFF") {
        linkIndicatorsEnabled = false;
        clearLinkIndicators();
        Serial.println(F("OK:LINK_OFF"));
      } else {
        Serial.println(F("ERR:USE_LINK_ON_OFF_?_REPORT"));
      }

    } else if (c.startsWith("#ESC ")) {
      String param = c.substring(5);
      param.trim();
      if (param == "?" || param == "STATUS") {
        printEscalationStatus();
      } else if (param == "ON") {
        escalation.enabled = true;
        escalation.lastErrTotal = getTotalLinkErrors();
        escalation.lastTimeoutTotal = getTotalTimeouts();
        escalation.lastRuleTotal = ruleFireCount;
        escalation.lastEvalMs = millis();
        Serial.println(F("OK:ESC_ON"));
      } else if (param == "OFF") {
        escalation.enabled = false;
        Serial.println(F("OK:ESC_OFF"));
      } else if (param == "RESET") {
        escalation.fireCount = 0;
        escalation.lastFireMs = 0;
        escalation.lastErrTotal = getTotalLinkErrors();
        escalation.lastTimeoutTotal = getTotalTimeouts();
        escalation.lastRuleTotal = ruleFireCount;
        escalation.lastEvalMs = millis();
        Serial.println(F("OK:ESC_RESET"));
      } else if (param.startsWith("SET ")) {
        // #ESC SET <ERR|TIMEOUT|RULE|ALL> <threshold> <window_ms> <LOG|ALERT|MACRO n>
        String rest = param.substring(4);
        int p1 = rest.indexOf(' ');
        int p2 = (p1 > 0) ? rest.indexOf(' ', p1 + 1) : -1;
        int p3 = (p2 > 0) ? rest.indexOf(' ', p2 + 1) : -1;
        if (p1 < 0 || p2 < 0 || p3 < 0) {
          Serial.println(F("ERR:FORMAT #ESC SET <ERR|TIMEOUT|RULE|ALL> <threshold> <window_ms> <LOG|ALERT|MACRO n>"));
        } else {
          String metric = rest.substring(0, p1);
          String thresholdStr = rest.substring(p1 + 1, p2);
          String windowStr = rest.substring(p2 + 1, p3);
          String actionStr = rest.substring(p3 + 1);
          metric.toUpperCase();
          actionStr.trim();

          uint8_t mask = 0;
          if (metric == "ERR") mask = ESC_METRIC_ERR;
          else if (metric == "TIMEOUT") mask = ESC_METRIC_TIMEOUT;
          else if (metric == "RULE") mask = ESC_METRIC_RULE;
          else if (metric == "ALL") mask = (uint8_t)(ESC_METRIC_ERR | ESC_METRIC_TIMEOUT | ESC_METRIC_RULE);

          uint16_t threshold = (uint16_t)max(1, thresholdStr.toInt());
          uint16_t windowMs = (uint16_t)max(200, windowStr.toInt());
          RuleActionType action;
          uint8_t actionArg = 0;
          if (mask == 0 || !parseEscAction(actionStr, action, actionArg)) {
            Serial.println(F("ERR:ESC_SET_ARGS"));
          } else {
            escalation.metricMask = mask;
            escalation.threshold = threshold;
            escalation.windowMs = windowMs;
            escalation.action = action;
            escalation.actionArg = actionArg;
            escalation.lastErrTotal = getTotalLinkErrors();
            escalation.lastTimeoutTotal = getTotalTimeouts();
            escalation.lastRuleTotal = ruleFireCount;
            escalation.lastEvalMs = millis();
            Serial.println(F("OK:ESC_SET"));
          }
        }
      } else {
        Serial.println(F("ERR:ESC_CMD"));
      }

    } else if (c.startsWith("#SAFE ")) {
      String param = c.substring(6);
      param.trim();
      if (param == "?" || param == "STATUS") {
        printFailsafeStatus();
      } else if (param == "ON") {
        failsafe.enabled = true;
        failsafe.lastErrTotal = getTotalLinkErrors();
        failsafe.lastTimeoutTotal = getTotalTimeouts();
        failsafe.lastEscTotal = escalation.fireCount;
        failsafe.lastEvalMs = millis();
        Serial.println(F("OK:SAFE_ON"));
      } else if (param == "OFF") {
        if (failsafe.active) exitFailsafe();
        failsafe.enabled = false;
        Serial.println(F("OK:SAFE_OFF"));
      } else if (param == "CLEAR") {
        if (failsafe.active) exitFailsafe();
        failsafe.fireCount = 0;
        failsafe.lastErrTotal = getTotalLinkErrors();
        failsafe.lastTimeoutTotal = getTotalTimeouts();
        failsafe.lastEscTotal = escalation.fireCount;
        failsafe.lastEvalMs = millis();
        Serial.println(F("OK:SAFE_CLEAR"));
      } else if (param.startsWith("SET ")) {
        // #SAFE SET <threshold> <window_ms> <hold_ms>
        String rest = param.substring(4);
        int p1 = rest.indexOf(' ');
        int p2 = (p1 > 0) ? rest.indexOf(' ', p1 + 1) : -1;
        if (p1 < 0 || p2 < 0) {
          Serial.println(F("ERR:FORMAT #SAFE SET <threshold> <window_ms> <hold_ms>"));
        } else {
          uint16_t th = (uint16_t)max(1, rest.substring(0, p1).toInt());
          uint16_t win = (uint16_t)max(200, rest.substring(p1 + 1, p2).toInt());
          uint16_t hold = (uint16_t)max(500, rest.substring(p2 + 1).toInt());
          failsafe.threshold = th;
          failsafe.windowMs = win;
          failsafe.holdMs = hold;
          failsafe.lastErrTotal = getTotalLinkErrors();
          failsafe.lastTimeoutTotal = getTotalTimeouts();
          failsafe.lastEscTotal = escalation.fireCount;
          failsafe.lastEvalMs = millis();
          Serial.println(F("OK:SAFE_SET"));
        }
      } else {
        Serial.println(F("ERR:SAFE_CMD"));
      }


    } else if (c.startsWith("#RULE ")) {
      // #RULE ON|OFF|?|LIST|CLEAR <id|ALL>|ADD <expr> [FOR <ms>] THEN <action>
      String param = c.substring(6);
      param.trim();

      if (param == "?") {
        uint8_t count = 0;
        for (uint8_t i = 0; i < MAX_RULES; i++) if (smartRules[i].enabled) count++;
        Serial.print(F("RULES="));
        Serial.println(rulesEnabled ? F("ON") : F("OFF"));
        Serial.print(F("RULE_COUNT="));
        Serial.println(count);
        Serial.print(F("RULE_FIRES="));
        Serial.println(ruleFireCount);
      } else if (param == "ON") {
        rulesEnabled = true;
        Serial.println(F("OK:RULES_ON"));
      } else if (param == "OFF") {
        rulesEnabled = false;
        Serial.println(F("OK:RULES_OFF"));
      } else if (param == "LIST") {
        Serial.println(F("=== RULES ==="));
        bool any = false;
        for (uint8_t i = 0; i < MAX_RULES; i++) {
          if (smartRules[i].enabled) {
            any = true;
            printRule(i, smartRules[i]);
          }
        }
        if (!any) Serial.println(F("(none)"));
      } else if (param.startsWith("CLEAR ")) {
        String arg = param.substring(6);
        arg.trim();
        if (arg == "ALL") {
          for (uint8_t i = 0; i < MAX_RULES; i++) smartRules[i].enabled = false;
          Serial.println(F("OK:RULE_CLEAR_ALL"));
        } else {
          int id = arg.toInt();
          if (id >= 0 && id < MAX_RULES) {
            smartRules[id].enabled = false;
            Serial.print(F("OK:RULE_CLEAR_"));
            Serial.println(id);
          } else {
            Serial.print(F("ERR:RULE_ID_0_TO_"));
            Serial.println(MAX_RULES - 1);
          }
        }
      } else if (param.startsWith("ADD ")) {
        String body = param.substring(4);
        body.trim();

        int thenPos = body.indexOf(" THEN ");
        if (thenPos <= 0) {
          Serial.println(F("ERR:FORMAT #RULE ADD <expr> [FOR <ms>] THEN <MACRO id|LOG|ALERT>"));
        } else {
          String left = body.substring(0, thenPos);
          String actionStr = body.substring(thenPos + 6);
          left.trim();
          actionStr.trim();

          uint16_t holdMs = 0;
          int forPos = left.indexOf(" FOR ");
          if (forPos > 0) {
            holdMs = (uint16_t)max(0, left.substring(forPos + 5).toInt());
            left = left.substring(0, forPos);
            left.trim();
          }

          RuleSourceType srcType;
          uint8_t srcIndex = 0;
          RuleOp op;
          int16_t rhs = 0;
          if (!parseRuleExpr(left, srcType, srcIndex, op, rhs)) {
            Serial.println(F("ERR:RULE_EXPR"));
            Serial.println(F("  Use A8..A15 or GPIO22..GPIO29 with > < >= <= == !="));
          } else {
            RuleActionType action = RULE_ACT_LOG;
            uint8_t actionArg = 0;
            String actionUp = actionStr;
            actionUp.toUpperCase();

            if (actionUp.startsWith("MACRO ")) {
              int id = actionUp.substring(6).toInt();
              if (id < 0 || id >= MAX_MACROS) {
                Serial.print(F("ERR:MACRO_ID_0_TO_"));
                Serial.println(MAX_MACROS - 1);
                return;
              }
              action = RULE_ACT_MACRO;
              actionArg = (uint8_t)id;
            } else if (actionUp == "LOG") {
              action = RULE_ACT_LOG;
            } else if (actionUp == "ALERT") {
              action = RULE_ACT_ALERT;
            } else {
              Serial.println(F("ERR:RULE_ACTION"));
              Serial.println(F("  Actions: MACRO <id>, LOG, ALERT"));
              return;
            }

            int slot = -1;
            for (uint8_t i = 0; i < MAX_RULES; i++) {
              if (!smartRules[i].enabled) {
                slot = i;
                break;
              }
            }
            if (slot < 0) {
              Serial.println(F("ERR:RULE_SLOTS_FULL"));
            } else {
              SmartRule& r = smartRules[slot];
              r.enabled = true;
              r.srcType = srcType;
              r.srcIndex = srcIndex;
              r.op = op;
              r.value = rhs;
              r.holdMs = holdMs;
              r.action = action;
              r.actionArg = actionArg;
              r.cooldownMs = 1000;
              r.sinceMs = 0;
              r.lastFireMs = 0;

              Serial.print(F("OK:RULE_ADD_"));
              Serial.println(slot);
              printRule((uint8_t)slot, r);
            }
          }
        }
      } else {
        Serial.println(F("ERR:RULE_CMD"));
      }

    } else if (c == "#HEALTH") {
      Serial.println(F("=== HEALTH ==="));
      Serial.print(F("Uptime(ms): ")); Serial.println(millis());
      Serial.print(F("FreeRAM: ")); Serial.println(getFreeRAM());
      Serial.print(F("WDT: "));
      if (wdtEnabled) {
        Serial.print(F("ON @"));
        Serial.print(wdtTimeoutMs);
        Serial.println(F("ms"));
      } else {
        Serial.println(F("OFF"));
      }
      Serial.print(F("LastReset: "));
      printResetCause();
      Serial.print(F("GPIO Events: ")); Serial.println(gpioEventCount);
      Serial.print(F("Threshold Events: ")); Serial.println(thresholdEventCount);
      Serial.print(F("Rule Fires: ")); Serial.println(ruleFireCount);
      Serial.print(F("Escalations: ")); Serial.println(escalation.fireCount);
      Serial.print(F("Failsafe Fires: ")); Serial.println(failsafe.fireCount);
      Serial.print(F("Failsafe Active: ")); Serial.println(failsafe.active ? F("YES") : F("NO"));
      Serial.print(F("Addr Local/Pass: ")); Serial.print(addrLocalHandled); Serial.print(F("/")); Serial.println(addrPassthrough);
      for (uint8_t id = 1; id <= 3; id++) {
        Serial.print(F("F")); Serial.print(id);
        Serial.print(F(" RX=")); Serial.print(fpgaRxBytes[id]);
        Serial.print(F(" TX=")); Serial.print(fpgaTxBytes[id]);
        Serial.print(F(" OK=")); Serial.print(fpgaFrameOk[id]);
        Serial.print(F(" ERR=")); Serial.print(fpgaFrameErr[id]);
        Serial.print(F(" DROP=")); Serial.print(fpgaDropped[id]);
        Serial.print(F(" TO=")); Serial.println(fpgaTimeouts[id]);
      }
      Serial.println(F("=============="));

    } else if (c == "#HEALTHRESET") {
      for (uint8_t id = 1; id <= 3; id++) {
        fpgaRxBytes[id] = fpgaTxBytes[id] = fpgaFrameOk[id] = 0;
        fpgaFrameErr[id] = fpgaDropped[id] = fpgaTimeouts[id] = 0;
      }
      gpioEventCount = 0;
      thresholdEventCount = 0;
      addrLocalHandled = 0;
      addrPassthrough = 0;
      ruleFireCount = 0;
      escalation.fireCount = 0;
      escalation.lastFireMs = 0;
      escalation.lastErrTotal = getTotalLinkErrors();
      escalation.lastTimeoutTotal = getTotalTimeouts();
      escalation.lastRuleTotal = 0;
      escalation.lastEvalMs = millis();
      if (failsafe.active) exitFailsafe();
      failsafe.fireCount = 0;
      failsafe.lastErrTotal = getTotalLinkErrors();
      failsafe.lastTimeoutTotal = getTotalTimeouts();
      failsafe.lastEscTotal = escalation.fireCount;
      failsafe.lastEvalMs = millis();
      eventLogCount = 0;
      eventLogHead = 0;
      Serial.println(F("OK:HEALTHRESET"));

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
      uint8_t raw[FRAME_MAX_PAYLOAD];
      uint8_t len = (uint8_t)min((int)data.length(), (int)FRAME_MAX_PAYLOAD);
      for (uint8_t i = 0; i < len; i++) raw[i] = (uint8_t)data[i];
      sendBytesToFPGA(fpga, activeFpga, raw, len, true);
      Serial.println(F("OK:SENT"));

    } else if (c == "#FPGAPING") {
      HardwareSerial& fpga = getFPGA();
      const uint8_t pingData[4] = {'P','I','N','G'};
      sendBytesToFPGA(fpga, activeFpga, pingData, 4, true);
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

    } else if (c.startsWith("#XYPLOT ")) {
      handleXYPlot(c.substring(8));

    } else if (c.startsWith("#LOG ")) {
      // Runtime event log: #LOG ? | #LOG CLEAR | #LOG LAST <n> [TYPE]
      String p = c.substring(5);
      p.trim();
      if (p == "?" || p == "STATUS") {
        Serial.print(F("EVLOG_COUNT=")); Serial.println(eventLogCount);
        Serial.print(F("EVLOG_CAP=")); Serial.println(EVENT_LOG_CAPACITY);
      } else if (p == "CLEAR") {
        eventLogCount = 0;
        eventLogHead = 0;
        Serial.println(F("OK:EVLOG_CLEAR"));
      } else if (p.startsWith("LAST ")) {
        String rest = p.substring(5);
        rest.trim();
        int sp = rest.indexOf(' ');
        String nStr = (sp > 0) ? rest.substring(0, sp) : rest;
        String typeStr = (sp > 0) ? rest.substring(sp + 1) : "ALL";

        int n = nStr.toInt();
        if (n <= 0) n = 10;
        if (n > EVENT_LOG_CAPACITY) n = EVENT_LOG_CAPACITY;

        int8_t ft = parseEventTypeFilter(typeStr);
        if (ft == -2) {
          Serial.println(F("ERR:LOG_TYPE"));
          Serial.println(F("  Types: ALL RULE GPIO THR ERR INFO"));
        } else {
          Serial.println(F("=== EVENT LOG ==="));
          printEventLogLast((uint8_t)n, ft);
        }
      } else {
        Serial.println(F("ERR:LOG_CMD"));
        Serial.println(F("  #LOG ? | #LOG CLEAR | #LOG LAST <n> [TYPE]"));
      }

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

    } else if (c.startsWith("#PWMSET ")) {
      handlePWMSet(c.substring(8));

    } else if (c.startsWith("#PWMSTOP ")) {
      handlePWMStop(c.substring(9));

    } else if (c.startsWith("#PWMFREQ ")) {
      handlePWMFreq(c.substring(9));

    } else if (c == "#MEMINFO") {
      handleMemInfo();

    } else if (c == "#UPTIME") {
      handleUptime();

    } else if (c == "#BENCHMARK") {
      handleBenchmark();

    } else if (c.startsWith("#STATSSTART ")) {
      handleStatsStart(c.substring(12));

    } else if (c.startsWith("#STATSSTOP ")) {
      handleStatsStop(c.substring(11));

    } else if (c.startsWith("#STATSSHOW ")) {
      handleStatsShow(c.substring(11));

    } else if (c == "#STATSRESET") {
      handleStatsReset();

    } else if (c.startsWith("#THRESHOLD ")) {
      handleThresholdSet(c.substring(11));

    } else if (c.startsWith("#THRESHOLDCLEAR ")) {
      handleThresholdClear(c.substring(16));

    } else if (c == "#THRESHOLDSTATUS") {
      handleThresholdStatus();

    } else if (c.startsWith("#HOOK ")) {
      // #HOOK GPIO <pin> <RISE|FALL|BOTH|NONE> <macro> [cooldown]
      // #HOOK THR <idx0-7> <ENTER|EXIT|BOTH|NONE> <macro> [cooldown]
      // #HOOK LIST
      String p = c.substring(6);
      p.trim();
      if (p == "LIST") {
        Serial.println(F("=== HOOKS ==="));
        for (uint8_t i = 0; i < NUM_GPIO; i++) {
          if (gpioHooks[i].enabled) {
            Serial.print(F("GPIO"));
            Serial.print(gpioPins[i]);
            Serial.print(F(" -> M"));
            Serial.print(gpioHooks[i].macroId);
            Serial.print(F(" mask="));
            Serial.print(gpioHooks[i].modeMask, HEX);
            Serial.print(F(" cd="));
            Serial.println(gpioHooks[i].cooldownMs);
          }
        }
        for (uint8_t i = 0; i < 8; i++) {
          if (thresholdHooks[i].enabled) {
            Serial.print(F("THR A"));
            Serial.print(i + 8);
            Serial.print(F(" -> M"));
            Serial.print(thresholdHooks[i].macroId);
            Serial.print(F(" mask="));
            Serial.print(thresholdHooks[i].modeMask, HEX);
            Serial.print(F(" cd="));
            Serial.println(thresholdHooks[i].cooldownMs);
          }
        }
      } else if (p.startsWith("GPIO ")) {
        String rest = p.substring(5);
        int p1 = rest.indexOf(' ');
        int p2 = rest.indexOf(' ', p1 + 1);
        int p3 = rest.indexOf(' ', p2 + 1);
        if (p1 > 0 && p2 > 0) {
          uint8_t pin = rest.substring(0, p1).toInt();
          String type = rest.substring(p1 + 1, p2);
          uint8_t macroId = rest.substring(p2 + 1, p3 > 0 ? p3 : rest.length()).toInt();
          uint16_t cooldown = (p3 > 0) ? rest.substring(p3 + 1).toInt() : 300;
          int8_t idx = -1;
          for (uint8_t i = 0; i < NUM_GPIO; i++) if (gpioPins[i] == pin) idx = i;
          if (idx >= 0 && macroId < MAX_MACROS) {
            type.trim();
            uint8_t mask = 0;
            if (type == "RISE" || type == "RISING") mask = 0x01;
            else if (type == "FALL" || type == "FALLING") mask = 0x02;
            else if (type == "BOTH") mask = 0x03;
            else if (type == "NONE") mask = 0x00;
            gpioHooks[idx].enabled = (mask != 0);
            gpioHooks[idx].modeMask = mask;
            gpioHooks[idx].macroId = macroId;
            gpioHooks[idx].cooldownMs = max((uint16_t)50, cooldown);
            gpioHooks[idx].lastTriggerMs = 0;
            Serial.println(F("OK:HOOK_GPIO"));
          } else {
            Serial.println(F("ERR:HOOK_GPIO_ARGS"));
          }
        }
      } else if (p.startsWith("THR ")) {
        String rest = p.substring(4);
        int p1 = rest.indexOf(' ');
        int p2 = rest.indexOf(' ', p1 + 1);
        int p3 = rest.indexOf(' ', p2 + 1);
        if (p1 > 0 && p2 > 0) {
          uint8_t idx = rest.substring(0, p1).toInt();
          String type = rest.substring(p1 + 1, p2);
          uint8_t macroId = rest.substring(p2 + 1, p3 > 0 ? p3 : rest.length()).toInt();
          uint16_t cooldown = (p3 > 0) ? rest.substring(p3 + 1).toInt() : 300;
          if (idx < 8 && macroId < MAX_MACROS) {
            type.trim();
            uint8_t mask = 0;
            if (type == "ENTER") mask = 0x01;
            else if (type == "EXIT") mask = 0x02;
            else if (type == "BOTH") mask = 0x03;
            else if (type == "NONE") mask = 0x00;
            thresholdHooks[idx].enabled = (mask != 0);
            thresholdHooks[idx].modeMask = mask;
            thresholdHooks[idx].macroId = macroId;
            thresholdHooks[idx].cooldownMs = max((uint16_t)50, cooldown);
            thresholdHooks[idx].lastTriggerMs = 0;
            Serial.println(F("OK:HOOK_THR"));
          } else {
            Serial.println(F("ERR:HOOK_THR_ARGS"));
          }
        }
      }

    } else if (c.startsWith("#SCROLL ")) {
      handleScroll(c.substring(8));

    } else if (c.startsWith("#MACRODEF ")) {
      handleMacroDef(c.substring(10));

    } else if (c.startsWith("#MACRORUN ")) {
      handleMacroRun(c.substring(10));

    } else if (c == "#MACROLIST") {
      handleMacroList();

    } else if (c.startsWith("#MACROCLEAR ")) {
      handleMacroClear(c.substring(12));

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
      Serial.println(F("SAM SMART ARDUINO MONITOR (MEGA ILI9486)"));

    } else if (c == "#VERSION") {
      Serial.print(F("SAM_VERSION=v"));
      Serial.println(F(SAM_FIRMWARE_VERSION));
      Serial.print(F("BUILD="));
      Serial.print(__DATE__);
      Serial.print(F(" "));
      Serial.println(__TIME__);

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

    } else if (c == "#MENU") {
      menuActive = true;
      menuManager->show();
      Serial.println(F("OK:MENU"));

    } else if (c == "#MENUHIDE") {
      menuActive = false;
      tft.fillScreen(0x0000);
      topPosX = topPosY = 0;
      bottomPosX = 0;
      bottomPosY = (viewMode == VIEW_FULL) ? 0 : bottomMinY;
      drawDivider();
      if (buttonsVisible) {
        for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
          drawButton(i);
        }
      }
      Serial.println(F("OK:MENUHIDE"));

    } else if (c == "#MENUBACK") {
      menuManager->goBack();
      Serial.println(F("OK:MENUBACK"));

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
      // Show GPIO register (all pin states as 24-bit value)
      showGPIORegister();

    } else if (c.startsWith("#GPIOSET ")) {
      // #GPIOSET <0x000000-0xFFFFFF or dec> - Set all 24 GPIO bits
      String regStr = c.substring(9);
      regStr.trim();
      uint32_t regVal = (uint32_t)strtoul(regStr.c_str(), NULL, 0);
      setGPIORegister(regVal);

    } else {
      Serial.print(F("ERR:UNKNOWN:"));
      Serial.println(c);
    }
  } else {
    showText(c);
  }
}
