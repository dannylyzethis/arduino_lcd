// MenuConfig.cpp - Menu Structure Implementation and FPGA Handlers

#include "MenuConfig.h"

// External reference to global variables from main sketch
extern HardwareSerial& getFPGA();
extern void sendHexBytes(uint8_t* data, uint8_t len);
extern void readFPGAResponse(uint8_t expectBytes, uint16_t timeout);
extern void applyTermination(HardwareSerial& fpga);

// FPGA Command byte sequences
// These are examples - customize for your FPGA protocol
uint8_t CMD_CONTROL_READ[] = {0x01, 0x00};        // Read control register
uint8_t CMD_CONTROL_WRITE[] = {0x01, 0x01, 0x00}; // Write control register
uint8_t CMD_STATUS_READ[] = {0x02, 0x00};         // Read status register
uint8_t CMD_STATUS_DETAIL[] = {0x02, 0x01};       // Read detailed status
uint8_t CMD_TEMP_READ[] = {0x03, 0x00};           // Read temperature
uint8_t CMD_TEMP_ALL[] = {0x03, 0xFF};            // Read all temp sensors
uint8_t CMD_FW_VERSION[] = {0x10, 0x00};          // Read FW version
uint8_t CMD_FW_BUILD[] = {0x10, 0x01};            // Read build number
uint8_t CMD_FW_ID[] = {0x10, 0x02};               // Read FW ID

// ========== Menu Action Callbacks ==========

void onControlRegRead(MenuItem* item) {
  Serial.println(F("=== Control Register Read ==="));

  HardwareSerial& fpga = getFPGA();

  // Send read command
  for (uint8_t i = 0; i < sizeof(CMD_CONTROL_READ); i++) {
    fpga.write(CMD_CONTROL_READ[i]);
  }
  applyTermination(fpga);

  // Wait for response (expecting 2 bytes)
  delay(50);
  readFPGAResponse(2, 1000);

  Serial.println(F("============================"));
}

void onControlRegWrite(MenuItem* item) {
  Serial.println(F("=== Control Register Write ==="));
  Serial.println(F("Enter value (0-255):"));

  // TODO: Implement value input via menu or serial
  // For now, write a default value
  uint8_t value = 0x00;

  HardwareSerial& fpga = getFPGA();
  fpga.write(CMD_CONTROL_WRITE[0]);
  fpga.write(CMD_CONTROL_WRITE[1]);
  fpga.write(value);
  applyTermination(fpga);

  Serial.print(F("Written: 0x"));
  Serial.println(value, HEX);
  Serial.println(F("============================="));
}

void onStatusRegRead(MenuItem* item) {
  Serial.println(F("=== Status Register Read ==="));

  HardwareSerial& fpga = getFPGA();

  // Send status read command
  for (uint8_t i = 0; i < sizeof(CMD_STATUS_READ); i++) {
    fpga.write(CMD_STATUS_READ[i]);
  }
  applyTermination(fpga);

  // Wait for response
  delay(50);
  readFPGAResponse(4, 1000);  // Expecting 4 bytes

  Serial.println(F("==========================="));
}

void onStatusDetail(MenuItem* item) {
  Serial.println(F("=== Detailed Status ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_STATUS_DETAIL); i++) {
    fpga.write(CMD_STATUS_DETAIL[i]);
  }
  applyTermination(fpga);

  delay(50);
  readFPGAResponse(8, 1000);  // Expecting more data

  Serial.println(F("======================"));
}

void onTempRead(MenuItem* item) {
  Serial.println(F("=== Temperature Read ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_TEMP_READ); i++) {
    fpga.write(CMD_TEMP_READ[i]);
  }
  applyTermination(fpga);

  delay(50);
  readFPGAResponse(2, 1000);

  Serial.println(F("======================="));
}

void onTempReadAll(MenuItem* item) {
  Serial.println(F("=== All Temperatures ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_TEMP_ALL); i++) {
    fpga.write(CMD_TEMP_ALL[i]);
  }
  applyTermination(fpga);

  delay(100);
  readFPGAResponse(16, 1000);  // Multiple sensors

  Serial.println(F("======================="));
}

void onFWVersion(MenuItem* item) {
  Serial.println(F("=== Firmware Version ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_FW_VERSION); i++) {
    fpga.write(CMD_FW_VERSION[i]);
  }
  applyTermination(fpga);

  delay(50);
  readFPGAResponse(4, 1000);

  Serial.println(F("======================="));
}

void onFWBuild(MenuItem* item) {
  Serial.println(F("=== Build Number ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_FW_BUILD); i++) {
    fpga.write(CMD_FW_BUILD[i]);
  }
  applyTermination(fpga);

  delay(50);
  readFPGAResponse(4, 1000);

  Serial.println(F("==================="));
}

void onFWID(MenuItem* item) {
  Serial.println(F("=== Firmware ID ==="));

  HardwareSerial& fpga = getFPGA();

  for (uint8_t i = 0; i < sizeof(CMD_FW_ID); i++) {
    fpga.write(CMD_FW_ID[i]);
  }
  applyTermination(fpga);

  delay(50);
  readFPGAResponse(8, 1000);

  Serial.println(F("=================="));
}

void onCustomBytes(MenuItem* item) {
  Serial.println(F("=== Custom Bytes ==="));
  Serial.println(F("Use #FPGABYTES command"));
  Serial.println(F("==================="));
}

void onBackAction(MenuItem* item) {
  // Handled by menu system
}

// ========== Menu Item Definitions ==========

// Control Register Menu Items
MenuItem controlMenuItems[] = {
  {"Read Control Reg", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlRegRead, NULL, 0, true, false},
  {"Write Control Reg", MENU_TYPE_ACTION, ACTION_CONTROL_REG, NULL, onControlRegWrite, NULL, 0, true, false},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false}
};

// Status Register Menu Items
MenuItem statusMenuItems[] = {
  {"Read Status", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusRegRead, NULL, 0, true, false},
  {"Detailed Status", MENU_TYPE_ACTION, ACTION_STATUS_REG, NULL, onStatusDetail, NULL, 0, true, false},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false}
};

// Temperature Menu Items
MenuItem tempMenuItems[] = {
  {"Read Temperature", MENU_TYPE_ACTION, ACTION_TEMPERATURE, NULL, onTempRead, NULL, 0, true, false},
  {"All Sensors", MENU_TYPE_ACTION, ACTION_TEMPERATURE, NULL, onTempReadAll, NULL, 0, true, false},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false}
};

// Firmware Info Menu Items
MenuItem fwMenuItems[] = {
  {"FW Version", MENU_TYPE_ACTION, ACTION_FW_INFO, NULL, onFWVersion, NULL, 0, true, false},
  {"Build Number", MENU_TYPE_ACTION, ACTION_FW_INFO, NULL, onFWBuild, NULL, 0, true, false},
  {"Firmware ID", MENU_TYPE_ACTION, ACTION_FW_INFO, NULL, onFWID, NULL, 0, true, false},
  {"< Back", MENU_TYPE_ACTION, ACTION_BACK, NULL, NULL, NULL, 0, true, false}
};

// Main Menu Items (will be filled in initMenus)
MenuItem mainMenuItems[] = {
  {"Control Register", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false},
  {"Status Register", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false},
  {"Temperature", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false},
  {"Firmware Info", MENU_TYPE_SUBMENU, ACTION_NONE, NULL, NULL, NULL, 0, true, false},
  {"Custom Bytes", MENU_TYPE_ACTION, ACTION_CUSTOM_BYTES, NULL, onCustomBytes, NULL, 0, true, false}
};

// Menu structures
Menu controlMenu;
Menu statusMenu;
Menu tempMenu;
Menu fwMenu;
Menu mainMenu;

void initMenus() {
  // Initialize submenus first
  controlMenu.title = "Control Register";
  controlMenu.items = controlMenuItems;
  controlMenu.itemCount = sizeof(controlMenuItems) / sizeof(MenuItem);
  controlMenu.parent = &mainMenu;
  controlMenu.selectedIndex = 0;
  controlMenu.scrollOffset = 0;

  statusMenu.title = "Status Register";
  statusMenu.items = statusMenuItems;
  statusMenu.itemCount = sizeof(statusMenuItems) / sizeof(MenuItem);
  statusMenu.parent = &mainMenu;
  statusMenu.selectedIndex = 0;
  statusMenu.scrollOffset = 0;

  tempMenu.title = "Temperature";
  tempMenu.items = tempMenuItems;
  tempMenu.itemCount = sizeof(tempMenuItems) / sizeof(MenuItem);
  tempMenu.parent = &mainMenu;
  tempMenu.selectedIndex = 0;
  tempMenu.scrollOffset = 0;

  fwMenu.title = "Firmware Info";
  fwMenu.items = fwMenuItems;
  fwMenu.itemCount = sizeof(fwMenuItems) / sizeof(MenuItem);
  fwMenu.parent = &mainMenu;
  fwMenu.selectedIndex = 0;
  fwMenu.scrollOffset = 0;

  // Link submenus to main menu items
  mainMenuItems[0].submenu = &controlMenu;
  mainMenuItems[1].submenu = &statusMenu;
  mainMenuItems[2].submenu = &tempMenu;
  mainMenuItems[3].submenu = &fwMenu;

  // Initialize main menu
  mainMenu.title = "FPGA Control";
  mainMenu.items = mainMenuItems;
  mainMenu.itemCount = sizeof(mainMenuItems) / sizeof(MenuItem);
  mainMenu.parent = NULL;  // Root menu
  mainMenu.selectedIndex = 0;
  mainMenu.scrollOffset = 0;

  // Set back button callbacks
  controlMenuItems[2].callback = [](MenuItem* item) {
    // Will be handled by menu manager
  };
  statusMenuItems[2].callback = [](MenuItem* item) {
    // Will be handled by menu manager
  };
  tempMenuItems[2].callback = [](MenuItem* item) {
    // Will be handled by menu manager
  };
  fwMenuItems[3].callback = [](MenuItem* item) {
    // Will be handled by menu manager
  };
}
