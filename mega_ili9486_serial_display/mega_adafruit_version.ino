/*
 * ILI9486/ILI9488 Serial Display Controller - ARDUINO MEGA 2560 VERSION
 * Using Adafruit TFTLCD Library
 * Adapted from working UNO ILI9341 project
 * Optimized for Arduino Mega 2560 (8KB RAM, 256KB Flash)
 */

#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>

// LCD Pin definitions for Mega 2560 with shield
// Most TFT shields use these pins
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

// Create TFT instance
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Display constants
#define BASE_CHAR_W 6
#define BASE_CHAR_H 8

// Current state
uint16_t screenW = 320;
uint16_t screenH = 480;
uint16_t textColor = 0xFFFF; // White
uint16_t bgColor = 0x0000;   // Black
bool opaqueText = false;
uint8_t textSize = 2;
uint16_t posX = 0;
uint16_t posY = 0;
uint8_t rotation = 0;

// Serial handling - Mega has more RAM
String cmd = "";
bool cmdReady = false;

void setup() {
  Serial.begin(115200); // Mega can handle higher baud rates
  Serial.println(F("Arduino Mega 2560 + ILI9486/ILI9488"));
  Serial.println(F("Serial Display Controller"));
  Serial.println(F("Using Adafruit TFTLCD Library"));
  Serial.println(F("Initializing..."));

  // Try to initialize with ILI9486
  uint16_t identifier = 0x9486;

  tft.reset();
  delay(100);

  identifier = tft.readID();
  Serial.print(F("LCD ID: 0x"));
  Serial.println(identifier, HEX);

  // Handle various ILI948x IDs
  if (identifier == 0x9486 || identifier == 0x9488) {
    Serial.print(F("Detected as ILI"));
    Serial.println(identifier, HEX);
  } else if (identifier == 0x0000 || identifier == 0xFFFF) {
    Serial.println(F("Unknown ID, trying ILI9486"));
    identifier = 0x9486;
  } else {
    Serial.print(F("Unknown display ID: 0x"));
    Serial.println(identifier, HEX);
    Serial.println(F("Trying anyway..."));
  }

  tft.begin(identifier);
  tft.setRotation(rotation);

  screenW = tft.width();
  screenH = tft.height();

  Serial.print(F("Resolution: "));
  Serial.print(screenW);
  Serial.print(F("x"));
  Serial.println(screenH);

  tft.fillScreen(0x0000);
  tft.setTextSize(textSize);
  updateTextColors();

  // Display startup message
  tft.setTextColor(0x07FF); // Cyan
  tft.println(F("MEGA 2560"));
  tft.setTextColor(0x07E0); // Green
  tft.print(F("ILI"));
  tft.println(identifier, HEX);
  tft.setTextColor(0xFFE0); // Yellow
  tft.println(F("Adafruit Lib"));
  tft.println(F("Ready..."));
  tft.println();

  textColor = 0xFFFF; // Reset to white
  updateTextColors();

  posY = tft.getCursorY();

  cmd.reserve(500); // Mega has plenty of RAM

  Serial.println(F("Ready. Type #HELP for commands"));
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
      cmd += c;
    }
  }
}

// Calculate lines for text
uint16_t getLines(const String& txt) {
  uint16_t len = txt.length();
  if (len == 0) return 1;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * textSize);
  if (charsPerLine == 0) charsPerLine = 1;
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
      if (s >= 1 && s <= 10) { // Mega can handle larger sizes
        textSize = s;
        tft.setTextSize(s);
        Serial.print(F("Size:"));
        Serial.println(s);
      }

    } else if (c.startsWith("#POS ")) {
      int sp = c.indexOf(' ', 5);
      if (sp > 0) {
        posX = c.substring(5, sp).toInt();
        posY = c.substring(sp + 1).toInt();
        tft.setCursor(posX, posY);
        Serial.println(F("Pos"));
      }

    } else if (c == "#INFO") {
      Serial.println(F("=== SYSTEM INFO ==="));
      Serial.println(F("Board: Arduino Mega 2560"));
      Serial.println(F("Display: ILI9486/ILI9488"));
      Serial.println(F("Library: Adafruit_TFTLCD"));
      Serial.print(F("Res: "));
      Serial.print(screenW);
      Serial.print(F("x"));
      Serial.println(screenH);
      Serial.print(F("Rotation: "));
      Serial.println(rotation);
      Serial.print(F("Pos: "));
      Serial.print(posX);
      Serial.print(F(","));
      Serial.println(posY);
      Serial.print(F("Size: "));
      Serial.println(textSize);
      Serial.print(F("Ch/Ln: "));
      Serial.println(screenW/(BASE_CHAR_W*textSize));
      Serial.print(F("Text Color: 0x"));
      Serial.println(textColor, HEX);
      Serial.print(F("Bg: "));
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
        rotation = r;
        tft.setRotation(r);
        screenW = tft.width();
        screenH = tft.height();
        tft.fillScreen(0);
        posX = posY = 0;
        Serial.print(F("Rot:"));
        Serial.print(r);
        Serial.print(F(" Res:"));
        Serial.print(screenW);
        Serial.print(F("x"));
        Serial.println(screenH);
      }

    } else if (c.startsWith("#PROG ")) {
      parseProg(c.substring(6));

    } else if (c == "#TEST") {
      testWrap();

    } else if (c == "#HELP") {
      help();

    } else if (c == "#ID") {
      Serial.println(F("COM LCD MEGA ILI9486 ADAFRUIT"));

    } else {
      Serial.println(F("?"));
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
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    textColor = c;
    updateTextColors();
    Serial.print(F("Col:"));
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
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    bgColor = c;
    opaqueText = true;
    updateTextColors();
    Serial.print(F("BgCol:"));
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
  else if (n == "PINK") return 0xFC9F;
  else if (n == "PURPLE") return 0x8010;
  else if (n == "NAVY") return 0x0010;
  else if (n == "DARKGREEN") return 0x03E0;
  else if (n == "MAROON") return 0x7800;
  return 0xFFFF;
}

// Shape parsing
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
  Serial.println(F("Testing text wrapping..."));
  tft.fillScreen(0);
  posX = posY = 0;
  showText("Short");
  delay(500);
  showText("Medium length text here");
  delay(500);
  showText("This is a very long text that will wrap to multiple lines and test our wrapping functionality on the larger ILI9486 display");
  delay(500);
  showText("After wrap");
  Serial.println(F("Test complete"));
}

// Help
void help() {
  Serial.println(F("=== ARDUINO MEGA + ILI9486/ILI9488 COMMANDS ==="));
  Serial.println(F("Adafruit TFTLCD Library Version"));
  Serial.println();
  Serial.println(F("TEXT COMMANDS:"));
  Serial.println(F("  <text>          - Display text"));
  Serial.println(F("  #SIZE <1-10>    - Set text size"));
  Serial.println(F("  #POS <x> <y>    - Set cursor position"));
  Serial.println();
  Serial.println(F("COLOR COMMANDS:"));
  Serial.println(F("  #COLOR <name>   - Set text/draw color"));
  Serial.println(F("  #BGCOLOR <name|NONE> - Set background"));
  Serial.println(F("  Colors: RED, GREEN, BLUE, YELLOW, CYAN,"));
  Serial.println(F("          MAGENTA, WHITE, BLACK, ORANGE,"));
  Serial.println(F("          PINK, PURPLE, NAVY, etc."));
  Serial.println();
  Serial.println(F("DRAWING COMMANDS:"));
  Serial.println(F("  #RECT <x y w h> - Draw rectangle"));
  Serial.println(F("  #FILL <x y w h> - Fill rectangle"));
  Serial.println(F("  #CIRCLE <x y r> - Draw circle"));
  Serial.println(F("  #LINE <x1 y1 x2 y2> - Draw line"));
  Serial.println(F("  #PROG <x y w h %> - Progress bar"));
  Serial.println();
  Serial.println(F("SCREEN COMMANDS:"));
  Serial.println(F("  #CLEAR / #CLR   - Clear screen"));
  Serial.println(F("  #ROT <0-3>      - Set rotation"));
  Serial.println();
  Serial.println(F("INFO & TEST:"));
  Serial.println(F("  #INFO           - Show system info"));
  Serial.println(F("  #ID             - Device ID"));
  Serial.println(F("  #TEST           - Test text wrapping"));
  Serial.println(F("  #HELP           - This help"));
}
