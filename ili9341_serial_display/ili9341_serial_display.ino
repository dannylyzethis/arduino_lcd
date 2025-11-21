// ILI9341 Split Screen: Top=Commands, Bottom=FPGA (pins 12/13)

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SoftwareSerial.h>
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

// FPGA Serial (RX=12, TX=13)
SoftwareSerial fpgaSerial(12, 13);

#define BASE_CHAR_W 6
#define BASE_CHAR_H 8
uint16_t screenW = 240;
uint16_t screenH = 320;
uint16_t dividerY;
uint16_t topMaxY;
uint16_t bottomMinY;
uint16_t bottomMaxY;
uint16_t topPosX = 0;
uint16_t topPosY = 0;
uint16_t topTextColor = 0xFFFF;
uint16_t topBgColor = 0x0000;
bool topOpaqueText = false;
uint8_t topTextSize = 2;
uint16_t bottomPosX = 0;
uint16_t bottomPosY = 0;
uint16_t bottomTextColor = 0x07E0;
uint16_t bottomBgColor = 0x0000;
bool bottomOpaqueText = false;
uint8_t bottomTextSize = 1;
uint16_t textColor = 0xFFFF;
uint16_t bgColor = 0x0000;
bool opaqueText = false;
uint8_t textSize = 2;
uint16_t posX = 0;
uint16_t posY = 0;

String cmd = "";
bool cmdReady = false;

String fpgaBuffer = "";
unsigned long fpgaBaud = 9600;
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

void setup() {
  Serial.begin(9600);
  fpgaSerial.begin(fpgaBaud);

  uint16_t ID = tft.readID();
  if (ID == 0xD3D3 || ID == 0x00D3) ID = 0x9341;
  tft.begin(ID);
  tft.setRotation(0);
  screenW = tft.width();
  screenH = tft.height();

  dividerY = screenH / 2;
  topMaxY = dividerY - 2;
  bottomMinY = dividerY + 2;
  bottomMaxY = screenH;
  bottomPosY = bottomMinY;

  tft.fillScreen(0);
  drawDivider();
  topPosY = 0;
  bottomPosY = bottomMinY;

  cmd.reserve(100);
  fpgaBuffer.reserve(80);
  initButtons();

  Serial.println(F("READY"));
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

  while (fpgaSerial.available()) {
    char c = fpgaSerial.read();
    Serial.write(c);
    Serial.flush();
    if (c == '\n' || c == '\r') {
      if (fpgaBuffer.length() > 0) {
        showTextBottom(fpgaBuffer);
        fpgaBuffer = "";
      }
    } else if (isPrintable(c)) {
      fpgaBuffer += c;
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

void drawDivider() {
  tft.drawLine(0, dividerY, screenW, dividerY, 0xFFFF);
}
uint8_t getLines(const String& txt) {
  uint8_t len = txt.length();
  if (len == 0) return 1;
  uint8_t charsPerLine = screenW / (BASE_CHAR_W * topTextSize);
  return (len + charsPerLine - 1) / charsPerLine;
}
uint8_t getLinesBottom(const String& txt) {
  uint8_t len = txt.length();
  if (len == 0) return 1;
  uint8_t charsPerLine = screenW / (BASE_CHAR_W * bottomTextSize);
  return (len + charsPerLine - 1) / charsPerLine;
}
void showText(const String& txt) {
  uint8_t lines = getLines(txt);
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

  uint8_t charsPerLine = screenW / (BASE_CHAR_W * topTextSize);
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

  posY = topPosY;
  posX = topPosX;
  textSize = topTextSize;
  textColor = topTextColor;
}

void showTextTop(const String& txt) {
  showText(txt);
}
void showTextBottom(const String& txt) {
  uint8_t lines = getLinesBottom(txt);
  uint16_t lineH = BASE_CHAR_H * bottomTextSize;
  uint16_t needH = lines * lineH;

  if (bottomPosY + needH > bottomMaxY) {
    tft.fillRect(0, bottomMinY, screenW, bottomMaxY - bottomMinY, 0x0000);
    bottomPosY = bottomMinY;
  }
  tft.setTextSize(bottomTextSize);
  if (bottomOpaqueText) {
    tft.setTextColor(bottomTextColor, bottomBgColor);
  } else {
    tft.setTextColor(bottomTextColor);
  }
  tft.setCursor(bottomPosX, bottomPosY);

  uint8_t charsPerLine = screenW / (BASE_CHAR_W * bottomTextSize);
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
void processCmd(String c) {
  c.trim();

  if (c.startsWith(">>>")) {
    String data = c.substring(3);
    data.trim();
    fpgaSerial.println(data);
    return;
  }

  if (c.startsWith("#")) {
    c.toUpperCase();
    
    if (c == "#CLEAR" || c == "#CLR") {
      tft.fillRect(0, 0, screenW, topMaxY, 0x0000);
      topPosX = topPosY = 0;
      posX = posY = 0;
      drawDivider();
      
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
      }
      
    } else if (c.startsWith("#POS ")) {
      int sp = c.indexOf(' ', 5);
      if (sp > 0) {
        topPosX = c.substring(5, sp).toInt();
        topPosY = c.substring(sp + 1).toInt();
        posX = topPosX;
        posY = topPosY;
        tft.setCursor(topPosX, topPosY);
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
        posX = posY = 0;
        drawDivider();

        initButtons();
        if (buttonsVisible) {
          buttonsVisible = false;  // Reset flag
          showButtons();           // Redraw with new positions
        }
      }

    } else if (c == "#CLRBOT") {
      tft.fillRect(0, bottomMinY, screenW, bottomMaxY - bottomMinY, 0x0000);
      bottomPosX = 0;
      bottomPosY = bottomMinY;
      drawDivider();

    } else if (c == "#CLRALL") {
      tft.fillScreen(0);
      topPosX = topPosY = 0;
      bottomPosX = 0;
      bottomPosY = bottomMinY;
      posX = posY = 0;
      drawDivider();

    } else if (c.startsWith("#BOTSIZE ")) {
      int s = c.substring(9).toInt();
      if (s >= 1 && s <= 5) {
        bottomTextSize = s;
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
      }

    } else if (c.startsWith("#FPGASEND ")) {
      String data = c.substring(10);
      fpgaSerial.println(data);

    } else if (c == "#FPGAPING") {
      fpgaSerial.println(F("PING"));

    } else if (c.startsWith("#FPGABYTES ")) {
      String hexData = c.substring(11);
      hexData.trim();
      sendHexBytes(hexData);

    } else if (c == "#SHOWBTNS") {
      showButtons();

    } else if (c == "#HIDEBTNS") {
      hideButtons();

    } else if (c == "#HELP") {
      help();
    }
  } else {
    showText(c);
  }
}

void updateTextColors() {
  if (topOpaqueText) {
    tft.setTextColor(topTextColor, topBgColor);
  } else {
    tft.setTextColor(topTextColor);
  }
  textColor = topTextColor;
  bgColor = topBgColor;
  opaqueText = topOpaqueText;
}

void setCol(String& n) {
  n.toUpperCase();
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    topTextColor = c;
    textColor = c;
    updateTextColors();
  }
}

void setBgCol(String& n) {
  n.trim();
  n.toUpperCase();
  if (n == "NONE") {
    topOpaqueText = false;
    opaqueText = false;
    updateTextColors();
    return;
  }
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    topBgColor = c;
    topOpaqueText = true;
    bgColor = c;
    opaqueText = true;
    updateTextColors();
  }
}

void setBotCol(String& n) {
  n.toUpperCase();
  uint16_t c = getColorFromName(n);
  if (c != 0xFFFF || n == "WHITE") {
    bottomTextColor = c;
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
  return 0xFFFF;
}

int sendHexBytes(String hexStr) {
  int byteCount = 0;
  int startIdx = 0;

  while (startIdx < hexStr.length()) {
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
        fpgaSerial.write((uint8_t)value);
        byteCount++;
      }
    }
    startIdx = endIdx;
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
  if (i == 4) tft.drawRect(v[0], v[1], v[2], v[3], textColor);
}

void parseFill(String p) {
  int v[4], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 4) tft.fillRect(v[0], v[1], v[2], v[3], textColor);
}

void parseCirc(String p) {
  int v[3], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 3; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 3) tft.drawCircle(v[0], v[1], v[2], textColor);
}

void parseLine(String p) {
  int v[4], i = 0, last = -1;
  for (int j = 0; j <= p.length() && i < 4; j++) {
    if (j == p.length() || p[j] == ' ') {
      v[i++] = p.substring(last + 1, j).toInt();
      last = j;
    }
  }
  if (i == 4) tft.drawLine(v[0], v[1], v[2], v[3], textColor);
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
  }
}

void help() {
  Serial.println(F("#CLR #SIZE #COLOR #BGCOLOR"));
  Serial.println(F("#CLRBOT #CLRALL #BOTSIZE"));
  Serial.println(F("#RECT #FILL #CIRCLE #LINE #PROG"));
  Serial.println(F("#SHOWBTNS #HIDEBTNS"));
  Serial.println(F("#FPGABAUD #FPGASEND #FPGABYTES"));
  Serial.println(F("#ROT"));
}

void initButtons() {
  uint16_t btnHeight = 40;
  uint16_t btnWidth = (screenW - 10) / 4;
  uint16_t btnY = bottomMaxY - btnHeight - 2;
  buttonTextY = btnY - 2;

  buttons[0].x = 5;
  buttons[0].y = btnY;
  buttons[0].w = btnWidth;
  buttons[0].h = btnHeight;
  buttons[0].label = "T";
  buttons[0].cmdBytes[0] = 0x54;
  buttons[0].cmdBytes[1] = 0x45;
  buttons[0].cmdBytes[2] = 0x4D;
  buttons[0].cmdBytes[3] = 0x50;
  buttons[0].cmdLen = 4;
  buttons[0].respType = RESP_TEMP;
  buttons[0].color = 0xFD20;

  buttons[1].x = 5 + btnWidth + 2;
  buttons[1].y = btnY;
  buttons[1].w = btnWidth;
  buttons[1].h = btnHeight;
  buttons[1].label = "S";
  buttons[1].cmdBytes[0] = 0x53;
  buttons[1].cmdBytes[1] = 0x54;
  buttons[1].cmdBytes[2] = 0x41;
  buttons[1].cmdBytes[3] = 0x54;
  buttons[1].cmdLen = 4;
  buttons[1].respType = RESP_STATUS;
  buttons[1].color = 0x07E0;

  buttons[2].x = 5 + (btnWidth + 2) * 2;
  buttons[2].y = btnY;
  buttons[2].w = btnWidth;
  buttons[2].h = btnHeight;
  buttons[2].label = "C";
  buttons[2].cmdBytes[0] = 0x43;
  buttons[2].cmdBytes[1] = 0x4E;
  buttons[2].cmdBytes[2] = 0x54;
  buttons[2].cmdBytes[3] = 0x52;
  buttons[2].cmdLen = 4;
  buttons[2].respType = RESP_COUNTER;
  buttons[2].color = 0x07FF;

  buttons[3].x = 5 + (btnWidth + 2) * 3;
  buttons[3].y = btnY;
  buttons[3].w = btnWidth;
  buttons[3].h = btnHeight;
  buttons[3].label = "D";
  buttons[3].cmdBytes[0] = 0x44;
  buttons[3].cmdBytes[1] = 0x41;
  buttons[3].cmdBytes[2] = 0x54;
  buttons[3].cmdBytes[3] = 0x41;
  buttons[3].cmdLen = 4;
  buttons[3].respType = RESP_RAW;
  buttons[3].color = 0x001F;
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

      tft.fillRect(btn.x + 1, btn.y + 1, btn.w - 2, btn.h - 2, 0xFFFF);
      delay(50);
      drawButton(i);

      handleButtonPress(i);

      break;
    }
  }
}

void handleButtonPress(uint8_t btnIdx) {
  Button &btn = buttons[btnIdx];

  while (fpgaSerial.available()) {
    fpgaSerial.read();
  }

  for (uint8_t i = 0; i < btn.cmdLen; i++) {
    fpgaSerial.write(btn.cmdBytes[i]);
  }

  Serial.print(F("[B] "));
  Serial.println(btn.label);

  if (btn.respType != RESP_NONE) {
    processButtonResponse(btn);
  }
}
void processButtonResponse(Button &btn) {
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
    if (fpgaSerial.available()) {
      respBytes[bytesRead++] = fpgaSerial.read();
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

  Serial.print(F("[F] "));
  Serial.println(result);
}