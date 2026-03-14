#ifndef SAM_RULES_H
#define SAM_RULES_H

#include "sam_config.h"

void ruleOpToText(RuleOp op, String& out);
bool ruleCompare(int16_t lhs, RuleOp op, int16_t rhs);
int16_t readRuleSourceValue(const SmartRule& r);
void triggerRuleAction(uint8_t ruleId, SmartRule& r, int16_t valueNow);
void evaluateRules();
bool parseRuleExpr(String expr, RuleSourceType& srcType, uint8_t& srcIndex, RuleOp& op, int16_t& rhs);
void printRule(uint8_t id, const SmartRule& r);

#endif // SAM_RULES_H
