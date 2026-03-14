#include "sam_display.h"
#include "sam_globals.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <math.h>

void drawDivider() {
  if (viewMode == VIEW_SPLIT) {
    tft.drawLine(0, dividerY, screenW, dividerY, 0xFFFF);
  }
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

void showText(const char* txt) {
  uint16_t len = strlen(txt);
  uint16_t lineH = BASE_CHAR_H * topTextSize;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * topTextSize);
  if (charsPerLine == 0) charsPerLine = 1;
  uint16_t lines = len > 0 ? (len + charsPerLine - 1) / charsPerLine : 1;
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

  uint16_t col = 0;
  for (uint16_t i = 0; i < len; i++) {
    tft.print(txt[i]);
    col++;
    if (col >= charsPerLine && i < len - 1) {
      col = 0;
      topPosY += lineH;
      if (topPosY >= topMaxY) break;
      tft.setCursor(0, topPosY);
    }
  }
  tft.println();

  topPosY = tft.getCursorY();
  if (topPosY >= topMaxY) topPosY = 0;
  topPosX = 0;
}

void showTextBottom(const char* txt) {
  if (viewMode == VIEW_FULL) {
    showText(txt);
    return;
  }

  uint16_t len = strlen(txt);
  uint16_t lineH = BASE_CHAR_H * bottomTextSize;
  uint16_t charsPerLine = screenW / (BASE_CHAR_W * bottomTextSize);
  if (charsPerLine == 0) charsPerLine = 1;
  uint16_t lines = len > 0 ? (len + charsPerLine - 1) / charsPerLine : 1;
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

  uint16_t col = 0;
  for (uint16_t i = 0; i < len; i++) {
    tft.print(txt[i]);
    col++;
    if (col >= charsPerLine && i < len - 1) {
      col = 0;
      bottomPosY += lineH;
      if (bottomPosY >= bottomMaxY) break;
      tft.setCursor(0, bottomPosY);
    }
  }
  tft.println();

  bottomPosY = tft.getCursorY();
  if (bottomPosY >= bottomMaxY) bottomPosY = bottomMinY;
  bottomPosX = 0;
}

void applyViewMode() {
  if (viewMode == VIEW_FULL) {
    dividerY = screenH - 1;
    topMaxY = screenH;
    bottomMinY = 0;
    bottomMaxY = screenH;
    topPosX = topPosY = 0;
    bottomPosX = 0;
    bottomPosY = 0;
  } else {
    dividerY = screenH / 2;
    topMaxY = dividerY - 2;
    bottomMinY = dividerY + 2;
    bottomMaxY = screenH;
    if (topPosY >= topMaxY) topPosY = 0;
    if (bottomPosY < bottomMinY || bottomPosY >= bottomMaxY) bottomPosY = bottomMinY;
  }
}

void clearLinkIndicators() {
  int baseX = screenW - 30;
  int baseY = 4;
  for (uint8_t id = 1; id <= 3; id++) {
    int y = baseY + (id - 1) * 8;
    tft.fillRect(baseX - 12, y - 4, 14, 10, 0x0000);
  }
}

void drawLinkIndicators() {
  static unsigned long lastDraw = 0;
  unsigned long now = millis();
  if (now - lastDraw < 300) return;
  lastDraw = now;

  int baseX = screenW - 30;
  int baseY = 4;
  for (uint8_t id = 1; id <= 3; id++) {
    uint16_t col = 0x7BEF; // idle gray
    if (now - fpgaLastErrorMs[id] < 2000) {
      col = 0xF800; // red
    } else if (now - fpgaLastActivityMs[id] < 1200) {
      col = 0x07E0; // green
    } else {
      col = 0xFFE0; // yellow
    }
    int y = baseY + (id - 1) * 8;
    tft.fillRect(baseX - 10, y - 2, 8, 6, 0x0000);
    tft.fillCircle(baseX, y, 2, col);
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

void handleGauge(String params) {
  int vals[5];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 5; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 4) {
    Serial.println(F("ERR:FORMAT #GAUGE <x> <y> <r> <val> [max]"));
    return;
  }

  int16_t cx = vals[0];
  int16_t cy = vals[1];
  int16_t r = vals[2];
  int16_t value = vals[3];
  int16_t maxVal = (count >= 5) ? vals[4] : 100;

  value = constrain(value, 0, maxVal);

  tft.drawCircle(cx, cy, r, topTextColor);
  tft.drawCircle(cx, cy, r - 1, topTextColor);

  int angle = map(value, 0, maxVal, 135, 405);
  if (angle >= 360) angle -= 360;

  for (int a = 135; a <= 405; a += 30) {
    int actualAngle = a >= 360 ? a - 360 : a;
    float rad = actualAngle * PI / 180.0;
    int16_t x1 = cx + (r - 5) * cos(rad);
    int16_t y1 = cy + (r - 5) * sin(rad);
    int16_t x2 = cx + r * cos(rad);
    int16_t y2 = cy + r * sin(rad);
    tft.drawLine(x1, y1, x2, y2, topTextColor);
  }

  float rad = angle * PI / 180.0;
  int16_t needleX = cx + (r - 10) * cos(rad);
  int16_t needleY = cy + (r - 10) * sin(rad);
  tft.drawLine(cx, cy, needleX, needleY, 0xF800);
  tft.fillCircle(cx, cy, 3, 0xF800);

  tft.setTextSize(1);
  tft.setTextColor(topTextColor);
  tft.setCursor(cx - 12, cy + r - 15);
  tft.print(value);

  Serial.print(F("OK:GAUGE "));
  Serial.print(value);
  Serial.print(F("/"));
  Serial.println(maxVal);
}

void handleBarGraph(String params) {
  int vals[20];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 20; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 5) {
    Serial.println(F("ERR:FORMAT #BARGRAPH <x> <y> <w> <h> <val1> ..."));
    return;
  }

  int16_t x = vals[0];
  int16_t y = vals[1];
  int16_t w = vals[2];
  int16_t h = vals[3];
  int numBars = count - 4;

  int maxVal = 0;
  for (int i = 0; i < numBars; i++) {
    if (vals[4 + i] > maxVal) maxVal = vals[4 + i];
  }
  if (maxVal == 0) maxVal = 100;

  tft.drawRect(x, y, w, h, topTextColor);

  int barWidth = (w - (numBars + 1) * 2) / numBars;
  if (barWidth < 1) barWidth = 1;

  for (int i = 0; i < numBars; i++) {
    int barX = x + 2 + i * (barWidth + 2);
    int value = vals[4 + i];
    int barHeight = map(value, 0, maxVal, 0, h - 4);

    uint16_t barColor = (i % 3 == 0) ? 0x07E0 : (i % 3 == 1) ? 0x07FF : 0xFFE0;
    tft.fillRect(barX, y + h - 2 - barHeight, barWidth, barHeight, barColor);
  }

  Serial.print(F("OK:BARGRAPH "));
  Serial.print(numBars);
  Serial.println(F(" bars"));
}

void handleNumBox(String params) {
  int vals[4];
  int count = 0;
  int lastPos = 0;
  String valStr = "";

  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        if (count == 0) vals[count++] = params.substring(lastPos, i).toInt();
        else if (count == 1) vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #NUMBOX <x> <y> <value> [decimals]"));
    return;
  }

  valStr = params.substring(lastPos);
  valStr.trim();

  int16_t x = vals[0];
  int16_t y = vals[1];

  tft.setTextSize(4);
  tft.setTextColor(topTextColor, topBgColor);
  tft.setCursor(x, y);
  tft.print(valStr);

  Serial.print(F("OK:NUMBOX "));
  Serial.println(valStr);
}

void handleTrend(String params) {
  int vals[50];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 50; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 6) {
    Serial.println(F("ERR:FORMAT #TREND <x> <y> <w> <h> <val1> <val2> ..."));
    return;
  }

  int16_t x = vals[0];
  int16_t y = vals[1];
  int16_t w = vals[2];
  int16_t h = vals[3];
  int numPoints = count - 4;

  int minVal = vals[4];
  int maxVal = vals[4];
  for (int i = 1; i < numPoints; i++) {
    if (vals[4 + i] < minVal) minVal = vals[4 + i];
    if (vals[4 + i] > maxVal) maxVal = vals[4 + i];
  }
  if (maxVal == minVal) maxVal = minVal + 1;

  tft.drawRect(x, y, w, h, topTextColor);
  tft.drawLine(x, y + h / 2, x + w, y + h / 2, 0x39E7);

  int xStep = (w - 4) / (numPoints - 1);
  for (int i = 0; i < numPoints - 1; i++) {
    int x1 = x + 2 + i * xStep;
    int y1 = y + 2 + map(vals[4 + i], minVal, maxVal, h - 4, 0);
    int x2 = x + 2 + (i + 1) * xStep;
    int y2 = y + 2 + map(vals[4 + i + 1], minVal, maxVal, h - 4, 0);

    tft.drawLine(x1, y1, x2, y2, topTextColor);
    tft.fillCircle(x1, y1, 2, topTextColor);
  }

  int xLast = x + 2 + (numPoints - 1) * xStep;
  int yLast = y + 2 + map(vals[4 + numPoints - 1], minVal, maxVal, h - 4, 0);
  tft.fillCircle(xLast, yLast, 2, topTextColor);

  Serial.print(F("OK:TREND "));
  Serial.print(numPoints);
  Serial.println(F(" points"));
}

void handleXYPlot(String params) {
  int vals[100];
  int count = 0;
  int lastPos = 0;
  bool showGrid = false;
  uint8_t plotStyle = 2;

  if (params.indexOf(" GRID") > 0) {
    showGrid = true;
    params.replace(" GRID", "");
  }

  int stylePos = params.indexOf(" STYLE ");
  if (stylePos > 0) {
    String styleStr = params.substring(stylePos + 7);
    styleStr.trim();
    styleStr.toUpperCase();

    if (styleStr.startsWith("POINTS")) {
      plotStyle = 0;
      params = params.substring(0, stylePos);
    } else if (styleStr.startsWith("LINES")) {
      plotStyle = 1;
      params = params.substring(0, stylePos);
    } else if (styleStr.startsWith("BOTH")) {
      plotStyle = 2;
      params = params.substring(0, stylePos);
    }
  }

  for (int i = 0; i <= params.length() && count < 100; i++) {
    if (i == params.length() || params[i] == ' ' || params[i] == ',') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 6) {
    Serial.println(F("ERR:FORMAT #XYPLOT <x> <y> <w> <h> <x1,y1> <x2,y2> ..."));
    return;
  }

  int16_t px = vals[0];
  int16_t py = vals[1];
  int16_t pw = vals[2];
  int16_t ph = vals[3];
  int numPairs = (count - 4) / 2;

  if (numPairs < 2) {
    Serial.println(F("ERR:MIN 2 POINTS"));
    return;
  }

  int xData[50];
  int yData[50];
  for (int i = 0; i < numPairs; i++) {
    xData[i] = vals[4 + i * 2];
    yData[i] = vals[4 + i * 2 + 1];
  }

  int xMin = xData[0], xMax = xData[0];
  int yMin = yData[0], yMax = yData[0];

  for (int i = 1; i < numPairs; i++) {
    if (xData[i] < xMin) xMin = xData[i];
    if (xData[i] > xMax) xMax = xData[i];
    if (yData[i] < yMin) yMin = yData[i];
    if (yData[i] > yMax) yMax = yData[i];
  }

  if (xMax == xMin) xMax = xMin + 1;
  if (yMax == yMin) yMax = yMin + 1;

  int xRange = xMax - xMin;
  int yRange = yMax - yMin;
  xMin -= xRange / 10;
  xMax += xRange / 10;
  yMin -= yRange / 10;
  yMax += yRange / 10;

  tft.drawRect(px, py, pw, ph, topTextColor);

  if (showGrid) {
    uint16_t gridColor = 0x39E7;

    for (int i = 1; i < 5; i++) {
      int gx = px + (pw * i) / 5;
      for (int gy = py + 2; gy < py + ph - 2; gy += 4) {
        tft.drawPixel(gx, gy, gridColor);
      }
    }

    for (int i = 1; i < 5; i++) {
      int gy = py + (ph * i) / 5;
      for (int gx = px + 2; gx < px + pw - 2; gx += 4) {
        tft.drawPixel(gx, gy, gridColor);
      }
    }
  }

  tft.setTextSize(1);
  tft.setTextColor(topTextColor);

  tft.setCursor(px + 2, py + ph - 10);
  tft.print(xMin);

  String xMaxStr = String(xMax);
  tft.setCursor(px + pw - 6 * xMaxStr.length() - 2, py + ph - 10);
  tft.print(xMax);

  tft.setCursor(px + 2, py + 2);
  tft.print(yMax);

  tft.setCursor(px + 2, py + ph - 20);
  tft.print(yMin);

  for (int i = 0; i < numPairs; i++) {
    int sx = px + 2 + map(xData[i], xMin, xMax, 0, pw - 4);
    int sy = py + 2 + map(yData[i], yMax, yMin, 0, ph - 4);

    sx = constrain(sx, px + 2, px + pw - 2);
    sy = constrain(sy, py + 2, py + ph - 2);

    if (plotStyle == 0 || plotStyle == 2) {
      tft.fillCircle(sx, sy, 2, topTextColor);
    }

    if (i < numPairs - 1 && (plotStyle == 1 || plotStyle == 2)) {
      int sx2 = px + 2 + map(xData[i + 1], xMin, xMax, 0, pw - 4);
      int sy2 = py + 2 + map(yData[i + 1], yMax, yMin, 0, ph - 4);

      sx2 = constrain(sx2, px + 2, px + pw - 2);
      sy2 = constrain(sy2, py + 2, py + ph - 2);

      tft.drawLine(sx, sy, sx2, sy2, topTextColor);
    }
  }

  Serial.print(F("OK:XYPLOT "));
  Serial.print(numPairs);
  Serial.print(F(" points ["));
  Serial.print(xMin);
  Serial.print(F(","));
  Serial.print(yMin);
  Serial.print(F(" - "));
  Serial.print(xMax);
  Serial.print(F(","));
  Serial.print(yMax);
  Serial.println(F("]"));
}

void handleScroll(String params) {
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #SCROLL <y> <text>"));
    Serial.println(F("  y: Y position (0-screenH)"));
    return;
  }

  int y = params.substring(0, sp).toInt();

  if (y < 0 || y >= screenH - 20) {
    Serial.println(F("ERR:Y_OUT_OF_RANGE"));
    return;
  }

  // Copy text into a stack buffer to avoid heap allocation
  char text[64];
  int textStart = sp + 1;
  int textLen = params.length() - textStart;
  if (textLen <= 0) {
    Serial.println(F("ERR:NO_TEXT"));
    return;
  }
  if (textLen > (int)sizeof(text) - 1) textLen = sizeof(text) - 1;
  params.substring(textStart, textStart + textLen).toCharArray(text, sizeof(text));

  tft.setTextSize(topTextSize);
  tft.setTextColor(topTextColor);

  // Estimate width from char count (6px * size per char) to avoid getTextBounds heap use
  int w = strlen(text) * BASE_CHAR_W * topTextSize;
  int h = BASE_CHAR_H * topTextSize;

  for (int x = screenW; x > -w; x -= 4) {
    tft.fillRect(0, y, screenW, h + 2, 0x0000);
    tft.setCursor(x, y);
    tft.print(text);
    delay(20);
    if (Serial.available()) {
      break;
    }
  }

  tft.fillRect(0, y, screenW, h + 2, 0x0000);
  Serial.println(F("SCROLL_DONE"));
}
