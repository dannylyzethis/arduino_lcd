// MenuConfig.h - Menu Structure and FPGA Command Definitions
// Defines the menu hierarchy and FPGA command mappings

#ifndef MENU_CONFIG_H
#define MENU_CONFIG_H

#include "MenuSystem.h"

// FPGA Command byte sequences
// Control Register commands
extern uint8_t CMD_CONTROL_READ[];
extern uint8_t CMD_CONTROL_WRITE[];

// Status Register commands
extern uint8_t CMD_STATUS_READ[];
extern uint8_t CMD_STATUS_DETAIL[];

// Temperature commands
extern uint8_t CMD_TEMP_READ[];
extern uint8_t CMD_TEMP_ALL[];

// Firmware Info commands
extern uint8_t CMD_FW_VERSION[];
extern uint8_t CMD_FW_BUILD[];
extern uint8_t CMD_FW_ID[];

// Menu action callbacks
void onControlRegRead(MenuItem* item);
void onControlRegWrite(MenuItem* item);
void onStatusRegRead(MenuItem* item);
void onStatusDetail(MenuItem* item);
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
