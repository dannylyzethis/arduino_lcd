#include "sam_fpga_serial.h"
#include "sam_globals.h"
#include "sam_display.h"
#include "sam_logging.h"
#include "sam_commands.h"

HardwareSerial& getFPGA() {
  switch (activeFpga) {
    case 2: return Serial2;
    case 3: return Serial3;
    default: return Serial1;
  }
}

// Apply termination character to serial stream
void applyTermination(HardwareSerial& serial) {
  switch (bypassTerm) {
    case TERM_LF:
      serial.write(0x0A);  // \n
      break;
    case TERM_CR:
      serial.write(0x0D);  // \r
      break;
    case TERM_CRLF:
      serial.write(0x0D);  // \r
      serial.write(0x0A);  // \n
      break;
    case TERM_CUSTOM:
      serial.write(customTermByte);  // Custom stop byte
      break;
    case TERM_NONE:
      // No termination
      break;
  }
}

uint8_t crc8Simple(const uint8_t* data, uint8_t len) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
  }
  return crc;
}

void sendBytesToFPGA(HardwareSerial& fpga, uint8_t fpgaId, const uint8_t* data, uint8_t len, bool addTermination) {
  if (!data || len == 0) return;

  if (protoFrameMode) {
    uint8_t crcInput[1 + FRAME_MAX_PAYLOAD];
    crcInput[0] = len;
    for (uint8_t i = 0; i < len; i++) crcInput[1 + i] = data[i];
    uint8_t crc = crc8Simple(crcInput, len + 1);
    fpga.write(FRAME_SYNC);
    fpga.write(len);
    for (uint8_t i = 0; i < len; i++) fpga.write(data[i]);
    fpga.write(crc);
    fpgaTxBytes[fpgaId] += (len + 3);
  } else {
    for (uint8_t i = 0; i < len; i++) fpga.write(data[i]);
    if (addTermination) applyTermination(fpga);
    fpgaTxBytes[fpgaId] += len + (addTermination ? 1 : 0);
  }

  fpgaLastActivityMs[fpgaId] = millis();
}

// Read response bytes from FPGA with timeout
void readFPGAResponse(uint8_t numBytes, uint16_t timeout) {
  HardwareSerial& fpga = getFPGA();
  uint8_t buffer[64];
  uint8_t bytesRead = 0;
  unsigned long startTime = millis();

  // Get RX mode
  FPGARxMode rxMode;
  switch (activeFpga) {
    case 1: rxMode = fpga1RxMode; break;
    case 2: rxMode = fpga2RxMode; break;
    case 3: rxMode = fpga3RxMode; break;
    default: rxMode = FPGA_RX_TEXT; break;
  }

  Serial.print(F("[FPGA"));
  Serial.print(activeFpga);
  Serial.print(F(" READ "));
  Serial.print(numBytes);
  Serial.print(F(" bytes] "));

  // Read bytes with timeout
  while (bytesRead < numBytes && (millis() - startTime) < timeout) {
    if (fpga.available()) {
      buffer[bytesRead++] = fpga.read();
    }
  }

  if (bytesRead == 0) {
    fpgaTimeouts[activeFpga]++;
    fpgaLastErrorMs[activeFpga] = millis();
    char em[EVENT_LOG_MSG_LEN];
    snprintf(em, sizeof(em), "F%u TIMEOUT", activeFpga);
    addEventLogC(EVLOG_ERR, em);
    setWhy(em);
    Serial.println(F("TIMEOUT"));
    showTextBottom("FPGA: TIMEOUT");
    return;
  }

  if (bytesRead < numBytes) {
    fpgaTimeouts[activeFpga]++;
    fpgaLastErrorMs[activeFpga] = millis();
    Serial.print(F("PARTIAL("));
    Serial.print(bytesRead);
    Serial.print(F("/"));
    Serial.print(numBytes);
    Serial.println(F(")"));
  }

  // Display based on RX mode
  if (rxMode == FPGA_RX_BINARY) {
    // Binary mode: show as hex
    String hexStr = "";
    for (uint8_t i = 0; i < bytesRead; i++) {
      if (i > 0) {
        Serial.print(F(" "));
        hexStr += " ";
      }
      Serial.print(F("0x"));
      if (buffer[i] < 16) Serial.print(F("0"));
      Serial.print(buffer[i], HEX);

      hexStr += "0x";
      if (buffer[i] < 16) hexStr += "0";
      hexStr += String(buffer[i], HEX);
    }
    Serial.println();
    showTextBottom(hexStr.c_str());

  } else {
    // Text mode: show as characters
    String textStr = "";
    for (uint8_t i = 0; i < bytesRead; i++) {
      char c = (char)buffer[i];
      Serial.write(c);
      if (isPrintable(c)) {
        textStr += c;
      } else if (c == '\n' || c == '\r') {
        // Skip line endings in display
      } else {
        // Non-printable: show as hex
        textStr += "[0x";
        if (buffer[i] < 16) textStr += "0";
        textStr += String(buffer[i], HEX);
        textStr += "]";
      }
    }
    Serial.println();
    showTextBottom(textStr.c_str());
  }

  Serial.print(F("("));
  Serial.print(bytesRead);
  Serial.print(F("/"));
  Serial.print(numBytes);
  Serial.println(F(" bytes)"));
}

// Set UART hardware stop bits (1 or 2) - ATmega2560 specific
// Directly manipulates UCSRnC registers
void setSerialStopBits(uint8_t serialNum, uint8_t stopBits) {
  if (stopBits != 1 && stopBits != 2) return;

  // USBS bit (bit 3) in UCSRnC register
  // 0 = 1 stop bit, 1 = 2 stop bits
  bool twoStopBits = (stopBits == 2);

  switch (serialNum) {
    case 1:  // Serial1 - UCSR1C
      if (twoStopBits) {
        UCSR1C |= (1 << USBS1);   // Set bit 3: 2 stop bits
      } else {
        UCSR1C &= ~(1 << USBS1);  // Clear bit 3: 1 stop bit
      }
      fpga1StopBits = stopBits;
      break;

    case 2:  // Serial2 - UCSR2C
      if (twoStopBits) {
        UCSR2C |= (1 << USBS2);   // Set bit 3: 2 stop bits
      } else {
        UCSR2C &= ~(1 << USBS2);  // Clear bit 3: 1 stop bit
      }
      fpga2StopBits = stopBits;
      break;

    case 3:  // Serial3 - UCSR3C
      if (twoStopBits) {
        UCSR3C |= (1 << USBS3);   // Set bit 3: 2 stop bits
      } else {
        UCSR3C &= ~(1 << USBS3);  // Clear bit 3: 1 stop bit
      }
      fpga3StopBits = stopBits;
      break;
  }
}

void checkFPGASerial(HardwareSerial& serial, uint8_t id) {
  // Get RX mode and parse mode for this serial port
  FPGARxMode rxMode;
  FPGAParseMode parseMode;
  switch (id) {
    case 1: rxMode = fpga1RxMode; parseMode = fpga1ParseMode; break;
    case 2: rxMode = fpga2RxMode; parseMode = fpga2ParseMode; break;
    case 3: rxMode = fpga3RxMode; parseMode = fpga3ParseMode; break;
    default: rxMode = FPGA_RX_TEXT; parseMode = PARSE_NONE; break;
  }

  while (serial.available()) {
    uint8_t byteVal = serial.read();
    fpgaRxBytes[id]++;
    fpgaLastActivityMs[id] = millis();

    if (bridgeMode) {
      Serial.print(F("[BRX"));
      Serial.print(id);
      Serial.print(F(" t="));
      Serial.print(millis());
      Serial.print(F("] 0x"));
      if (byteVal < 16) Serial.print(F("0"));
      Serial.println(byteVal, HEX);
    }

    if (protoFrameMode) {
      FrameRxState& st = frameRx[id];
      if (st.stage == 0) {
        if (byteVal == FRAME_SYNC) {
          st.stage = 1;
        } else {
          fpgaDropped[id]++;
        }
        continue;
      } else if (st.stage == 1) {
        if (byteVal == 0 || byteVal > FRAME_MAX_PAYLOAD) {
          st.stage = 0;
          fpgaFrameErr[id]++;
          fpgaLastErrorMs[id] = millis();
          continue;
        }
        st.len = byteVal;
        st.idx = 0;
        st.stage = 2;
        continue;
      } else if (st.stage == 2) {
        st.payload[st.idx++] = byteVal;
        if (st.idx >= st.len) st.stage = 3;
        continue;
      } else {  // stage 3 crc
        uint8_t crcInput[1 + FRAME_MAX_PAYLOAD];
        crcInput[0] = st.len;
        for (uint8_t i = 0; i < st.len; i++) crcInput[1 + i] = st.payload[i];
        uint8_t expectCrc = crc8Simple(crcInput, st.len + 1);
        if (expectCrc != byteVal) {
          fpgaFrameErr[id]++;
          fpgaLastErrorMs[id] = millis();
          st.stage = 0;
          continue;
        }
        fpgaFrameOk[id]++;
        st.stage = 0;
        for (uint8_t p = 0; p < st.len; p++) {
          uint8_t frameByte = st.payload[p];
          byteVal = frameByte;
          // fall-through to normal display parsing for each payload byte

          if (rxMode == FPGA_RX_BINARY) {
            // Binary mode: display based on parse mode
            String byteStr = "";

            switch (parseMode) {
              case PARSE_ASCII:
                // Show as ASCII with [XX] for non-printable
                if (isPrintable(byteVal)) {
                  byteStr = String((char)byteVal);
                  Serial.write(byteVal);
                } else {
                  byteStr = "[";
                  if (byteVal < 16) byteStr += "0";
                  byteStr += String(byteVal, HEX);
                  byteStr += "]";
                  Serial.print(byteStr);
                }
                break;

              case PARSE_DEC:
                // Show as decimal
                byteStr = String(byteVal);
                Serial.print(byteVal);
                Serial.print(F(" "));
                break;

              case PARSE_MIXED:
                // Show both hex and ASCII: "0x41='A'"
                byteStr = "0x";
                if (byteVal < 16) byteStr += "0";
                byteStr += String(byteVal, HEX);
                if (isPrintable(byteVal)) {
                  byteStr += "='";
                  byteStr += (char)byteVal;
                  byteStr += "'";
                }
                Serial.print(byteStr);
                Serial.print(F(" "));
                break;

              case PARSE_NONE:
              default:
                // Default hex display
                byteStr = "0x";
                if (byteVal < 16) byteStr += "0";
                byteStr += String(byteVal, HEX);
                Serial.print(F("0x"));
                if (byteVal < 16) Serial.print(F("0"));
                Serial.print(byteVal, HEX);
                Serial.print(F(" "));
                break;
            }

            // Add to buffer for LCD display
            if (fpgaBuffer.length() < 160) {
              if (fpgaBuffer.length() > 0 && parseMode != PARSE_ASCII) {
                fpgaBuffer += " ";
              }
              fpgaBuffer += byteStr;
            } else {
              // Buffer full, display and start new line
              showTextBottom(fpgaBuffer.c_str());
              fpgaBuffer = byteStr;
            }

            // Display every 10 bytes or on pause
            static uint8_t byteCount = 0;
            byteCount++;
            if (byteCount >= 10) {
              if (fpgaBuffer.length() > 0) {
                showTextBottom(fpgaBuffer.c_str());
                fpgaBuffer = "";
              }
              byteCount = 0;
            }

          } else {
            // Text mode: parse as characters
            char c = (char)byteVal;
            Serial.write(c);  // Echo to PC

            if (c == '\n' || c == '\r') {
              if (fpgaBuffer.length() > 0) {
                showTextBottom(fpgaBuffer.c_str());
                fpgaBuffer = "";
              }
            } else if (isPrintable(c)) {
              if (fpgaBuffer.length() < 180) {
                fpgaBuffer += c;
              } else {
                showTextBottom(fpgaBuffer.c_str());
                fpgaBuffer = "";
                fpgaBuffer += c;
              }
            }
          }
        }
        continue;
      }
    }

    if (rxMode == FPGA_RX_BINARY) {
      // Binary mode: display based on parse mode
      String byteStr = "";

      switch (parseMode) {
        case PARSE_ASCII:
          // Show as ASCII with [XX] for non-printable
          if (isPrintable(byteVal)) {
            byteStr = String((char)byteVal);
            Serial.write(byteVal);
          } else {
            byteStr = "[";
            if (byteVal < 16) byteStr += "0";
            byteStr += String(byteVal, HEX);
            byteStr += "]";
            Serial.print(byteStr);
          }
          break;

        case PARSE_DEC:
          // Show as decimal
          byteStr = String(byteVal);
          Serial.print(byteVal);
          Serial.print(F(" "));
          break;

        case PARSE_MIXED:
          // Show both hex and ASCII: "0x41='A'"
          byteStr = "0x";
          if (byteVal < 16) byteStr += "0";
          byteStr += String(byteVal, HEX);
          if (isPrintable(byteVal)) {
            byteStr += "='";
            byteStr += (char)byteVal;
            byteStr += "'";
          }
          Serial.print(byteStr);
          Serial.print(F(" "));
          break;

        case PARSE_NONE:
        default:
          // Default hex display
          byteStr = "0x";
          if (byteVal < 16) byteStr += "0";
          byteStr += String(byteVal, HEX);
          Serial.print(F("0x"));
          if (byteVal < 16) Serial.print(F("0"));
          Serial.print(byteVal, HEX);
          Serial.print(F(" "));
          break;
      }

      // Add to buffer for LCD display
      if (fpgaBuffer.length() < 160) {
        if (fpgaBuffer.length() > 0 && parseMode != PARSE_ASCII) {
          fpgaBuffer += " ";
        }
        fpgaBuffer += byteStr;
      } else {
        // Buffer full, display and start new line
        showTextBottom(fpgaBuffer.c_str());
        fpgaBuffer = byteStr;
      }

      // Display every 10 bytes or on pause
      static uint8_t byteCount = 0;
      byteCount++;
      if (byteCount >= 10) {
        if (fpgaBuffer.length() > 0) {
          showTextBottom(fpgaBuffer.c_str());
          fpgaBuffer = "";
        }
        byteCount = 0;
      }

    } else {
      // Text mode: parse as characters
      char c = (char)byteVal;
      Serial.write(c);  // Echo to PC

      if (c == '\n' || c == '\r') {
        if (fpgaBuffer.length() > 0) {
          showTextBottom(fpgaBuffer.c_str());
          fpgaBuffer = "";
        }
      } else if (isPrintable(c)) {
        if (fpgaBuffer.length() < 180) {
          fpgaBuffer += c;
        } else {
          showTextBottom(fpgaBuffer.c_str());
          fpgaBuffer = "";
          fpgaBuffer += c;
        }
      }
    }
  }
}

bool parseHexBytesToBuffer(String hexStr, uint8_t* outBuf, uint8_t maxLen, uint8_t* outLen) {
  if (!outBuf || !outLen) return false;
  uint8_t byteCount = 0;
  int startIdx = 0;
  while (startIdx < hexStr.length() && byteCount < maxLen) {
    while (startIdx < hexStr.length() && hexStr[startIdx] == ' ') startIdx++;
    if (startIdx >= hexStr.length()) break;
    int endIdx = startIdx;
    while (endIdx < hexStr.length() && hexStr[endIdx] != ' ') endIdx++;
    String token = hexStr.substring(startIdx, endIdx);
    token.trim();
    if (token.startsWith("0x") || token.startsWith("0X")) token = token.substring(2);
    if (token.length() > 0) {
      long value = strtol(token.c_str(), NULL, 16);
      if (value < 0 || value > 255) return false;
      outBuf[byteCount++] = (uint8_t)value;
    }
    startIdx = endIdx;
  }
  *outLen = byteCount;
  return byteCount > 0;
}

int sendHexBytes(String hexStr) {
  uint8_t buffer[FRAME_MAX_PAYLOAD];
  uint8_t byteCount = 0;
  if (!parseHexBytesToBuffer(hexStr, buffer, FRAME_MAX_PAYLOAD, &byteCount)) return 0;
  HardwareSerial& fpga = getFPGA();
  sendBytesToFPGA(fpga, activeFpga, buffer, byteCount, true);
  return byteCount;
}

void handleAddrSend(String hexData) {
  uint8_t payload[FRAME_MAX_PAYLOAD];
  uint8_t count = 0;
  if (!parseHexBytesToBuffer(hexData, payload, FRAME_MAX_PAYLOAD, &count)) {
    Serial.println(F("ERR:HEX"));
    return;
  }
  if (count < 1) {
    Serial.println(F("ERR:NO_BYTES"));
    return;
  }

  HardwareSerial& fpga = getFPGA();
  uint8_t target = payload[0];
  if (protoAddrMode && target == localAddress) {
    String localCmd = "";
    for (uint8_t i = 1; i < count; i++) localCmd += (char)payload[i];
    localCmd.trim();
    addrLocalHandled++;
    if (localCmd.length() > 0) {
      processCmd(localCmd);
      Serial.println(F("OK:ADDR_LOCAL"));
    } else {
      Serial.println(F("OK:ADDR_LOCAL_EMPTY"));
    }
    return;
  }

  sendBytesToFPGA(fpga, activeFpga, payload, count, false);
  addrPassthrough++;
  Serial.println(F("OK:ADDR_PASS"));
}
