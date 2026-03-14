#ifndef SAM_FAILSAFE_H
#define SAM_FAILSAFE_H

#include "sam_config.h"

uint8_t pickWdtCfgForMs(uint16_t ms, uint16_t& appliedMs);
void setWatchdog(bool enable, uint16_t requestedMs);
void printResetCause();
const __FlashStringHelper* profileName(uint8_t p);
unsigned long getTotalLinkErrors();
unsigned long getTotalTimeouts();
uint8_t calcLinkQuality(uint8_t id);
const __FlashStringHelper* linkQualityLabel(uint8_t score);
void printLinkSummary();
bool parseEscAction(String actionRaw, RuleActionType& action, uint8_t& actionArg);
void fireEscalation(unsigned long dErr, unsigned long dTo, unsigned long dRule);
void evaluateEscalation();
void printEscalationStatus();
void printFailsafeStatus();
void exitFailsafe();
void enterFailsafe(unsigned long dErr, unsigned long dTo, unsigned long dEsc);
void evaluateFailsafe();
void applySmartProfile(uint8_t profile);
void printSmartSummary();

#endif // SAM_FAILSAFE_H
