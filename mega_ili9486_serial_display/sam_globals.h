#ifndef SAM_GLOBALS_H
#define SAM_GLOBALS_H

#include "sam_config.h"
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include "MenuSystem.h"

// ========== DISPLAY OBJECTS ==========
extern MCUFRIEND_kbv tft;
extern TouchScreen ts;

// ========== TOUCH CALIBRATION ==========
extern int16_t TS_MINX;
extern int16_t TS_MAXX;
extern int16_t TS_MINY;
extern int16_t TS_MAXY;

// ========== TOUCH TEST ==========
extern bool touchTestMode;

// ========== MENU SYSTEM ==========
extern MenuManager* menuManager;
extern bool menuActive;

// ========== SCREEN LAYOUT ==========
extern uint16_t screenW;
extern uint16_t screenH;
extern uint16_t dividerY;
extern uint16_t topMaxY;
extern uint16_t bottomMinY;
extern uint16_t bottomMaxY;

// ========== TOP SCREEN ==========
extern uint16_t topPosX;
extern uint16_t topPosY;
extern uint16_t topTextColor;
extern uint16_t topBgColor;
extern bool topOpaqueText;
extern uint8_t topTextSize;

// ========== BOTTOM SCREEN ==========
extern uint16_t bottomPosX;
extern uint16_t bottomPosY;
extern uint16_t bottomTextColor;
extern uint16_t bottomBgColor;
extern bool bottomOpaqueText;
extern uint8_t bottomTextSize;

// ========== COMMAND HANDLING ==========
extern String cmd;
extern bool cmdReady;
extern String fpgaBuffer;
extern unsigned long serialRxEnableMs;

// ========== FPGA SETTINGS ==========
extern uint8_t activeFpga;
extern unsigned long fpga1Baud;
extern unsigned long fpga2Baud;
extern unsigned long fpga3Baud;
extern uint8_t fpga1StopBits;
extern uint8_t fpga2StopBits;
extern uint8_t fpga3StopBits;

extern FPGARxMode fpga1RxMode;
extern FPGARxMode fpga2RxMode;
extern FPGARxMode fpga3RxMode;

extern FPGAParseMode fpga1ParseMode;
extern FPGAParseMode fpga2ParseMode;
extern FPGAParseMode fpga3ParseMode;

extern TermMode bypassTerm;
extern uint8_t customTermByte;

// ========== BUTTONS ==========
extern Button buttons[NUM_BUTTONS];
extern bool buttonsVisible;
extern uint16_t buttonTextY;

// ========== TOUCH DEBOUNCE ==========
extern unsigned long lastTouch;
extern unsigned long lastViewToggleTouch;

// ========== GPIO ==========
extern const uint8_t gpioPins[NUM_GPIO];
extern GPIOMode gpioModes[NUM_GPIO];
extern uint8_t gpioStates[NUM_GPIO];
extern uint8_t gpioPrevStates[NUM_GPIO];
extern bool gpioEventEnable[NUM_GPIO];
extern bool gpioEventRising[NUM_GPIO];
extern bool gpioEventFalling[NUM_GPIO];
extern GPIOEvent gpioEventQueue[GPIO_EVENT_QUEUE_SIZE];
extern uint8_t gpioEventHead;
extern uint8_t gpioEventTail;

// ========== SPI ==========
extern int8_t spiCSPin;
extern uint32_t spiSpeed;
extern uint8_t spiMode;

// ========== ANALOG ==========
extern const uint8_t analogPins[8];
extern uint8_t analogAvgSamples;
extern AnalogRefMode analogRefMode;

// ========== DATA LOGGING ==========
extern uint16_t logStartAddr;
extern uint16_t logEndAddr;
extern bool loggingActive;
extern unsigned long logInterval;
extern unsigned long lastLogTime;
extern uint16_t logWriteAddr;
extern uint16_t logEntryCount;
extern uint8_t logSource;

// ========== STATISTICS ==========
extern Stats analogStats[8];
extern Threshold thresholds[8];

// ========== MACROS ==========
extern Macro macros[MAX_MACROS];

// ========== SMART RULES ==========
extern SmartRule smartRules[MAX_RULES];
extern bool rulesEnabled;
extern unsigned long ruleEvalLastMs;
extern unsigned long ruleFireCount;
extern const uint16_t RULE_EVAL_INTERVAL_MS;

// ========== EVENT LOG ==========
extern EventLogEntry eventLog[EVENT_LOG_CAPACITY];
extern uint8_t eventLogHead;
extern uint8_t eventLogCount;

// ========== MACRO DEPTH ==========
extern uint8_t macroRunDepth;
extern const uint8_t MAX_MACRO_DEPTH;

// ========== VIEW MODE ==========
extern ViewMode viewMode;

// ========== PROTOCOL ==========
extern bool protoAddrMode;
extern bool protoFrameMode;
extern bool bridgeMode;
extern uint8_t localAddress;
extern const uint8_t FRAME_SYNC;
extern const uint8_t FRAME_MAX_PAYLOAD;
extern FrameRxState frameRx[4];

// ========== RUNTIME COUNTERS ==========
extern unsigned long fpgaRxBytes[4];
extern unsigned long fpgaTxBytes[4];
extern unsigned long fpgaFrameOk[4];
extern unsigned long fpgaFrameErr[4];
extern unsigned long fpgaDropped[4];
extern unsigned long fpgaTimeouts[4];
extern unsigned long fpgaLastActivityMs[4];
extern unsigned long fpgaLastErrorMs[4];
extern unsigned long gpioEventCount;
extern unsigned long thresholdEventCount;
extern unsigned long addrLocalHandled;
extern unsigned long addrPassthrough;

// ========== WATCH MODE ==========
extern bool watchEnabled;
extern bool linkIndicatorsEnabled;
extern String watchSource;
extern unsigned long watchIntervalMs;
extern unsigned long watchLastMs;
extern uint8_t smartProfile;

// ========== WATCHDOG ==========
extern bool wdtEnabled;
extern uint16_t wdtTimeoutMs;
extern uint8_t bootResetCause;
extern char lastWhy[40];
extern unsigned long lastWhyMs;

// ========== ESCALATION & FAILSAFE ==========
extern EscalationPolicy escalation;
extern FailsafePolicy failsafe;

// ========== EVENT HOOKS ==========
extern EventHook gpioHooks[NUM_GPIO];
extern EventHook thresholdHooks[8];

// ========== EEPROM PROTECTION ==========
extern ProtectZone protectZones[MAX_PROTECT_ZONES];

// ========== WAVEFORM ==========
extern int8_t wavePin;
extern uint8_t waveType;
extern uint16_t waveFreq;
extern uint8_t waveDuty;
extern bool waveActive;
extern unsigned long waveLastUpdate;
extern uint8_t wavePhase;
extern const uint8_t sineLUT[32];

// ========== FREQUENCY MONITOR ==========
extern int8_t freqMonPin;
extern unsigned long freqMonDuration;
extern unsigned long freqMonLastUpdate;
extern volatile unsigned long pulseCount;
extern bool freqMonActive;

// ========== PWM TRACKING ==========
extern bool pwmPinActive[47];

// ========== BUTTON CONFIGS ==========
extern ButtonConfig btnUp;
extern ButtonConfig btnDown;
extern ButtonConfig btnLeft;
extern ButtonConfig btnRight;

#endif // SAM_GLOBALS_H
