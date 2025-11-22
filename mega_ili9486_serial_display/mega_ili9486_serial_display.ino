/*
 * ILI9486 Serial Display Controller - ARDUINO MEGA 2560 VERSION
 * Enhanced Edition v2.1 with Advanced Features
 * Target: Arduino Mega 2560 + ILI9486 320x480 Display
 * Memory: 8KB RAM, 256KB Flash (vs Uno: 2KB RAM, 32KB Flash)
 *
 * New Features v2.1:
 * - Text alignment (LEFT, CENTER, RIGHT)
 * - Scrolling marquee text
 * - Line graphs with auto-scaling
 * - Bar charts/graphs
 * - Monochrome bitmap display
 * - Text boxes with borders
 * - Grid drawing
 * - Display inversion
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

// Display constants
#define BASE_CHAR_W 6
#define BASE_CHAR_H 8

// Current state
uint16_t screenW = 320;
uint16_t screenH = 480;
uint16_t textColor = 0xFFFF; // White
uint16_t bgColor = 0x0000;   // Black
uint16_t fillColor = 0xFFFF; // White (for shapes)
bool opaqueText = false;
uint8_t textSize = 2;
uint16_t posX = 0;
uint16_t posY = 0;
uint8_t rotation = 0;

// Text alignment: 0=LEFT, 1=CENTER, 2=RIGHT
uint8_t textAlign = 0;

// Serial handling - Mega has more RAM, so we can use larger buffers
String cmd = "";
bool cmdReady = false;

// Feature flags
bool autoScroll = true;
bool echoCommands = true;
bool invertDisplay = false;

// Graph data storage (for plotting)
#define MAX_GRAPH_POINTS 100
int16_t graphData[MAX_GRAPH_POINTS];
uint8_t graphPointCount = 0;

void setup() {
  Serial.begin(115200); // Mega can handle higher baud rates
  Serial.println(F("Arduino Mega 2560 + ILI9486"));
  Serial.println(F("Serial Display Controller v2.1"));
  Serial.println(F("Enhanced Edition - Initializing..."));

  uint16_t ID = tft.readID();
  Serial.print(F("LCD ID: 0x"));
  Serial.println(ID, HEX);

  // ILI9486 specific handling
  // ILI9486 may return different IDs depending on shield/breakout board
  if (ID == 0x9486 || ID == 0x9488 || ID == 0x00D3 || ID == 0xD3D3) {
    ID = 0x9486;
    Serial.println(F("Detected as ILI9486"));
  } else if (ID == 0x0000 || ID == 0xFFFF) {
    // Some displays return 0x0000 or 0xFFFF on first read
    Serial.println(F("Unknown ID, defaulting to ILI9486"));
    ID = 0x9486;
  }

  tft.begin(ID);
  tft.setRotation(rotation);
  screenW = tft.width();
  screenH = tft.height();

  Serial.print(F("Resolution: "));
  Serial.print(screenW);
  Serial.print(F("x"));
  Serial.println(screenH);

  tft.fillScreen(0);
  tft.setTextSize(textSize);
  updateTextColors();

  // Display startup message
  tft.setTextColor(0x07FF); // Cyan
  tft.println(F("MEGA 2560 + ILI9486"));
  tft.setTextColor(0x07E0); // Green
  tft.println(F("Serial Display v2.0"));
  tft.setTextColor(0xFFE0); // Yellow
  tft.println(F("Ready for commands..."));
  tft.println();

  textColor = 0xFFFF; // Reset to white
  updateTextColors();

  posY = tft.getCursorY();

  cmd.reserve(500); // Mega has plenty of RAM (max command length ~490 chars)

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
  if (charsPerLine == 0) charsPerLine = 1; // Safety check
  return (len + charsPerLine - 1) / charsPerLine;
}

// Display text with wrapping fix and alignment support
void showText(const String& txt) {
  uint16_t lines = getLines(txt);
  uint16_t lineH = BASE_CHAR_H * textSize;
  uint16_t needH = lines * lineH;

  // Auto-scroll if enabled
  if (autoScroll && posY + needH > screenH) {
    tft.fillScreen(0);
    posY = 0;
  }

  // Calculate X position based on alignment
  uint16_t startX = posX;
  if (textAlign == 1) { // CENTER
    uint16_t textWidth = txt.length() * BASE_CHAR_W * textSize;
    startX = (screenW - textWidth) / 2;
    if (startX > screenW) startX = 0; // Safety check for overflow
  } else if (textAlign == 2) { // RIGHT
    uint16_t textWidth = txt.length() * BASE_CHAR_W * textSize;
    startX = screenW - textWidth;
    if (startX > screenW) startX = 0; // Safety check for overflow
  }

  tft.setCursor(startX, posY);

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
        tft.setCursor(textAlign == 0 ? 0 : startX, posY);
      }
    }
    tft.println();
  } else {
    tft.println(txt);
  }

  posY = tft.getCursorY();
  posX = 0;

  if (echoCommands) {
    Serial.print(lines);
    Serial.print(F("L: "));
    Serial.println(txt);
  }
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

    } else if (c.startsWith("#FILLCOLOR ") || c.startsWith("#FCOLOR ")) {
      int pos = c.indexOf(' ') + 1;
      String col = c.substring(pos);
      setFillCol(col);

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
      showInfo();

    } else if (c.startsWith("#RECT ")) {
      parseRect(c.substring(6));

    } else if (c.startsWith("#FILLRECT ") || c.startsWith("#FRECT ")) {
      int pos = c.indexOf(' ') + 1;
      parseFillRect(c.substring(pos));

    } else if (c.startsWith("#ROUNDRECT ") || c.startsWith("#RRECT ")) {
      int pos = c.indexOf(' ') + 1;
      parseRoundRect(c.substring(pos));

    } else if (c.startsWith("#FILLROUNDRECT ") || c.startsWith("#FRRECT ")) {
      int pos = c.indexOf(' ') + 1;
      parseFillRoundRect(c.substring(pos));

    } else if (c.startsWith("#CIRCLE ")) {
      parseCirc(c.substring(8));

    } else if (c.startsWith("#FILLCIRCLE ") || c.startsWith("#FCIRCLE ")) {
      int pos = c.indexOf(' ') + 1;
      parseFillCirc(c.substring(pos));

    } else if (c.startsWith("#TRIANGLE ") || c.startsWith("#TRI ")) {
      int pos = c.indexOf(' ') + 1;
      parseTriangle(c.substring(pos));

    } else if (c.startsWith("#FILLTRIANGLE ") || c.startsWith("#FTRI ")) {
      int pos = c.indexOf(' ') + 1;
      parseFillTriangle(c.substring(pos));

    } else if (c.startsWith("#LINE ")) {
      parseLine(c.substring(6));

    } else if (c.startsWith("#PIXEL ") || c.startsWith("#PX ")) {
      int pos = c.indexOf(' ') + 1;
      parsePixel(c.substring(pos));

    } else if (c.startsWith("#HLINE ")) {
      parseHLine(c.substring(7));

    } else if (c.startsWith("#VLINE ")) {
      parseVLine(c.substring(7));

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
      testDisplay();

    } else if (c == "#TESTWRAP") {
      testWrap();

    } else if (c == "#TESTCOLORS") {
      testColors();

    } else if (c == "#TESTSHAPES") {
      testShapes();

    } else if (c.startsWith("#SCROLL ")) {
      String val = c.substring(8);
      if (val == "ON") {
        autoScroll = true;
        Serial.println(F("AutoScroll:ON"));
      } else if (val == "OFF") {
        autoScroll = false;
        Serial.println(F("AutoScroll:OFF"));
      }

    } else if (c.startsWith("#ECHO ")) {
      String val = c.substring(6);
      if (val == "ON") {
        echoCommands = true;
        Serial.println(F("Echo:ON"));
      } else if (val == "OFF") {
        echoCommands = false;
        Serial.println(F("Echo:OFF"));
      }

    } else if (c.startsWith("#ALIGN ")) {
      String val = c.substring(7);
      if (val == "LEFT") {
        textAlign = 0;
        Serial.println(F("Align:LEFT"));
      } else if (val == "CENTER") {
        textAlign = 1;
        Serial.println(F("Align:CENTER"));
      } else if (val == "RIGHT") {
        textAlign = 2;
        Serial.println(F("Align:RIGHT"));
      }

    } else if (c.startsWith("#MARQUEE ")) {
      parseMarquee(c.substring(9));

    } else if (c.startsWith("#GRAPH ")) {
      parseGraph(c.substring(7));

    } else if (c.startsWith("#BARGRAPH ") || c.startsWith("#BAR ")) {
      int pos = c.indexOf(' ') + 1;
      parseBarGraph(c.substring(pos));

    } else if (c.startsWith("#BITMAP ")) {
      parseBitmap(c.substring(8));

    } else if (c.startsWith("#TEXTBOX ")) {
      parseTextBox(c.substring(9));

    } else if (c.startsWith("#GRID ")) {
      parseGrid(c.substring(6));

    } else if (c.startsWith("#INVERT ")) {
      String val = c.substring(8);
      if (val == "ON") {
        tft.invertDisplay(true);
        invertDisplay = true;
        Serial.println(F("Invert:ON"));
      } else if (val == "OFF") {
        tft.invertDisplay(false);
        invertDisplay = false;
        Serial.println(F("Invert:OFF"));
      }

    } else if (c == "#RESET") {
      resetDisplay();

    } else if (c == "#HELP") {
      help();

    } else if (c == "#ID") {
      Serial.println(F("COM LCD MEGA ILI9486"));

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
  } else {
    Serial.println(F("?Col"));
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
  } else {
    Serial.println(F("?BgCol"));
  }
}

// Fill color setting
void setFillCol(String& n) {
  n.trim();
  n.toUpperCase();
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    fillColor = c;
    Serial.print(F("FillCol:"));
    Serial.println(n);
  } else {
    Serial.println(F("?FillCol"));
  }
}

// Get color from name - Extended color palette
uint16_t getColorFromName(String& n) {
  // Basic colors
  if (n == "RED") return 0xF800;
  else if (n == "GREEN") return 0x07E0;
  else if (n == "BLUE") return 0x001F;
  else if (n == "YELLOW") return 0xFFE0;
  else if (n == "CYAN") return 0x07FF;
  else if (n == "MAGENTA") return 0xF81F;
  else if (n == "WHITE") return 0xFFFF;
  else if (n == "BLACK") return 0x0000;

  // Extended colors
  else if (n == "ORANGE") return 0xFD20;
  else if (n == "PINK") return 0xFC9F;
  else if (n == "PURPLE") return 0x8010;
  else if (n == "NAVY") return 0x0010;
  else if (n == "DARKGREEN") return 0x03E0;
  else if (n == "DARKCYAN") return 0x03EF;
  else if (n == "MAROON") return 0x7800;
  else if (n == "OLIVE") return 0x7BE0;
  else if (n == "LIGHTGREY") return 0xC618;
  else if (n == "DARKGREY") return 0x7BEF;
  else if (n == "GREENYELLOW") return 0xAFE5;
  else if (n == "BROWN") return 0x9A60;

  return 0xFFFF; // Default to white
}

// Shape parsing functions
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

void parseFillRect(String p) {
  int v[4], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 4) {
    tft.fillRect(v[0], v[1], v[2], v[3], fillColor);
    Serial.println(F("FillRect"));
  }
}

void parseRoundRect(String p) {
  int v[5], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 5; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 5) {
    tft.drawRoundRect(v[0], v[1], v[2], v[3], v[4], textColor);
    Serial.println(F("RoundRect"));
  }
}

void parseFillRoundRect(String p) {
  int v[5], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 5; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 5) {
    tft.fillRoundRect(v[0], v[1], v[2], v[3], v[4], fillColor);
    Serial.println(F("FillRoundRect"));
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
    Serial.println(F("Circle"));
  }
}

void parseFillCirc(String p) {
  int v[3], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 3; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 3) {
    tft.fillCircle(v[0], v[1], v[2], fillColor);
    Serial.println(F("FillCircle"));
  }
}

void parseTriangle(String p) {
  int v[6], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 6; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 6) {
    tft.drawTriangle(v[0], v[1], v[2], v[3], v[4], v[5], textColor);
    Serial.println(F("Triangle"));
  }
}

void parseFillTriangle(String p) {
  int v[6], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 6; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 6) {
    tft.fillTriangle(v[0], v[1], v[2], v[3], v[4], v[5], fillColor);
    Serial.println(F("FillTriangle"));
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

void parseHLine(String p) {
  int v[3], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 3; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 3) {
    tft.drawFastHLine(v[0], v[1], v[2], textColor);
    Serial.println(F("HLine"));
  }
}

void parseVLine(String p) {
  int v[3], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 3; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 3) {
    tft.drawFastVLine(v[0], v[1], v[2], textColor);
    Serial.println(F("VLine"));
  }
}

void parsePixel(String p) {
  int v[2], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 2; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 2) {
    tft.drawPixel(v[0], v[1], textColor);
    Serial.println(F("Pixel"));
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
      tft.fillRect(v[0]+1, v[1]+1, fw-2, v[3]-2, fillColor);
    }
    Serial.print(pct);
    Serial.println(F("%"));
  }
}

// Show system info
void showInfo() {
  Serial.println(F("=== SYSTEM INFO ==="));
  Serial.println(F("Board: Arduino Mega 2560"));
  Serial.println(F("Display: ILI9486"));
  Serial.print(F("Resolution: "));
  Serial.print(screenW);
  Serial.print(F("x"));
  Serial.println(screenH);
  Serial.print(F("Rotation: "));
  Serial.println(rotation);
  Serial.print(F("Position: "));
  Serial.print(posX);
  Serial.print(F(","));
  Serial.println(posY);
  Serial.print(F("Text Size: "));
  Serial.println(textSize);
  Serial.print(F("Text Align: "));
  Serial.println(textAlign == 0 ? F("LEFT") : textAlign == 1 ? F("CENTER") : F("RIGHT"));
  Serial.print(F("Chars/Line: "));
  Serial.println(screenW/(BASE_CHAR_W*textSize));
  Serial.print(F("Text Color: 0x"));
  Serial.println(textColor, HEX);
  Serial.print(F("Fill Color: 0x"));
  Serial.println(fillColor, HEX);
  Serial.print(F("Background: "));
  if (opaqueText) {
    Serial.print(F("0x"));
    Serial.println(bgColor, HEX);
  } else {
    Serial.println(F("None"));
  }
  Serial.print(F("AutoScroll: "));
  Serial.println(autoScroll ? F("ON") : F("OFF"));
  Serial.print(F("Echo: "));
  Serial.println(echoCommands ? F("ON") : F("OFF"));
  Serial.print(F("Invert: "));
  Serial.println(invertDisplay ? F("ON") : F("OFF"));
  Serial.print(F("Graph Points: "));
  Serial.println(graphPointCount);
  Serial.print(F("Free RAM: "));
  Serial.print(getFreeRam());
  Serial.println(F(" bytes"));
}

// Get free RAM
int getFreeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// Reset display
void resetDisplay() {
  tft.fillScreen(0);
  posX = posY = 0;
  textSize = 2;
  textColor = 0xFFFF;
  fillColor = 0xFFFF;
  bgColor = 0x0000;
  opaqueText = false;
  rotation = 0;
  autoScroll = true;
  echoCommands = true;
  textAlign = 0; // LEFT
  invertDisplay = false;
  graphPointCount = 0;
  tft.invertDisplay(false);
  tft.setRotation(0);
  tft.setTextSize(textSize);
  updateTextColors();
  screenW = tft.width();
  screenH = tft.height();
  Serial.println(F("Reset"));
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

// Test colors
void testColors() {
  Serial.println(F("Testing colors..."));
  tft.fillScreen(0);
  int x = 10, y = 10;
  int boxW = 60, boxH = 40;

  // Use direct color values to save RAM (no String array needed)
  uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFE0, 0x07FF, 0xF81F,
                       0xFD20, 0xFC9F, 0x8010, 0x9A60, 0x7BE0, 0x0010};

  for (int i = 0; i < 12; i++) {
    tft.fillRect(x, y, boxW, boxH, colors[i]);

    x += boxW + 5;
    if (x > screenW - boxW) {
      x = 10;
      y += boxH + 5;
    }
  }

  Serial.println(F("Color test complete"));
}

// Test shapes
void testShapes() {
  Serial.println(F("Testing shapes..."));
  tft.fillScreen(0);

  // Note: Coordinates optimized for portrait mode (320x480)
  // May render partially off-screen in landscape mode

  // Rectangles
  tft.drawRect(10, 10, 80, 60, 0xF800);
  tft.fillRect(100, 10, 80, 60, 0x07E0);

  // Circles
  tft.drawCircle(230, 40, 30, 0x001F);
  tft.fillCircle(230, 120, 30, 0xFFE0);

  // Triangles
  tft.drawTriangle(20, 150, 70, 150, 45, 100, 0x07FF);
  tft.fillTriangle(120, 150, 170, 150, 145, 100, 0xF81F);

  // Rounded rectangles
  tft.drawRoundRect(10, 180, 80, 60, 10, 0xFD20);
  tft.fillRoundRect(100, 180, 80, 60, 10, 0xFC9F);

  // Lines
  tft.drawLine(220, 200, 300, 240, 0xFFFF);
  tft.drawFastHLine(10, 260, 150, 0xFFE0);
  tft.drawFastVLine(280, 260, 100, 0x07FF);

  Serial.println(F("Shape test complete"));
}

// Main test
void testDisplay() {
  Serial.println(F("Running full display test..."));
  testColors();
  delay(2000);
  testShapes();
  delay(2000);
  testWrap();
  Serial.println(F("All tests complete"));
}

// ===== NEW ENHANCED FEATURES =====

// Scrolling marquee text
// Format: #MARQUEE <y> <speed> <text>
void parseMarquee(String p) {
  int sp1 = p.indexOf(' ');
  if (sp1 < 0) return;

  int y = p.substring(0, sp1).toInt();
  int sp2 = p.indexOf(' ', sp1 + 1);
  if (sp2 < 0) return;

  int speed = p.substring(sp1 + 1, sp2).toInt();
  String text = p.substring(sp2 + 1);

  if (speed < 1) speed = 1;
  if (speed > 100) speed = 100;

  uint16_t textWidth = text.length() * BASE_CHAR_W * textSize;
  int16_t x = screenW;

  Serial.println(F("Marquee..."));

  // Scroll text across screen
  while (x > -(int16_t)textWidth) {
    // Clear text area
    tft.fillRect(0, y, screenW, BASE_CHAR_H * textSize, bgColor);

    // Draw text at current position
    tft.setCursor(x, y);
    tft.print(text);

    x -= speed;
    delay(50); // Small delay for smooth animation

    // Check for serial interrupt to stop marquee
    if (Serial.available()) {
      Serial.println(F("Stopped"));
      while(Serial.available()) Serial.read(); // Clear buffer
      return;
    }
  }

  Serial.println(F("Done"));
}

// Line graph plotting
// Format: #GRAPH <x> <y> <w> <h> <val1,val2,val3,...>
void parseGraph(String p) {
  int v[4], i = 0, last = -1;

  // Parse x, y, w, h
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }

  if (i != 4) return;

  int x = v[0], y = v[1], w = v[2], h = v[3];
  String dataStr = p.substring(last + 1);

  // Parse comma-separated values
  graphPointCount = 0;
  int lastComma = -1;
  for (int j = 0; j <= dataStr.length() && graphPointCount < MAX_GRAPH_POINTS; j++) {
    if (j == dataStr.length() || dataStr[j] == ',') {
      if (j > lastComma + 1) {
        graphData[graphPointCount++] = dataStr.substring(lastComma + 1, j).toInt();
      }
      lastComma = j;
    }
  }

  if (graphPointCount < 2) {
    Serial.println(F("?Graph needs 2+ points"));
    return;
  }

  // Draw graph border
  tft.drawRect(x, y, w, h, textColor);

  // Find min/max for scaling
  int16_t minVal = graphData[0], maxVal = graphData[0];
  for (int i = 1; i < graphPointCount; i++) {
    if (graphData[i] < minVal) minVal = graphData[i];
    if (graphData[i] > maxVal) maxVal = graphData[i];
  }

  // Prevent division by zero
  if (maxVal == minVal) maxVal = minVal + 1;

  // Plot lines between points
  float xStep = (float)(w - 4) / (graphPointCount - 1);
  for (int i = 0; i < graphPointCount - 1; i++) {
    int x1 = x + 2 + (int)(i * xStep);
    int x2 = x + 2 + (int)((i + 1) * xStep);

    int y1 = y + h - 2 - (int)((graphData[i] - minVal) * (h - 4) / (maxVal - minVal));
    int y2 = y + h - 2 - (int)((graphData[i + 1] - minVal) * (h - 4) / (maxVal - minVal));

    tft.drawLine(x1, y1, x2, y2, fillColor);
  }

  Serial.print(F("Graph:"));
  Serial.print(graphPointCount);
  Serial.println(F("pts"));
}

// Bar graph
// Format: #BAR <x> <y> <w> <h> <val1,val2,val3,...>
void parseBarGraph(String p) {
  int v[4], i = 0, last = -1;

  // Parse x, y, w, h
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }

  if (i != 4) return;

  int x = v[0], y = v[1], w = v[2], h = v[3];
  String dataStr = p.substring(last + 1);

  // Parse comma-separated values
  int barCount = 0;
  int16_t barData[20]; // Max 20 bars
  int lastComma = -1;
  for (int j = 0; j <= dataStr.length() && barCount < 20; j++) {
    if (j == dataStr.length() || dataStr[j] == ',') {
      if (j > lastComma + 1) {
        barData[barCount++] = dataStr.substring(lastComma + 1, j).toInt();
      }
      lastComma = j;
    }
  }

  if (barCount < 1) {
    Serial.println(F("?Bar needs 1+ values"));
    return;
  }

  // Draw border
  tft.drawRect(x, y, w, h, textColor);

  // Find max for scaling
  int16_t maxVal = barData[0];
  for (int i = 1; i < barCount; i++) {
    if (barData[i] > maxVal) maxVal = barData[i];
  }

  if (maxVal == 0) maxVal = 1;

  // Draw bars
  int barWidth = (w - 2) / barCount;
  for (int i = 0; i < barCount; i++) {
    int barH = (barData[i] * (h - 2)) / maxVal;
    int barX = x + 1 + i * barWidth;
    int barY = y + h - 1 - barH;

    tft.fillRect(barX, barY, barWidth - 1, barH, fillColor);
  }

  Serial.print(F("BarGraph:"));
  Serial.print(barCount);
  Serial.println(F("bars"));
}

// Simple monochrome bitmap display
// Format: #BITMAP <x> <y> <w> <h> <hexdata>
// hexdata is hex string like "FF00FF00" (1 bit per pixel, MSB first)
void parseBitmap(String p) {
  int v[4], i = 0, last = -1;

  // Parse x, y, w, h
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }

  if (i != 4) return;

  int x = v[0], y = v[1], w = v[2], h = v[3];
  String hexData = p.substring(last + 1);
  hexData.trim();
  hexData.toUpperCase();

  int bytesNeeded = ((w * h) + 7) / 8;

  // Draw bitmap
  int bitIndex = 0;
  for (int py = 0; py < h; py++) {
    for (int px = 0; px < w; px++) {
      int bytePos = bitIndex / 8;
      int bitPos = 7 - (bitIndex % 8);

      if (bytePos * 2 + 1 < hexData.length()) {
        // Parse hex byte
        char hexByte[3] = {hexData[bytePos * 2], hexData[bytePos * 2 + 1], 0};
        uint8_t byte = strtol(hexByte, NULL, 16);

        // Check bit
        if (byte & (1 << bitPos)) {
          tft.drawPixel(x + px, y + py, textColor);
        } else if (opaqueText) {
          tft.drawPixel(x + px, y + py, bgColor);
        }
      }

      bitIndex++;
    }
  }

  Serial.println(F("Bitmap"));
}

// Text box with border
// Format: #TEXTBOX <x> <y> <w> <h> <text>
void parseTextBox(String p) {
  int v[4], i = 0, last = -1;

  // Parse x, y, w, h
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }

  if (i != 4) return;

  int x = v[0], y = v[1], w = v[2], h = v[3];
  String text = p.substring(last + 1);

  // Draw box
  tft.drawRect(x, y, w, h, textColor);

  // Draw text inside, centered
  uint16_t textW = text.length() * BASE_CHAR_W * textSize;
  uint16_t textH = BASE_CHAR_H * textSize;
  int textX = x + (w - textW) / 2;
  int textY = y + (h - textH) / 2;

  if (textX < x + 2) textX = x + 2;
  if (textY < y + 2) textY = y + 2;

  tft.setCursor(textX, textY);
  tft.print(text);

  Serial.println(F("TextBox"));
}

// Draw grid
// Format: #GRID <x> <y> <w> <h> <spacing>
void parseGrid(String p) {
  int v[5], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 5; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 5) {
    int x = v[0], y = v[1], w = v[2], h = v[3], spacing = v[4];

    // Draw vertical lines
    for (int gx = x; gx <= x + w; gx += spacing) {
      tft.drawFastVLine(gx, y, h, textColor);
    }

    // Draw horizontal lines
    for (int gy = y; gy <= y + h; gy += spacing) {
      tft.drawFastHLine(x, gy, w, textColor);
    }

    Serial.println(F("Grid"));
  }
}

// Help - comprehensive
void help() {
  Serial.println(F("=== ARDUINO MEGA + ILI9486 COMMANDS ==="));
  Serial.println(F("Enhanced Edition v2.1"));
  Serial.println();
  Serial.println(F("TEXT COMMANDS:"));
  Serial.println(F("  <text>          - Display text"));
  Serial.println(F("  #SIZE <1-10>    - Set text size"));
  Serial.println(F("  #POS <x> <y>    - Set cursor position"));
  Serial.println(F("  #ALIGN <LEFT|CENTER|RIGHT> - Text align"));
  Serial.println();
  Serial.println(F("COLOR COMMANDS:"));
  Serial.println(F("  #COLOR <name>   - Set text/draw color"));
  Serial.println(F("  #BGCOLOR <name|NONE> - Set background"));
  Serial.println(F("  #FCOLOR <name>  - Set fill color"));
  Serial.println(F("  Colors: RED, GREEN, BLUE, YELLOW, CYAN,"));
  Serial.println(F("          MAGENTA, WHITE, BLACK, ORANGE,"));
  Serial.println(F("          PINK, PURPLE, NAVY, DARKGREEN,"));
  Serial.println(F("          MAROON, OLIVE, BROWN, etc."));
  Serial.println();
  Serial.println(F("DRAWING COMMANDS:"));
  Serial.println(F("  #RECT <x y w h>      - Draw rectangle"));
  Serial.println(F("  #FRECT <x y w h>     - Fill rectangle"));
  Serial.println(F("  #RRECT <x y w h r>   - Draw round rect"));
  Serial.println(F("  #FRRECT <x y w h r>  - Fill round rect"));
  Serial.println(F("  #CIRCLE <x y r>      - Draw circle"));
  Serial.println(F("  #FCIRCLE <x y r>     - Fill circle"));
  Serial.println(F("  #TRI <x1 y1 x2 y2 x3 y3> - Triangle"));
  Serial.println(F("  #FTRI <x1 y1 x2 y2 x3 y3> - Fill tri"));
  Serial.println(F("  #LINE <x1 y1 x2 y2>  - Draw line"));
  Serial.println(F("  #HLINE <x y len>     - Horizontal line"));
  Serial.println(F("  #VLINE <x y len>     - Vertical line"));
  Serial.println(F("  #PIXEL <x y>         - Draw pixel"));
  Serial.println(F("  #PROG <x y w h %>    - Progress bar"));
  Serial.println(F("  #GRID <x y w h spacing> - Draw grid"));
  Serial.println();
  Serial.println(F("ADVANCED FEATURES:"));
  Serial.println(F("  #MARQUEE <y speed text> - Scroll text"));
  Serial.println(F("  #GRAPH <x y w h val,val,...> - Line graph"));
  Serial.println(F("  #BAR <x y w h val,val,...> - Bar chart"));
  Serial.println(F("  #BITMAP <x y w h hex> - Monochrome bitmap"));
  Serial.println(F("  #TEXTBOX <x y w h text> - Bordered text"));
  Serial.println();
  Serial.println(F("SCREEN COMMANDS:"));
  Serial.println(F("  #CLEAR / #CLR   - Clear screen"));
  Serial.println(F("  #ROT <0-3>      - Set rotation"));
  Serial.println(F("  #INVERT <ON|OFF> - Invert display"));
  Serial.println(F("  #RESET          - Reset all settings"));
  Serial.println();
  Serial.println(F("SETTINGS:"));
  Serial.println(F("  #SCROLL <ON|OFF> - Auto-scroll"));
  Serial.println(F("  #ECHO <ON|OFF>   - Echo responses"));
  Serial.println();
  Serial.println(F("INFO & TEST:"));
  Serial.println(F("  #INFO           - Show system info"));
  Serial.println(F("  #ID             - Device ID"));
  Serial.println(F("  #TEST           - Full test"));
  Serial.println(F("  #TESTWRAP       - Test text wrap"));
  Serial.println(F("  #TESTCOLORS     - Test colors"));
  Serial.println(F("  #TESTSHAPES     - Test shapes"));
  Serial.println(F("  #HELP           - This help"));
}
