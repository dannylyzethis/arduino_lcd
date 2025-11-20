/*
 * ILI9341 Serial Display Controller - SPLIT SCREEN VERSION
 * Top half: Serial command display
 * Bottom half: Bidirectional FPGA communication via SoftwareSerial
 *
 * Command prefix system:
 * - # prefix: Arduino commands (#CLR, #COLOR, etc.)
 * - >>> prefix: Direct FPGA forwarding (>>> PING)
 * - No prefix: Display text on top section
 * - FPGA responses: Always forwarded to USB serial
 *
 * FPGA Connection: Pin 10 = RX, Pin 11 = TX
 * LCD Shield: Uses D2-D9 (data), A0-A4 (control)
 * Optimized for Arduino Uno R3 (under 2KB RAM)
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SoftwareSerial.h>

MCUFRIEND_kbv tft;

// SoftwareSerial for FPGA (RX=pin 10, TX=pin 11)
// Pins 10 and 11 are free when LCD uses 8-bit parallel mode
// Note: A4 is used by LCD shield for RST, so we can't use it
SoftwareSerial fpgaSerial(10, 11);

// Display constants
#define BASE_CHAR_W 6
#define BASE_CHAR_H 8

// Split screen layout
uint16_t screenW = 240;
uint16_t screenH = 320;
uint16_t dividerY;  // Y position of divider line
uint16_t topMaxY;   // Max Y for top section
uint16_t bottomMinY; // Min Y for bottom section
uint16_t bottomMaxY; // Max Y for bottom section

// Top section state (Serial commands)
uint16_t topPosX = 0;
uint16_t topPosY = 0;
uint16_t topTextColor = 0xFFFF; // White
uint16_t topBgColor = 0x0000;
bool topOpaqueText = false;
uint8_t topTextSize = 2;

// Bottom section state (FPGA monitor)
uint16_t bottomPosX = 0;
uint16_t bottomPosY = 0;
uint16_t bottomTextColor = 0x07E0; // Green
uint16_t bottomBgColor = 0x0000;
bool bottomOpaqueText = false;
uint8_t bottomTextSize = 1;

// Legacy compatibility (points to top section)
uint16_t textColor = 0xFFFF;
uint16_t bgColor = 0x0000;
bool opaqueText = false;
uint8_t textSize = 2;
uint16_t posX = 0;
uint16_t posY = 0;

// Serial handling - optimized
String cmd = "";
bool cmdReady = false;

// FPGA serial handling
String fpgaBuffer = "";
unsigned long fpgaBaud = 9600; // Default FPGA baud rate

void setup() {
  Serial.begin(9600);
  Serial.println(F("ILI9341 Split Screen Ready"));

  // Initialize FPGA serial
  fpgaSerial.begin(fpgaBaud);

  // Initialize display
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3 || ID == 0x00D3) ID = 0x9341;
  tft.begin(ID);
  tft.setRotation(0);
  screenW = tft.width();
  screenH = tft.height();

  // Calculate split screen layout
  dividerY = screenH / 2;
  topMaxY = dividerY - 2;
  bottomMinY = dividerY + 2;
  bottomMaxY = screenH;
  bottomPosY = bottomMinY;

  tft.fillScreen(0);

  // Draw divider line
  drawDivider();

  // Initialize top section
  tft.setTextSize(topTextSize);
  updateTextColors();
  tft.setCursor(0, 0);
  tft.println(F("CMD Display"));
  tft.println(F("Ver. 2.0 Split"));
  topPosY = tft.getCursorY();

  // Initialize bottom section
  tft.setTextSize(bottomTextSize);
  tft.setTextColor(bottomTextColor);
  tft.setCursor(0, bottomMinY);
  tft.println(F("FPGA Monitor"));
  bottomPosY = tft.getCursorY();

  cmd.reserve(100);
  fpgaBuffer.reserve(80);

  Serial.println(F("Top: Serial Cmds"));
  Serial.println(F("Bottom: FPGA"));
}

void loop() {
  // Process command serial
  if (cmdReady) {
    processCmd(cmd);
    cmd = "";
    cmdReady = false;
  }

  // Process FPGA serial - forward responses to USB
  while (fpgaSerial.available()) {
    char c = fpgaSerial.read();

    // Forward to USB serial
    Serial.write(c);

    // Also display on LCD bottom
    if (c == '\n' || c == '\r') {
      if (fpgaBuffer.length() > 0) {
        showTextBottom(fpgaBuffer);
        fpgaBuffer = "";
      }
    } else if (isPrintable(c)) {
      fpgaBuffer += c;
      // Prevent overflow
      if (fpgaBuffer.length() > 70) {
        showTextBottom(fpgaBuffer);
        fpgaBuffer = "";
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
      cmd += c;
    }
  }
}

// Draw divider line
void drawDivider() {
  tft.drawLine(0, dividerY, screenW, dividerY, 0xFFFF);
}

// Calculate lines for text (top section)
uint8_t getLines(const String& txt) {
  uint8_t len = txt.length();
  if (len == 0) return 1;
  uint8_t charsPerLine = screenW / (BASE_CHAR_W * topTextSize);
  return (len + charsPerLine - 1) / charsPerLine;
}

// Calculate lines for text (bottom section)
uint8_t getLinesBottom(const String& txt) {
  uint8_t len = txt.length();
  if (len == 0) return 1;
  uint8_t charsPerLine = screenW / (BASE_CHAR_W * bottomTextSize);
  return (len + charsPerLine - 1) / charsPerLine;
}

// Display text in TOP section with wrapping fix
void showText(const String& txt) {
  uint8_t lines = getLines(txt);
  uint16_t lineH = BASE_CHAR_H * topTextSize;
  uint16_t needH = lines * lineH;

  // Clear top section if needed
  if (topPosY + needH > topMaxY) {
    tft.fillRect(0, 0, screenW, topMaxY, 0x0000);
    topPosY = 0;
  }

  // Set text properties for top section
  tft.setTextSize(topTextSize);
  if (topOpaqueText) {
    tft.setTextColor(topTextColor, topBgColor);
  } else {
    tft.setTextColor(topTextColor);
  }
  tft.setCursor(topPosX, topPosY);

  // For long text, print in chunks
  uint8_t charsPerLine = screenW / (BASE_CHAR_W * topTextSize);
  if (txt.length() > charsPerLine) {
    int start = 0;
    while (start < txt.length()) {
      int end = min(start + charsPerLine, (int)txt.length());
      tft.print(txt.substring(start, end));
      start = end;
      if (start < txt.length()) {
        topPosY += lineH;
        if (topPosY >= topMaxY) break; // Don't overflow into bottom
        tft.setCursor(0, topPosY);
      }
    }
    tft.println();
  } else {
    tft.println(txt);
  }

  topPosY = tft.getCursorY();
  if (topPosY >= topMaxY) topPosY = 0; // Wrap around
  topPosX = 0;

  // Update legacy variables
  posY = topPosY;
  posX = topPosX;
  textSize = topTextSize;
  textColor = topTextColor;

  Serial.print(lines);
  Serial.print(F("L: "));
  Serial.println(txt);
}

// Display text in TOP section (simple wrapper)
void showTextTop(const String& txt) {
  showText(txt);
}

// Display text in BOTTOM section (FPGA monitor)
void showTextBottom(const String& txt) {
  uint8_t lines = getLinesBottom(txt);
  uint16_t lineH = BASE_CHAR_H * bottomTextSize;
  uint16_t needH = lines * lineH;

  // Clear bottom section if needed
  if (bottomPosY + needH > bottomMaxY) {
    tft.fillRect(0, bottomMinY, screenW, bottomMaxY - bottomMinY, 0x0000);
    bottomPosY = bottomMinY;
  }

  // Set text properties for bottom section
  tft.setTextSize(bottomTextSize);
  if (bottomOpaqueText) {
    tft.setTextColor(bottomTextColor, bottomBgColor);
  } else {
    tft.setTextColor(bottomTextColor);
  }
  tft.setCursor(bottomPosX, bottomPosY);

  // For long text, print in chunks
  uint8_t charsPerLine = screenW / (BASE_CHAR_W * bottomTextSize);
  if (txt.length() > charsPerLine) {
    int start = 0;
    while (start < txt.length()) {
      int end = min(start + charsPerLine, (int)txt.length());
      tft.print(txt.substring(start, end));
      start = end;
      if (start < txt.length()) {
        bottomPosY += lineH;
        if (bottomPosY >= bottomMaxY) break; // Don't overflow
        tft.setCursor(0, bottomPosY);
      }
    }
    tft.println();
  } else {
    tft.println(txt);
  }

  bottomPosY = tft.getCursorY();
  if (bottomPosY >= bottomMaxY) bottomPosY = bottomMinY; // Wrap around
  bottomPosX = 0;
}

// Process commands
void processCmd(String c) {
  c.trim();

  // Direct FPGA forwarding with >>> prefix
  if (c.startsWith(">>>")) {
    String data = c.substring(3);
    data.trim();
    fpgaSerial.println(data);
    Serial.print(F("[FPGA>] "));
    Serial.println(data);
    return;
  }

  if (c.startsWith("#")) {
    c.toUpperCase();
    
    if (c == "#CLEAR" || c == "#CLR") {
      tft.fillRect(0, 0, screenW, topMaxY, 0x0000);
      topPosX = topPosY = 0;
      posX = posY = 0;
      drawDivider();
      Serial.println(F("CLR Top"));
      
    } else if (c.startsWith("#COLOR ")) {
      String col = c.substring(7);
      setCol(col);
      
    } else if (c.startsWith("#BGCOLOR ")) {
      String col = c.substring(9);
      setBgCol(col);
      
    } else if (c.startsWith("#SIZE ")) {
      int s = c.substring(6).toInt();
      if (s >= 1 && s <= 5) {
        topTextSize = s;
        textSize = s;
        tft.setTextSize(s);
        Serial.print(F("Top Size:"));
        Serial.println(s);
      }
      
    } else if (c.startsWith("#POS ")) {
      int sp = c.indexOf(' ', 5);
      if (sp > 0) {
        topPosX = c.substring(5, sp).toInt();
        topPosY = c.substring(sp + 1).toInt();
        posX = topPosX;
        posY = topPosY;
        tft.setCursor(topPosX, topPosY);
        Serial.println(F("Top Pos"));
      }
      
    } else if (c == "#INFO") {
      Serial.print(F("Res:"));
      Serial.print(screenW);
      Serial.print('x');
      Serial.println(screenH);
      Serial.print(F("Divider Y:"));
      Serial.println(dividerY);
      Serial.println(F("== TOP =="));
      Serial.print(F("Pos:"));
      Serial.print(topPosX);
      Serial.print(',');
      Serial.println(topPosY);
      Serial.print(F("Sz:"));
      Serial.println(topTextSize);
      Serial.print(F("Col:0x"));
      Serial.println(topTextColor, HEX);
      Serial.println(F("== BOTTOM =="));
      Serial.print(F("Pos:"));
      Serial.print(bottomPosX);
      Serial.print(',');
      Serial.println(bottomPosY);
      Serial.print(F("Sz:"));
      Serial.println(bottomTextSize);
      Serial.print(F("Col:0x"));
      Serial.println(bottomTextColor, HEX);
      Serial.print(F("FPGA Baud:"));
      Serial.println(fpgaBaud);

    } else if (c.startsWith("#RECT ")) {
      parseRect(c.substring(6));
      
    } else if (c.startsWith("#CIRCLE ")) {
      parseCirc(c.substring(8));
      
    } else if (c.startsWith("#LINE ")) {
      parseLine(c.substring(6));
      
    } else if (c.startsWith("#FILL ")) {
      parseFill(c.substring(6));
      
    } else if (c.startsWith("#ROT")) {
      int r = c.substring(4).toInt();
      if (r >= 0 && r <= 3) {
        tft.setRotation(r);
        screenW = tft.width();
        screenH = tft.height();
        // Recalculate split screen
        dividerY = screenH / 2;
        topMaxY = dividerY - 2;
        bottomMinY = dividerY + 2;
        bottomMaxY = screenH;
        tft.fillScreen(0);
        topPosX = topPosY = 0;
        bottomPosX = 0;
        bottomPosY = bottomMinY;
        posX = posY = 0;
        drawDivider();
        Serial.print(F("Rot:"));
        Serial.println(r);
      }
      
    } else if (c.startsWith("#PROG ")) {
      parseProg(c.substring(6));
      
    } else if (c == "#TEST") {
      testWrap();

    } else if (c == "#CLRBOT") {
      tft.fillRect(0, bottomMinY, screenW, bottomMaxY - bottomMinY, 0x0000);
      bottomPosX = 0;
      bottomPosY = bottomMinY;
      drawDivider();
      Serial.println(F("CLR Bot"));

    } else if (c == "#CLRALL") {
      tft.fillScreen(0);
      topPosX = topPosY = 0;
      bottomPosX = 0;
      bottomPosY = bottomMinY;
      posX = posY = 0;
      drawDivider();
      Serial.println(F("CLR All"));

    } else if (c.startsWith("#BOTSIZE ")) {
      int s = c.substring(9).toInt();
      if (s >= 1 && s <= 5) {
        bottomTextSize = s;
        Serial.print(F("Bot Size:"));
        Serial.println(s);
      }

    } else if (c.startsWith("#BOTCOLOR ")) {
      String col = c.substring(10);
      setBotCol(col);

    } else if (c.startsWith("#FPGABAUD ")) {
      unsigned long baud = c.substring(10).toInt();
      if (baud >= 300 && baud <= 115200) {
        fpgaBaud = baud;
        fpgaSerial.end();
        fpgaSerial.begin(fpgaBaud);
        Serial.print(F("FPGA Baud:"));
        Serial.println(baud);
      }

    } else if (c.startsWith("#FPGASEND ")) {
      String data = c.substring(10);
      fpgaSerial.println(data);
      Serial.print(F("Sent to FPGA: "));
      Serial.println(data);

    } else if (c == "#FPGAPING") {
      fpgaSerial.println(F("PING"));
      Serial.println(F("Ping sent to FPGA"));

    } else if (c == "#HELP") {
      help();
      

    } else if (c == "#ID") {
      Serial.println("COM LCD");
      
    }  else {

      Serial.println(F("?"));
    }
  } else {
    showText(c);
  }
}

// Update text colors based on current state (top section)
void updateTextColors() {
  if (topOpaqueText) {
    tft.setTextColor(topTextColor, topBgColor);
  } else {
    tft.setTextColor(topTextColor);
  }
  // Update legacy variables
  textColor = topTextColor;
  bgColor = topBgColor;
  opaqueText = topOpaqueText;
}

// Color setting (top section)
void setCol(String& n) {
  n.toUpperCase();
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    topTextColor = c;
    textColor = c;
    updateTextColors();
    Serial.print(F("Top Col:"));
    Serial.println(n);
  }
}

// Background color setting (top section)
void setBgCol(String& n) {
  n.trim();
  n.toUpperCase();
  if (n == "NONE") {
    topOpaqueText = false;
    opaqueText = false;
    updateTextColors();
    Serial.println(F("Top Bg:None"));
    return;
  }
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    topBgColor = c;
    topOpaqueText = true;
    bgColor = c;
    opaqueText = true;
    updateTextColors();
    Serial.print(F("Top BgCol:"));
    Serial.println(n);
  }
}

// Color setting for bottom section
void setBotCol(String& n) {
  n.toUpperCase();
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    bottomTextColor = c;
    Serial.print(F("Bot Col:"));
    Serial.println(n);
  }
}

// Get color from name
uint16_t getColorFromName(String& n) {
  if (n == "RED") return 0xF800;
  else if (n == "GREEN") return 0x07E0;
  else if (n == "BLUE") return 0x001F;
  else if (n == "YELLOW") return 0xFFE0;
  else if (n == "CYAN") return 0x07FF;
  else if (n == "MAGENTA") return 0xF81F;
  else if (n == "WHITE") return 0xFFFF;
  else if (n == "BLACK") return 0x0000;
  else if (n == "ORANGE") return 0xFD20;
  else if (n == "PINK") return 0xF81F;
  return 0xFFFF; // Default to white if invalid, but could ignore
}

// Shape parsing - compact
void parseRect(String p) {
  int v[4], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 4) {
    tft.drawRect(v[0], v[1], v[2], v[3], textColor);
    Serial.println(F("Rect"));
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
    tft.fillRect(v[0], v[1], v[2], v[3], textColor);
    Serial.println(F("Fill"));
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
    tft.drawCircle(v[0], v[1], v[2], textColor);
    Serial.println(F("Circ"));
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
    tft.drawLine(v[0], v[1], v[2], v[3], textColor);
    Serial.println(F("Line"));
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
    tft.drawRect(v[0], v[1], v[2], v[3], textColor);
    if (fw > 2) {
      tft.fillRect(v[0]+1, v[1]+1, fw-2, v[3]-2, textColor);
    }
    Serial.print(pct);
    Serial.println(F("%"));
  }
}

// Test wrapping
void testWrap() {
  Serial.println(F("Test..."));
  showText("Short");
  delay(500);
  showText("Medium length text here");
  delay(500);
  showText("This is a very long text that will wrap to multiple lines and test our wrapping fix");
  delay(500);
  showText("After wrap");
  Serial.println(F("Done"));
}

// Help - compact
void help() {
  Serial.println(F("== LCD Split Screen =="));
  Serial.println(F("Text: type & enter"));
  Serial.println(F("== TOP Section =="));
  Serial.println(F("#CLR - Clear top"));
  Serial.println(F("#COLOR <name>"));
  Serial.println(F("#BGCOLOR <name|NONE>"));
  Serial.println(F("#SIZE <1-5>"));
  Serial.println(F("#POS <x> <y>"));
  Serial.println(F("== BOTTOM (FPGA) =="));
  Serial.println(F("#CLRBOT - Clear bot"));
  Serial.println(F("#CLRALL - Clear all"));
  Serial.println(F("#BOTSIZE <1-5>"));
  Serial.println(F("#BOTCOLOR <name>"));
  Serial.println(F("#FPGABAUD <rate>"));
  Serial.println(F("#FPGASEND <data> - Send to FPGA"));
  Serial.println(F("#FPGAPING - Send ping"));
  Serial.println(F(">>> <data> - Direct FPGA forward"));
  Serial.println(F("  (Response auto-forwarded to USB)"));
  Serial.println(F("== Graphics =="));
  Serial.println(F("#RECT <x y w h>"));
  Serial.println(F("#FILL <x y w h>"));
  Serial.println(F("#CIRCLE <x y r>"));
  Serial.println(F("#LINE <x1 y1 x2 y2>"));
  Serial.println(F("#PROG <x y w h %>"));
  Serial.println(F("== Other =="));
  Serial.println(F("#ROT <0-3>"));
  Serial.println(F("#TEST - Test wrap"));
  Serial.println(F("#INFO - Settings"));
  Serial.println(F("#ID - COM LCD"));
}