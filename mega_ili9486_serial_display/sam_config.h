#ifndef SAM_CONFIG_H
#define SAM_CONFIG_H

#include <Arduino.h>
#include <avr/wdt.h>

// ========== FIRMWARE IDENTITY ==========
#define SAM_FIRMWARE_NAME "SAM Smart Arduino Monitor"
#define SAM_FIRMWARE_VERSION "1.2.0"

// ========== HARDWARE PIN DEFINES ==========
#define YP A3  // Y+ is on Analog3
#define XM A2  // X- is on Analog2
#define YM 9   // Y- is on Digital9
#define XP 8   // X+ is on Digital8

#define MINPRESSURE 10
#define MAXPRESSURE 1000

// ========== DISPLAY CONSTANTS ==========
#define BASE_CHAR_W 6
#define BASE_CHAR_H 8

// ========== GPIO CONSTANTS ==========
#define NUM_GPIO 24
#define GPIO_EVENT_QUEUE_SIZE 16

// ========== TOUCH ==========
#define TOUCH_DEBOUNCE 200  // milliseconds

// ========== BUTTON CONSTANTS ==========
#define NUM_BUTTONS 4

// ========== DATA LOGGING ==========
#define LOG_ENTRY_SIZE 8  // Timestamp(4) + Value(2) + Source(1) + Flags(1)

// ========== MACRO CONSTANTS ==========
#define MAX_MACROS 3
#define MAX_MACRO_CMDS 5

// ========== RULE CONSTANTS ==========
#define MAX_RULES 6

// ========== EEPROM PROTECTION ==========
#define MAX_PROTECT_ZONES 4

// ========== EVENT LOG ==========
#define EVENT_LOG_CAPACITY 6
#define EVENT_LOG_MSG_LEN 22

// ========== PROTOCOL FRAME ==========
// FRAME_SYNC and FRAME_MAX_PAYLOAD are const variables in sam_globals

// ========== MACRO DEPTH ==========
// MAX_MACRO_DEPTH is a const variable in sam_globals

// ========== RULE EVAL INTERVAL ==========
// RULE_EVAL_INTERVAL_MS is a const variable in sam_globals

// ========== EEPROM CONFIGURATION MAP ==========
#define CONFIG_MAGIC 0xA5
#define CONFIG_VERSION 1

#define ADDR_MAGIC 0
#define ADDR_VERSION 1
#define ADDR_ROTATION 2
#define ADDR_FPGA_SEL 3
#define ADDR_FPGA1_BAUD 4         // 3 bytes (4,5,6)
#define ADDR_FPGA2_BAUD 7         // 3 bytes (7,8,9)
#define ADDR_FPGA3_BAUD 10        // 3 bytes (10,11,12)
#define ADDR_FPGA1_TERM 13
#define ADDR_FPGA2_TERM 14
#define ADDR_FPGA3_TERM 15
#define ADDR_FPGA1_STOP 16
#define ADDR_FPGA2_STOP 17
#define ADDR_FPGA3_STOP 18
#define ADDR_FPGA1_RX 19
#define ADDR_FPGA2_RX 20
#define ADDR_FPGA3_RX 21
#define ADDR_FPGA1_PARSE 22
#define ADDR_FPGA2_PARSE 23
#define ADDR_FPGA3_PARSE 24
// 25-31: Reserved
#define ADDR_BTN_UP 32      // 16 bytes: [0]=length, [1-15]=data
#define ADDR_BTN_DOWN 48    // 16 bytes
#define ADDR_BTN_LEFT 64    // 16 bytes
#define ADDR_BTN_RIGHT 80   // 16 bytes
// Extended config
#define ADDR_VIEW_MODE 96   // 0=split, 1=full
#define ADDR_PROTO_FLAGS 97 // bit0=addr mode, bit1=frame mode, bit2=bridge
#define ADDR_PROTO_ADDR 98  // local address byte
// 99: Reserved

#define ADDR_FPGA_ZONE 100    // FPGA dedicated space (200 bytes)
#define ADDR_USER_ZONE 300    // User settings (200 bytes)

// ========== ESCALATION METRIC FLAGS ==========
#define ESC_METRIC_ERR 0x01
#define ESC_METRIC_TIMEOUT 0x02
#define ESC_METRIC_RULE 0x04

// ========== ENUMERATIONS ==========

enum FPGARxMode {
  FPGA_RX_TEXT,    // Parse as text (printable chars, line endings)
  FPGA_RX_BINARY   // Raw bytes displayed as hex
};

enum FPGAParseMode {
  PARSE_NONE,      // No special parsing (default hex display)
  PARSE_ASCII,     // Show as ASCII characters with [XX] for non-printable
  PARSE_DEC,       // Show as decimal numbers
  PARSE_MIXED      // Show both hex and ASCII: "0x41 'A'"
};

enum TermMode {
  TERM_NONE,   // No termination
  TERM_LF,     // \n (0x0A)
  TERM_CR,     // \r (0x0D)
  TERM_CRLF,   // \r\n (0x0D 0x0A)
  TERM_CUSTOM  // Custom byte value
};

enum GPIOMode {
  GPIO_INPUT,
  GPIO_INPUT_PULLUP,
  GPIO_OUTPUT
};

enum RuleSourceType {
  RULE_SRC_ANALOG = 0,
  RULE_SRC_GPIO = 1
};

enum RuleOp {
  RULE_OP_GT = 0,
  RULE_OP_LT,
  RULE_OP_GE,
  RULE_OP_LE,
  RULE_OP_EQ,
  RULE_OP_NE
};

enum RuleActionType {
  RULE_ACT_MACRO = 0,
  RULE_ACT_LOG,
  RULE_ACT_ALERT
};

enum SmartProfileMode {
  PROFILE_CUSTOM = 0,
  PROFILE_SAFE,
  PROFILE_BALANCED,
  PROFILE_PERF
};

enum EventLogType {
  EVLOG_RULE = 0,
  EVLOG_GPIO,
  EVLOG_THR,
  EVLOG_ERR,
  EVLOG_INFO
};

enum ViewMode {
  VIEW_SPLIT = 0,
  VIEW_FULL = 1
};

enum AnalogRefMode {
  AREF_DEFAULT = 0,
  AREF_INTERNAL = 1
};

// ========== STRUCT DEFINITIONS ==========

struct Button {
  uint16_t x, y, w, h;
  const char* label;
  uint8_t cmdBytes[15];   // Bytes to send to FPGA (up to 15 bytes)
  uint8_t cmdLen;         // Number of bytes in command
  uint16_t color;
};

struct ButtonConfig {
  uint8_t length;      // Number of bytes to send (0-15)
  uint8_t data[15];    // Button data bytes
};

struct GPIOEvent {
  uint8_t pin;
  uint8_t newState;
  bool rising;  // true=rising, false=falling
};

struct Stats {
  uint16_t minVal;
  uint16_t maxVal;
  uint32_t sum;
  uint32_t count;
  bool active;
};

struct Threshold {
  uint16_t lowThreshold;
  uint16_t highThreshold;
  bool active;
  bool alertState;  // true if currently in alert state
};

struct Macro {
  String commands[MAX_MACRO_CMDS];
  uint8_t cmdCount;
  bool defined;
};

struct SmartRule {
  bool enabled;
  RuleSourceType srcType;
  uint8_t srcIndex;        // Analog: 0..7 => A8..A15, GPIO: 0..7 => pins 22..29 index
  RuleOp op;
  int16_t value;
  uint16_t holdMs;         // Condition must remain true for this duration
  RuleActionType action;
  uint8_t actionArg;       // Used for MACRO id
  uint16_t cooldownMs;     // Minimum time between firings
  unsigned long sinceMs;   // When condition became true
  unsigned long lastFireMs;
};

struct EventLogEntry {
  unsigned long ms;
  uint8_t type;
  char msg[EVENT_LOG_MSG_LEN];
};

struct EscalationPolicy {
  bool enabled;
  uint8_t metricMask;
  uint16_t threshold;
  uint16_t windowMs;
  uint16_t cooldownMs;
  RuleActionType action;
  uint8_t actionArg;
  unsigned long lastEvalMs;
  unsigned long lastErrTotal;
  unsigned long lastTimeoutTotal;
  unsigned long lastRuleTotal;
  unsigned long lastFireMs;
  unsigned long fireCount;
};

struct FailsafePolicy {
  bool enabled;
  bool active;
  uint16_t threshold;
  uint16_t windowMs;
  uint16_t holdMs;
  unsigned long lastEvalMs;
  unsigned long lastErrTotal;
  unsigned long lastTimeoutTotal;
  unsigned long lastEscTotal;
  unsigned long enterMs;
  unsigned long fireCount;
  bool prevBridge;
  bool prevWatch;
  bool prevLinks;
  bool prevRules;
};

struct EventHook {
  bool enabled;
  uint8_t macroId;
  uint8_t modeMask;
  uint16_t cooldownMs;
  unsigned long lastTriggerMs;
};

struct ProtectZone {
  uint16_t startAddr;
  uint16_t endAddr;
  bool enabled;
};

struct FrameRxState {
  uint8_t stage;     // 0=sync,1=len,2=payload,3=crc
  uint8_t len;
  uint8_t idx;
  uint8_t crc;
  uint8_t payload[64]; // FRAME_MAX_PAYLOAD = 64
};

#endif // SAM_CONFIG_H
