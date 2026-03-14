#include "sam_i2c.h"
#include "sam_globals.h"
#include <Wire.h>
#include "sam_commands.h"

void i2cScan() {
  Serial.println(F("Scanning I2C bus (0x00-0x7F)..."));
  uint8_t count = 0;

  for (uint8_t addr = 0; addr < 128; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("  Device found at 0x"));
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println(F("No I2C devices found"));
  } else {
    Serial.print(F("Found "));
    Serial.print(count);
    Serial.println(F(" device(s)"));
  }
}

void handleI2CRead(String params) {
  // Parse: <addr> <reg> <bytes>
  int addr, reg, numBytes;
  int sp1 = params.indexOf(' ');
  int sp2 = params.indexOf(' ', sp1 + 1);

  if (sp1 == -1 || sp2 == -1) {
    Serial.println(F("ERR:FORMAT #I2CREAD <addr> <reg> <bytes>"));
    return;
  }

  addr = parseHexOrDec(params.substring(0, sp1));
  reg = parseHexOrDec(params.substring(sp1 + 1, sp2));
  numBytes = params.substring(sp2 + 1).toInt();

  if (addr < 0 || addr > 127 || numBytes < 1 || numBytes > 32) {
    Serial.println(F("ERR:INVALID_PARAMS"));
    return;
  }

  // Write register address
  Wire.beginTransmission(addr);
  Wire.write(reg);
  uint8_t error = Wire.endTransmission(false); // Repeated start

  if (error != 0) {
    Serial.print(F("ERR:I2C_WRITE_FAIL:"));
    Serial.println(error);
    return;
  }

  // Read bytes
  Wire.requestFrom((uint8_t)addr, (uint8_t)numBytes);

  Serial.print(F("I2C[0x"));
  if (addr < 16) Serial.print("0");
  Serial.print(addr, HEX);
  Serial.print(F("] Reg 0x"));
  if (reg < 16) Serial.print("0");
  Serial.print(reg, HEX);
  Serial.print(F(": "));

  uint8_t bytesRead = 0;
  while (Wire.available() && bytesRead < numBytes) {
    uint8_t b = Wire.read();
    if (bytesRead > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (b < 16) Serial.print("0");
    Serial.print(b, HEX);
    bytesRead++;
  }
  Serial.println();
}

void handleI2CWrite(String params) {
  // Parse: <addr> <reg> <data...> or <addr> <data...>
  // First, parse all hex/dec numbers
  uint8_t values[32];
  uint8_t count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length(); i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        values[count++] = parseHexOrDec(token);
        if (count >= 32) break;
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #I2CWRITE <addr> <data...>"));
    return;
  }

  uint8_t addr = values[0];
  if (addr > 127) {
    Serial.println(F("ERR:INVALID_ADDR"));
    return;
  }

  // Write to I2C
  Wire.beginTransmission(addr);
  for (uint8_t i = 1; i < count; i++) {
    Wire.write(values[i]);
  }
  uint8_t error = Wire.endTransmission();

  if (error != 0) {
    Serial.print(F("ERR:I2C_WRITE_FAIL:"));
    Serial.println(error);
    return;
  }

  Serial.print(F("I2C[0x"));
  if (addr < 16) Serial.print("0");
  Serial.print(addr, HEX);
  Serial.print(F("] Wrote "));
  Serial.print(count - 1);
  Serial.println(F(" bytes"));
}
