#ifndef SAM_SPI_H
#define SAM_SPI_H

#include "sam_config.h"

void handleSPIBegin(String params);
void handleSPIEnd();
void handleSPITransfer(String params);
void handleSPISettings(String params);

#endif // SAM_SPI_H
