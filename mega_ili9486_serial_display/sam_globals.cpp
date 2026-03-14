#include "sam_globals.h"
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// ========== DISPLAY OBJECTS ==========
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// ========== TOUCH CALIBRATION ==========
int16_t TS_MINX = 120;
int16_t TS_MAXX = 900;
int16_t TS_MINY = 70;
int16_t TS_MAXY = 920;

// ========== TOUCH TEST ==========
bool touchTestMode = false;

// ========== MENU SYSTEM ==========
MenuManager* menuManager = NULL;
bool menuActive = false;

// ========== SCREEN LAYOUT ==========
uint16_t screenW = 320;
uint16_t screenH = 480;
uint16_t dividerY;
uint16_t topMaxY;
uint16_t bottomMinY;
uint16_t bottomMaxY;

// ========== TOP SCREEN ==========
uint16_t topPosX = 0;
uint16_t topPosY = 0;
uint16_t topTextColor = 0xFFFF;  // White
uint16_t topBgColor = 0x0000;    // Black
bool topOpaqueText = false;
uint8_t topTextSize = 2;

// ========== BOTTOM SCREEN ==========
uint16_t bottomPosX = 0;
uint16_t bottomPosY = 0;
uint16_t bottomTextColor = 0x07E0;  // Green
uint16_t bottomBgColor = 0x0000;    // Black
bool bottomOpaqueText = false;
uint8_t bottomTextSize = 1;

// ========== COMMAND HANDLING ==========
String cmd = "";
bool cmdReady = false;
String fpgaBuffer = "";
unsigned long serialRxEnableMs = 0;

// ========== FPGA SETTINGS ==========
uint8_t activeFpga = 1;  // 1, 2, or 3
unsigned long fpga1Baud = 9600;
unsigned long fpga2Baud = 9600;
unsigned long fpga3Baud = 9600;
uint8_t fpga1StopBits = 1;
uint8_t fpga2StopBits = 1;
uint8_t fpga3StopBits = 1;

FPGARxMode fpga1RxMode = FPGA_RX_TEXT;
FPGARxMode fpga2RxMode = FPGA_RX_TEXT;
FPGARxMode fpga3RxMode = FPGA_RX_TEXT;

FPGAParseMode fpga1ParseMode = PARSE_NONE;
FPGAParseMode fpga2ParseMode = PARSE_NONE;
FPGAParseMode fpga3ParseMode = PARSE_NONE;

TermMode bypassTerm = TERM_LF;
uint8_t customTermByte = 0xFF;

// ========== BUTTONS ==========
Button buttons[NUM_BUTTONS];
bool buttonsVisible = false;
uint16_t buttonTextY;

// ========== TOUCH DEBOUNCE ==========
unsigned long lastTouch = 0;
unsigned long lastViewToggleTouch = 0;

// ========== GPIO ==========
const uint8_t gpioPins[NUM_GPIO] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45};
GPIOMode gpioModes[NUM_GPIO];
uint8_t gpioStates[NUM_GPIO];
uint8_t gpioPrevStates[NUM_GPIO];
bool gpioEventEnable[NUM_GPIO];
bool gpioEventRising[NUM_GPIO];
bool gpioEventFalling[NUM_GPIO];
GPIOEvent gpioEventQueue[GPIO_EVENT_QUEUE_SIZE];
uint8_t gpioEventHead = 0;
uint8_t gpioEventTail = 0;

// ========== SPI ==========
int8_t spiCSPin = -1;
uint32_t spiSpeed = 1000000;
uint8_t spiMode = SPI_MODE0;

// ========== ANALOG ==========
const uint8_t analogPins[8] = {A8, A9, A10, A11, A12, A13, A14, A15};
uint8_t analogAvgSamples = 1;
AnalogRefMode analogRefMode = AREF_DEFAULT;

// ========== DATA LOGGING ==========
uint16_t logStartAddr = 2000;
uint16_t logEndAddr = 4095;
bool loggingActive = false;
unsigned long logInterval = 1000;
unsigned long lastLogTime = 0;
uint16_t logWriteAddr = 2000;
uint16_t logEntryCount = 0;
uint8_t logSource = 0;

// ========== STATISTICS ==========
Stats analogStats[8];
Threshold thresholds[8];

// ========== MACROS ==========
Macro macros[MAX_MACROS];

// ========== SMART RULES ==========
SmartRule smartRules[MAX_RULES];
bool rulesEnabled = true;
unsigned long ruleEvalLastMs = 0;
unsigned long ruleFireCount = 0;
const uint16_t RULE_EVAL_INTERVAL_MS = 100;

// ========== EVENT LOG ==========
EventLogEntry eventLog[EVENT_LOG_CAPACITY];
uint8_t eventLogHead = 0;
uint8_t eventLogCount = 0;

// ========== MACRO DEPTH ==========
uint8_t macroRunDepth = 0;
const uint8_t MAX_MACRO_DEPTH = 2;

// ========== VIEW MODE ==========
ViewMode viewMode = VIEW_SPLIT;

// ========== PROTOCOL ==========
bool protoAddrMode = false;
bool protoFrameMode = false;
bool bridgeMode = false;
uint8_t localAddress = 0x01;
const uint8_t FRAME_SYNC = 0xAA;
const uint8_t FRAME_MAX_PAYLOAD = 64;
FrameRxState frameRx[4];

// ========== RUNTIME COUNTERS ==========
unsigned long fpgaRxBytes[4] = {0, 0, 0, 0};
unsigned long fpgaTxBytes[4] = {0, 0, 0, 0};
unsigned long fpgaFrameOk[4] = {0, 0, 0, 0};
unsigned long fpgaFrameErr[4] = {0, 0, 0, 0};
unsigned long fpgaDropped[4] = {0, 0, 0, 0};
unsigned long fpgaTimeouts[4] = {0, 0, 0, 0};
unsigned long fpgaLastActivityMs[4] = {0, 0, 0, 0};
unsigned long fpgaLastErrorMs[4] = {0, 0, 0, 0};
unsigned long gpioEventCount = 0;
unsigned long thresholdEventCount = 0;
unsigned long addrLocalHandled = 0;
unsigned long addrPassthrough = 0;

// ========== WATCH MODE ==========
bool watchEnabled = false;
bool linkIndicatorsEnabled = false;
String watchSource = "A8";
unsigned long watchIntervalMs = 1000;
unsigned long watchLastMs = 0;
uint8_t smartProfile = PROFILE_CUSTOM;

// ========== WATCHDOG ==========
bool wdtEnabled = false;
uint16_t wdtTimeoutMs = 1000;
uint8_t bootResetCause = 0;
char lastWhy[40] = "BOOT";
unsigned long lastWhyMs = 0;

// ========== ESCALATION & FAILSAFE ==========
EscalationPolicy escalation = {false, (uint8_t)(ESC_METRIC_ERR | ESC_METRIC_TIMEOUT), 3, 2000, 5000, RULE_ACT_ALERT, 0, 0, 0, 0, 0, 0, 0};
FailsafePolicy failsafe = {false, false, 4, 2000, 8000, 0, 0, 0, 0, 0, 0, false, false, false, false};

// ========== EVENT HOOKS ==========
EventHook gpioHooks[NUM_GPIO];
EventHook thresholdHooks[8];

// ========== EEPROM PROTECTION ==========
ProtectZone protectZones[MAX_PROTECT_ZONES] = {
  {0, 99, true},      // Zone 0: Config area (0-99) - protected by default
  {100, 299, true},   // Zone 1: FPGA dedicated space (200 bytes)
  {300, 499, true},   // Zone 2: User settings (200 bytes)
  {0, 0, false}       // Zone 3: User-defined
};

// ========== WAVEFORM ==========
int8_t wavePin = -1;
uint8_t waveType = 0;
uint16_t waveFreq = 1000;
uint8_t waveDuty = 128;
bool waveActive = false;
unsigned long waveLastUpdate = 0;
uint8_t wavePhase = 0;

// Sine lookup table (32 values, PROGMEM)
const uint8_t PROGMEM sineLUT[32] = {
  128, 152, 176, 198, 217, 233, 245, 253,
  255, 253, 245, 233, 217, 198, 176, 152,
  128, 103,  79,  57,  38,  22,  10,   2,
    0,   2,  10,  22,  38,  57,  79, 103
};

// ========== FREQUENCY MONITOR ==========
int8_t freqMonPin = -1;
unsigned long freqMonDuration = 1000;
unsigned long freqMonLastUpdate = 0;
volatile unsigned long pulseCount = 0;
bool freqMonActive = false;

// ========== PWM TRACKING ==========
bool pwmPinActive[47] = {false};

// ========== BUTTON CONFIGS ==========
ButtonConfig btnUp    = {2, {0x55, 0x50}};  // Default: "UP"
ButtonConfig btnDown  = {2, {0x44, 0x4E}};  // Default: "DN"
ButtonConfig btnLeft  = {2, {0x4C, 0x54}};  // Default: "LT"
ButtonConfig btnRight = {2, {0x52, 0x54}};  // Default: "RT"
