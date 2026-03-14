#ifndef SAM_COMMANDS_H
#define SAM_COMMANDS_H

#include "sam_config.h"

void processCmd(String c);
long parseHexOrDec(String str);
int getFreeRAM();
void updateWatch();
void handleMemInfo();
void handleUptime();
void handleBenchmark();
void help();

#endif // SAM_COMMANDS_H
