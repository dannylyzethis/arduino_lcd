#include "sam_logging.h"
#include "sam_globals.h"
#include <EEPROM.h>
#include "sam_commands.h"

const __FlashStringHelper* eventTypeName(uint8_t t) {
  switch (t) {
    case EVLOG_RULE: return F("RULE");
    case EVLOG_GPIO: return F("GPIO");
    case EVLOG_THR:  return F("THR");
    case EVLOG_ERR:  return F("ERR");
    default:         return F("INFO");
  }
}

void addEventLog(uint8_t type, const String& msg) {
  EventLogEntry& e = eventLog[eventLogHead];
  e.ms = millis();
  e.type = type;

  String m = msg;
  if (m.length() >= (EVENT_LOG_MSG_LEN - 1)) {
    m = m.substring(0, EVENT_LOG_MSG_LEN - 1);
  }
  m.toCharArray(e.msg, EVENT_LOG_MSG_LEN);

  eventLogHead = (eventLogHead + 1) % EVENT_LOG_CAPACITY;
  if (eventLogCount < EVENT_LOG_CAPACITY) eventLogCount++;
}

void addEventLogC(uint8_t type, const char* msg) {
  EventLogEntry& e = eventLog[eventLogHead];
  e.ms = millis();
  e.type = type;
  if (msg) {
    strncpy(e.msg, msg, EVENT_LOG_MSG_LEN - 1);
    e.msg[EVENT_LOG_MSG_LEN - 1] = '\0';
  } else {
    e.msg[0] = '\0';
  }
  eventLogHead = (eventLogHead + 1) % EVENT_LOG_CAPACITY;
  if (eventLogCount < EVENT_LOG_CAPACITY) eventLogCount++;
}

void setWhy(const char* msg) {
  if (!msg) return;
  strncpy(lastWhy, msg, sizeof(lastWhy) - 1);
  lastWhy[sizeof(lastWhy) - 1] = '\0';
  lastWhyMs = millis();
}

int8_t parseEventTypeFilter(String s) {
  s.trim();
  s.toUpperCase();
  if (s == "ALL" || s.length() == 0) return -1;
  if (s == "RULE") return EVLOG_RULE;
  if (s == "GPIO") return EVLOG_GPIO;
  if (s == "THR" || s == "THRESH") return EVLOG_THR;
  if (s == "ERR" || s == "ERROR") return EVLOG_ERR;
  if (s == "INFO") return EVLOG_INFO;
  return -2;
}

void printEventLogLast(uint8_t wanted, int8_t filterType) {
  if (eventLogCount == 0) {
    Serial.println(F("(no events)"));
    return;
  }

  uint8_t printed = 0;
  uint8_t scanned = 0;
  int idx = (int)eventLogHead - 1;
  if (idx < 0) idx += EVENT_LOG_CAPACITY;

  while (scanned < eventLogCount && printed < wanted) {
    EventLogEntry& e = eventLog[idx];
    if (filterType < 0 || e.type == (uint8_t)filterType) {
      Serial.print(F("["));
      Serial.print(e.ms);
      Serial.print(F("ms] "));
      Serial.print(eventTypeName(e.type));
      Serial.print(F(" "));
      Serial.println(e.msg);
      printed++;
    }
    scanned++;
    idx--;
    if (idx < 0) idx = EVENT_LOG_CAPACITY - 1;
  }

  if (printed == 0) Serial.println(F("(no matching events)"));
}

void updateDataLogger() {
  unsigned long now = millis();

  if (now - lastLogTime >= logInterval) {
    lastLogTime = now;

    // Read value based on source
    uint16_t value = 0;
    if (logSource <= 7) {
      // Analog pin A8-A15
      value = analogRead(analogPins[logSource]);
    } else if (logSource == 15) {
      // GPIO register (stored as lower 16 bits for EEPROM log entry)
      uint32_t regVal = 0;
      for (uint8_t i = 0; i < NUM_GPIO; i++) {
        if (gpioModes[i] != GPIO_OUTPUT) {
          if (digitalRead(gpioPins[i])) regVal |= (1UL << i);
        } else {
          if (gpioStates[i]) regVal |= (1UL << i);
        }
      }
      value = (uint16_t)(regVal & 0xFFFF);
    }

    // Write log entry to EEPROM
    // Format: [timestamp:4][value:2][source:1][flags:1]
    uint16_t addr = logWriteAddr;

    // Write timestamp (4 bytes)
    EEPROM.write(addr++, (now >> 24) & 0xFF);
    EEPROM.write(addr++, (now >> 16) & 0xFF);
    EEPROM.write(addr++, (now >> 8) & 0xFF);
    EEPROM.write(addr++, now & 0xFF);

    // Write value (2 bytes)
    EEPROM.write(addr++, (value >> 8) & 0xFF);
    EEPROM.write(addr++, value & 0xFF);

    // Write source (1 byte)
    EEPROM.write(addr++, logSource);

    // Write flags (1 byte) - reserved
    EEPROM.write(addr++, 0);

    // Update write address (circular buffer)
    logWriteAddr = addr;
    if (logWriteAddr > logEndAddr - LOG_ENTRY_SIZE) {
      logWriteAddr = logStartAddr;
    }

    logEntryCount++;
    if (logEntryCount > (logEndAddr - logStartAddr + 1) / LOG_ENTRY_SIZE) {
      logEntryCount = (logEndAddr - logStartAddr + 1) / LOG_ENTRY_SIZE;
    }
  }
}

void handleLogStart(String params) {
  // Parse: <interval> <source>
  // source: 0-7=A8-A15, 15=GPIO_REG
  int sp = params.indexOf(' ');

  if (sp == -1) {
    logInterval = params.toInt();
    // Keep current source
  } else {
    logInterval = params.substring(0, sp).toInt();
    logSource = params.substring(sp + 1).toInt();
  }

  if (logInterval < 10) logInterval = 10;  // Min 10ms
  if (logSource > 15) logSource = 0;

  // Initialize logging position
  logWriteAddr = logStartAddr;
  logEntryCount = 0;

  loggingActive = true;
  lastLogTime = millis();

  Serial.print(F("LOG_START: "));
  Serial.print(logInterval);
  Serial.print(F("ms, source="));
  if (logSource <= 7) {
    Serial.print(F("A"));
    Serial.println(logSource + 8);
  } else if (logSource == 15) {
    Serial.println(F("GPIO_REG"));
  } else {
    Serial.println(logSource);
  }
}

void handleLogStop() {
  loggingActive = false;
  Serial.println(F("LOG_STOPPED"));
  Serial.print(F("Entries logged: "));
  Serial.println(logEntryCount);
}

void handleLogRead(String params) {
  uint16_t numEntries = 10;  // Default
  if (params.length() > 0) {
    numEntries = params.toInt();
  }

  if (numEntries > logEntryCount) numEntries = logEntryCount;

  Serial.print(F("=== LOG DATA ("));
  Serial.print(numEntries);
  Serial.println(F(" entries) ==="));
  Serial.println(F("Time(ms)   Value  Source"));

  // Read from oldest to newest
  uint16_t readAddr = logWriteAddr - (numEntries * LOG_ENTRY_SIZE);
  if (readAddr < logStartAddr) {
    readAddr = logEndAddr - (logStartAddr - readAddr) + 1;
  }

  for (uint16_t i = 0; i < numEntries; i++) {
    // Read timestamp
    unsigned long timestamp = 0;
    timestamp |= ((unsigned long)EEPROM.read(readAddr++)) << 24;
    timestamp |= ((unsigned long)EEPROM.read(readAddr++)) << 16;
    timestamp |= ((unsigned long)EEPROM.read(readAddr++)) << 8;
    timestamp |= EEPROM.read(readAddr++);

    // Read value
    uint16_t value = 0;
    value |= ((uint16_t)EEPROM.read(readAddr++)) << 8;
    value |= EEPROM.read(readAddr++);

    // Read source
    uint8_t source = EEPROM.read(readAddr++);

    // Skip flags
    readAddr++;

    // Wrap address
    if (readAddr > logEndAddr) {
      readAddr = logStartAddr;
    }

    // Print entry
    Serial.print(timestamp);
    Serial.print(F("\t"));
    Serial.print(value);
    Serial.print(F("\t"));
    if (source <= 7) {
      Serial.print(F("A"));
      Serial.println(source + 8);
    } else if (source == 15) {
      Serial.println(F("GPIO"));
    } else {
      Serial.println(source);
    }
  }
}

void handleLogClear() {
  logWriteAddr = logStartAddr;
  logEntryCount = 0;
  Serial.println(F("LOG_CLEARED"));
}

void handleLogConfig(String params) {
  // #LOGCONFIG <source>
  logSource = params.toInt();
  if (logSource > 15) logSource = 0;

  Serial.print(F("LOG_SOURCE="));
  if (logSource <= 7) {
    Serial.print(F("A"));
    Serial.println(logSource + 8);
  } else if (logSource == 15) {
    Serial.println(F("GPIO_REG"));
  } else {
    Serial.println(logSource);
  }
}

void handleLogStatus() {
  Serial.println(F("=== LOG STATUS ==="));
  Serial.print(F("Active: "));
  Serial.println(loggingActive ? F("YES") : F("NO"));
  Serial.print(F("Interval: "));
  Serial.print(logInterval);
  Serial.println(F("ms"));
  Serial.print(F("Source: "));
  if (logSource <= 7) {
    Serial.print(F("A"));
    Serial.println(logSource + 8);
  } else if (logSource == 15) {
    Serial.println(F("GPIO_REG"));
  } else {
    Serial.println(logSource);
  }
  Serial.print(F("Entries: "));
  Serial.println(logEntryCount);
  Serial.print(F("Next addr: 0x"));
  Serial.println(logWriteAddr, HEX);
}

void handleLogZone(String params) {
  // #LOGZONE <start> <end> - Configure logging address range
  // #LOGZONE ? - Query current range

  if (params == "?") {
    Serial.print(F("LOG_ZONE: 0x"));
    Serial.print(logStartAddr, HEX);
    Serial.print(F(" - 0x"));
    Serial.println(logEndAddr, HEX);
    uint16_t capacity = (logEndAddr - logStartAddr + 1) / LOG_ENTRY_SIZE;
    Serial.print(F("Capacity: "));
    Serial.print(capacity);
    Serial.println(F(" entries"));
    return;
  }

  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #LOGZONE <start> <end>"));
    return;
  }

  uint16_t start = parseHexOrDec(params.substring(0, sp));
  uint16_t end = parseHexOrDec(params.substring(sp + 1));

  if (start >= EEPROM.length() || end >= EEPROM.length()) {
    Serial.println(F("ERR:ADDR_RANGE"));
    return;
  }

  if (start >= end) {
    Serial.println(F("ERR:START >= END"));
    return;
  }

  // Check if any protection zones overlap
  for (uint8_t i = 0; i < MAX_PROTECT_ZONES; i++) {
    if (protectZones[i].enabled) {
      if ((start >= protectZones[i].startAddr && start <= protectZones[i].endAddr) ||
          (end >= protectZones[i].startAddr && end <= protectZones[i].endAddr)) {
        Serial.print(F("ERR:OVERLAPS_ZONE "));
        Serial.println(i);
        return;
      }
    }
  }

  logStartAddr = start;
  logEndAddr = end;
  logWriteAddr = logStartAddr;
  logEntryCount = 0;

  Serial.print(F("LOG_ZONE: 0x"));
  Serial.print(logStartAddr, HEX);
  Serial.print(F(" - 0x"));
  Serial.println(logEndAddr, HEX);
}
