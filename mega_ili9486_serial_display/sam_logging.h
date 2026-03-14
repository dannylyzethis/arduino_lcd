#ifndef SAM_LOGGING_H
#define SAM_LOGGING_H

#include "sam_config.h"

const __FlashStringHelper* eventTypeName(uint8_t t);
void addEventLog(uint8_t type, const String& msg);
void addEventLogC(uint8_t type, const char* msg);
void setWhy(const char* msg);
int8_t parseEventTypeFilter(String s);
void printEventLogLast(uint8_t wanted, int8_t filterType);

void updateDataLogger();
void handleLogStart(String params);
void handleLogStop();
void handleLogRead(String params);
void handleLogClear();
void handleLogConfig(String params);
void handleLogStatus();
void handleLogZone(String params);

#endif // SAM_LOGGING_H
