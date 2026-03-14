#ifndef SAM_FPGA_SERIAL_H
#define SAM_FPGA_SERIAL_H

#include "sam_config.h"
#include <HardwareSerial.h>

HardwareSerial& getFPGA();
void applyTermination(HardwareSerial& serial);
uint8_t crc8Simple(const uint8_t* data, uint8_t len);
void sendBytesToFPGA(HardwareSerial& fpga, uint8_t fpgaId, const uint8_t* data, uint8_t len, bool addTermination);
void readFPGAResponse(uint8_t numBytes, uint16_t timeout);
void setSerialStopBits(uint8_t serialNum, uint8_t stopBits);
void checkFPGASerial(HardwareSerial& serial, uint8_t id);
bool parseHexBytesToBuffer(String hexStr, uint8_t* outBuf, uint8_t maxLen, uint8_t* outLen);
int sendHexBytes(String hexStr);
void handleAddrSend(String hexData);

#endif // SAM_FPGA_SERIAL_H
