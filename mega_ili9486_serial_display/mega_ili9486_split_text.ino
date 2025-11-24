/*
 * ILI9486/9488 Split Screen MEGA - TEXT-BASED EDITION
 * Arduino Mega 2560 + ILI9486/9488 (320x480)
 * Top=Commands, Bottom=FPGA/Serial Devices
 * Hardware Serial1/2/3 for FPGA (easier than registers!)
 *
 * Serial Ports:
 * - Serial:  USB/PC (115200 baud)
 * - Serial1: FPGA1 (TX1=18, RX1=19) - Default
 * - Serial2: FPGA2 (TX2=16, RX2=17)
 * - Serial3: FPGA3 (TX3=14, RX3=15)
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

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
  tft.println();

  topPosY = tft.getCursorY();

  Serial.println(F("=== MEGA Split Screen ==="));
  Serial.println(F("Text-Based Commands"));
  Serial.print(F("Resolution: "));
  Serial.print(screenW);
  Serial.print(F("x"));
  Serial.println(screenH);
  Serial.println(F("Type #HELP for commands"));
  Serial.println(F("READY"));
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

void processCmd(String c) {
  c.trim();
  if (c.length() == 0) return;

  // FPGA passthrough
  if (c.startsWith(">>>")) {
    String data = c.substring(3);
    data.trim();
    HardwareSerial& fpga = getFPGA();
    fpga.println(data);
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

    } else if (c.startsWith("#FPGASEL ")) {
      uint8_t sel = c.substring(9).toInt();
      if (sel >= 1 && sel <= 3) {
        activeFpga = sel;
        Serial.print(F("OK:FPGA"));
        Serial.println(sel);
      } else {
        Serial.println(F("ERR:USE_1_2_3"));
      }

    } else if (c.startsWith("#FPGA1BAUD ")) {
      unsigned long baud = c.substring(11).toInt();
      if (baud >= 300 && baud <= 115200) {
        fpga1Baud = baud;
        Serial1.end();
        Serial1.begin(fpga1Baud);
        Serial.print(F("OK:FPGA1_"));
        Serial.println(baud);
      }

    } else if (c.startsWith("#FPGA2BAUD ")) {
      unsigned long baud = c.substring(11).toInt();
      if (baud >= 300 && baud <= 115200) {
        fpga2Baud = baud;
        Serial2.end();
        Serial2.begin(fpga2Baud);
        Serial.print(F("OK:FPGA2_"));
        Serial.println(baud);
      }

    } else if (c.startsWith("#FPGA3BAUD ")) {
      unsigned long baud = c.substring(11).toInt();
      if (baud >= 300 && baud <= 115200) {
        fpga3Baud = baud;
        Serial3.end();
        Serial3.begin(fpga3Baud);
        Serial.print(F("OK:FPGA3_"));
        Serial.println(baud);
      }

    } else if (c.startsWith("#FPGASEND ")) {
      String data = c.substring(10);
      HardwareSerial& fpga = getFPGA();
      fpga.println(data);
      Serial.println(F("OK:SENT"));

    } else if (c == "#FPGAPING") {
      HardwareSerial& fpga = getFPGA();
      fpga.println(F("PING"));
      Serial.println(F("OK:PING"));

    } else if (c.startsWith("#FPGABYTES ")) {
      String hexData = c.substring(11);
      hexData.trim();
      int sent = sendHexBytes(hexData);
      Serial.print(F("OK:SENT_"));
      Serial.println(sent);

    } else if (c == "#HELP") {
      help();

    } else if (c == "#ID") {
      Serial.println(F("COM LCD MEGA ILI9486 SPLIT TEXT"));

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
  Serial.println(F("  #FPGASEL <1|2|3> - Select FPGA"));
  Serial.println(F("  #FPGA1BAUD <baud>"));
  Serial.println(F("  #FPGA2BAUD <baud>"));
  Serial.println(F("  #FPGA3BAUD <baud>"));
  Serial.println(F("  #FPGASEND <text>"));
  Serial.println(F("  #FPGAPING"));
  Serial.println(F("  #FPGABYTES <hex>"));
  Serial.println();
  Serial.println(F("DRAWING:"));
  Serial.println(F("  #RECT <x y w h>"));
  Serial.println(F("  #FILL <x y w h>"));
  Serial.println(F("  #CIRCLE <x y r>"));
  Serial.println(F("  #LINE <x1 y1 x2 y2>"));
  Serial.println(F("  #PROG <x y w h %>"));
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
