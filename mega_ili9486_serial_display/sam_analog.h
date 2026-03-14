#ifndef SAM_ANALOG_H
#define SAM_ANALOG_H

#include "sam_config.h"

void handleAnalogRead(String params);
void handleAnalogReadAll();
void handleAnalogRef(String params);
void handleAnalogAvg(String params);
void handleStatsStart(String params);
void handleStatsStop(String params);
void handleStatsShow(String params);
void handleStatsReset();
void updateStats();
void handleThresholdSet(String params);
void handleThresholdClear(String params);
void handleThresholdStatus();
void checkThresholds();

#endif // SAM_ANALOG_H
