#include "sam_spi.h"
#include "sam_globals.h"
#include <SPI.h>
#include "sam_commands.h"

void handleSPIBegin(String params) {
  int csPin = params.toInt();

  if (csPin < 0 || csPin > 53) {
    Serial.println(F("ERR:INVALID_PIN"));
    return;
  }

  spiCSPin = csPin;
  pinMode(spiCSPin, OUTPUT);
  digitalWrite(spiCSPin, HIGH);

  SPI.begin();
  SPI.beginTransaction(SPISettings(spiSpeed, MSBFIRST, spiMode));

  Serial.print(F("SPI initialized, CS="));
  Serial.println(spiCSPin);
}

void handleSPIEnd() {
  if (spiCSPin < 0) {
    Serial.println(F("ERR:SPI_NOT_INIT"));
    return;
  }

  SPI.endTransaction();
  SPI.end();
  spiCSPin = -1;

  Serial.println(F("SPI ended"));
}

void handleSPITransfer(String params) {
  if (spiCSPin < 0) {
    Serial.println(F("ERR:SPI_NOT_INIT (use #SPIBEGIN first)"));
    return;
  }

  // Parse bytes
  uint8_t txBytes[64];
  uint8_t rxBytes[64];
  uint8_t count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length(); i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        txBytes[count++] = parseHexOrDec(token);
        if (count >= 64) break;
      }
      lastPos = i + 1;
    }
  }

  if (count == 0) {
    Serial.println(F("ERR:NO_DATA"));
    return;
  }

  // Transfer bytes
  digitalWrite(spiCSPin, LOW);
  for (uint8_t i = 0; i < count; i++) {
    rxBytes[i] = SPI.transfer(txBytes[i]);
  }
  digitalWrite(spiCSPin, HIGH);

  // Print TX
  Serial.print(F("SPI TX: "));
  for (uint8_t i = 0; i < count; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (txBytes[i] < 16) Serial.print("0");
    Serial.print(txBytes[i], HEX);
  }
  Serial.println();

  // Print RX
  Serial.print(F("SPI RX: "));
  for (uint8_t i = 0; i < count; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (rxBytes[i] < 16) Serial.print("0");
    Serial.print(rxBytes[i], HEX);
  }
  Serial.println();
}

void handleSPISettings(String params) {
  // Parse: <speed> <mode>
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #SPISETTINGS <speed> <mode>"));
    return;
  }

  uint32_t speed = params.substring(0, sp).toInt();
  uint8_t mode = params.substring(sp + 1).toInt();

  if (speed < 100 || speed > 16000000) {
    Serial.println(F("ERR:SPEED_RANGE (100-16000000 Hz)"));
    return;
  }

  if (mode > 3) {
    Serial.println(F("ERR:MODE_RANGE (0-3)"));
    return;
  }

  spiSpeed = speed;
  spiMode = mode;

  // Update settings if SPI is active
  if (spiCSPin >= 0) {
    SPI.endTransaction();
    SPI.beginTransaction(SPISettings(spiSpeed, MSBFIRST, spiMode));
  }

  Serial.print(F("SPI settings: "));
  Serial.print(spiSpeed);
  Serial.print(F(" Hz, Mode "));
  Serial.println(spiMode);
}
