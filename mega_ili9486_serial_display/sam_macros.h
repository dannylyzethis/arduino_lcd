#ifndef SAM_MACROS_H
#define SAM_MACROS_H

#include "sam_config.h"

void runMacroSafe(uint8_t id, const __FlashStringHelper* sourceTag);
void handleMacroDef(String params);
void handleMacroRun(String params);
void handleMacroList();
void handleMacroClear(String params);

#endif // SAM_MACROS_H
