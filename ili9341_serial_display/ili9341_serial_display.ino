/*
 * ILI9341 Serial Display Controller - A+ EDITION
 * Production-ready with robust error handling
 * Optimized for Arduino Uno R3 (under 2KB RAM)
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

// Display constants
#define BASE_CHAR_W 6
#define BASE_CHAR_H 8
#define MAX_CMD_LEN 100
#define PROG_BAR_BORDER 1
#define PROG_BAR_INNER_OFFSET 2

// Current state
uint16_t screenW = 240;
uint16_t screenH = 320;
uint16_t textColor = 0xFFFF; // White
uint16_t bgColor = 0x0000;   // Black (default, but not used initially)
bool opaqueText = false;
uint8_t textSize = 2;
uint16_t posX = 0;
uint16_t posY = 0;

// Serial handling - optimized
String cmd = "";
bool cmdReady = false;

void setup() {
  Serial.begin(9600);
  Serial.println(F("ILI9341 Ready"));
  
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3 || ID == 0x00D3) ID = 0x9341;
  tft.begin(ID);
  tft.setRotation(0);
  tft.fillScreen(0);
  tft.setTextSize(textSize);
  updateTextColors();
  
  tft.println(F("Serial Display"));
  tft.println(F("Ver. 2.0 A+"));
  tft.println(F("Ready..."));
  posY = tft.getCursorY();

  cmd.reserve(MAX_CMD_LEN);
}

void loop() {
  if (cmdReady) {
    processCmd(cmd);
    cmd = "";
    cmdReady = false;
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
        // Discard overflow and signal error
        cmd = "";
        Serial.println(F("ERR:CMD_TOO_LONG"));
        // Flush remaining input until newline
        while (Serial.available() && Serial.read() != '\n');
        cmdReady = false;
        return;
      }
    }
  }
}

// Calculate lines for text
uint16_t getLines(const String& txt) {
  uint16_t len = txt.length();
  if (len == 0) return 1;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * textSize);
  if (charsPerLine == 0) return 1; // Safety check
  return (len + charsPerLine - 1) / charsPerLine;
}

// Display text with wrapping fix
void showText(const String& txt) {
  uint16_t lines = getLines(txt);
  uint16_t lineH = BASE_CHAR_H * textSize;
  uint16_t needH = lines * lineH;

  // Clear if needed
  if (posY + needH > screenH) {
    tft.fillScreen(0);
    posY = 0;
  }

  tft.setCursor(posX, posY);

  // For long text, print in chunks
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * textSize);
  if (charsPerLine == 0) charsPerLine = 1; // Safety check

  if (txt.length() > charsPerLine) {
    int start = 0;
    while (start < txt.length()) {
      int end = min(start + charsPerLine, (int)txt.length());
      tft.print(txt.substring(start, end));
      start = end;
      if (start < txt.length()) {
        posY += lineH;
        tft.setCursor(0, posY);
      }
    }
    tft.println();
  } else {
    tft.println(txt);
  }

  posY = tft.getCursorY();
  posX = 0;

  Serial.print(lines);
  Serial.print(F("L: "));
  Serial.println(txt);
}

// Process commands
void processCmd(String c) {
  c.trim();
  
  if (c.startsWith("#")) {
    c.toUpperCase();
    
    if (c == "#CLEAR" || c == "#CLR") {
      tft.fillScreen(0);
      posX = posY = 0;
      Serial.println(F("CLR"));
      
    } else if (c.startsWith("#COLOR ")) {
      String col = c.substring(7);
      setCol(col);
      
    } else if (c.startsWith("#BGCOLOR ")) {
      String col = c.substring(9);
      setBgCol(col);
      
    } else if (c.startsWith("#SIZE ")) {
      int s = c.substring(6).toInt();
      if (s >= 1 && s <= 5) {
        textSize = s;
        tft.setTextSize(s);
        Serial.print(F("Size:"));
        Serial.println(s);
      } else {
        Serial.print(F("ERR:SIZE_MUST_BE_1-5:"));
        Serial.println(s);
      }

    } else if (c.startsWith("#POS ")) {
      int sp = c.indexOf(' ', 5);
      if (sp > 0) {
        int x = c.substring(5, sp).toInt();
        int y = c.substring(sp + 1).toInt();
        if (x >= 0 && x < screenW && y >= 0 && y < screenH) {
          posX = x;
          posY = y;
          tft.setCursor(posX, posY);
          Serial.print(F("Pos:"));
          Serial.print(posX);
          Serial.print(',');
          Serial.println(posY);
        } else {
          Serial.println(F("ERR:POS_OUT_OF_BOUNDS"));
        }
      } else {
        Serial.println(F("ERR:POS_NEEDS_X_Y"));
      }
      
    } else if (c == "#INFO") {
      Serial.print(F("Res:"));
      Serial.print(screenW);
      Serial.print('x');
      Serial.println(screenH);
      Serial.print(F("Pos:"));
      Serial.print(posX);
      Serial.print(',');
      Serial.println(posY);
      Serial.print(F("Sz:"));
      Serial.println(textSize);
      Serial.print(F("Ch/Ln:"));
      Serial.println(screenW/(BASE_CHAR_W*textSize));
      Serial.print(F("Bg:"));
      if (opaqueText) {
        Serial.print(F("0x"));
        Serial.println(bgColor, HEX);
      } else {
        Serial.println(F("None"));
      }
      
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
        tft.fillScreen(0);
        posX = posY = 0;
        Serial.print(F("Rot:"));
        Serial.println(r);
      } else {
        Serial.print(F("ERR:ROT_MUST_BE_0-3:"));
        Serial.println(r);
      }
      
    } else if (c.startsWith("#PROG ")) {
      parseProg(c.substring(6));
      
    } else if (c == "#TEST") {
      testWrap();
      
    } else if (c == "#HELP") {
      help();
      

    } else if (c == "#ID") {
      Serial.println(F("COM LCD"));

    } else {
      Serial.print(F("ERR:UNKNOWN_CMD:"));
      Serial.println(c);
    }
  } else {
    showText(c);
  }
}

// Update text colors based on current state
void updateTextColors() {
  if (opaqueText) {
    tft.setTextColor(textColor, bgColor);
  } else {
    tft.setTextColor(textColor);
  }
}

// Color setting
void setCol(String& n) {
  n.toUpperCase();
  uint16_t c;
  if (getColorFromName(n, c)) {
    textColor = c;
    updateTextColors();
    Serial.print(F("Col:"));
    Serial.println(n);
  } else {
    Serial.print(F("ERR:INVALID_COLOR:"));
    Serial.println(n);
  }
}

// Background color setting
void setBgCol(String& n) {
  n.trim();
  n.toUpperCase();
  if (n == "NONE") {
    opaqueText = false;
    updateTextColors();
    Serial.println(F("Bg:None"));
    return;
  }
  uint16_t c;
  if (getColorFromName(n, c)) {
    bgColor = c;
    opaqueText = true;
    updateTextColors();
    Serial.print(F("BgCol:"));
    Serial.println(n);
  } else {
    Serial.print(F("ERR:INVALID_BGCOLOR:"));
    Serial.println(n);
  }
}

// Get color from name - returns true if valid, false otherwise
bool getColorFromName(const String& n, uint16_t& colorOut) {
  if (n == "RED") { colorOut = 0xF800; return true; }
  else if (n == "GREEN") { colorOut = 0x07E0; return true; }
  else if (n == "BLUE") { colorOut = 0x001F; return true; }
  else if (n == "YELLOW") { colorOut = 0xFFE0; return true; }
  else if (n == "CYAN") { colorOut = 0x07FF; return true; }
  else if (n == "MAGENTA") { colorOut = 0xF81F; return true; }
  else if (n == "WHITE") { colorOut = 0xFFFF; return true; }
  else if (n == "BLACK") { colorOut = 0x0000; return true; }
  else if (n == "ORANGE") { colorOut = 0xFD20; return true; }
  else if (n == "PINK") { colorOut = 0xF81F; return true; }
  return false; // Invalid color name
}

// Generic parameter parser - eliminates code duplication
uint8_t parseParams(const String& p, int* values, uint8_t maxParams) {
  uint8_t count = 0;
  int last = -1;
  for (int j = 0; j <= p.length() && count < maxParams; j++) {
    if (j == p.length() || p[j] == ' ') {
      if (j > last + 1) { // Avoid empty strings
        values[count++] = p.substring(last + 1, j).toInt();
      }
      last = j;
    }
  }
  return count;
}

// Shape parsing - refactored with validation
void parseRect(String p) {
  int v[4];
  if (parseParams(p, v, 4) == 4) {
    if (v[2] > 0 && v[3] > 0) { // Width and height must be positive
      tft.drawRect(v[0], v[1], v[2], v[3], textColor);
      Serial.println(F("Rect"));
    } else {
      Serial.println(F("ERR:INVALID_RECT_DIMS"));
    }
  } else {
    Serial.println(F("ERR:RECT_NEEDS_4_PARAMS"));
  }
}

void parseFill(String p) {
  int v[4];
  if (parseParams(p, v, 4) == 4) {
    if (v[2] > 0 && v[3] > 0) {
      tft.fillRect(v[0], v[1], v[2], v[3], textColor);
      Serial.println(F("Fill"));
    } else {
      Serial.println(F("ERR:INVALID_FILL_DIMS"));
    }
  } else {
    Serial.println(F("ERR:FILL_NEEDS_4_PARAMS"));
  }
}

void parseCirc(String p) {
  int v[3];
  if (parseParams(p, v, 3) == 3) {
    if (v[2] > 0) { // Radius must be positive
      tft.drawCircle(v[0], v[1], v[2], textColor);
      Serial.println(F("Circ"));
    } else {
      Serial.println(F("ERR:INVALID_RADIUS"));
    }
  } else {
    Serial.println(F("ERR:CIRCLE_NEEDS_3_PARAMS"));
  }
}

void parseLine(String p) {
  int v[4];
  if (parseParams(p, v, 4) == 4) {
    tft.drawLine(v[0], v[1], v[2], v[3], textColor);
    Serial.println(F("Line"));
  } else {
    Serial.println(F("ERR:LINE_NEEDS_4_PARAMS"));
  }
}

void parseProg(String p) {
  int v[5];
  if (parseParams(p, v, 5) == 5) {
    if (v[2] > 0 && v[3] > 0) {
      int pct = constrain(v[4], 0, 100);
      int fw = (v[2] * pct) / 100;
      tft.drawRect(v[0], v[1], v[2], v[3], textColor);
      if (fw > PROG_BAR_INNER_OFFSET) {
        tft.fillRect(v[0] + PROG_BAR_BORDER, v[1] + PROG_BAR_BORDER,
                     fw - PROG_BAR_INNER_OFFSET, v[3] - PROG_BAR_INNER_OFFSET, textColor);
      }
      Serial.print(pct);
      Serial.println(F("%"));
    } else {
      Serial.println(F("ERR:INVALID_PROG_DIMS"));
    }
  } else {
    Serial.println(F("ERR:PROG_NEEDS_5_PARAMS"));
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

// Help - comprehensive
void help() {
  Serial.println(F("== LCD Commands A+ =="));
  Serial.println(F("Text: type & enter"));
  Serial.println(F("#CLR - Clear screen"));
  Serial.println(F("#COLOR <name>"));
  Serial.println(F("  RED|GREEN|BLUE|YELLOW"));
  Serial.println(F("  CYAN|MAGENTA|WHITE"));
  Serial.println(F("  BLACK|ORANGE|PINK"));
  Serial.println(F("#BGCOLOR <name|NONE>"));
  Serial.println(F("#SIZE <1-5>"));
  Serial.println(F("#POS <x> <y>"));
  Serial.println(F("#RECT <x y w h>"));
  Serial.println(F("#FILL <x y w h>"));
  Serial.println(F("#CIRCLE <x y r>"));
  Serial.println(F("#LINE <x1 y1 x2 y2>"));
  Serial.println(F("#PROG <x y w h %>"));
  Serial.println(F("#ROT <0-3>"));
  Serial.println(F("#TEST - Test wrap"));
  Serial.println(F("#INFO - Settings"));
  Serial.println(F("#ID - Device ID"));
  Serial.println(F("#HELP - This help"));
  Serial.println();
  Serial.println(F("Errors prefixed ERR:"));
}