#ifndef SAM_TOUCH_H
#define SAM_TOUCH_H

#include "sam_config.h"

void initButtons();
void drawButton(uint8_t idx);
void showButtons();
void hideButtons();
void checkTouch();

#endif // SAM_TOUCH_H
