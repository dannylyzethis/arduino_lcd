// MenuConfig.cpp - Menu Structure Implementation and FPGA Handlers

#include "MenuConfig.h"

// External references to global variables from main sketch
extern MCUFRIEND_kbv tft;
extern HardwareSerial& getFPGA();
extern void applyTermination(HardwareSerial& fpga);
extern uint16_t screenW, screenH;

// FPGA Command byte sequences
// Control Registers 0-7 (0x01 0x00 through 0x01 0x07)
uint8_t CMD_CONTROL_REG[8][2] = {
  {0x01, 0x00},  // Control Register 0
  {0x01, 0x01},  // Control Register 1
  {0x01, 0x02},  // Control Register 2
  {0x01, 0x03},  // Control Register 3
  {0x01, 0x04},  // Control Register 4
  {0x01, 0x05},  // Control Register 5
  {0x01, 0x06},  // Control Register 6
  {0x01, 0x07}   // Control Register 7
};

// Status Registers 0-7 (0x02 0x00 through 0x02 0x07)
uint8_t CMD_STATUS_REG[8][2] = {
  {0x02, 0x00},  // Status Register 0
  {0x02, 0x01},  // Status Register 1
  {0x02, 0x02},  // Status Register 2
  {0x02, 0x03},  // Status Register 3
  {0x02, 0x04},  // Status Register 4
  {0x02, 0x05},  // Status Register 5
  {0x02, 0x06},  // Status Register 6
  {0x02, 0x07}   // Status Register 7
};

// Temperature commands
uint8_t CMD_TEMP_READ[] = {0x03, 0x00};
uint8_t CMD_TEMP_ALL[] = {0x03, 0xFF};

// Firmware Info commands
uint8_t CMD_FW_VERSION[] = {0x10, 0x00};
uint8_t CMD_FW_BUILD[] = {0x10, 0x01};
uint8_t CMD_FW_ID[] = {0x10, 0x02};

// ========== Helper Functions ==========

bool readFPGA64BitRegister(uint8_t* cmdBytes, uint8_t cmdLen, Register64* result) {
  if (!result) return false;

  HardwareSerial& fpga = getFPGA();

  // Send command
  for (uint8_t i = 0; i < cmdLen; i++) {
    fpga.write(cmdBytes[i]);
  }
  applyTermination(fpga);

  // Wait for response (8 bytes for 64-bit register)
  delay(50);

  uint8_t bytesReceived = 0;
  unsigned long startTime = millis();

  while (bytesReceived < 8 && (millis() - startTime) < 1000) {
    if (fpga.available()) {
      result->bytes[bytesReceived++] = fpga.read();
    }
  }

  if (bytesReceived == 8) {
    Serial.print(F("Received 64-bit register: 0x"));
    for (int i = 7; i >= 0; i--) {
      if (result->bytes[i] < 0x10) Serial.print(F("0"));
      Serial.print(result->bytes[i], HEX);
    }
    Serial.println();
    return true;
  } else {
    Serial.print(F("ERR: Only received "));
    Serial.print(bytesReceived);
    Serial.println(F(" bytes"));
    return false;
  }
}

void displayRegisterOnScreen(const char* title, Register64* reg, void (*parseFunc)(Register64*, MCUFRIEND_kbv*)) {
  // Clear screen
  tft.fillScreen(0x0000);

  // Title
  tft.setTextColor(0x07FF, 0x0000);  // Cyan
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.print(title);

  // Raw value
  tft.setTextColor(0xFFFF, 0x0000);  // White
  tft.setTextSize(1);
  tft.setCursor(5, 30);
  tft.print(F("Raw: 0x"));
  for (int i = 7; i >= 0; i--) {
    if (reg->bytes[i] < 0x10) tft.print(F("0"));
    tft.print(reg->bytes[i], HEX);
  }

  // Call custom parsing function
  if (parseFunc) {
    parseFunc(reg, &tft);
  }

  // Display "Touch to exit" message
  tft.setTextColor(0xFFE0, 0x0000);  // Yellow
  tft.setTextSize(2);
  tft.setCursor(5, screenH - 30);
  tft.print(F("Touch to exit"));
}

// ========== Control Register Interpretation Functions ==========

// Control Register 0: System Configuration
// Bits 0-3: Clock divider, Bits 4-7: Power mode, Bit 8: Enable, Bits 16-31: Custom value
void parseControlReg0(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);  // Green
  tft->setTextSize(1);

  uint8_t clockDiv = reg->bytes[0] & 0x0F;
  uint8_t powerMode = (reg->bytes[0] >> 4) & 0x0F;
  bool enabled = reg->bytes[1] & 0x01;
  uint16_t customVal = (reg->bytes[3] << 8) | reg->bytes[2];

  tft->setCursor(5, 50);
  tft->print(F("Clock Div: "));
  tft->print(clockDiv);

  tft->setCursor(5, 65);
  tft->print(F("Power Mode: "));
  tft->print(powerMode);

  tft->setCursor(5, 80);
  tft->print(F("Enabled: "));
  tft->print(enabled ? "YES" : "NO");

  tft->setCursor(5, 95);
  tft->print(F("Custom Val: "));
  tft->print(customVal);
}

// Control Register 1: Data Path Configuration
// Bits 0-7: Input mux, Bits 8-15: Output mux, Bits 16-23: Filter setting
void parseControlReg1(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);
  tft->setTextSize(1);

  uint8_t inputMux = reg->bytes[0];
  uint8_t outputMux = reg->bytes[1];
  uint8_t filter = reg->bytes[2];

  tft->setCursor(5, 50);
  tft->print(F("Input Mux: 0x"));
  if (inputMux < 0x10) tft->print(F("0"));
  tft->print(inputMux, HEX);

  tft->setCursor(5, 65);
  tft->print(F("Output Mux: 0x"));
  if (outputMux < 0x10) tft->print(F("0"));
  tft->print(outputMux, HEX);

  tft->setCursor(5, 80);
  tft->print(F("Filter: "));
  tft->print(filter);
}

// Control Register 2: Timing Configuration
// Bits 0-15: Period, Bits 16-31: Duty cycle, Bits 32-47: Phase
void parseControlReg2(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);
  tft->setTextSize(1);

  uint16_t period = (reg->bytes[1] << 8) | reg->bytes[0];
  uint16_t duty = (reg->bytes[3] << 8) | reg->bytes[2];
  uint16_t phase = (reg->bytes[5] << 8) | reg->bytes[4];

  tft->setCursor(5, 50);
  tft->print(F("Period: "));
  tft->print(period);

  tft->setCursor(5, 65);
  tft->print(F("Duty Cycle: "));
  tft->print(duty);

  tft->setCursor(5, 80);
  tft->print(F("Phase: "));
  tft->print(phase);

  tft->setCursor(5, 95);
  tft->print(F("Duty %: "));
  if (period > 0) {
    tft->print((duty * 100) / period);
    tft->print(F("%"));
  }
}

// Control Register 3: Interrupt Configuration
// Each bit represents an interrupt enable
void parseControlReg3(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);
  tft->setTextSize(1);

  tft->setCursor(5, 50);
  tft->print(F("Int Enables:"));

  const char* intNames[8] = {
    "RX", "TX", "ERR", "OVF", "TMR", "GPIO", "ADC", "DAC"
  };

  for (uint8_t i = 0; i < 8; i++) {
    if (reg->bytes[0] & (1 << i)) {
      tft->setCursor(5, 65 + i * 15);
      tft->print(F("- "));
      tft->print(intNames[i]);
    }
  }
}

// Control Register 4: DMA Configuration
// Bits 0-31: Source address, Bits 32-47: Transfer count
void parseControlReg4(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);
  tft->setTextSize(1);

  uint32_t srcAddr = reg->low;
  uint16_t count = (reg->bytes[5] << 8) | reg->bytes[4];

  tft->setCursor(5, 50);
  tft->print(F("Src Addr: 0x"));
  tft->print(srcAddr, HEX);

  tft->setCursor(5, 65);
  tft->print(F("Count: "));
  tft->print(count);
}

// Control Register 5: Calibration Values
// 4x 16-bit calibration coefficients
void parseControlReg5(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);
  tft->setTextSize(1);

  tft->setCursor(5, 50);
  tft->print(F("Calibration:"));

  for (uint8_t i = 0; i < 4; i++) {
    uint16_t cal = (reg->bytes[i*2+1] << 8) | reg->bytes[i*2];
    tft->setCursor(5, 65 + i * 15);
    tft->print(F("Cal"));
    tft->print(i);
    tft->print(F(": "));
    tft->print(cal);
  }
}

// Control Register 6: Feature Flags
// Bit field configuration
void parseControlReg6(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);
  tft->setTextSize(1);

  tft->setCursor(5, 50);
  tft->print(F("Features:"));

  const char* features[16] = {
    "ENC", "DEC", "CRC", "FEC",
    "AGC", "AFC", "AES", "RSA",
    "FIR", "IIR", "FFT", "DCT",
    "H264", "VP9", "JPEG", "PNG"
  };

  uint8_t y = 65;
  for (uint8_t i = 0; i < 16; i++) {
    if (reg->bytes[i/8] & (1 << (i%8))) {
      tft->setCursor(5 + (i/8)*80, y);
      tft->print(features[i]);
      if (i % 8 == 7) y += 15;
    }
  }
}

// Control Register 7: User Defined
// Display as 8 hex bytes
void parseControlReg7(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0x07E0, 0x0000);
  tft->setTextSize(1);

  tft->setCursor(5, 50);
  tft->print(F("User Data:"));

  for (uint8_t i = 0; i < 8; i++) {
    tft->setCursor(5, 65 + i * 15);
    tft->print(F("Byte"));
    tft->print(i);
    tft->print(F(": 0x"));
    if (reg->bytes[i] < 0x10) tft->print(F("0"));
    tft->print(reg->bytes[i], HEX);
    tft->print(F(" ("));
    tft->print(reg->bytes[i]);
    tft->print(F(")"));
  }
}

// ========== Status Register Interpretation Functions ==========

// Status Register 0: System Status
// Overall system health and state
void parseStatusReg0(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xFFE0, 0x0000);  // Yellow
  tft->setTextSize(1);

  bool ready = reg->bytes[0] & 0x01;
  bool error = reg->bytes[0] & 0x02;
  bool busy = reg->bytes[0] & 0x04;
  uint8_t state = (reg->bytes[0] >> 4) & 0x0F;

  tft->setCursor(5, 50);
  tft->print(F("Ready: "));
  tft->setTextColor(ready ? 0x07E0 : 0xF800, 0x0000);
  tft->print(ready ? "YES" : "NO");

  tft->setTextColor(0xFFE0, 0x0000);
  tft->setCursor(5, 65);
  tft->print(F("Error: "));
  tft->setTextColor(error ? 0xF800 : 0x07E0, 0x0000);
  tft->print(error ? "YES" : "NO");

  tft->setTextColor(0xFFE0, 0x0000);
  tft->setCursor(5, 80);
  tft->print(F("Busy: "));
  tft->print(busy ? "YES" : "NO");

  tft->setCursor(5, 95);
  tft->print(F("State: "));
  tft->print(state);
}

// Status Register 1: Error Flags
// Detailed error conditions
void parseStatusReg1(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xF800, 0x0000);  // Red for errors
  tft->setTextSize(1);

  tft->setCursor(5, 50);
  tft->print(F("Errors:"));

  const char* errors[16] = {
    "PARITY", "FRAME", "OVERRUN", "UNDERRUN",
    "TIMEOUT", "CRC", "ALIGN", "COLLISION",
    "BUFFER", "FIFO", "DMA", "MEMORY",
    "VOLTAGE", "CURRENT", "TEMP", "CLOCK"
  };

  uint8_t errorCount = 0;
  for (uint8_t i = 0; i < 16; i++) {
    if (reg->bytes[i/8] & (1 << (i%8))) {
      tft->setCursor(5, 65 + errorCount * 15);
      tft->print(F("- "));
      tft->print(errors[i]);
      errorCount++;
    }
  }

  if (errorCount == 0) {
    tft->setTextColor(0x07E0, 0x0000);  // Green
    tft->setCursor(5, 65);
    tft->print(F("No errors"));
  }
}

// Status Register 2: Counter Values
// Various event counters
void parseStatusReg2(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xFFE0, 0x0000);
  tft->setTextSize(1);

  uint16_t rxCount = (reg->bytes[1] << 8) | reg->bytes[0];
  uint16_t txCount = (reg->bytes[3] << 8) | reg->bytes[2];
  uint16_t errCount = (reg->bytes[5] << 8) | reg->bytes[4];

  tft->setCursor(5, 50);
  tft->print(F("RX Count: "));
  tft->print(rxCount);

  tft->setCursor(5, 65);
  tft->print(F("TX Count: "));
  tft->print(txCount);

  tft->setCursor(5, 80);
  tft->print(F("Err Count: "));
  tft->print(errCount);
}

// Status Register 3: FIFO Status
// Buffer fill levels
void parseStatusReg3(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xFFE0, 0x0000);
  tft->setTextSize(1);

  uint16_t rxFifo = (reg->bytes[1] << 8) | reg->bytes[0];
  uint16_t txFifo = (reg->bytes[3] << 8) | reg->bytes[2];
  bool rxFull = reg->bytes[4] & 0x01;
  bool txEmpty = reg->bytes[4] & 0x02;

  tft->setCursor(5, 50);
  tft->print(F("RX FIFO: "));
  tft->print(rxFifo);
  if (rxFull) {
    tft->setTextColor(0xF800, 0x0000);
    tft->print(F(" FULL"));
  }

  tft->setTextColor(0xFFE0, 0x0000);
  tft->setCursor(5, 65);
  tft->print(F("TX FIFO: "));
  tft->print(txFifo);
  if (txEmpty) {
    tft->setTextColor(0x07E0, 0x0000);
    tft->print(F(" EMPTY"));
  }
}

// Status Register 4: Performance Metrics
// Timing and throughput
void parseStatusReg4(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xFFE0, 0x0000);
  tft->setTextSize(1);

  uint32_t cycles = reg->low;
  uint32_t throughput = reg->high;

  tft->setCursor(5, 50);
  tft->print(F("Cycles: "));
  tft->print(cycles);

  tft->setCursor(5, 65);
  tft->print(F("Thruput: "));
  tft->print(throughput);
  tft->print(F(" B/s"));
}

// Status Register 5: Link Status
// Communication link health
void parseStatusReg5(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xFFE0, 0x0000);
  tft->setTextSize(1);

  bool linkUp = reg->bytes[0] & 0x01;
  uint8_t signalStrength = reg->bytes[1];
  uint16_t latency = (reg->bytes[3] << 8) | reg->bytes[2];

  tft->setCursor(5, 50);
  tft->print(F("Link: "));
  tft->setTextColor(linkUp ? 0x07E0 : 0xF800, 0x0000);
  tft->print(linkUp ? "UP" : "DOWN");

  tft->setTextColor(0xFFE0, 0x0000);
  tft->setCursor(5, 65);
  tft->print(F("Signal: "));
  tft->print(signalStrength);
  tft->print(F("%"));

  tft->setCursor(5, 80);
  tft->print(F("Latency: "));
  tft->print(latency);
  tft->print(F(" us"));
}

// Status Register 6: Sensor Values
// Environmental sensors
void parseStatusReg6(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xFFE0, 0x0000);
  tft->setTextSize(1);

  uint16_t temp = (reg->bytes[1] << 8) | reg->bytes[0];
  uint16_t voltage = (reg->bytes[3] << 8) | reg->bytes[2];
  uint16_t current = (reg->bytes[5] << 8) | reg->bytes[4];

  tft->setCursor(5, 50);
  tft->print(F("Temp: "));
  tft->print(temp / 10);
  tft->print(F("."));
  tft->print(temp % 10);
  tft->print(F(" C"));

  tft->setCursor(5, 65);
  tft->print(F("Voltage: "));
  tft->print(voltage / 1000);
  tft->print(F("."));
  tft->print(voltage % 1000);
  tft->print(F(" V"));

  tft->setCursor(5, 80);
  tft->print(F("Current: "));
  tft->print(current);
  tft->print(F(" mA"));
}

// Status Register 7: Version Info
// Firmware and hardware versions
void parseStatusReg7(Register64* reg, MCUFRIEND_kbv* tft) {
  tft->setTextColor(0xFFE0, 0x0000);
  tft->setTextSize(1);

  uint16_t fwMajor = reg->bytes[0];
  uint16_t fwMinor = reg->bytes[1];
  uint16_t hwRev = reg->bytes[2];
  uint32_t buildNum = reg->high;

  tft->setCursor(5, 50);
  tft->print(F("FW Ver: "));
  tft->print(fwMajor);
  tft->print(F("."));
  tft->print(fwMinor);

  tft->setCursor(5, 65);
  tft->print(F("HW Rev: "));
  tft->print(hwRev);

  tft->setCursor(5, 80);
  tft->print(F("Build: "));
  tft->print(buildNum);
}

// ========== Menu Action Callbacks ==========

void onControlReg(MenuItem* item) {
  if (!item) return;

  uint8_t regIdx = item->regIndex;
  if (regIdx > 7) return;

  Serial.print(F("=== Control Register "));
  Serial.print(regIdx);
  Serial.println(F(" ==="));

  Register64 reg;
  if (readFPGA64BitRegister(CMD_CONTROL_REG[regIdx], 2, &reg)) {
    // Display on screen with custom parsing
    void (*parseFunc)(Register64*, MCUFRIEND_kbv*) = NULL;

    switch (regIdx) {
      case 0: parseFunc = parseControlReg0; break;
      case 1: parseFunc = parseControlReg1; break;
      case 2: parseFunc = parseControlReg2; break;
      case 3: parseFunc = parseControlReg3; break;
      case 4: parseFunc = parseControlReg4; break;
      case 5: parseFunc = parseControlReg5; break;
      case 6: parseFunc = parseControlReg6; break;
      case 7: parseFunc = parseControlReg7; break;
    }

    char title[32];
    sprintf(title, "Control Reg %d", regIdx);
    displayRegisterOnScreen(title, &reg, parseFunc);

    // Wait for touch to return
    delay(2000);
  }

  Serial.println(F("========================"));
}

void onStatusReg(MenuItem* item) {
  if (!item) return;

  uint8_t regIdx = item->regIndex;
  if (regIdx > 7) return;

  Serial.print(F("=== Status Register "));
  Serial.print(regIdx);
  Serial.println(F(" ==="));

  Register64 reg;
  if (readFPGA64BitRegister(CMD_STATUS_REG[regIdx], 2, &reg)) {
    // Display on screen with custom parsing
    void (*parseFunc)(Register64*, MCUFRIEND_kbv*) = NULL;

    switch (regIdx) {
      case 0: parseFunc = parseStatusReg0; break;
      case 1: parseFunc = parseStatusReg1; break;
      case 2: parseFunc = parseStatusReg2; break;
      case 3: parseFunc = parseStatusReg3; break;
      case 4: parseFunc = parseStatusReg4; break;
      case 5: parseFunc = parseStatusReg5; break;
      case 6: parseFunc = parseStatusReg6; break;
      case 7: parseFunc = parseStatusReg7; break;
    }

    char title[32];
    sprintf(title, "Status Reg %d", regIdx);
    displayRegisterOnScreen(title, &reg, parseFunc);

    // Wait for touch to return
    delay(2000);
  }

  Serial.println(F("======================="));
}

void onTempRead(MenuItem* item) {
  Serial.println(F("=== Temperature Read ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_TEMP_READ); i++) {
    fpga.write(CMD_TEMP_READ[i]);
  }
  applyTermination(fpga);

  delay(50);
  // Read 2 bytes temperature value
  uint8_t tempBytes[2];
  if (fpga.available() >= 2) {
    tempBytes[0] = fpga.read();
    tempBytes[1] = fpga.read();
    uint16_t temp = (tempBytes[1] << 8) | tempBytes[0];

    Serial.print(F("Temperature: "));
    Serial.print(temp / 10);
    Serial.print(F("."));
    Serial.print(temp % 10);
    Serial.println(F(" C"));
  }

  Serial.println(F("======================="));
}

void onTempReadAll(MenuItem* item) {
  Serial.println(F("=== All Temperatures ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_TEMP_ALL); i++) {
    fpga.write(CMD_TEMP_ALL[i]);
  }
  applyTermination(fpga);

  delay(100);
  // Read multiple temperature sensors (expecting 16 bytes = 8 sensors)
  Serial.println(F("Reading 8 sensors..."));
  for (uint8_t i = 0; i < 8; i++) {
    if (fpga.available() >= 2) {
      uint8_t low = fpga.read();
      uint8_t high = fpga.read();
      uint16_t temp = (high << 8) | low;

      Serial.print(F("Sensor "));
      Serial.print(i);
      Serial.print(F(": "));
      Serial.print(temp / 10);
      Serial.print(F("."));
      Serial.print(temp % 10);
      Serial.println(F(" C"));
    }
  }

  Serial.println(F("======================="));
}

void onFWVersion(MenuItem* item) {
  Serial.println(F("=== Firmware Version ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_FW_VERSION); i++) {
    fpga.write(CMD_FW_VERSION[i]);
  }
  applyTermination(fpga);

  delay(50);
  if (fpga.available() >= 4) {
    uint8_t major = fpga.read();
    uint8_t minor = fpga.read();
    uint8_t patch = fpga.read();
    uint8_t build = fpga.read();

    Serial.print(F("Version: "));
    Serial.print(major);
    Serial.print(F("."));
    Serial.print(minor);
    Serial.print(F("."));
    Serial.print(patch);
    Serial.print(F(" (build "));
    Serial.print(build);
    Serial.println(F(")"));
  }

  Serial.println(F("======================="));
}

void onFWBuild(MenuItem* item) {
  Serial.println(F("=== Build Number ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_FW_BUILD); i++) {
    fpga.write(CMD_FW_BUILD[i]);
  }
  applyTermination(fpga);

  delay(50);
  if (fpga.available() >= 4) {
    uint32_t buildNum = 0;
    for (uint8_t i = 0; i < 4; i++) {
      buildNum |= ((uint32_t)fpga.read() << (i * 8));
    }

    Serial.print(F("Build Number: "));
    Serial.println(buildNum);
  }

  Serial.println(F("==================="));
}

void onFWID(MenuItem* item) {
  Serial.println(F("=== Firmware ID ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_FW_ID); i++) {
    fpga.write(CMD_FW_ID[i]);
  }
  applyTermination(fpga);

  delay(50);
  if (fpga.available() >= 8) {
    Serial.print(F("FW ID: "));
    for (uint8_t i = 0; i < 8; i++) {
      uint8_t b = fpga.read();
      if (b < 0x10) Serial.print(F("0"));
      Serial.print(b, HEX);
    }
    Serial.println();
  }

  Serial.println(F("=================="));
}

void onCustomBytes(MenuItem* item) {
  Serial.println(F("=== Custom Bytes ==="));
  Serial.println(F("Use #FPGABYTES command"));
  Serial.println(F("==================="));
}

void onBackAction(MenuItem* item) {
  // Handled by menu system
}

// ========== Menu Item Definitions ==========

// Control Register Menu Items (8 registers)
MenuItem controlMenuItems[] = {
  {"Control Reg 0", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 0},
  {"Control Reg 1", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 1},
  {"Control Reg 2", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 2},
  {"Control Reg 3", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 3},
  {"Control Reg 4", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 4},
  {"Control Reg 5", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 5},
  {"Control Reg 6", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 6},
  {"Control Reg 7", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlReg, NULL, 0, true, false, 7},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false, 0}
};

// Status Register Menu Items (8 registers)
MenuItem statusMenuItems[] = {
  {"Status Reg 0", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 0},
  {"Status Reg 1", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 1},
  {"Status Reg 2", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 2},
  {"Status Reg 3", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 3},
  {"Status Reg 4", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 4},
  {"Status Reg 5", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 5},
  {"Status Reg 6", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 6},
  {"Status Reg 7", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusReg, NULL, 0, true, false, 7},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false, 0}
};

// Temperature Menu Items
MenuItem tempMenuItems[] = {
  {"Read Temperature", MENU_TYPE_ACTION, ACTION_TEMPERATURE, NULL, onTempRead, NULL, 0, true, false, 0},
  {"All Sensors", MENU_TYPE_ACTION, ACTION_TEMPERATURE, NULL, onTempReadAll, NULL, 0, true, false, 0},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false, 0}
};

// Firmware Info Menu Items
MenuItem fwMenuItems[] = {
  {"FW Version", MENU_TYPE_ACTION, ACTION_FW_INFO, NULL, onFWVersion, NULL, 0, true, false, 0},
  {"Build Number", MENU_TYPE_ACTION, ACTION_FW_INFO, NULL, onFWBuild, NULL, 0, true, false, 0},
  {"Firmware ID", MENU_TYPE_ACTION, ACTION_FW_INFO, NULL, onFWID, NULL, 0, true, false, 0},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false, 0}
};

// Main Menu Items
MenuItem mainMenuItems[] = {
  {"Control Register", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false, 0},
  {"Status Register", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false, 0},
  {"Temperature", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false, 0},
  {"Firmware Info", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false, 0},
  {"Custom Bytes", MENU_TYPE_ACTION, ACTION_CUSTOM_BYTES, NULL, onCustomBytes, NULL, 0, true, false, 0}
};

// Menu structures
Menu controlMenu;
Menu statusMenu;
Menu tempMenu;
Menu fwMenu;
Menu mainMenu;

void initMenus() {
  // Initialize submenus first
  controlMenu.title = "Control Register";
  controlMenu.items = controlMenuItems;
  controlMenu.itemCount = sizeof(controlMenuItems) / sizeof(MenuItem);
  controlMenu.parent = &mainMenu;
  controlMenu.selectedIndex = 0;
  controlMenu.scrollOffset = 0;

  statusMenu.title = "Status Register";
  statusMenu.items = statusMenuItems;
  statusMenu.itemCount = sizeof(statusMenuItems) / sizeof(MenuItem);
  statusMenu.parent = &mainMenu;
  statusMenu.selectedIndex = 0;
  statusMenu.scrollOffset = 0;

  tempMenu.title = "Temperature";
  tempMenu.items = tempMenuItems;
  tempMenu.itemCount = sizeof(tempMenuItems) / sizeof(MenuItem);
  tempMenu.parent = &mainMenu;
  tempMenu.selectedIndex = 0;
  tempMenu.scrollOffset = 0;

  fwMenu.title = "Firmware Info";
  fwMenu.items = fwMenuItems;
  fwMenu.itemCount = sizeof(fwMenuItems) / sizeof(MenuItem);
  fwMenu.parent = &mainMenu;
  fwMenu.selectedIndex = 0;
  fwMenu.scrollOffset = 0;

  // Link submenus to main menu items
  mainMenuItems[0].submenu = &controlMenu;
  mainMenuItems[1].submenu = &statusMenu;
  mainMenuItems[2].submenu = &tempMenu;
  mainMenuItems[3].submenu = &fwMenu;

  // Initialize main menu
  mainMenu.title = "FPGA Control";
  mainMenu.items = mainMenuItems;
  mainMenu.itemCount = sizeof(mainMenuItems) / sizeof(MenuItem);
  mainMenu.parent = NULL;  // Root menu
  mainMenu.selectedIndex = 0;
  mainMenu.scrollOffset = 0;

  // Set back button callbacks to goBack
  for (uint8_t i = 0; i < 4; i++) {
    controlMenuItems[8].callback = onBackAction;
    statusMenuItems[8].callback = onBackAction;
    tempMenuItems[2].callback = onBackAction;
    fwMenuItems[3].callback = onBackAction;
  }
}
