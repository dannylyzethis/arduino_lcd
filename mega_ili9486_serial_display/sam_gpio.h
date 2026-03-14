#ifndef SAM_GPIO_H
#define SAM_GPIO_H

#include "sam_config.h"

void initGPIO();
void pollGPIO();
void queueGPIOEvent(uint8_t gpioIdx, uint8_t newState, bool rising);
void processGPIOEvents();
void setGPIOMode(uint8_t pin, String& mode);
void writeGPIO(uint8_t pin, String& val);
void readGPIO(uint8_t pin);
void readAllGPIO();
void setGPIOEvent(uint8_t pin, String& type);
void showGPIORegister();
void setGPIORegister(uint32_t regVal);
int8_t findGPIOIndex(uint8_t pin);
bool isPinBusyByGPIO(uint8_t pin);

#endif // SAM_GPIO_H
