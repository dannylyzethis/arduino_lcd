#ifndef SAM_I2C_H
#define SAM_I2C_H

#include "sam_config.h"

void i2cScan();
void handleI2CRead(String params);
void handleI2CWrite(String params);

#endif // SAM_I2C_H
