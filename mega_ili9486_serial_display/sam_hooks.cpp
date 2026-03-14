#include "sam_hooks.h"
#include "sam_globals.h"
#include "sam_macros.h"

void triggerGPIOHook(uint8_t gpioIdx, bool rising) {
  if (gpioIdx >= NUM_GPIO) return;
  EventHook& hook = gpioHooks[gpioIdx];
  if (!hook.enabled) return;
  if (rising && !(hook.modeMask & 0x01)) return;
  if (!rising && !(hook.modeMask & 0x02)) return;
  unsigned long now = millis();
  if (now - hook.lastTriggerMs < hook.cooldownMs) return;
  hook.lastTriggerMs = now;
  runMacroSafe(hook.macroId, F("GPIO"));
}

void triggerThresholdHook(uint8_t analogIdx, bool entering) {
  if (analogIdx >= 8) return;
  EventHook& hook = thresholdHooks[analogIdx];
  if (!hook.enabled) return;
  if (entering && !(hook.modeMask & 0x01)) return;
  if (!entering && !(hook.modeMask & 0x02)) return;
  unsigned long now = millis();
  if (now - hook.lastTriggerMs < hook.cooldownMs) return;
  hook.lastTriggerMs = now;
  runMacroSafe(hook.macroId, F("THRESH"));
}
