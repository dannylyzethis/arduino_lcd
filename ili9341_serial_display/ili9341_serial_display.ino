/*
 * ILI9341 Split Screen MEGA A+ EDITION
 * Arduino Mega 2560 - Production Ready
 * Top=Commands, Bottom=FPGA/Serial Devices
 * Hardware Serial1 for FPGA (TX1=18, RX1=19)
 * Optional Serial2 & Serial3 for additional devices
 * 8KB RAM allows advanced features
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

MCUFRIEND_kbv tft;

// Touchscreen pins
#define YP A3
#define XM A2
#define YM 9
#define XP 8
#define TS_MINX 120
#define TS_MAXX 900
#define TS_MINY 70
#define TS_MAXY 920
#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Display constants
#define BASE_CHAR_W 6
#define BASE_CHAR_H 8
#define MAX_CMD_LEN 200        // Mega has 8KB RAM
#define MAX_FPGA_BUF 150
#define MAX_HEX_BYTES 128
#define CMD_HISTORY_SIZE 5

// Screen layout
uint16_t screenW = 240;
uint16_t screenH = 320;
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
#define REG_MEGA_USER0     0x19
#define REG_MEGA_USER1     0x1A
#define REG_MEGA_USER2     0x1B
#define REG_MEGA_USER3     0x1C
#define REG_MEGA_USER4     0x1D
#define REG_MEGA_USER5     0x1E
#define REG_MEGA_USER6     0x1F

// Touch buttons
enum ResponseType {
  RESP_NONE,
  RESP_TEMP,
  RESP_STATUS,
  RESP_COUNTER,
  RESP_RAW
};

struct Button {
  uint16_t x, y, w, h;
  const char* label;
  uint8_t cmdBytes[8];
  uint8_t cmdLen;
  ResponseType respType;
  uint16_t color;
};

#define NUM_BUTTONS 4
Button buttons[NUM_BUTTONS];
bool buttonsVisible = false;
uint16_t buttonTextY;

unsigned long lastTouch = 0;
#define TOUCH_DEBOUNCE 200
#define RESPONSE_TIMEOUT 500

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
  for (int i = REG_MEGA_USER0; i <= REG_MEGA_USER6; i++) {
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

  Serial.begin(9600);      // USB/PC connection
  Serial1.begin(registers[REG_FPGA1_BAUD]);  // FPGA1 (pins 18/19)
  Serial2.begin(registers[REG_FPGA2_BAUD]);  // FPGA2 (pins 16/17)
  Serial3.begin(registers[REG_FPGA3_BAUD]);  // FPGA3 (pins 14/15)

  uint16_t ID = tft.readID();
  if (ID == 0xD3D3 || ID == 0x00D3) ID = 0x9341;
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
  initButtons();

  Serial.println(F("=== Arduino MEGA A+ ==="));
  Serial.println(F("ILI9341 Split Screen v3.0"));
  Serial.println(F("Serial1/2/3 FPGA Support"));
  Serial.print(F("RAM: 8KB | Regs: "));
  Serial.println(NUM_REGISTERS);
}

void loop() {
  if (buttonsVisible) {
    checkTouch();
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
      // Buffer overflow protection
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

    } else if (c == "#SHOWBTNS") {
      showButtons();
      Serial.println(F("OK:BTNS_ON"));

    } else if (c == "#HIDEBTNS") {
      hideButtons();
      Serial.println(F("OK:BTNS_OFF"));

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

    } else if (c.startsWith("#TRIANGLE ")) {
      parseTriangle(c.substring(10));

    } else if (c.startsWith("#ROUNDRECT ")) {
      parseRoundRect(c.substring(11));

    } else if (c.startsWith("#ARC ")) {
      parseArc(c.substring(5));

    } else if (c == "#HELP") {
      help();

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

// Generic parameter parser
uint8_t parseParams(const String& p, int* values, uint8_t maxParams) {
  uint8_t count = 0;
  int last = -1;
  for (int j = 0; j <= p.length() && count < maxParams; j++) {
    if (j == p.length() || p[j] == ' ') {
      if (j > last + 1) {
        values[count++] = p.substring(last + 1, j).toInt();
      }
      last = j;
    }
  }
  return count;
}

void parseTriangle(String p) {
  int v[6];
  if (parseParams(p, v, 6) == 6) {
    tft.drawTriangle(v[0], v[1], v[2], v[3], v[4], v[5], registers[REG_TOP_COLOR]);
    Serial.println(F("OK:TRIANGLE"));
  } else {
    Serial.println(F("ERR:TRIANGLE_NEEDS_6_PARAMS"));
  }
}

void parseRoundRect(String p) {
  int v[5];
  if (parseParams(p, v, 5) == 5) {
    if (v[2] > 0 && v[3] > 0 && v[4] >= 0) {
      tft.drawRoundRect(v[0], v[1], v[2], v[3], v[4], registers[REG_TOP_COLOR]);
      Serial.println(F("OK:ROUNDRECT"));
    } else {
      Serial.println(F("ERR:INVALID_ROUNDRECT_DIMS"));
    }
  } else {
    Serial.println(F("ERR:ROUNDRECT_NEEDS_5_PARAMS"));
  }
}

void parseArc(String p) {
  int v[6];
  if (parseParams(p, v, 6) == 6) {
    // Approximate arc with lines (GFX library doesn't have arc primitive)
    int x = v[0], y = v[1], r = v[2];
    int start = v[3], end = v[4];
    for (int angle = start; angle <= end; angle += 5) {
      float rad = angle * 3.14159 / 180.0;
      int x1 = x + r * cos(rad);
      int y1 = y + r * sin(rad);
      tft.drawPixel(x1, y1, registers[REG_TOP_COLOR]);
    }
    Serial.println(F("OK:ARC"));
  } else {
    Serial.println(F("ERR:ARC_NEEDS_6_PARAMS"));
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
    initButtons();
    if (buttonsVisible) {
      buttonsVisible = false;
      showButtons();
    }
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
  Serial.println(F("== MEGA A+ Edition =="));
  Serial.println(F("=Register System (32 regs)="));
  Serial.println(F("R00-07: Display & FPGA1"));
  Serial.println(F("R08-0F: FPGA User Data"));
  Serial.println(F("R10-1F: Mega Features"));
  Serial.println();
  Serial.println(F("Key Registers:"));
  Serial.println(F("  R12=FPGA Select (1/2/3)"));
  Serial.println(F("  R13=Bot BG Color"));
  Serial.println(F("  R14=Divider Color"));
  Serial.println(F("  R15=Divider Pos (%)"));
  Serial.println();
  Serial.println(F("=Commands="));
  Serial.println(F("#W addr val - Write reg (hex)"));
  Serial.println(F("#R addr - Read register"));
  Serial.println(F("#CLR - Clear screen"));
  Serial.println(F("#FPGASEL <1|2|3> - Select FPGA"));
  Serial.println(F("#FPGABYTES hex - Send to FPGA"));
  Serial.println(F(">>>text - FPGA passthrough"));
  Serial.println(F("#SHOWBTNS / #HIDEBTNS"));
  Serial.println(F("#TRIANGLE x1 y1 x2 y2 x3 y3"));
  Serial.println(F("#ROUNDRECT x y w h r"));
  Serial.println(F("#ARC x y r start end"));
  Serial.println(F("#HISTORY - Show cmd history"));
  Serial.println(F("#STATS - Show statistics"));
  Serial.println(F("#REGINFO - Full reg list"));
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
  Serial.println(F("R18=Timestamp R19-1F=Mega User"));
}

void initButtons() {
  uint16_t btnHeight = 40;
  uint16_t btnWidth = (screenW - 10) / 4;
  uint16_t btnY = bottomMaxY - btnHeight - 2;
  buttonTextY = btnY - 2;

  buttons[0] = {5, btnY, btnWidth, btnHeight, "T", {0x54, 0x45, 0x4D, 0x50}, 4, RESP_TEMP, 0xFD20};
  buttons[1] = {5 + btnWidth + 2, btnY, btnWidth, btnHeight, "S", {0x53, 0x54, 0x41, 0x54}, 4, RESP_STATUS, 0x07E0};
  buttons[2] = {5 + (btnWidth + 2) * 2, btnY, btnWidth, btnHeight, "C", {0x43, 0x4E, 0x54, 0x52}, 4, RESP_COUNTER, 0x07FF};
  buttons[3] = {5 + (btnWidth + 2) * 3, btnY, btnWidth, btnHeight, "D", {0x44, 0x41, 0x54, 0x41}, 4, RESP_RAW, 0x001F};
}

void drawButton(uint8_t idx) {
  Button &btn = buttons[idx];
  tft.fillRect(btn.x, btn.y, btn.w, btn.h, btn.color);
  tft.drawRect(btn.x, btn.y, btn.w, btn.h, 0xFFFF);
  tft.setTextColor(0x0000);
  tft.setTextSize(2);

  uint8_t labelLen = strlen(btn.label);
  int16_t textW = labelLen * 6 * 2;
  int16_t textH = 8 * 2;
  int16_t textX = btn.x + (btn.w - textW) / 2;
  int16_t textY = btn.y + (btn.h - textH) / 2;

  tft.setCursor(textX, textY);
  tft.print(btn.label);
}

void showButtons() {
  if (buttonsVisible) return;
  bottomMaxY = buttonTextY;
  tft.fillRect(0, bottomMinY, screenW, screenH - bottomMinY, 0x0000);
  bottomPosX = 0;
  bottomPosY = bottomMinY;

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    drawButton(i);
  }

  buttonsVisible = true;
  drawDivider();
}

void hideButtons() {
  if (!buttonsVisible) return;
  bottomMaxY = screenH;
  tft.fillRect(0, buttonTextY, screenW, screenH - buttonTextY, 0x0000);
  buttonsVisible = false;
  drawDivider();
}

void checkTouch() {
  unsigned long now = millis();
  if (now - lastTouch < TOUCH_DEBOUNCE) return;

  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z < MINPRESSURE || p.z > MAXPRESSURE) return;

  int16_t px, py;
  uint8_t rotation = tft.getRotation();

  switch(rotation) {
    case 0:
      px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
      py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
      break;
    case 1:
      px = map(p.y, TS_MINY, TS_MAXY, 0, screenW);
      py = map(p.x, TS_MAXX, TS_MINX, 0, screenH);
      break;
    case 2:
      px = map(p.x, TS_MAXX, TS_MINX, 0, screenW);
      py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
      break;
    case 3:
      px = map(p.y, TS_MAXY, TS_MINY, 0, screenW);
      py = map(p.x, TS_MINX, TS_MAXX, 0, screenH);
      break;
  }

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    Button &btn = buttons[i];
    if (px >= btn.x && px < (btn.x + btn.w) &&
        py >= btn.y && py < (btn.y + btn.h)) {
      lastTouch = now;
      handleButtonPress(i);
      break;
    }
  }
}

void handleButtonPress(uint8_t btnIdx) {
  Button &btn = buttons[btnIdx];
  HardwareSerial& fpga = getFPGASerial();

  while (fpga.available()) {
    fpga.read();
  }

  for (uint8_t i = 0; i < btn.cmdLen; i++) {
    fpga.write(btn.cmdBytes[i]);
  }

  if (btn.respType != RESP_NONE) {
    processButtonResponse(btn);
  }
}

void processButtonResponse(Button &btn) {
  HardwareSerial& fpga = getFPGASerial();
  uint8_t respBytes[8];
  uint8_t bytesRead = 0;
  unsigned long startTime = millis();

  uint8_t expectedBytes = 0;
  switch (btn.respType) {
    case RESP_STATUS:  expectedBytes = 1; break;
    case RESP_TEMP:    expectedBytes = 2; break;
    case RESP_COUNTER: expectedBytes = 2; break;
    case RESP_RAW:     expectedBytes = 4; break;
    default: return;
  }

  while (bytesRead < expectedBytes && (millis() - startTime) < RESPONSE_TIMEOUT) {
    if (fpga.available()) {
      respBytes[bytesRead++] = fpga.read();
    }
  }

  if (bytesRead == 0) {
    showTextBottom(F("?"));
    return;
  }

  String result = "";

  switch (btn.respType) {
    case RESP_TEMP: {
      if (bytesRead >= 2) {
        int16_t temp = (respBytes[0] << 8) | respBytes[1];
        float tempC = temp / 10.0;
        result = String(tempC, 1) + "C";
      }
      break;
    }
    case RESP_STATUS: {
      result = "0x";
      if (respBytes[0] < 16) result += "0";
      result += String(respBytes[0], HEX);
      break;
    }
    case RESP_COUNTER: {
      if (bytesRead >= 2) {
        uint16_t count = (respBytes[0] << 8) | respBytes[1];
        result = String(count);
      }
      break;
    }
    case RESP_RAW: {
      for (uint8_t i = 0; i < bytesRead; i++) {
        if (i > 0) result += " ";
        if (respBytes[i] < 16) result += "0";
        result += String(respBytes[i], HEX);
      }
      break;
    }
    default:
      result = "?";
      break;
  }

  showTextBottom(result);
}
