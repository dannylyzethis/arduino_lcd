#ifndef SAM_EEPROM_H
#define SAM_EEPROM_H

#include "sam_config.h"

bool isProtected(uint16_t addr);
void saveConfig();
bool loadConfig();
void handleEEPROMRead(String params);
void handleEEPROMWrite(String params);
void handleEEPROMDump(String params);
void handleEEPROMClear();
void handleEEPROMProtect(String params);
void handleConfigSave();
void handleConfigLoad();
void handleConfigReset();
void handleConfigShow();
void handleBtnConfig(String params);
void handleFPGAWrite(String params);
void handleFPGARead(String params);

#endif // SAM_EEPROM_H
