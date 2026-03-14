#include "sam_analog.h"
#include "sam_globals.h"
#include "sam_display.h"
#include "sam_logging.h"
#include "sam_hooks.h"

void handleAnalogRead(String params) {
  params.trim();
  params.toUpperCase();

  int pinNum = -1;
  if (params.startsWith("A")) {
    pinNum = params.substring(1).toInt();
  } else {
    pinNum = params.toInt();
  }

  if (pinNum < 8 || pinNum > 15) {
    Serial.println(F("ERR:PIN_RANGE (A8-A15 or 8-15)"));
    return;
  }

  uint8_t analogPin = analogPins[pinNum - 8];

  uint32_t sum = 0;
  for (uint8_t i = 0; i < analogAvgSamples; i++) {
    sum += analogRead(analogPin);
    if (analogAvgSamples > 1) delay(1);
  }
  uint16_t value = sum / analogAvgSamples;

  float voltage;
  if (analogRefMode == AREF_INTERNAL) {
    voltage = (value * 1.1) / 1023.0;
  } else {
    voltage = (value * 5.0) / 1023.0;
  }

  Serial.print(F("A"));
  Serial.print(pinNum);
  Serial.print(F(" = "));
  Serial.print(value);
  Serial.print(F(" ("));
  Serial.print(voltage, 3);
  Serial.print(F("V)"));
  if (analogAvgSamples > 1) {
    Serial.print(F(" [avg "));
    Serial.print(analogAvgSamples);
    Serial.print(F("]"));
  }
  Serial.println();
}

void handleAnalogReadAll() {
  Serial.println(F("Analog Inputs (A8-A15):"));

  const char* refStr;
  float refVoltage;
  if (analogRefMode == AREF_INTERNAL) {
    refStr = "INTERNAL";
    refVoltage = 1.1;
  } else {
    refStr = "DEFAULT";
    refVoltage = 5.0;
  }

  Serial.print(F("  Reference: "));
  Serial.print(refStr);
  Serial.print(F(" ("));
  Serial.print(refVoltage, 1);
  Serial.print(F("V), Averaging: "));
  Serial.println(analogAvgSamples);

  for (uint8_t i = 0; i < 8; i++) {
    uint8_t pinNum = i + 8;
    uint8_t analogPin = analogPins[i];

    uint32_t sum = 0;
    for (uint8_t j = 0; j < analogAvgSamples; j++) {
      sum += analogRead(analogPin);
      if (analogAvgSamples > 1) delay(1);
    }
    uint16_t value = sum / analogAvgSamples;

    float voltage = (value * refVoltage) / 1023.0;

    Serial.print(F("  A"));
    Serial.print(pinNum);
    Serial.print(F(": "));
    if (value < 10) Serial.print(F("   "));
    else if (value < 100) Serial.print(F("  "));
    else if (value < 1000) Serial.print(F(" "));
    Serial.print(value);
    Serial.print(F(" = "));
    Serial.print(voltage, 3);
    Serial.println(F("V"));
  }
}

void handleAnalogRef(String params) {
  params.trim();
  params.toUpperCase();

  if (params == "?") {
    Serial.print(F("ANALOGREF="));
    if (analogRefMode == AREF_INTERNAL) {
      Serial.println(F("INTERNAL (1.1V)"));
    } else {
      Serial.println(F("DEFAULT (5V)"));
    }
    return;
  }

  if (params == "DEFAULT") {
    analogRefMode = AREF_DEFAULT;
    analogReference(DEFAULT);
    Serial.println(F("Analog reference: DEFAULT (5V)"));
  } else if (params == "INTERNAL") {
    analogRefMode = AREF_INTERNAL;
    analogReference(INTERNAL1V1);
    Serial.println(F("Analog reference: INTERNAL (1.1V)"));
    Serial.println(F("WARNING: Max input 1.1V!"));
  } else if (params == "EXTERNAL") {
    Serial.println(F("ERR:EXTERNAL_NOT_AVAILABLE"));
    Serial.println(F("AREF pin used by LCD shield"));
    Serial.println(F("Use DEFAULT or INTERNAL only"));
  } else {
    Serial.println(F("ERR:USE DEFAULT|INTERNAL"));
  }
}

void handleAnalogAvg(String params) {
  int samples = params.toInt();

  if (samples < 1 || samples > 16) {
    Serial.println(F("ERR:RANGE (1-16 samples)"));
    return;
  }

  analogAvgSamples = samples;

  Serial.print(F("Analog averaging: "));
  Serial.print(analogAvgSamples);
  Serial.println(F(" samples"));
  if (samples > 1) {
    Serial.println(F("Note: Adds ~1ms delay per sample"));
  }
}

void handleStatsStart(String params) {
  int source = params.toInt();

  if (source < 0 || source > 7) {
    Serial.println(F("ERR:SOURCE_0_TO_7"));
    Serial.println(F("  0=A8, 1=A9, ... 7=A15"));
    return;
  }

  analogStats[source].minVal = 1023;
  analogStats[source].maxVal = 0;
  analogStats[source].sum = 0;
  analogStats[source].count = 0;
  analogStats[source].active = true;

  Serial.print(F("STATS_START: A"));
  Serial.println(source + 8);
}

void handleStatsStop(String params) {
  int source = params.toInt();

  if (source < 0 || source > 7) {
    Serial.println(F("ERR:SOURCE_0_TO_7"));
    return;
  }

  analogStats[source].active = false;
  Serial.print(F("STATS_STOP: A"));
  Serial.println(source + 8);
}

void handleStatsShow(String params) {
  int source = params.toInt();

  if (source < 0 || source > 7) {
    Serial.println(F("ERR:SOURCE_0_TO_7"));
    return;
  }

  Stats& s = analogStats[source];

  Serial.print(F("=== STATS A"));
  Serial.print(source + 8);
  Serial.println(F(" ==="));

  Serial.print(F("Active: "));
  Serial.println(s.active ? F("YES") : F("NO"));

  if (s.count == 0) {
    Serial.println(F("No data collected"));
  } else {
    Serial.print(F("Samples: "));
    Serial.println(s.count);

    Serial.print(F("Min: "));
    Serial.print(s.minVal);
    Serial.print(F(" ("));
    Serial.print((s.minVal * 5000UL) / 1024);
    Serial.println(F("mV)"));

    Serial.print(F("Max: "));
    Serial.print(s.maxVal);
    Serial.print(F(" ("));
    Serial.print((s.maxVal * 5000UL) / 1024);
    Serial.println(F("mV)"));

    uint16_t avg = s.sum / s.count;
    Serial.print(F("Avg: "));
    Serial.print(avg);
    Serial.print(F(" ("));
    Serial.print((avg * 5000UL) / 1024);
    Serial.println(F("mV)"));

    Serial.print(F("Range: "));
    Serial.println(s.maxVal - s.minVal);
  }

  Serial.println(F("================"));
}

void handleStatsReset() {
  for (uint8_t i = 0; i < 8; i++) {
    analogStats[i].minVal = 1023;
    analogStats[i].maxVal = 0;
    analogStats[i].sum = 0;
    analogStats[i].count = 0;
    analogStats[i].active = false;
  }
  Serial.println(F("STATS_RESET: All channels cleared"));
}

void updateStats() {
  for (uint8_t i = 0; i < 8; i++) {
    if (analogStats[i].active) {
      uint16_t val = analogRead(analogPins[i]);

      if (val < analogStats[i].minVal) analogStats[i].minVal = val;
      if (val > analogStats[i].maxVal) analogStats[i].maxVal = val;
      analogStats[i].sum += val;
      analogStats[i].count++;
    }
  }
}

void handleThresholdSet(String params) {
  int vals[3];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 3; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 3) {
    Serial.println(F("ERR:FORMAT #THRESHOLD <pin> <low> <high>"));
    Serial.println(F("  pin: 0=A8, 1=A9, ... 7=A15"));
    Serial.println(F("  low/high: 0-1023"));
    return;
  }

  int pin = vals[0];
  uint16_t low = vals[1];
  uint16_t high = vals[2];

  if (pin < 0 || pin > 7) {
    Serial.println(F("ERR:PIN_0_TO_7"));
    return;
  }

  if (low > 1023) low = 1023;
  if (high > 1023) high = 1023;
  if (low >= high) {
    Serial.println(F("ERR:LOW_MUST_BE_LESS_THAN_HIGH"));
    return;
  }

  thresholds[pin].lowThreshold = low;
  thresholds[pin].highThreshold = high;
  thresholds[pin].active = true;
  thresholds[pin].alertState = false;

  Serial.print(F("THRESHOLD_SET: A"));
  Serial.print(pin + 8);
  Serial.print(F(", Low="));
  Serial.print(low);
  Serial.print(F(", High="));
  Serial.println(high);
}

void handleThresholdClear(String params) {
  int pin = params.toInt();

  if (pin < 0 || pin > 7) {
    Serial.println(F("ERR:PIN_0_TO_7"));
    return;
  }

  thresholds[pin].active = false;
  thresholds[pin].alertState = false;

  Serial.print(F("THRESHOLD_CLEAR: A"));
  Serial.println(pin + 8);
}

void handleThresholdStatus() {
  Serial.println(F("=== THRESHOLD STATUS ==="));
  for (uint8_t i = 0; i < 8; i++) {
    if (thresholds[i].active) {
      Serial.print(F("A"));
      Serial.print(i + 8);
      Serial.print(F(": Low="));
      Serial.print(thresholds[i].lowThreshold);
      Serial.print(F(", High="));
      Serial.print(thresholds[i].highThreshold);
      Serial.print(F(", Alert="));
      Serial.println(thresholds[i].alertState ? F("YES") : F("NO"));
    }
  }
  Serial.println(F("========================"));
}

void checkThresholds() {
  for (uint8_t i = 0; i < 8; i++) {
    if (thresholds[i].active) {
      uint16_t val = analogRead(analogPins[i]);

      bool inAlert = (val < thresholds[i].lowThreshold || val > thresholds[i].highThreshold);

      if (inAlert && !thresholds[i].alertState) {
        thresholds[i].alertState = true;
        thresholdEventCount++;
        triggerThresholdHook(i, true);
        char em[EVENT_LOG_MSG_LEN];
        snprintf(em, sizeof(em), "A%u ENTER %u", i + 8, val);
        addEventLogC(EVLOG_THR, em);
        setWhy(em);

        Serial.print(F("ALERT: A"));
        Serial.print(i + 8);
        Serial.print(F(" = "));
        Serial.print(val);
        if (val < thresholds[i].lowThreshold) {
          Serial.print(F(" < "));
          Serial.println(thresholds[i].lowThreshold);
        } else {
          Serial.print(F(" > "));
          Serial.println(thresholds[i].highThreshold);
        }

        tft.fillRect(0, screenH - 20, screenW, 20, 0xF800);
        tft.setTextColor(0xFFFF, 0xF800);
        tft.setTextSize(2);
        tft.setCursor(5, screenH - 18);
        tft.print(F("ALERT A"));
        tft.print(i + 8);
        tft.print(F(": "));
        tft.print(val);

      } else if (!inAlert && thresholds[i].alertState) {
        thresholds[i].alertState = false;
        thresholdEventCount++;
        triggerThresholdHook(i, false);
        char em[EVENT_LOG_MSG_LEN];
        snprintf(em, sizeof(em), "A%u EXIT %u", i + 8, val);
        addEventLogC(EVLOG_THR, em);
        setWhy(em);

        Serial.print(F("CLEAR: A"));
        Serial.print(i + 8);
        Serial.print(F(" = "));
        Serial.println(val);

        tft.fillRect(0, screenH - 20, screenW, 20, 0x0000);
      }
    }
  }
}
