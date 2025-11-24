/*
 * ILI9486/ILI9488 Split Screen MEGA EDITION
 * Arduino Mega 2560 + ILI9486/9488 (320x480)
 * Top=Commands, Bottom=FPGA/Serial Devices
 * Hardware Serial1 for FPGA (TX1=18, RX1=19)
 * Optional Serial2 & Serial3 for additional devices
 * 8KB RAM allows advanced features
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

// Display constants
#define BASE_CHAR_W 6
#define BASE_CHAR_H 8
#define MAX_CMD_LEN 300        // Mega has 8KB RAM
#define MAX_FPGA_BUF 200
#define MAX_HEX_BYTES 128
#define CMD_HISTORY_SIZE 10

// Screen layout
uint16_t screenW = 320;
uint16_t screenH = 480;
uint16_t dividerY;
uint16_t topMaxY;
uint16_t bottomMinY;
uint16_t bottomMaxY;
uint16_t topPosX = 0;
uint16_t topPosY = 0;
bool topOpaqueText = false;
uint16_t bottomPosX = 0;
uint16_t bottomPosY = 0;

// Command handling
String cmd = "";
bool cmdReady = false;
String fpgaBuffer = "";

// Command history
String cmdHistory[CMD_HISTORY_SIZE];
uint8_t historyIndex = 0;
uint8_t historyCount = 0;

// Registers - expanded to 32 for Mega
#define NUM_REGISTERS 32
uint32_t registers[NUM_REGISTERS];

// Register definitions
#define REG_TOP_COLOR      0x00
#define REG_TOP_BGCOLOR    0x01
#define REG_TOP_SIZE       0x02
#define REG_BOT_COLOR      0x03
#define REG_BOT_SIZE       0x04
#define REG_FPGA1_BAUD     0x05
#define REG_FPGA_FRAME     0x06
#define REG_ROTATION       0x07
#define REG_FPGA_USER0     0x08
#define REG_FPGA_USER1     0x09
#define REG_FPGA_USER2     0x0A
#define REG_FPGA_USER3     0x0B
#define REG_FPGA_USER4     0x0C
#define REG_FPGA_USER5     0x0D
#define REG_FPGA_USER6     0x0E
#define REG_FPGA_USER7     0x0F
// New Mega-specific registers
#define REG_FPGA2_BAUD     0x10
#define REG_FPGA3_BAUD     0x11
#define REG_FPGA_SELECT    0x12  // Which serial to use (1,2,3)
#define REG_BOT_BGCOLOR    0x13
#define REG_DIVIDER_COLOR  0x14
#define REG_DIVIDER_POS    0x15  // Custom divider position (%)
#define REG_LOG_ENABLE     0x16
#define REG_STATS_ENABLE   0x17
#define REG_TIMESTAMP      0x18

// Statistics
uint32_t cmdCount = 0;
uint32_t fpgaBytesRx = 0;
uint32_t fpgaBytesTx = 0;

void initRegisters() {
  registers[REG_TOP_COLOR] = 0xFFFF;      // White
  registers[REG_TOP_BGCOLOR] = 0x0000;    // Black
  registers[REG_TOP_SIZE] = 2;
  registers[REG_BOT_COLOR] = 0x07E0;      // Green
  registers[REG_BOT_BGCOLOR] = 0x0000;    // Black
  registers[REG_BOT_SIZE] = 1;
  registers[REG_FPGA1_BAUD] = 9600;
  registers[REG_FPGA2_BAUD] = 9600;
  registers[REG_FPGA3_BAUD] = 9600;
  registers[REG_FPGA_FRAME] = 0;
  registers[REG_ROTATION] = 0;
  registers[REG_FPGA_SELECT] = 1;         // Default to Serial1
  registers[REG_DIVIDER_COLOR] = 0xFFFF;  // White
  registers[REG_DIVIDER_POS] = 50;        // 50% split
  registers[REG_LOG_ENABLE] = 0;
  registers[REG_STATS_ENABLE] = 0;
  registers[REG_TIMESTAMP] = 0;

  for (int i = REG_FPGA_USER0; i <= REG_FPGA_USER7; i++) {
    registers[i] = 0;
  }
}

HardwareSerial& getFPGASerial() {
  switch (registers[REG_FPGA_SELECT]) {
    case 2: return Serial2;
    case 3: return Serial3;
    default: return Serial1;
  }
}

void setup() {
  initRegisters();

  Serial.begin(115200);      // USB/PC connection
  Serial1.begin(registers[REG_FPGA1_BAUD]);  // FPGA1 (pins 18/19)
  Serial2.begin(registers[REG_FPGA2_BAUD]);  // FPGA2 (pins 16/17)
  Serial3.begin(registers[REG_FPGA3_BAUD]);  // FPGA3 (pins 14/15)

  uint16_t ID = tft.readID();
  Serial.print(F("LCD ID: 0x"));
  Serial.println(ID, HEX);

  tft.begin(ID);
  tft.setRotation(registers[REG_ROTATION]);
  screenW = tft.width();
  screenH = tft.height();

  updateDividerPosition();

  tft.fillScreen(0);
  drawDivider();
  topPosY = 0;
  bottomPosY = bottomMinY;

  cmd.reserve(MAX_CMD_LEN);
  fpgaBuffer.reserve(MAX_FPGA_BUF);

  Serial.println(F("=== Arduino MEGA 2560 ==="));
  Serial.println(F("ILI9486/9488 Split Screen"));
  Serial.println(F("Serial1/2/3 FPGA Support"));
  Serial.print(F("Resolution: "));
  Serial.print(screenW);
  Serial.print(F("x"));
  Serial.println(screenH);
  Serial.print(F("RAM: 8KB | Regs: "));
  Serial.println(NUM_REGISTERS);

  // Startup message on top screen
  tft.setTextColor(0x07FF); // Cyan
  tft.setTextSize(2);
  tft.println(F("MEGA 2560"));
  tft.setTextColor(0x07E0); // Green
  tft.println(F("ILI9486/9488"));
  tft.setTextColor(0xFFE0); // Yellow
  tft.println(F("Split Screen"));
  tft.println(F("Ready..."));
  topPosY = tft.getCursorY();
}

void loop() {
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
  while (serial.available()) {
    char c = serial.read();
    Serial.write(c);  // Echo to PC
    fpgaBytesRx++;

    if (c == '\n' || c == '\r') {
      if (fpgaBuffer.length() > 0) {
        if (registers[REG_STATS_ENABLE]) {
          fpgaBuffer = "S" + String(id) + ":" + fpgaBuffer;
        }
        showTextBottom(fpgaBuffer);
        fpgaBuffer = "";
      }
    } else if (isPrintable(c)) {
      if (fpgaBuffer.length() < MAX_FPGA_BUF) {
        fpgaBuffer += c;
      } else {
        showTextBottom(fpgaBuffer);
        fpgaBuffer = "";
        fpgaBuffer += c;
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
      if (cmd.length() < MAX_CMD_LEN) {
        cmd += c;
      } else {
        cmd = "";
        Serial.println(F("ERR:CMD_TOO_LONG"));
        while (Serial.available() && Serial.read() != '\n');
        cmdReady = false;
        return;
      }
    }
  }
}

void updateDividerPosition() {
  uint8_t divPercent = constrain(registers[REG_DIVIDER_POS], 10, 90);
  dividerY = (screenH * divPercent) / 100;
  topMaxY = dividerY - 2;
  bottomMinY = dividerY + 2;
  bottomMaxY = screenH;
}

void drawDivider() {
  uint16_t color = registers[REG_DIVIDER_COLOR];
  tft.drawLine(0, dividerY, screenW, dividerY, color);
}

uint16_t getLines(const String& txt) {
  uint16_t len = txt.length();
  if (len == 0) return 1;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * registers[REG_TOP_SIZE]);
  if (charsPerLine == 0) return 1;
  return (len + charsPerLine - 1) / charsPerLine;
}

uint16_t getLinesBottom(const String& txt) {
  uint16_t len = txt.length();
  if (len == 0) return 1;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * registers[REG_BOT_SIZE]);
  if (charsPerLine == 0) return 1;
  return (len + charsPerLine - 1) / charsPerLine;
}

void showText(const String& txt) {
  uint8_t ts = registers[REG_TOP_SIZE];
  uint16_t lines = getLines(txt);
  uint16_t lineH = BASE_CHAR_H * ts;
  uint16_t needH = lines * lineH;

  if (topPosY + needH > topMaxY) {
    tft.fillRect(0, 0, screenW, topMaxY, 0x0000);
    topPosY = 0;
  }

  tft.setTextSize(ts);
  if (topOpaqueText) {
    tft.setTextColor(registers[REG_TOP_COLOR], registers[REG_TOP_BGCOLOR]);
  } else {
    tft.setTextColor(registers[REG_TOP_COLOR]);
  }
  tft.setCursor(topPosX, topPosY);

  uint16_t charsPerLine = screenW / (BASE_CHAR_W * ts);
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
  uint8_t bs = registers[REG_BOT_SIZE];
  uint16_t lines = getLinesBottom(txt);
  uint16_t lineH = BASE_CHAR_H * bs;
  uint16_t needH = lines * lineH;

  if (bottomPosY + needH > bottomMaxY) {
    tft.fillRect(0, bottomMinY, screenW, bottomMaxY - bottomMinY, registers[REG_BOT_BGCOLOR]);
    bottomPosY = bottomMinY;
  }

  tft.setTextSize(bs);
  tft.setTextColor(registers[REG_BOT_COLOR]);
  tft.setCursor(bottomPosX, bottomPosY);

  uint16_t charsPerLine = screenW / (BASE_CHAR_W * bs);
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

void addToHistory(const String& c) {
  if (c.length() == 0) return;
  cmdHistory[historyIndex] = c;
  historyIndex = (historyIndex + 1) % CMD_HISTORY_SIZE;
  if (historyCount < CMD_HISTORY_SIZE) historyCount++;
}

void processCmd(String c) {
  c.trim();
  if (c.length() == 0) return;

  addToHistory(c);
  cmdCount++;

  // FPGA passthrough
  if (c.startsWith(">>>")) {
    String data = c.substring(3);
    data.trim();
    HardwareSerial& fpga = getFPGASerial();
    fpga.println(data);
    fpgaBytesTx += data.length() + 2;
    Serial.print(F("OK:TX_S"));
    Serial.println(registers[REG_FPGA_SELECT]);
    return;
  }

  if (c.startsWith("#")) {
    c.toUpperCase();

    if (c == "#CLR") {
      tft.fillScreen(0);
      topPosX = topPosY = 0;
      bottomPosX = 0;
      bottomPosY = bottomMinY;
      drawDivider();
      Serial.println(F("OK:CLR"));

    } else if (c.startsWith("#FPGABYTES ")) {
      String hexData = c.substring(11);
      hexData.trim();
      int sent = sendHexBytes(hexData);
      Serial.print(F("OK:SENT_"));
      Serial.println(sent);

    } else if (c.startsWith("#FPGASEL ")) {
      uint8_t sel = c.substring(9).toInt();
      if (sel >= 1 && sel <= 3) {
        registers[REG_FPGA_SELECT] = sel;
        Serial.print(F("OK:FPGA"));
        Serial.println(sel);
      } else {
        Serial.println(F("ERR:FPGA_SEL_1_2_3"));
      }

    } else if (c.startsWith("#W ")) {
      parseRegWrite(c.substring(3));

    } else if (c.startsWith("#R ")) {
      uint8_t addr = strtol(c.substring(3).c_str(), NULL, 16);
      if (addr < NUM_REGISTERS) {
        Serial.print(F("R"));
        if (addr < 16) Serial.print("0");
        Serial.print(addr, HEX);
        Serial.print(F("=0x"));
        Serial.println(registers[addr], HEX);
      } else {
        Serial.println(F("ERR:INVALID_REG_ADDR"));
      }

    } else if (c == "#REGINFO") {
      regInfo();

    } else if (c == "#HISTORY") {
      showHistory();

    } else if (c == "#STATS") {
      showStats();

    } else if (c == "#HELP") {
      help();

    } else if (c == "#ID") {
      Serial.println(F("COM LCD MEGA ILI9486 SPLIT"));

    } else {
      Serial.print(F("ERR:UNKNOWN_CMD:"));
      Serial.println(c);
    }
  } else {
    showText(c);
  }
}

void parseRegWrite(String params) {
  params.trim();
  int sp = params.indexOf(' ');
  if (sp > 0) {
    uint8_t addr = strtol(params.substring(0, sp).c_str(), NULL, 16);
    uint32_t value = strtoul(params.substring(sp + 1).c_str(), NULL, 16);
    if (addr < NUM_REGISTERS) {
      writeRegister(addr, value);
      Serial.print(F("OK:R"));
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print(F("=0x"));
      Serial.println(value, HEX);
    } else {
      Serial.println(F("ERR:INVALID_REG_ADDR"));
    }
  } else {
    Serial.println(F("ERR:W_NEEDS_ADDR_VALUE"));
  }
}

int sendHexBytes(String hexStr) {
  uint8_t buffer[MAX_HEX_BYTES];
  int byteCount = 0;
  int startIdx = 0;

  while (startIdx < hexStr.length() && byteCount < MAX_HEX_BYTES) {
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

  HardwareSerial& fpga = getFPGASerial();
  uint8_t mode = registers[REG_FPGA_FRAME] & 0xFF;
  uint8_t termChar = (registers[REG_FPGA_FRAME] >> 8) & 0xFF;

  if (mode == 1 || mode == 3) {
    fpga.write((uint8_t)byteCount);
  }

  for (int i = 0; i < byteCount; i++) {
    fpga.write(buffer[i]);
  }

  if (mode == 2 || mode == 3) {
    fpga.write(termChar);
  }

  fpgaBytesTx += byteCount;
  return byteCount;
}

void writeRegister(uint8_t addr, uint32_t value) {
  if (addr >= NUM_REGISTERS) return;
  registers[addr] = value;

  if (addr == REG_FPGA1_BAUD) {
    Serial1.end();
    Serial1.begin(value);
  } else if (addr == REG_FPGA2_BAUD) {
    Serial2.end();
    Serial2.begin(value);
  } else if (addr == REG_FPGA3_BAUD) {
    Serial3.end();
    Serial3.begin(value);
  } else if (addr == REG_TOP_BGCOLOR) {
    topOpaqueText = true;
  } else if (addr == REG_ROTATION) {
    uint8_t rot = value & 0x03;
    tft.setRotation(rot);
    screenW = tft.width();
    screenH = tft.height();
    updateDividerPosition();
    tft.fillScreen(0);
    topPosX = topPosY = 0;
    bottomPosX = 0;
    bottomPosY = bottomMinY;
    drawDivider();
  } else if (addr == REG_DIVIDER_POS) {
    updateDividerPosition();
    tft.fillScreen(0);
    topPosX = topPosY = 0;
    bottomPosX = 0;
    bottomPosY = bottomMinY;
    drawDivider();
  } else if (addr == REG_DIVIDER_COLOR) {
    drawDivider();
  }
}

void showHistory() {
  Serial.println(F("=Command History="));
  if (historyCount == 0) {
    Serial.println(F("(empty)"));
    return;
  }

  int start = (historyIndex - historyCount + CMD_HISTORY_SIZE) % CMD_HISTORY_SIZE;
  for (uint8_t i = 0; i < historyCount; i++) {
    int idx = (start + i) % CMD_HISTORY_SIZE;
    Serial.print(i + 1);
    Serial.print(F(": "));
    Serial.println(cmdHistory[idx]);
  }
}

void showStats() {
  Serial.println(F("=== Statistics ==="));
  Serial.print(F("Commands: "));
  Serial.println(cmdCount);
  Serial.print(F("FPGA RX: "));
  Serial.print(fpgaBytesRx);
  Serial.println(F(" bytes"));
  Serial.print(F("FPGA TX: "));
  Serial.print(fpgaBytesTx);
  Serial.println(F(" bytes"));
  Serial.print(F("Free RAM: "));
  Serial.println(freeRam());
  Serial.print(F("Active FPGA: Serial"));
  Serial.println(registers[REG_FPGA_SELECT]);
}

int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void help() {
  Serial.println(F("== MEGA ILI9486 Split Screen =="));
  Serial.println(F("=Register System (32 regs)="));
  Serial.println(F("R00=TopCol R01=TopBG R02=TopSize"));
  Serial.println(F("R03=BotCol R04=BotSize"));
  Serial.println(F("R05=FPGA1Baud R06=FPGAFrame R07=Rotation"));
  Serial.println(F("R08-0F=FPGA User (8x 32-bit)"));
  Serial.println(F("R10=FPGA2Baud R11=FPGA3Baud"));
  Serial.println(F("R12=FPGASelect R13=BotBG"));
  Serial.println(F("R14=DivColor R15=DivPos(%)"));
  Serial.println(F("R16=LogEnable R17=StatsEnable"));
  Serial.println();
  Serial.println(F("=Commands="));
  Serial.println(F("#W addr val - Write reg (hex)"));
  Serial.println(F("#R addr - Read register"));
  Serial.println(F("#CLR - Clear screen"));
  Serial.println(F("#FPGASEL <1|2|3> - Select FPGA"));
  Serial.println(F("#FPGABYTES hex - Send to FPGA"));
  Serial.println(F(">>>text - FPGA passthrough"));
  Serial.println(F("#HISTORY - Show cmd history"));
  Serial.println(F("#STATS - Show statistics"));
  Serial.println(F("#REGINFO - Full reg list"));
  Serial.println(F("#HELP - This help"));
  Serial.println();
  Serial.println(F("=Hardware Serial Ports="));
  Serial.println(F("Serial1: TX1=18, RX1=19"));
  Serial.println(F("Serial2: TX2=16, RX2=17"));
  Serial.println(F("Serial3: TX3=14, RX3=15"));
}

void regInfo() {
  Serial.println(F("=Full Register Map="));
  Serial.println(F("R00=TopCol R01=TopBG R02=TopSize"));
  Serial.println(F("R03=BotCol R04=BotSize"));
  Serial.println(F("R05=FPGA1Baud R06=FPGAFrame R07=Rotation"));
  Serial.println(F("R08-0F=FPGA User (8x 32-bit)"));
  Serial.println(F("R10=FPGA2Baud R11=FPGA3Baud"));
  Serial.println(F("R12=FPGASelect R13=BotBG"));
  Serial.println(F("R14=DivColor R15=DivPos(%)"));
  Serial.println(F("R16=LogEnable R17=StatsEnable"));
  Serial.println(F("R18=Timestamp"));
}
