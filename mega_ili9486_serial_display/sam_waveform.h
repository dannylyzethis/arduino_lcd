#ifndef SAM_WAVEFORM_H
#define SAM_WAVEFORM_H

#include "sam_config.h"

void pulseISR();
void updateWaveform();
void handleWaveGen(String params);
void handleWaveStop();
void handlePWMSet(String params);
void handlePWMStop(String params);
void handlePWMFreq(String params);
void updateFreqMonitor();
void handleFreqMon(String params);
void handleFreqStop();
void handlePulseIn(String params);
void handleFreqCount(String params);
bool isPwmCapablePin(uint8_t pin);
bool isPinBusyByPWM(uint8_t pin);

#endif // SAM_WAVEFORM_H
