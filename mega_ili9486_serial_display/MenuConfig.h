// MenuConfig.h - Menu Structure and FPGA Command Definitions
// Defines the menu hierarchy and FPGA command mappings

#ifndef MENU_CONFIG_H
#define MENU_CONFIG_H

#include "MenuSystem.h"

// 64-bit register structure for easy parsing
struct Register64 {
  union {
    uint64_t value;           // Full 64-bit value
    uint8_t bytes[8];         // Individual bytes
    struct {
      uint32_t low;           // Lower 32 bits
      uint32_t high;          // Upper 32 bits
    };
  };
};

// FPGA Command byte sequences
// Control Register commands (0x01 0x00 through 0x01 0x07)
extern uint8_t CMD_CONTROL_REG[8][2];

// Status Register commands (0x02 0x00 through 0x02 0x07)
extern uint8_t CMD_STATUS_REG[8][2];

// Temperature commands
extern uint8_t CMD_TEMP_READ[];
extern uint8_t CMD_TEMP_ALL[];

// Firmware Info commands
extern uint8_t CMD_FW_VERSION[];
extern uint8_t CMD_FW_BUILD[];
extern uint8_t CMD_FW_ID[];

// Register interpretation functions (customize these for your FPGA)
void parseControlReg0(Register64* reg, MCUFRIEND_kbv* tft);
void parseControlReg1(Register64* reg, MCUFRIEND_kbv* tft);
void parseControlReg2(Register64* reg, MCUFRIEND_kbv* tft);
void parseControlReg3(Register64* reg, MCUFRIEND_kbv* tft);
void parseControlReg4(Register64* reg, MCUFRIEND_kbv* tft);
void parseControlReg5(Register64* reg, MCUFRIEND_kbv* tft);
void parseControlReg6(Register64* reg, MCUFRIEND_kbv* tft);
void parseControlReg7(Register64* reg, MCUFRIEND_kbv* tft);

void parseStatusReg0(Register64* reg, MCUFRIEND_kbv* tft);
void parseStatusReg1(Register64* reg, MCUFRIEND_kbv* tft);
void parseStatusReg2(Register64* reg, MCUFRIEND_kbv* tft);
void parseStatusReg3(Register64* reg, MCUFRIEND_kbv* tft);
void parseStatusReg4(Register64* reg, MCUFRIEND_kbv* tft);
void parseStatusReg5(Register64* reg, MCUFRIEND_kbv* tft);
void parseStatusReg6(Register64* reg, MCUFRIEND_kbv* tft);
void parseStatusReg7(Register64* reg, MCUFRIEND_kbv* tft);

// Helper function to read 64-bit register from FPGA
bool readFPGA64BitRegister(uint8_t* cmdBytes, uint8_t cmdLen, Register64* result);

// Helper function to display register on screen
void displayRegisterOnScreen(const char* title, Register64* reg, void (*parseFunc)(Register64*, MCUFRIEND_kbv*));

// Menu action callbacks
void onControlReg(MenuItem* item);  // Handles all control registers
void onStatusReg(MenuItem* item);   // Handles all status registers
void onTempRead(MenuItem* item);
void onTempReadAll(MenuItem* item);
void onFWVersion(MenuItem* item);
void onFWBuild(MenuItem* item);
void onFWID(MenuItem* item);
void onCustomBytes(MenuItem* item);
void onBackAction(MenuItem* item);

// Menu definitions
extern MenuItem controlMenuItems[];
extern MenuItem statusMenuItems[];
extern MenuItem tempMenuItems[];
extern MenuItem fwMenuItems[];
extern MenuItem mainMenuItems[];

extern Menu controlMenu;
extern Menu statusMenu;
extern Menu tempMenu;
extern Menu fwMenu;
extern Menu mainMenu;

// Initialize all menus
void initMenus();

#endif // MENU_CONFIG_H
