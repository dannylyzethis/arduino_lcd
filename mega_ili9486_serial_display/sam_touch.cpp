#include "sam_touch.h"
#include "sam_globals.h"
#include <TouchScreen.h>
#include "sam_display.h"
#include "sam_fpga_serial.h"
#include "sam_eeprom.h"

void initButtons() {
  uint16_t btnHeight = 45;
  uint16_t btnWidth = (screenW - 15) / 4;
  uint16_t btnY = screenH - btnHeight - 2;
  buttonTextY = btnY - 2;

  buttons[0].x = 5;
  buttons[0].y = btnY;
  buttons[0].w = btnWidth;
  buttons[0].h = btnHeight;
  buttons[0].label = "UP";
  buttons[0].cmdLen = btnUp.length;
  for (uint8_t i = 0; i < btnUp.length && i < 15; i++) {
    buttons[0].cmdBytes[i] = btnUp.data[i];
  }
  buttons[0].color = 0x07E0;

  buttons[1].x = 5 + btnWidth + 3;
  buttons[1].y = btnY;
  buttons[1].w = btnWidth;
  buttons[1].h = btnHeight;
  buttons[1].label = "DN";
  buttons[1].cmdLen = btnDown.length;
  for (uint8_t i = 0; i < btnDown.length && i < 15; i++) {
    buttons[1].cmdBytes[i] = btnDown.data[i];
  }
  buttons[1].color = 0x07E0;

  buttons[2].x = 5 + (btnWidth + 3) * 2;
  buttons[2].y = btnY;
  buttons[2].w = btnWidth;
  buttons[2].h = btnHeight;
  buttons[2].label = "LT";
  buttons[2].cmdLen = btnLeft.length;
  for (uint8_t i = 0; i < btnLeft.length && i < 15; i++) {
    buttons[2].cmdBytes[i] = btnLeft.data[i];
  }
  buttons[2].color = 0x07E0;

  buttons[3].x = 5 + (btnWidth + 3) * 3;
  buttons[3].y = btnY;
  buttons[3].w = btnWidth;
  buttons[3].h = btnHeight;
  buttons[3].label = "RT";
  buttons[3].cmdLen = btnRight.length;
  for (uint8_t i = 0; i < btnRight.length && i < 15; i++) {
    buttons[3].cmdBytes[i] = btnRight.data[i];
  }
  buttons[3].color = 0x07E0;
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

  tft.fillRect(0, bottomMinY, screenW, screenH - bottomMinY, bottomBgColor);
  bottomPosX = 0;
  bottomPosY = bottomMinY;

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    drawButton(i);
  }

  buttonsVisible = true;
  drawDivider();

  Serial.println(F("=== BUTTONS VISIBLE ==="));
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    Serial.print(F("BTN"));
    Serial.print(i);
    Serial.print(F(" ["));
    Serial.print(buttons[i].label);
    Serial.print(F("]: X="));
    Serial.print(buttons[i].x);
    Serial.print(F("-"));
    Serial.print(buttons[i].x + buttons[i].w);
    Serial.print(F(", Y="));
    Serial.print(buttons[i].y);
    Serial.print(F("-"));
    Serial.println(buttons[i].y + buttons[i].h);
  }
}

void hideButtons() {
  if (!buttonsVisible) return;

  bottomMaxY = screenH;

  tft.fillRect(0, buttonTextY, screenW, screenH - buttonTextY, bottomBgColor);

  buttonsVisible = false;
  drawDivider();
}

void checkTouch() {
  unsigned long now = millis();
  if (now - lastTouch < TOUCH_DEBOUNCE) return;

  TSPoint p = ts.getPoint();

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  digitalWrite(XM, HIGH);
  digitalWrite(YP, HIGH);

  if (p.z < MINPRESSURE || p.z > MAXPRESSURE) return;

  lastTouch = now;  // debounce all touch paths

  Serial.print(F("[TOUCH] Raw: X="));
  Serial.print(p.x);
  Serial.print(F(" Y="));
  Serial.print(p.y);
  Serial.print(F(" Z="));
  Serial.println(p.z);

  if (touchTestMode) {
    lastTouch = now;
    Serial.print(F("[TOUCH] Raw: X="));
    Serial.print(p.x);
    Serial.print(F(" Y="));
    Serial.print(p.y);
    Serial.print(F(" Z="));
    Serial.print(p.z);

    uint8_t rotation = tft.getRotation();
    int16_t px, py;
    switch (rotation) {
      case 0:
        px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
        py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
        break;
      case 1:
        px = map(p.y, TS_MAXY, TS_MINY, 0, screenW);
        py = map(p.x, TS_MAXX, TS_MINX, 0, screenH);
        break;
      case 2:
        px = map(p.x, TS_MAXX, TS_MINX, 0, screenW);
        py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
        break;
      case 3:
        px = map(p.y, TS_MINY, TS_MAXY, 0, screenW);
        py = map(p.x, TS_MINX, TS_MAXX, 0, screenH);
        break;
      default:
        px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
        py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
        break;
    }

    Serial.print(F(" | Mapped: X="));
    Serial.print(px);
    Serial.print(F(" Y="));
    Serial.print(py);
    Serial.print(F(" | Rot="));
    Serial.println(rotation);
    return;
  }

  int16_t px, py;
  uint8_t rotation = tft.getRotation();

  switch (rotation) {
    case 0:
      px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
      py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
      break;
    case 1:
      px = map(p.y, TS_MAXY, TS_MINY, 0, screenW);
      py = map(p.x, TS_MAXX, TS_MINX, 0, screenH);
      break;
    case 2:
      px = map(p.x, TS_MAXX, TS_MINX, 0, screenW);
      py = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
      break;
    case 3:
      px = map(p.y, TS_MINY, TS_MAXY, 0, screenW);
      py = map(p.x, TS_MINX, TS_MAXX, 0, screenH);
      break;
    default:
      px = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
      py = map(p.y, TS_MAXY, TS_MINY, 0, screenH);
      break;
  }

  Serial.print(F("[TOUCH] Mapped: X="));
  Serial.print(px);
  Serial.print(F(" Y="));
  Serial.print(py);
  Serial.print(F(" | Rot="));
  Serial.print(rotation);
  Serial.print(F(" | Cal: X["));
  Serial.print(TS_MINX);
  Serial.print(F("-"));
  Serial.print(TS_MAXX);
  Serial.print(F("] Y["));
  Serial.print(TS_MINY);
  Serial.print(F("-"));
  Serial.print(TS_MAXY);
  Serial.println(F("]"));

  if (px >= (screenW - 36) && py <= 36) {
    if (now - lastViewToggleTouch >= 500) {
      viewMode = (viewMode == VIEW_FULL) ? VIEW_SPLIT : VIEW_FULL;
      applyViewMode();
      tft.fillScreen(0x0000);
      drawDivider();
      saveConfig();
      lastViewToggleTouch = now;
      Serial.print(F("OK:VIEW_"));
      Serial.println(viewMode == VIEW_FULL ? F("FULL_TOUCH") : F("SPLIT_TOUCH"));
    }
    return;
  }

  bool buttonHit = false;
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    Button &btn = buttons[i];

    Serial.print(F("  Checking BTN"));
    Serial.print(i);
    Serial.print(F(" ["));
    Serial.print(btn.label);
    Serial.print(F("]: X="));
    Serial.print(btn.x);
    Serial.print(F("-"));
    Serial.print(btn.x + btn.w);
    Serial.print(F(", Y="));
    Serial.print(btn.y);
    Serial.print(F("-"));
    Serial.print(btn.y + btn.h);

    if (px >= btn.x && px < (btn.x + btn.w) &&
        py >= btn.y && py < (btn.y + btn.h)) {
      Serial.println(F(" --> HIT!"));
      buttonHit = true;

      lastTouch = now;

      tft.fillRect(btn.x + 1, btn.y + 1, btn.w - 2, btn.h - 2, 0xFFFF);
      delay(50);
      drawButton(i);

      HardwareSerial& fpga = getFPGA();
      sendBytesToFPGA(fpga, activeFpga, btn.cmdBytes, btn.cmdLen, true);

      Serial.print(F("[BTN>FPGA"));
      Serial.print(activeFpga);
      Serial.print(F("] "));
      Serial.print(btn.label);
      Serial.print(F(" = "));

      String hexDisplay = "";
      for (uint8_t j = 0; j < btn.cmdLen; j++) {
        if (j > 0) {
          Serial.print(F(" "));
          hexDisplay += " ";
        }
        Serial.print(F("0x"));
        if (btn.cmdBytes[j] < 16) Serial.print(F("0"));
        Serial.print(btn.cmdBytes[j], HEX);

        hexDisplay += "0x";
        if (btn.cmdBytes[j] < 16) hexDisplay += "0";
        hexDisplay += String(btn.cmdBytes[j], HEX);
      }
      Serial.println();

      String msg = "TX: " + hexDisplay;
      showTextBottom(msg.c_str());

      break;
    } else {
      Serial.println(F(" - miss"));
    }
  }

  if (!buttonHit) {
    Serial.println(F("[TOUCH] No button was hit at these coordinates"));
  }
}
