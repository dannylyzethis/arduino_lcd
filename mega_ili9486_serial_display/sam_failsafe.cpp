#include "sam_failsafe.h"
#include "sam_globals.h"
#include "sam_display.h"
#include "sam_logging.h"
#include "sam_macros.h"
#include "sam_commands.h"
#include <avr/wdt.h>

uint8_t pickWdtCfgForMs(uint16_t ms, uint16_t& appliedMs) {
  if (ms <= 15)   { appliedMs = 15;   return WDTO_15MS; }
  if (ms <= 30)   { appliedMs = 30;   return WDTO_30MS; }
  if (ms <= 60)   { appliedMs = 60;   return WDTO_60MS; }
  if (ms <= 120)  { appliedMs = 120;  return WDTO_120MS; }
  if (ms <= 250)  { appliedMs = 250;  return WDTO_250MS; }
  if (ms <= 500)  { appliedMs = 500;  return WDTO_500MS; }
  if (ms <= 1000) { appliedMs = 1000; return WDTO_1S; }
  if (ms <= 2000) { appliedMs = 2000; return WDTO_2S; }
  if (ms <= 4000) { appliedMs = 4000; return WDTO_4S; }
  appliedMs = 8000;
  return WDTO_8S;
}

void setWatchdog(bool enable, uint16_t requestedMs) {
  if (!enable) {
    wdt_disable();
    wdtEnabled = false;
    return;
  }
  uint16_t applied = 1000;
  uint8_t cfg = pickWdtCfgForMs(requestedMs, applied);
  wdt_enable(cfg);
  wdt_reset();
  wdtEnabled = true;
  wdtTimeoutMs = applied;
}

void printResetCause() {
  if (bootResetCause == 0) {
    Serial.println(F("UNKNOWN"));
    return;
  }
  bool any = false;
  if (bootResetCause & _BV(PORF)) { Serial.print(F("POR ")); any = true; }
  if (bootResetCause & _BV(EXTRF)) { Serial.print(F("EXT ")); any = true; }
  if (bootResetCause & _BV(BORF)) { Serial.print(F("BROWNOUT ")); any = true; }
  if (bootResetCause & _BV(WDRF)) { Serial.print(F("WDT ")); any = true; }
  if (!any) Serial.print(F("OTHER"));
  Serial.println();
}

const __FlashStringHelper* profileName(uint8_t p) {
  switch (p) {
    case PROFILE_SAFE: return F("SAFE");
    case PROFILE_BALANCED: return F("BALANCED");
    case PROFILE_PERF: return F("PERF");
    default: return F("CUSTOM");
  }
}

unsigned long getTotalLinkErrors() {
  unsigned long total = 0;
  for (uint8_t id = 1; id <= 3; id++) {
    total += fpgaFrameErr[id];
    total += fpgaDropped[id];
  }
  return total;
}

unsigned long getTotalTimeouts() {
  unsigned long total = 0;
  for (uint8_t id = 1; id <= 3; id++) total += fpgaTimeouts[id];
  return total;
}

uint8_t calcLinkQuality(uint8_t id) {
  if (id < 1 || id > 3) return 0;
  unsigned long now = millis();
  unsigned long age = now - fpgaLastActivityMs[id];
  unsigned long rx = fpgaRxBytes[id];
  unsigned long err = fpgaFrameErr[id] + fpgaDropped[id] + fpgaTimeouts[id];
  int16_t score = 100;

  if (age > 8000) score -= 55;
  else if (age > 3000) score -= 30;
  else if (age > 1200) score -= 12;

  if (rx == 0 && age > 3000) score -= 15;

  if (rx > 0) {
    uint8_t errPct = (uint8_t)min(100UL, (err * 100UL) / rx);
    score -= (int16_t)min((uint8_t)60, (uint8_t)(errPct * 2));
  } else {
    score -= (int16_t)min(35UL, err * 5UL);
  }

  if (score < 0) score = 0;
  if (score > 100) score = 100;
  return (uint8_t)score;
}

const __FlashStringHelper* linkQualityLabel(uint8_t score) {
  if (score >= 90) return F("EXCELLENT");
  if (score >= 75) return F("GOOD");
  if (score >= 55) return F("FAIR");
  if (score >= 35) return F("POOR");
  return F("BAD");
}

void printLinkSummary() {
  Serial.println(F("=== LINK QUALITY ==="));
  Serial.print(F("Indicators: "));
  Serial.println(linkIndicatorsEnabled ? F("ON") : F("OFF"));
  for (uint8_t id = 1; id <= 3; id++) {
    uint8_t score = calcLinkQuality(id);
    Serial.print(F("F"));
    Serial.print(id);
    Serial.print(F(": Q="));
    Serial.print(score);
    Serial.print(F("% "));
    Serial.print(linkQualityLabel(score));
    Serial.print(F(" RX="));
    Serial.print(fpgaRxBytes[id]);
    Serial.print(F(" ERR="));
    Serial.print(fpgaFrameErr[id] + fpgaDropped[id]);
    Serial.print(F(" TO="));
    Serial.println(fpgaTimeouts[id]);
  }
}

bool parseEscAction(String actionRaw, RuleActionType& action, uint8_t& actionArg) {
  String actionUp = actionRaw;
  actionUp.trim();
  actionUp.toUpperCase();
  if (actionUp.startsWith("MACRO ")) {
    int id = actionUp.substring(6).toInt();
    if (id < 0 || id >= MAX_MACROS) return false;
    action = RULE_ACT_MACRO;
    actionArg = (uint8_t)id;
    return true;
  }
  if (actionUp == "LOG") {
    action = RULE_ACT_LOG;
    actionArg = 0;
    return true;
  }
  if (actionUp == "ALERT") {
    action = RULE_ACT_ALERT;
    actionArg = 0;
    return true;
  }
  return false;
}

void fireEscalation(unsigned long dErr, unsigned long dTo, unsigned long dRule) {
  escalation.fireCount++;
  escalation.lastFireMs = millis();

  char msg[EVENT_LOG_MSG_LEN];
  snprintf(msg, sizeof(msg), "ESC dE=%lu dT=%lu dR=%lu", dErr, dTo, dRule);
  addEventLogC(EVLOG_ERR, msg);
  setWhy(msg);

  if (escalation.action == RULE_ACT_MACRO) {
    runMacroSafe(escalation.actionArg, F("ESC"));
    Serial.print(F("ESCALATE:MACRO "));
    Serial.println(escalation.actionArg);
  } else if (escalation.action == RULE_ACT_LOG) {
    Serial.print(F("ESCALATE: "));
    Serial.println(msg);
  } else {
    String lcdMsg = "ESCALATE ";
    lcdMsg += msg;
    showTextBottom(lcdMsg);
    Serial.print(F("ESCALATE:ALERT "));
    Serial.println(msg);
  }
}

void evaluateEscalation() {
  if (!escalation.enabled) return;
  unsigned long now = millis();
  if (now - escalation.lastEvalMs < escalation.windowMs) return;

  unsigned long totalErr = getTotalLinkErrors();
  unsigned long totalTo = getTotalTimeouts();
  unsigned long totalRule = ruleFireCount;
  unsigned long dErr = totalErr - escalation.lastErrTotal;
  unsigned long dTo = totalTo - escalation.lastTimeoutTotal;
  unsigned long dRule = totalRule - escalation.lastRuleTotal;
  bool hit = false;

  if ((escalation.metricMask & ESC_METRIC_ERR) && dErr >= escalation.threshold) hit = true;
  if ((escalation.metricMask & ESC_METRIC_TIMEOUT) && dTo >= escalation.threshold) hit = true;
  if ((escalation.metricMask & ESC_METRIC_RULE) && dRule >= escalation.threshold) hit = true;

  if (hit && (now - escalation.lastFireMs >= escalation.cooldownMs)) {
    fireEscalation(dErr, dTo, dRule);
  }

  escalation.lastErrTotal = totalErr;
  escalation.lastTimeoutTotal = totalTo;
  escalation.lastRuleTotal = totalRule;
  escalation.lastEvalMs = now;
}

void printEscalationStatus() {
  Serial.print(F("ESC="));
  Serial.println(escalation.enabled ? F("ON") : F("OFF"));
  Serial.print(F("ESC_METRICS="));
  if (escalation.metricMask & ESC_METRIC_ERR) Serial.print(F("ERR "));
  if (escalation.metricMask & ESC_METRIC_TIMEOUT) Serial.print(F("TIMEOUT "));
  if (escalation.metricMask & ESC_METRIC_RULE) Serial.print(F("RULE "));
  Serial.println();
  Serial.print(F("ESC_THRESHOLD=")); Serial.println(escalation.threshold);
  Serial.print(F("ESC_WINDOW_MS=")); Serial.println(escalation.windowMs);
  Serial.print(F("ESC_COOLDOWN_MS=")); Serial.println(escalation.cooldownMs);
  Serial.print(F("ESC_ACTION="));
  if (escalation.action == RULE_ACT_MACRO) {
    Serial.print(F("MACRO "));
    Serial.println(escalation.actionArg);
  } else if (escalation.action == RULE_ACT_LOG) {
    Serial.println(F("LOG"));
  } else {
    Serial.println(F("ALERT"));
  }
  Serial.print(F("ESC_FIRES=")); Serial.println(escalation.fireCount);
}

void printFailsafeStatus() {
  Serial.print(F("SAFE="));
  Serial.println(failsafe.enabled ? F("ON") : F("OFF"));
  Serial.print(F("SAFE_ACTIVE="));
  Serial.println(failsafe.active ? F("YES") : F("NO"));
  Serial.print(F("SAFE_THRESHOLD=")); Serial.println(failsafe.threshold);
  Serial.print(F("SAFE_WINDOW_MS=")); Serial.println(failsafe.windowMs);
  Serial.print(F("SAFE_HOLD_MS=")); Serial.println(failsafe.holdMs);
  Serial.print(F("SAFE_FIRES=")); Serial.println(failsafe.fireCount);
}

void exitFailsafe() {
  if (!failsafe.active) return;
  bridgeMode = failsafe.prevBridge;
  watchEnabled = failsafe.prevWatch;
  linkIndicatorsEnabled = failsafe.prevLinks;
  rulesEnabled = failsafe.prevRules;
  failsafe.active = false;
  failsafe.lastErrTotal = getTotalLinkErrors();
  failsafe.lastTimeoutTotal = getTotalTimeouts();
  failsafe.lastEscTotal = escalation.fireCount;
  failsafe.lastEvalMs = millis();
  addEventLogC(EVLOG_INFO, "SAFE EXIT");
  setWhy("SAFE EXIT");
  Serial.println(F("SAFE:EXIT"));
}

void enterFailsafe(unsigned long dErr, unsigned long dTo, unsigned long dEsc) {
  failsafe.active = true;
  failsafe.enterMs = millis();
  failsafe.fireCount++;
  failsafe.prevBridge = bridgeMode;
  failsafe.prevWatch = watchEnabled;
  failsafe.prevLinks = linkIndicatorsEnabled;
  failsafe.prevRules = rulesEnabled;

  bridgeMode = false;
  watchEnabled = false;
  linkIndicatorsEnabled = false;
  rulesEnabled = false;

  char em[EVENT_LOG_MSG_LEN];
  snprintf(em, sizeof(em), "SAFE e%lu t%lu s%lu", dErr, dTo, dEsc);
  addEventLogC(EVLOG_ERR, em);
  setWhy(em);
  showTextBottom("SAFE MODE ACTIVE");
  Serial.print(F("SAFE:ENTER "));
  Serial.println(em);
}

void evaluateFailsafe() {
  if (!failsafe.enabled) return;
  unsigned long now = millis();

  if (failsafe.active) {
    if (now - failsafe.enterMs >= failsafe.holdMs) {
      exitFailsafe();
    } else {
      return;
    }
  }

  if (now - failsafe.lastEvalMs < failsafe.windowMs) return;

  unsigned long totalErr = getTotalLinkErrors();
  unsigned long totalTo = getTotalTimeouts();
  unsigned long totalEsc = escalation.fireCount;
  unsigned long dErr = totalErr - failsafe.lastErrTotal;
  unsigned long dTo = totalTo - failsafe.lastTimeoutTotal;
  unsigned long dEsc = totalEsc - failsafe.lastEscTotal;
  unsigned long score = dErr + dTo + dEsc;

  if (score >= failsafe.threshold) {
    enterFailsafe(dErr, dTo, dEsc);
  }

  failsafe.lastErrTotal = totalErr;
  failsafe.lastTimeoutTotal = totalTo;
  failsafe.lastEscTotal = totalEsc;
  failsafe.lastEvalMs = now;
}

void applySmartProfile(uint8_t profile) {
  smartProfile = profile;
  if (profile == PROFILE_SAFE) {
    protoFrameMode = true;
    bridgeMode = false;
    watchEnabled = true;
    watchSource = "A8";
    watchIntervalMs = 1000;
    linkIndicatorsEnabled = true;
    rulesEnabled = true;
    escalation.enabled = true;
    escalation.metricMask = (uint8_t)(ESC_METRIC_ERR | ESC_METRIC_TIMEOUT);
    escalation.threshold = 2;
    escalation.windowMs = 2000;
    escalation.cooldownMs = 5000;
    escalation.action = RULE_ACT_ALERT;
    escalation.actionArg = 0;
    escalation.lastErrTotal = getTotalLinkErrors();
    escalation.lastTimeoutTotal = getTotalTimeouts();
    escalation.lastRuleTotal = ruleFireCount;
    escalation.lastEvalMs = millis();
  } else if (profile == PROFILE_BALANCED) {
    protoFrameMode = false;
    bridgeMode = false;
    watchEnabled = true;
    watchSource = "GPIO";
    watchIntervalMs = 1500;
    linkIndicatorsEnabled = true;
    rulesEnabled = true;
    escalation.enabled = true;
    escalation.metricMask = (uint8_t)(ESC_METRIC_ERR | ESC_METRIC_TIMEOUT | ESC_METRIC_RULE);
    escalation.threshold = 3;
    escalation.windowMs = 3000;
    escalation.cooldownMs = 6000;
    escalation.action = RULE_ACT_LOG;
    escalation.actionArg = 0;
    escalation.lastErrTotal = getTotalLinkErrors();
    escalation.lastTimeoutTotal = getTotalTimeouts();
    escalation.lastRuleTotal = ruleFireCount;
    escalation.lastEvalMs = millis();
  } else if (profile == PROFILE_PERF) {
    protoFrameMode = false;
    bridgeMode = false;
    watchEnabled = false;
    linkIndicatorsEnabled = false;
    rulesEnabled = true;
    escalation.enabled = false;
  }
}

void printSmartSummary() {
  Serial.println(F("=== SMART SUMMARY ==="));
  Serial.print(F("Profile: "));
  Serial.println(profileName(smartProfile));
  Serial.print(F("Uptime(ms): "));
  Serial.println(millis());
  Serial.print(F("FreeRAM: "));
  int freeRam = getFreeRAM();
  Serial.println(freeRam);
  Serial.print(F("WDT: "));
  if (wdtEnabled) {
    Serial.print(F("ON @"));
    Serial.print(wdtTimeoutMs);
    Serial.println(F("ms"));
  } else {
    Serial.println(F("OFF"));
  }
  printLinkSummary();
  Serial.print(F("Rule Fires: ")); Serial.println(ruleFireCount);
  Serial.print(F("GPIO Events: ")); Serial.println(gpioEventCount);
  Serial.print(F("Threshold Events: ")); Serial.println(thresholdEventCount);
  Serial.print(F("Event Log Count: ")); Serial.println(eventLogCount);
  printEscalationStatus();
  printFailsafeStatus();

  int16_t score = 100;
  uint8_t q1 = calcLinkQuality(1);
  uint8_t q2 = calcLinkQuality(2);
  uint8_t q3 = calcLinkQuality(3);
  uint8_t avgQ = (uint8_t)((q1 + q2 + q3) / 3);
  if (avgQ < 60) score -= 20;
  else if (avgQ < 80) score -= 10;
  if (freeRam < 1500) score -= 20;
  else if (freeRam < 2200) score -= 10;
  if (getTotalLinkErrors() > 0) score -= min(20UL, getTotalLinkErrors());
  if (!rulesEnabled) score -= 5;
  if (score < 0) score = 0;
  Serial.print(F("SMART_SCORE="));
  Serial.println(score);
  Serial.println(F("====================="));
}
