#include "sam_rules.h"
#include "sam_globals.h"
#include "sam_display.h"
#include "sam_logging.h"
#include "sam_macros.h"

void ruleOpToText(RuleOp op, String& out) {
  switch (op) {
    case RULE_OP_GT: out = ">"; break;
    case RULE_OP_LT: out = "<"; break;
    case RULE_OP_GE: out = ">="; break;
    case RULE_OP_LE: out = "<="; break;
    case RULE_OP_EQ: out = "=="; break;
    case RULE_OP_NE: out = "!="; break;
  }
}

bool ruleCompare(int16_t lhs, RuleOp op, int16_t rhs) {
  switch (op) {
    case RULE_OP_GT: return lhs > rhs;
    case RULE_OP_LT: return lhs < rhs;
    case RULE_OP_GE: return lhs >= rhs;
    case RULE_OP_LE: return lhs <= rhs;
    case RULE_OP_EQ: return lhs == rhs;
    case RULE_OP_NE: return lhs != rhs;
  }
  return false;
}

int16_t readRuleSourceValue(const SmartRule& r) {
  if (r.srcType == RULE_SRC_ANALOG) {
    if (r.srcIndex < 8) return (int16_t)analogRead(analogPins[r.srcIndex]);
    return 0;
  }
  if (r.srcType == RULE_SRC_GPIO) {
    if (r.srcIndex < NUM_GPIO) return (int16_t)digitalRead(gpioPins[r.srcIndex]);
    return 0;
  }
  return 0;
}

void triggerRuleAction(uint8_t ruleId, SmartRule& r, int16_t valueNow) {
  ruleFireCount++;

  if (r.action == RULE_ACT_MACRO) {
    runMacroSafe(r.actionArg, F("RULE"));
    Serial.print(F("RULE_FIRE #"));
    Serial.print(ruleId);
    Serial.print(F(" MACRO "));
    Serial.println(r.actionArg);
  } else if (r.action == RULE_ACT_LOG) {
    Serial.print(F("RULE_LOG #"));
    Serial.print(ruleId);
    Serial.print(F(" val="));
    Serial.println(valueNow);
  } else {
    char msg[28];
    snprintf(msg, sizeof(msg), "RULE%u ALERT %d", ruleId, valueNow);
    showTextBottom(msg);
    Serial.print(F("RULE_ALERT #"));
    Serial.print(ruleId);
    Serial.print(F(" val="));
    Serial.println(valueNow);
  }

  char em[EVENT_LOG_MSG_LEN];
  snprintf(em, sizeof(em), "R%u v=%d", ruleId, valueNow);
  addEventLogC(EVLOG_RULE, em);
  setWhy(em);
}

void evaluateRules() {
  if (!rulesEnabled) return;
  unsigned long now = millis();
  if (now - ruleEvalLastMs < RULE_EVAL_INTERVAL_MS) return;
  ruleEvalLastMs = now;

  for (uint8_t i = 0; i < MAX_RULES; i++) {
    SmartRule& r = smartRules[i];
    if (!r.enabled) continue;

    int16_t v = readRuleSourceValue(r);
    bool condition = ruleCompare(v, r.op, r.value);

    if (!condition) {
      r.sinceMs = 0;
      continue;
    }

    if (r.sinceMs == 0) r.sinceMs = now;
    if (r.holdMs > 0 && (now - r.sinceMs) < r.holdMs) continue;
    if (now - r.lastFireMs < r.cooldownMs) continue;

    r.lastFireMs = now;
    triggerRuleAction(i, r, v);
  }
}

bool parseRuleExpr(String expr, RuleSourceType& srcType, uint8_t& srcIndex, RuleOp& op, int16_t& rhs) {
  expr.trim();
  expr.toUpperCase();

  const char* ops[] = {">=", "<=", "==", "!=", ">", "<"};
  RuleOp opVals[] = {RULE_OP_GE, RULE_OP_LE, RULE_OP_EQ, RULE_OP_NE, RULE_OP_GT, RULE_OP_LT};

  int opPos = -1;
  uint8_t opSel = 0;
  for (uint8_t i = 0; i < 6; i++) {
    opPos = expr.indexOf(ops[i]);
    if (opPos > 0) {
      opSel = i;
      break;
    }
  }
  if (opPos <= 0) return false;

  String lhs = expr.substring(0, opPos);
  String rhsStr = expr.substring(opPos + strlen(ops[opSel]));
  lhs.trim();
  rhsStr.trim();

  if (lhs.startsWith("A")) {
    int pin = lhs.substring(1).toInt();
    if (pin < 8 || pin > 15) return false;
    srcType = RULE_SRC_ANALOG;
    srcIndex = (uint8_t)(pin - 8);
  } else if (lhs.startsWith("GPIO")) {
    int pin = lhs.substring(4).toInt();
    int8_t idx = -1;
    for (uint8_t i = 0; i < NUM_GPIO; i++) {
      if (gpioPins[i] == pin) {
        idx = i;
        break;
      }
    }
    if (idx < 0) return false;
    srcType = RULE_SRC_GPIO;
    srcIndex = (uint8_t)idx;
  } else {
    return false;
  }

  op = opVals[opSel];
  rhs = (int16_t)rhsStr.toInt();
  return true;
}

void printRule(uint8_t id, const SmartRule& r) {
  if (!r.enabled) return;

  String opText = "";
  ruleOpToText(r.op, opText);

  Serial.print(F("R"));
  Serial.print(id);
  Serial.print(F(": "));
  if (r.srcType == RULE_SRC_ANALOG) {
    Serial.print(F("A"));
    Serial.print(r.srcIndex + 8);
  } else {
    Serial.print(F("GPIO"));
    Serial.print(gpioPins[r.srcIndex]);
  }
  Serial.print(opText);
  Serial.print(r.value);

  if (r.holdMs > 0) {
    Serial.print(F(" FOR "));
    Serial.print(r.holdMs);
    Serial.print(F("ms"));
  }

  Serial.print(F(" THEN "));
  if (r.action == RULE_ACT_MACRO) {
    Serial.print(F("MACRO "));
    Serial.print(r.actionArg);
  } else if (r.action == RULE_ACT_LOG) {
    Serial.print(F("LOG"));
  } else {
    Serial.print(F("ALERT"));
  }

  Serial.print(F(" CD="));
  Serial.print(r.cooldownMs);
  Serial.println(F("ms"));
}
