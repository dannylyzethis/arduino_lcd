#include "sam_gpio.h"
#include "sam_globals.h"
#include "sam_display.h"
#include "sam_logging.h"
#include "sam_hooks.h"
#include "sam_waveform.h"

void initGPIO() {
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    gpioModes[i] = GPIO_INPUT;
    pinMode(gpioPins[i], INPUT);
    gpioStates[i] = digitalRead(gpioPins[i]);
    gpioPrevStates[i] = gpioStates[i];
    gpioEventEnable[i] = false;
    gpioEventRising[i] = false;
    gpioEventFalling[i] = false;
  }
  Serial.println(F("GPIO: Pins 22-45 initialized"));
}

void pollGPIO() {
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioModes[i] != GPIO_OUTPUT) {
      uint8_t newState = digitalRead(gpioPins[i]);

      if (newState != gpioPrevStates[i]) {
        gpioStates[i] = newState;

        if (gpioEventEnable[i]) {
          bool rising = (newState == HIGH && gpioPrevStates[i] == LOW);
          bool falling = (newState == LOW && gpioPrevStates[i] == HIGH);

          if ((rising && gpioEventRising[i]) || (falling && gpioEventFalling[i])) {
            queueGPIOEvent(i, newState, rising);
          }
        }

        gpioPrevStates[i] = newState;
      }
    }
  }
}

void queueGPIOEvent(uint8_t gpioIdx, uint8_t newState, bool rising) {
  uint8_t nextHead = (gpioEventHead + 1) % GPIO_EVENT_QUEUE_SIZE;
  if (nextHead != gpioEventTail) {
    gpioEventQueue[gpioEventHead].pin = gpioIdx;
    gpioEventQueue[gpioEventHead].newState = newState;
    gpioEventQueue[gpioEventHead].rising = rising;
    gpioEventHead = nextHead;
  }
}

void processGPIOEvents() {
  while (gpioEventTail != gpioEventHead) {
    GPIOEvent &evt = gpioEventQueue[gpioEventTail];

    String msg = "GPIO";
    msg += gpioPins[evt.pin];
    msg += evt.rising ? " RISE" : " FALL";
    showTextBottom(msg);

    Serial.print(F("[GPIO"));
    Serial.print(gpioPins[evt.pin]);
    Serial.print(F("] "));
    Serial.println(evt.rising ? F("RISING") : F("FALLING"));

    addEventLog(EVLOG_GPIO, msg);
    setWhy(msg.c_str());
    gpioEventCount++;
    triggerGPIOHook(evt.pin, evt.rising);

    gpioEventTail = (gpioEventTail + 1) % GPIO_EVENT_QUEUE_SIZE;
  }
}

void setGPIOMode(uint8_t pin, String& mode) {
  int8_t idx = findGPIOIndex(pin);
  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_45_ONLY"));
    return;
  }
  if (isPinBusyByPWM(pin)) {
    Serial.println(F("ERR:PIN_BUSY_PWM"));
    return;
  }

  mode.trim();
  if (mode == "IN") {
    gpioModes[idx] = GPIO_INPUT;
    pinMode(pin, INPUT);
    gpioStates[idx] = digitalRead(pin);
    gpioPrevStates[idx] = gpioStates[idx];
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_IN"));
  } else if (mode == "INPU" || mode == "PULLUP") {
    gpioModes[idx] = GPIO_INPUT_PULLUP;
    pinMode(pin, INPUT_PULLUP);
    gpioStates[idx] = digitalRead(pin);
    gpioPrevStates[idx] = gpioStates[idx];
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_INPU"));
  } else if (mode == "OUT") {
    gpioModes[idx] = GPIO_OUTPUT;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, gpioStates[idx]);
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_OUT"));
  } else {
    Serial.println(F("ERR:USE_IN_INPU_OUT"));
  }
}

void writeGPIO(uint8_t pin, String& val) {
  int8_t idx = findGPIOIndex(pin);
  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_45_ONLY"));
    return;
  }
  if (isPinBusyByPWM(pin)) {
    Serial.println(F("ERR:PIN_BUSY_PWM"));
    return;
  }

  if (gpioModes[idx] != GPIO_OUTPUT) {
    Serial.println(F("ERR:NOT_OUTPUT"));
    return;
  }

  val.trim();
  uint8_t state = 0;
  if (val == "1" || val == "HIGH" || val == "H") {
    state = HIGH;
  } else if (val == "0" || val == "LOW" || val == "L") {
    state = LOW;
  } else {
    Serial.println(F("ERR:USE_0_1_HIGH_LOW"));
    return;
  }

  digitalWrite(pin, state);
  gpioStates[idx] = state;
  Serial.print(F("OK:GPIO"));
  Serial.print(pin);
  Serial.print(F("="));
  Serial.println(state);
}

void readGPIO(uint8_t pin) {
  int8_t idx = findGPIOIndex(pin);
  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_45_ONLY"));
    return;
  }

  uint8_t state = digitalRead(pin);
  gpioStates[idx] = state;

  Serial.print(F("GPIO"));
  Serial.print(pin);
  Serial.print(F("="));
  Serial.println(state);
}

void readAllGPIO() {
  Serial.print(F("GPIO: "));
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    uint8_t state = digitalRead(gpioPins[i]);
    gpioStates[i] = state;
    Serial.print(gpioPins[i]);
    Serial.print(F("="));
    Serial.print(state);
    if (i < NUM_GPIO - 1) Serial.print(F(" "));
  }
  Serial.println();
}

void setGPIOEvent(uint8_t pin, String& type) {
  int8_t idx = findGPIOIndex(pin);
  if (idx < 0) {
    Serial.println(F("ERR:PIN_22_45_ONLY"));
    return;
  }
  if (isPinBusyByPWM(pin)) {
    Serial.println(F("ERR:PIN_BUSY_PWM"));
    return;
  }

  type.trim();
  if (type == "NONE") {
    gpioEventEnable[idx] = false;
    gpioEventRising[idx] = false;
    gpioEventFalling[idx] = false;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_NONE"));
  } else if (type == "RISING" || type == "RISE") {
    gpioEventEnable[idx] = true;
    gpioEventRising[idx] = true;
    gpioEventFalling[idx] = false;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_RISE"));
  } else if (type == "FALLING" || type == "FALL") {
    gpioEventEnable[idx] = true;
    gpioEventRising[idx] = false;
    gpioEventFalling[idx] = true;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_FALL"));
  } else if (type == "BOTH" || type == "CHANGE") {
    gpioEventEnable[idx] = true;
    gpioEventRising[idx] = true;
    gpioEventFalling[idx] = true;
    Serial.print(F("OK:GPIO"));
    Serial.print(pin);
    Serial.println(F("_EV_BOTH"));
  } else {
    Serial.println(F("ERR:USE_NONE_RISING_FALLING_BOTH"));
  }
}

void showGPIORegister() {
  uint32_t regVal = 0;
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioStates[i]) {
      regVal |= (1UL << i);
    }
  }

  char hx[9];
  snprintf(hx, sizeof(hx), "%06lX", regVal & 0xFFFFFFUL);
  Serial.print(F("GPIOREG=0x"));
  Serial.print(hx);
  Serial.print(F(" ("));
  Serial.print(regVal & 0xFFFFFFUL);
  Serial.print(F(") BIN="));
  for (int i = NUM_GPIO - 1; i >= 0; i--) {
    Serial.print((regVal >> i) & 1);
  }
  Serial.println();
}

void setGPIORegister(uint32_t regVal) {
  regVal &= 0xFFFFFFUL;
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    uint8_t pin = gpioPins[i];
    if (gpioModes[i] == GPIO_OUTPUT) {
      if (isPinBusyByPWM(pin)) continue;
      uint8_t bitVal = (regVal >> i) & 1;
      digitalWrite(pin, bitVal);
      gpioStates[i] = bitVal;
    }
  }

  char hx[9];
  snprintf(hx, sizeof(hx), "%06lX", regVal);
  Serial.print(F("OK:GPIOSET=0x"));
  Serial.println(hx);
}

int8_t findGPIOIndex(uint8_t pin) {
  for (uint8_t i = 0; i < NUM_GPIO; i++) {
    if (gpioPins[i] == pin) return (int8_t)i;
  }
  return -1;
}

bool isPinBusyByGPIO(uint8_t pin) {
  int8_t idx = findGPIOIndex(pin);
  if (idx < 0) return false;
  if (gpioModes[idx] == GPIO_OUTPUT) return true;
  if (gpioEventEnable[idx]) return true;
  return false;
}
