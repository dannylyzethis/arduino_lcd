#ifndef SAM_HOOKS_H
#define SAM_HOOKS_H

#include "sam_config.h"

void triggerGPIOHook(uint8_t gpioIdx, bool rising);
void triggerThresholdHook(uint8_t analogIdx, bool entering);

#endif // SAM_HOOKS_H
