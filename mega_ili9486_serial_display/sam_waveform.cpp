#include "sam_waveform.h"
#include "sam_globals.h"
#include "sam_gpio.h"
#include <avr/pgmspace.h>

void pulseISR() {
  pulseCount++;
}

bool isPwmCapablePin(uint8_t pin) {
  return ((pin >= 2 && pin <= 13) || (pin >= 44 && pin <= 46));
}

bool isPinBusyByPWM(uint8_t pin) {
  if (waveActive && (int8_t)pin == wavePin) return true;
  if (pin < 47 && pwmPinActive[pin]) return true;
  return false;
}

void updateWaveform() {
  unsigned long now = micros();
  unsigned long period = 1000000UL / waveFreq;  // Period in microseconds

  // Update phase based on time
  if (now - waveLastUpdate >= period / 256) {
    waveLastUpdate = now;
    wavePhase++;

    uint8_t pwmValue = 0;

    switch (waveType) {
      case 0:  // Square wave
        pwmValue = (wavePhase < 128) ? 255 : 0;
        break;

      case 1:  // Triangle wave
        if (wavePhase < 128) {
          pwmValue = wavePhase * 2;
        } else {
          pwmValue = 255 - ((wavePhase - 128) * 2);
        }
        break;

      case 2:  // Sine wave (using lookup table)
        {
          uint8_t idx = (wavePhase * 32) / 256;
          pwmValue = pgm_read_byte(&sineLUT[idx]);
        }
        break;
    }

    analogWrite(wavePin, pwmValue);
  }
}

void handleWaveGen(String params) {
  // Parse: <pin> <type> <freq>
  // type: 0=square, 1=triangle, 2=sine
  int vals[3];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 3; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        token.toUpperCase();

        if (count == 1) {  // Wave type
          if (token == "SQUARE") vals[count++] = 0;
          else if (token == "TRIANGLE" || token == "TRI") vals[count++] = 1;
          else if (token == "SINE" || token == "SIN") vals[count++] = 2;
          else vals[count++] = token.toInt();
        } else {
          vals[count++] = token.toInt();
        }
      }
      lastPos = i + 1;
    }
  }

  if (count < 3) {
    Serial.println(F("ERR:FORMAT #WAVEGEN <pin> <type> <freq>"));
    Serial.println(F("  type: SQUARE, TRIANGLE, SINE (or 0,1,2)"));
    Serial.println(F("  freq: 1-10000 Hz"));
    return;
  }

  int8_t pin = vals[0];
  waveType = vals[1];
  waveFreq = vals[2];

  // Validate
  if (waveType > 2) waveType = 0;
  if (waveFreq < 1) waveFreq = 1;
  if (waveFreq > 10000) waveFreq = 10000;

  // Check if pin supports PWM (Mega PWM pins: 2-13, 44-46)
  if (!isPwmCapablePin(pin)) {
    Serial.println(F("ERR:PIN_NO_PWM"));
    Serial.println(F("PWM pins: 2-13, 44-46"));
    return;
  }

  if (isPinBusyByGPIO((uint8_t)pin)) {
    Serial.println(F("ERR:PIN_BUSY_GPIO"));
    return;
  }
  if ((uint8_t)pin < 47 && pwmPinActive[(uint8_t)pin]) {
    Serial.println(F("ERR:PIN_BUSY_PWM"));
    return;
  }

  if (waveActive && wavePin >= 0 && wavePin != pin) {
    analogWrite(wavePin, 0);
  }

  wavePin = pin;
  pinMode(wavePin, OUTPUT);
  waveActive = true;
  wavePhase = 0;
  waveLastUpdate = micros();

  Serial.print(F("WAVE_START: Pin="));
  Serial.print(wavePin);
  Serial.print(F(", Type="));
  switch (waveType) {
    case 0: Serial.print(F("SQUARE")); break;
    case 1: Serial.print(F("TRIANGLE")); break;
    case 2: Serial.print(F("SINE")); break;
  }
  Serial.print(F(", Freq="));
  Serial.print(waveFreq);
  Serial.println(F("Hz"));
}

void handleWaveStop() {
  if (wavePin >= 0) {
    analogWrite(wavePin, 0);
    wavePin = -1;
  }
  waveActive = false;
  Serial.println(F("WAVE_STOPPED"));
}

void handlePWMSet(String params) {
  // Parse: <pin> <duty>
  // duty: 0-255
  int vals[2];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #PWMSET <pin> <duty>"));
    Serial.println(F("  pin: PWM-capable pin (2-13, 44-46)"));
    Serial.println(F("  duty: 0-255 (0=off, 255=full on)"));
    return;
  }

  int pin = vals[0];
  int duty = vals[1];

  // Check if pin supports PWM (Mega PWM pins: 2-13, 44-46)
  if (!isPwmCapablePin(pin)) {
    Serial.println(F("ERR:PIN_NO_PWM"));
    Serial.println(F("PWM pins: 2-13, 44-46"));
    return;
  }

  if (isPinBusyByGPIO((uint8_t)pin)) {
    Serial.println(F("ERR:PIN_BUSY_GPIO"));
    return;
  }
  if (waveActive && wavePin == pin) {
    Serial.println(F("ERR:PIN_BUSY_PWM"));
    return;
  }

  // Validate duty cycle
  if (duty < 0) duty = 0;
  if (duty > 255) duty = 255;

  pinMode(pin, OUTPUT);
  analogWrite(pin, duty);
  if (pin >= 0 && pin < 47) pwmPinActive[pin] = (duty > 0);

  Serial.print(F("PWM_SET: Pin="));
  Serial.print(pin);
  Serial.print(F(", Duty="));
  Serial.print(duty);
  Serial.print(F(" ("));
  Serial.print((duty * 100) / 255);
  Serial.println(F("%)"));
}

void handlePWMStop(String params) {
  int pin = params.toInt();

  // Check if pin supports PWM
  if (!isPwmCapablePin(pin)) {
    Serial.println(F("ERR:PIN_NO_PWM"));
    Serial.println(F("PWM pins: 2-13, 44-46"));
    return;
  }

  analogWrite(pin, 0);
  if (pin >= 0 && pin < 47) pwmPinActive[pin] = false;
  Serial.print(F("PWM_STOPPED: Pin="));
  Serial.println(pin);
}

void handlePWMFreq(String params) {
  // Parse: <pin> <freq>
  int vals[2];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #PWMFREQ <pin> <freq>"));
    Serial.println(F("  pin: PWM pin (2-13, 44-46)"));
    Serial.println(F("  freq: 31, 62, 122, 244, 488, 976, 1953, 3906, 7812, 15625, 31250 Hz"));
    Serial.println(F("  Note: Affects all pins on same timer"));
    return;
  }

  int pin = vals[0];
  unsigned long freq = vals[1];

  // Check if pin supports PWM
  if (!isPwmCapablePin(pin)) {
    Serial.println(F("ERR:PIN_NO_PWM"));
    return;
  }

  // Determine which timer controls this pin
  // Timer 0 (pins 4, 13) - DO NOT MODIFY (used by millis/delay)
  // Timer 1 (pins 11, 12)
  // Timer 2 (pins 9, 10)
  // Timer 3 (pins 2, 3, 5)
  // Timer 4 (pins 6, 7, 8)
  // Timer 5 (pins 44, 45, 46)

  uint8_t timer = 0;
  if (pin == 11 || pin == 12) timer = 1;
  else if (pin == 9 || pin == 10) timer = 2;
  else if (pin == 2 || pin == 3 || pin == 5) timer = 3;
  else if (pin == 6 || pin == 7 || pin == 8) timer = 4;
  else if (pin >= 44 && pin <= 46) timer = 5;
  else if (pin == 4 || pin == 13) {
    Serial.println(F("ERR:TIMER0_PROTECTED"));
    Serial.println(F("Pins 4,13 use Timer0 (millis/delay)"));
    return;
  }

  // Map frequency to prescaler value
  // Base freq for 16MHz: 16000000 / (prescaler * 256)
  // For 16-bit timers (1,3,4,5): can use prescaler 1,8,64,256,1024
  // For 8-bit timer (2): can use prescaler 1,8,32,64,128,256,1024
  uint8_t prescaler = 0;
  uint16_t csrValue = 0;

  if (timer == 2) {
    // Timer 2 (8-bit) prescaler options
    if (freq >= 31250) { prescaler = 1; csrValue = 1; }       // 62500 Hz
    else if (freq >= 7812) { prescaler = 8; csrValue = 2; }   // 7812.5 Hz
    else if (freq >= 1953) { prescaler = 32; csrValue = 3; }  // 1953 Hz
    else if (freq >= 976) { prescaler = 64; csrValue = 4; }   // 976.5 Hz
    else if (freq >= 488) { prescaler = 128; csrValue = 5; }  // 488 Hz
    else if (freq >= 244) { prescaler = 256; csrValue = 6; }  // 244 Hz
    else { prescaler = 1024; csrValue = 7; }                  // 61 Hz
  } else {
    // Timers 1, 3, 4, 5 (16-bit) prescaler options
    if (freq >= 31250) { prescaler = 1; csrValue = 1; }       // 62500 Hz
    else if (freq >= 7812) { prescaler = 8; csrValue = 2; }   // 7812.5 Hz
    else if (freq >= 976) { prescaler = 64; csrValue = 3; }   // 976.5 Hz
    else if (freq >= 244) { prescaler = 256; csrValue = 4; }  // 244 Hz
    else { prescaler = 1024; csrValue = 5; }                  // 61 Hz
  }

  // Set prescaler for the appropriate timer
  switch (timer) {
    case 1:
      TCCR1B = (TCCR1B & 0xF8) | csrValue;
      break;
    case 2:
      TCCR2B = (TCCR2B & 0xF8) | csrValue;
      break;
    case 3:
      TCCR3B = (TCCR3B & 0xF8) | csrValue;
      break;
    case 4:
      TCCR4B = (TCCR4B & 0xF8) | csrValue;
      break;
    case 5:
      TCCR5B = (TCCR5B & 0xF8) | csrValue;
      break;
  }

  unsigned long actualFreq = 16000000UL / (prescaler * 256UL);

  Serial.print(F("PWM_FREQ: Timer"));
  Serial.print(timer);
  Serial.print(F(", Prescaler="));
  Serial.print(prescaler);
  Serial.print(F(", Freq="));
  Serial.print(actualFreq);
  Serial.println(F("Hz"));
  Serial.println(F("WARNING: Affects all pins on this timer!"));
}

void handlePulseIn(String params) {
  // Parse: <pin> <state> <timeout>
  int vals[3];
  int count = 0;
  int lastPos = 0;
  uint8_t state = HIGH;

  for (int i = 0; i <= params.length() && count < 3; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        String token = params.substring(lastPos, i);
        token.toUpperCase();

        if (count == 1) {  // State parameter
          if (token == "HIGH" || token == "1") state = HIGH;
          else if (token == "LOW" || token == "0") state = LOW;
          else vals[count] = token.toInt();  // Allow numeric
          count++;
        } else {
          vals[count++] = token.toInt();
        }
      }
      lastPos = i + 1;
    }
  }

  if (count < 3) {
    Serial.println(F("ERR:FORMAT #PULSEIN <pin> <state> <timeout>"));
    Serial.println(F("  state: HIGH, LOW, 1, or 0"));
    Serial.println(F("  timeout: microseconds (max 3000000)"));
    return;
  }

  uint8_t pin = vals[0];
  unsigned long timeout = vals[2];

  if (timeout > 3000000UL) {
    Serial.println(F("ERR:TIMEOUT_MAX_3000000"));
    return;
  }

  // Configure pin as input
  pinMode(pin, INPUT);

  // Measure pulse
  unsigned long duration = pulseIn(pin, state, timeout);

  // Report result
  Serial.print(F("PULSE_IN: Pin="));
  Serial.print(pin);
  Serial.print(F(", State="));
  Serial.print(state == HIGH ? F("HIGH") : F("LOW"));
  Serial.print(F(", Duration="));
  Serial.print(duration);
  Serial.println(F("us"));

  if (duration == 0) {
    Serial.println(F("NOTE: Timeout or no pulse detected"));
  }
}

void handleFreqCount(String params) {
  // Parse: <pin> <duration>
  int vals[2];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #FREQCOUNT <pin> <duration>"));
    Serial.println(F("  duration: milliseconds (10-10000)"));
    return;
  }

  uint8_t pin = vals[0];
  unsigned long duration = vals[1];

  if (duration < 10 || duration > 10000) {
    Serial.println(F("ERR:DURATION_RANGE (10-10000 ms)"));
    return;
  }

  // Configure pin as input
  pinMode(pin, INPUT);

  // Count rising edges
  unsigned long startTime = millis();
  unsigned long count_pulses = 0;
  uint8_t lastState = digitalRead(pin);

  while (millis() - startTime < duration) {
    uint8_t currentState = digitalRead(pin);
    if (currentState == HIGH && lastState == LOW) {
      count_pulses++;
    }
    lastState = currentState;
  }

  // Calculate frequency
  float actualDuration = millis() - startTime;
  float frequency = (count_pulses * 1000.0) / actualDuration;

  // Report result
  Serial.print(F("FREQ_COUNT: Pin="));
  Serial.print(pin);
  Serial.print(F(", Pulses="));
  Serial.print(count_pulses);
  Serial.print(F(", Duration="));
  Serial.print((unsigned long)actualDuration);
  Serial.print(F("ms, Freq="));
  Serial.print(frequency, 2);
  Serial.println(F("Hz"));
}

void handleFreqMon(String params) {
  // Parse: <pin> <duration>
  int vals[2];
  int count = 0;
  int lastPos = 0;

  for (int i = 0; i <= params.length() && count < 2; i++) {
    if (i == params.length() || params[i] == ' ') {
      if (i > lastPos) {
        vals[count++] = params.substring(lastPos, i).toInt();
      }
      lastPos = i + 1;
    }
  }

  if (count < 2) {
    Serial.println(F("ERR:FORMAT #FREQMON <pin> <duration>"));
    Serial.println(F("  duration: measurement window (100-10000 ms)"));
    Serial.println(F("  Interrupt pins: 2, 3, 18-21"));
    return;
  }

  uint8_t pin = vals[0];
  freqMonDuration = vals[1];

  if (freqMonDuration < 100 || freqMonDuration > 10000) {
    Serial.println(F("ERR:DURATION_RANGE (100-10000 ms)"));
    return;
  }

  // Check if pin supports interrupts (Mega: 2, 3, 18, 19, 20, 21)
  uint8_t interruptNum = digitalPinToInterrupt(pin);
  if (interruptNum == NOT_AN_INTERRUPT) {
    Serial.println(F("ERR:PIN_NO_INTERRUPT"));
    Serial.println(F("Interrupt pins: 2, 3, 18, 19, 20, 21"));
    return;
  }

  // Stop any existing monitoring
  if (freqMonActive) {
    detachInterrupt(digitalPinToInterrupt(freqMonPin));
  }

  // Configure pin and interrupt
  freqMonPin = pin;
  pinMode(freqMonPin, INPUT);
  pulseCount = 0;
  freqMonLastUpdate = millis();
  freqMonActive = true;

  attachInterrupt(interruptNum, pulseISR, RISING);

  Serial.print(F("FREQ_MON_START: Pin="));
  Serial.print(freqMonPin);
  Serial.print(F(", Window="));
  Serial.print(freqMonDuration);
  Serial.println(F("ms"));
}

void updateFreqMonitor() {
  unsigned long now = millis();

  if (now - freqMonLastUpdate >= freqMonDuration) {
    // Calculate frequency
    float actualDuration = now - freqMonLastUpdate;
    float frequency = (pulseCount * 1000.0) / actualDuration;

    // Report
    Serial.print(F("FREQ: "));
    Serial.print(frequency, 2);
    Serial.print(F("Hz ("));
    Serial.print(pulseCount);
    Serial.println(F(" pulses)"));

    // Reset for next window
    pulseCount = 0;
    freqMonLastUpdate = now;
  }
}

void handleFreqStop() {
  if (freqMonActive && freqMonPin >= 0) {
    detachInterrupt(digitalPinToInterrupt(freqMonPin));
    freqMonPin = -1;
    freqMonActive = false;
    Serial.println(F("FREQ_MON_STOPPED"));
  } else {
    Serial.println(F("ERR:NOT_ACTIVE"));
  }
}
