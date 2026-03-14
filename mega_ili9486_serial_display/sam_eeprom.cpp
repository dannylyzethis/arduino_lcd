#include "sam_eeprom.h"
#include "sam_globals.h"
#include <EEPROM.h>
#include "sam_display.h"
#include "sam_touch.h"
#include "sam_commands.h"

bool isProtected(uint16_t addr) {
  for (uint8_t i = 0; i < MAX_PROTECT_ZONES; i++) {
    if (protectZones[i].enabled) {
      if (addr >= protectZones[i].startAddr && addr <= protectZones[i].endAddr) {
        return true;
      }
    }
  }
  return false;
}

// Save configuration to EEPROM
void saveConfig() {
  // Magic byte and version
  EEPROM.write(ADDR_MAGIC, CONFIG_MAGIC);
  EEPROM.write(ADDR_VERSION, CONFIG_VERSION);

  // Display settings
  EEPROM.write(ADDR_ROTATION, tft.getRotation());

  // FPGA settings
  EEPROM.write(ADDR_FPGA_SEL, activeFpga);

  // Store baud rates as 3-byte values (max 16777215)
  EEPROM.write(ADDR_FPGA1_BAUD, (fpga1Baud >> 16) & 0xFF);
  EEPROM.write(ADDR_FPGA1_BAUD + 1, (fpga1Baud >> 8) & 0xFF);
  EEPROM.write(ADDR_FPGA1_BAUD + 2, fpga1Baud & 0xFF);

  EEPROM.write(ADDR_FPGA2_BAUD, (fpga2Baud >> 16) & 0xFF);
  EEPROM.write(ADDR_FPGA2_BAUD + 1, (fpga2Baud >> 8) & 0xFF);
  EEPROM.write(ADDR_FPGA2_BAUD + 2, fpga2Baud & 0xFF);

  EEPROM.write(ADDR_FPGA3_BAUD, (fpga3Baud >> 16) & 0xFF);
  EEPROM.write(ADDR_FPGA3_BAUD + 1, (fpga3Baud >> 8) & 0xFF);
  EEPROM.write(ADDR_FPGA3_BAUD + 2, fpga3Baud & 0xFF);

  EEPROM.write(ADDR_FPGA1_TERM, (uint8_t)bypassTerm);  // Store term mode
  EEPROM.write(ADDR_FPGA2_TERM, customTermByte);  // Store custom term byte
  EEPROM.write(ADDR_FPGA3_TERM, 0);  // Reserved

  EEPROM.write(ADDR_FPGA1_STOP, fpga1StopBits);
  EEPROM.write(ADDR_FPGA2_STOP, fpga2StopBits);
  EEPROM.write(ADDR_FPGA3_STOP, fpga3StopBits);

  EEPROM.write(ADDR_FPGA1_RX, (uint8_t)fpga1RxMode);
  EEPROM.write(ADDR_FPGA2_RX, (uint8_t)fpga2RxMode);
  EEPROM.write(ADDR_FPGA3_RX, (uint8_t)fpga3RxMode);

  EEPROM.write(ADDR_FPGA1_PARSE, (uint8_t)fpga1ParseMode);
  EEPROM.write(ADDR_FPGA2_PARSE, (uint8_t)fpga2ParseMode);
  EEPROM.write(ADDR_FPGA3_PARSE, (uint8_t)fpga3ParseMode);

  // Button configurations
  EEPROM.write(ADDR_BTN_UP, btnUp.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_UP + 1 + i, btnUp.data[i]);
  }

  EEPROM.write(ADDR_BTN_DOWN, btnDown.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_DOWN + 1 + i, btnDown.data[i]);
  }

  EEPROM.write(ADDR_BTN_LEFT, btnLeft.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_LEFT + 1 + i, btnLeft.data[i]);
  }

  EEPROM.write(ADDR_BTN_RIGHT, btnRight.length);
  for (uint8_t i = 0; i < 15; i++) {
    EEPROM.write(ADDR_BTN_RIGHT + 1 + i, btnRight.data[i]);
  }

  EEPROM.write(ADDR_VIEW_MODE, (uint8_t)viewMode);
  uint8_t protoFlags = 0;
  if (protoAddrMode) protoFlags |= 0x01;
  if (protoFrameMode) protoFlags |= 0x02;
  if (bridgeMode) protoFlags |= 0x04;
  EEPROM.write(ADDR_PROTO_FLAGS, protoFlags);
  EEPROM.write(ADDR_PROTO_ADDR, localAddress);
}

// Load configuration from EEPROM
bool loadConfig() {
  // Check magic byte
  if (EEPROM.read(ADDR_MAGIC) != CONFIG_MAGIC) {
    return false;  // No valid config
  }

  // Check version
  uint8_t ver = EEPROM.read(ADDR_VERSION);
  if (ver != CONFIG_VERSION) {
    return false;  // Version mismatch
  }

  // Load display settings (will be applied after tft.begin)
  uint8_t rot = EEPROM.read(ADDR_ROTATION);
  if (rot > 3) rot = 0;
  // Note: rotation applied in setup() after tft.begin()

  // Load FPGA settings
  activeFpga = EEPROM.read(ADDR_FPGA_SEL);
  if (activeFpga < 1 || activeFpga > 3) activeFpga = 1;

  // Load baud rates (3 bytes each)
  fpga1Baud = ((unsigned long)EEPROM.read(ADDR_FPGA1_BAUD) << 16) |
              ((unsigned long)EEPROM.read(ADDR_FPGA1_BAUD + 1) << 8) |
              EEPROM.read(ADDR_FPGA1_BAUD + 2);
  if (fpga1Baud == 0 || fpga1Baud > 115200) fpga1Baud = 9600;

  fpga2Baud = ((unsigned long)EEPROM.read(ADDR_FPGA2_BAUD) << 16) |
              ((unsigned long)EEPROM.read(ADDR_FPGA2_BAUD + 1) << 8) |
              EEPROM.read(ADDR_FPGA2_BAUD + 2);
  if (fpga2Baud == 0 || fpga2Baud > 115200) fpga2Baud = 9600;

  fpga3Baud = ((unsigned long)EEPROM.read(ADDR_FPGA3_BAUD) << 16) |
              ((unsigned long)EEPROM.read(ADDR_FPGA3_BAUD + 1) << 8) |
              EEPROM.read(ADDR_FPGA3_BAUD + 2);
  if (fpga3Baud == 0 || fpga3Baud > 115200) fpga3Baud = 9600;

  // Load termination settings
  bypassTerm = (TermMode)EEPROM.read(ADDR_FPGA1_TERM);
  customTermByte = EEPROM.read(ADDR_FPGA2_TERM);

  // Load stop bits
  fpga1StopBits = EEPROM.read(ADDR_FPGA1_STOP);
  fpga2StopBits = EEPROM.read(ADDR_FPGA2_STOP);
  fpga3StopBits = EEPROM.read(ADDR_FPGA3_STOP);
  if (fpga1StopBits < 1 || fpga1StopBits > 2) fpga1StopBits = 1;
  if (fpga2StopBits < 1 || fpga2StopBits > 2) fpga2StopBits = 1;
  if (fpga3StopBits < 1 || fpga3StopBits > 2) fpga3StopBits = 1;

  // Load RX modes
  fpga1RxMode = (FPGARxMode)EEPROM.read(ADDR_FPGA1_RX);
  fpga2RxMode = (FPGARxMode)EEPROM.read(ADDR_FPGA2_RX);
  fpga3RxMode = (FPGARxMode)EEPROM.read(ADDR_FPGA3_RX);

  // Load parse modes
  fpga1ParseMode = (FPGAParseMode)EEPROM.read(ADDR_FPGA1_PARSE);
  fpga2ParseMode = (FPGAParseMode)EEPROM.read(ADDR_FPGA2_PARSE);
  fpga3ParseMode = (FPGAParseMode)EEPROM.read(ADDR_FPGA3_PARSE);

  // Load button configurations
  btnUp.length = EEPROM.read(ADDR_BTN_UP);
  if (btnUp.length > 15) btnUp.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnUp.data[i] = EEPROM.read(ADDR_BTN_UP + 1 + i);
  }

  btnDown.length = EEPROM.read(ADDR_BTN_DOWN);
  if (btnDown.length > 15) btnDown.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnDown.data[i] = EEPROM.read(ADDR_BTN_DOWN + 1 + i);
  }

  btnLeft.length = EEPROM.read(ADDR_BTN_LEFT);
  if (btnLeft.length > 15) btnLeft.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnLeft.data[i] = EEPROM.read(ADDR_BTN_LEFT + 1 + i);
  }

  btnRight.length = EEPROM.read(ADDR_BTN_RIGHT);
  if (btnRight.length > 15) btnRight.length = 15;
  for (uint8_t i = 0; i < 15; i++) {
    btnRight.data[i] = EEPROM.read(ADDR_BTN_RIGHT + 1 + i);
  }

  uint8_t vm = EEPROM.read(ADDR_VIEW_MODE);
  viewMode = (vm == 1) ? VIEW_FULL : VIEW_SPLIT;

  uint8_t protoFlags = EEPROM.read(ADDR_PROTO_FLAGS);
  protoAddrMode = (protoFlags & 0x01) != 0;
  protoFrameMode = (protoFlags & 0x02) != 0;
  bridgeMode = (protoFlags & 0x04) != 0;
  localAddress = EEPROM.read(ADDR_PROTO_ADDR);
  if (localAddress == 0) localAddress = 0x01;

  return true;
}

void handleEEPROMRead(String params) {
  int addr = parseHexOrDec(params);

  if (addr < 0 || addr >= EEPROM.length()) {
    Serial.print(F("ERR:ADDR_RANGE (0-"));
    Serial.print(EEPROM.length() - 1);
    Serial.println(F(")"));
    return;
  }

  uint8_t value = EEPROM.read(addr);

  Serial.print(F("EEPROM["));
  Serial.print(addr);
  Serial.print(F("] = 0x"));
  if (value < 16) Serial.print("0");
  Serial.print(value, HEX);
  Serial.print(F(" ("));
  Serial.print(value);
  Serial.println(F(")"));
}

void handleEEPROMWrite(String params) {
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMWRITE <addr> <value>"));
    return;
  }

  int addr = parseHexOrDec(params.substring(0, sp));
  uint8_t value = parseHexOrDec(params.substring(sp + 1));

  if (addr < 0 || addr >= EEPROM.length()) {
    Serial.print(F("ERR:ADDR_RANGE (0-"));
    Serial.print(EEPROM.length() - 1);
    Serial.println(F(")"));
    return;
  }

  // Check write protection
  if (isProtected(addr)) {
    Serial.print(F("ERR:PROTECTED_ADDR "));
    Serial.println(addr);
    return;
  }

  EEPROM.write(addr, value);

  Serial.print(F("EEPROM["));
  Serial.print(addr);
  Serial.print(F("] = 0x"));
  if (value < 16) Serial.print("0");
  Serial.println(value, HEX);
}

void handleEEPROMDump(String params) {
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMDUMP <start> <end>"));
    return;
  }

  int startAddr = parseHexOrDec(params.substring(0, sp));
  int endAddr = parseHexOrDec(params.substring(sp + 1));

  if (startAddr < 0 || endAddr >= EEPROM.length() || startAddr > endAddr) {
    Serial.println(F("ERR:INVALID_RANGE"));
    return;
  }

  Serial.println(F("EEPROM Dump:"));
  Serial.println(F("Addr  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
  Serial.println(F("------+------------------------------------------------"));

  for (int addr = startAddr; addr <= endAddr; addr += 16) {
    // Print address
    if (addr < 0x1000) Serial.print("0");
    if (addr < 0x100) Serial.print("0");
    if (addr < 0x10) Serial.print("0");
    Serial.print(addr, HEX);
    Serial.print(F("  | "));

    // Print 16 bytes
    for (int i = 0; i < 16; i++) {
      int a = addr + i;
      if (a <= endAddr) {
        uint8_t b = EEPROM.read(a);
        if (b < 16) Serial.print("0");
        Serial.print(b, HEX);
      } else {
        Serial.print(F("  "));
      }
      Serial.print(F(" "));
    }
    Serial.println();
  }
}

void handleEEPROMClear() {
  Serial.print(F("Clearing EEPROM ("));
  Serial.print(EEPROM.length());
  Serial.println(F(" bytes)..."));

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);
    if (i % 256 == 0) {
      Serial.print(F("."));
    }
  }

  Serial.println();
  Serial.println(F("EEPROM cleared (all 0xFF)"));
}

void handleEEPROMProtect(String params) {
  // #EEPROMPROTECT <zone> <start> <end> - Configure protection zone
  // #EEPROMPROTECT <zone> OFF - Disable protection zone
  // #EEPROMPROTECT ? - Show all protection zones

  if (params == "?") {
    Serial.println(F("=== EEPROM PROTECTION ==="));
    for (uint8_t i = 0; i < MAX_PROTECT_ZONES; i++) {
      Serial.print(F("Zone "));
      Serial.print(i);
      Serial.print(F(": "));
      if (protectZones[i].enabled) {
        Serial.print(F("0x"));
        Serial.print(protectZones[i].startAddr, HEX);
        Serial.print(F(" - 0x"));
        Serial.println(protectZones[i].endAddr, HEX);
      } else {
        Serial.println(F("DISABLED"));
      }
    }
    return;
  }

  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMPROTECT <zone> <start> <end>"));
    return;
  }

  uint8_t zone = params.substring(0, sp).toInt();
  if (zone >= MAX_PROTECT_ZONES) {
    Serial.print(F("ERR:ZONE_RANGE (0-"));
    Serial.print(MAX_PROTECT_ZONES - 1);
    Serial.println(F(")"));
    return;
  }

  String rest = params.substring(sp + 1);
  rest.trim();

  if (rest.equalsIgnoreCase("OFF")) {
    protectZones[zone].enabled = false;
    Serial.print(F("Zone "));
    Serial.print(zone);
    Serial.println(F(" DISABLED"));
    return;
  }

  sp = rest.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #EEPROMPROTECT <zone> <start> <end>"));
    return;
  }

  uint16_t start = parseHexOrDec(rest.substring(0, sp));
  uint16_t end = parseHexOrDec(rest.substring(sp + 1));

  if (start >= EEPROM.length() || end >= EEPROM.length()) {
    Serial.println(F("ERR:ADDR_RANGE"));
    return;
  }

  if (start > end) {
    Serial.println(F("ERR:START > END"));
    return;
  }

  protectZones[zone].startAddr = start;
  protectZones[zone].endAddr = end;
  protectZones[zone].enabled = true;

  Serial.print(F("Zone "));
  Serial.print(zone);
  Serial.print(F(": 0x"));
  Serial.print(start, HEX);
  Serial.print(F(" - 0x"));
  Serial.println(end, HEX);
}

void handleConfigSave() {
  saveConfig();
  Serial.println(F("CONFIG_SAVED"));
}

void handleConfigLoad() {
  if (loadConfig()) {
    // Apply rotation
    uint8_t rot = EEPROM.read(ADDR_ROTATION);
    if (rot > 3) rot = 0;
    tft.setRotation(rot);

    // Update screen dimensions
    screenW = tft.width();
    screenH = tft.height();
    dividerY = screenH / 2;
    topMaxY = dividerY - 2;
    bottomMinY = dividerY + 2;
    bottomMaxY = screenH;

    // Reinitialize buttons with new config
    initButtons();

    Serial.println(F("CONFIG_LOADED"));
    Serial.print(F("Rotation: "));
    Serial.println(rot);
  } else {
    Serial.println(F("ERR:NO_CONFIG_FOUND"));
  }
}

void handleConfigReset() {
  // Reset display to defaults
  tft.setRotation(0);
  screenW = tft.width();
  screenH = tft.height();
  dividerY = screenH / 2;
  topMaxY = dividerY - 2;
  bottomMinY = dividerY + 2;
  bottomMaxY = screenH;

  // Reset FPGA defaults
  activeFpga = 1;
  fpga1Baud = 9600;
  fpga2Baud = 9600;
  fpga3Baud = 9600;

  // Reset button configs to defaults
  btnUp.length = 2;
  btnUp.data[0] = 0x55;
  btnUp.data[1] = 0x50;

  btnDown.length = 2;
  btnDown.data[0] = 0x44;
  btnDown.data[1] = 0x4E;

  btnLeft.length = 2;
  btnLeft.data[0] = 0x4C;
  btnLeft.data[1] = 0x54;

  btnRight.length = 2;
  btnRight.data[0] = 0x52;
  btnRight.data[1] = 0x54;

  // Reinitialize buttons
  initButtons();

  Serial.println(F("CONFIG_RESET"));
}

void handleConfigShow() {
  Serial.println(F("=== CONFIGURATION ==="));

  // Display settings
  Serial.print(F("Rotation: "));
  Serial.println(tft.getRotation());

  // FPGA settings
  Serial.print(F("FPGA Selected: "));
  Serial.println(activeFpga);
  Serial.print(F("FPGA1 Baud: "));
  Serial.println(fpga1Baud);
  Serial.print(F("FPGA2 Baud: "));
  Serial.println(fpga2Baud);
  Serial.print(F("FPGA3 Baud: "));
  Serial.println(fpga3Baud);

  // Button configurations
  Serial.println(F("Button Configs:"));
  Serial.print(F("  UP ["));
  Serial.print(btnUp.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnUp.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnUp.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnUp.data[i], HEX);
  }
  Serial.println();

  Serial.print(F("  DOWN ["));
  Serial.print(btnDown.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnDown.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnDown.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnDown.data[i], HEX);
  }
  Serial.println();

  Serial.print(F("  LEFT ["));
  Serial.print(btnLeft.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnLeft.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnLeft.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnLeft.data[i], HEX);
  }
  Serial.println();

  Serial.print(F("  RIGHT ["));
  Serial.print(btnRight.length);
  Serial.print(F("]: "));
  for (uint8_t i = 0; i < btnRight.length; i++) {
    if (i > 0) Serial.print(F(" "));
    Serial.print(F("0x"));
    if (btnRight.data[i] < 16) Serial.print(F("0"));
    Serial.print(btnRight.data[i], HEX);
  }
  Serial.println();
}

void handleBtnConfig(String params) {
  // #BTNCONFIG <btn> <len> <bytes...>
  // #BTNCONFIG <btn> ?

  int sp1 = params.indexOf(' ');
  if (sp1 == -1) {
    Serial.println(F("ERR:FORMAT #BTNCONFIG <UP|DOWN|LEFT|RIGHT> <len> <bytes...>"));
    return;
  }

  String btnName = params.substring(0, sp1);
  btnName.toUpperCase();

  ButtonConfig* targetBtn = NULL;
  if (btnName == "UP") targetBtn = &btnUp;
  else if (btnName == "DOWN") targetBtn = &btnDown;
  else if (btnName == "LEFT") targetBtn = &btnLeft;
  else if (btnName == "RIGHT") targetBtn = &btnRight;
  else {
    Serial.println(F("ERR:UNKNOWN_BTN (use UP, DOWN, LEFT, RIGHT)"));
    return;
  }

  String rest = params.substring(sp1 + 1);
  rest.trim();

  // Query mode
  if (rest == "?") {
    Serial.print(btnName);
    Serial.print(F(" ["));
    Serial.print(targetBtn->length);
    Serial.print(F("]: "));
    for (uint8_t i = 0; i < targetBtn->length; i++) {
      if (i > 0) Serial.print(F(" "));
      Serial.print(F("0x"));
      if (targetBtn->data[i] < 16) Serial.print(F("0"));
      Serial.print(targetBtn->data[i], HEX);
    }
    Serial.println();
    return;
  }

  // Configure mode
  int sp2 = rest.indexOf(' ');
  if (sp2 == -1) {
    Serial.println(F("ERR:FORMAT #BTNCONFIG <btn> <len> <bytes...>"));
    return;
  }

  uint8_t len = rest.substring(0, sp2).toInt();
  if (len == 0 || len > 15) {
    Serial.println(F("ERR:LENGTH (1-15)"));
    return;
  }

  // Parse bytes
  String bytesStr = rest.substring(sp2 + 1);
  targetBtn->length = len;

  int byteIdx = 0;
  int pos = 0;
  while (pos < bytesStr.length() && byteIdx < len) {
    // Skip whitespace
    while (pos < bytesStr.length() && bytesStr.charAt(pos) == ' ') pos++;
    if (pos >= bytesStr.length()) break;

    // Find next space or end
    int nextSpace = bytesStr.indexOf(' ', pos);
    if (nextSpace == -1) nextSpace = bytesStr.length();

    String byteStr = bytesStr.substring(pos, nextSpace);
    targetBtn->data[byteIdx++] = parseHexOrDec(byteStr);

    pos = nextSpace + 1;
  }

  if (byteIdx < len) {
    Serial.println(F("ERR:NOT_ENOUGH_BYTES"));
    return;
  }

  // Reinitialize buttons
  initButtons();

  Serial.print(btnName);
  Serial.println(F(" CONFIGURED"));
}

void handleFPGAWrite(String params) {
  // #FPGAWRITE <addr> <value...>
  // Write to FPGA zone (100-299)

  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #FPGAWRITE <addr> <value...>"));
    return;
  }

  uint16_t addr = parseHexOrDec(params.substring(0, sp));

  // Validate FPGA zone
  if (addr < ADDR_FPGA_ZONE || addr >= ADDR_USER_ZONE) {
    Serial.print(F("ERR:FPGA_ZONE ("));
    Serial.print(ADDR_FPGA_ZONE);
    Serial.print(F("-"));
    Serial.print(ADDR_USER_ZONE - 1);
    Serial.println(F(")"));
    return;
  }

  // Parse bytes to write
  String bytesStr = params.substring(sp + 1);
  int pos = 0;
  uint16_t writeAddr = addr;
  uint8_t bytesWritten = 0;

  while (pos < bytesStr.length() && writeAddr < ADDR_USER_ZONE) {
    // Skip whitespace
    while (pos < bytesStr.length() && bytesStr.charAt(pos) == ' ') pos++;
    if (pos >= bytesStr.length()) break;

    // Find next space or end
    int nextSpace = bytesStr.indexOf(' ', pos);
    if (nextSpace == -1) nextSpace = bytesStr.length();

    String byteStr = bytesStr.substring(pos, nextSpace);
    uint8_t value = parseHexOrDec(byteStr);

    // Temporarily disable protection to write to FPGA zone
    protectZones[1].enabled = false;
    EEPROM.write(writeAddr++, value);
    protectZones[1].enabled = true;

    bytesWritten++;
    pos = nextSpace + 1;
  }

  Serial.print(F("FPGA_WRITE: "));
  Serial.print(bytesWritten);
  Serial.print(F(" bytes at 0x"));
  Serial.println(addr, HEX);
}

void handleFPGARead(String params) {
  // #FPGAREAD <addr> [count]
  // Read from FPGA zone (100-299)

  int sp = params.indexOf(' ');
  uint16_t addr;
  uint8_t count = 1;

  if (sp == -1) {
    addr = parseHexOrDec(params);
  } else {
    addr = parseHexOrDec(params.substring(0, sp));
    count = params.substring(sp + 1).toInt();
    if (count == 0) count = 1;
    if (count > 200) count = 200;
  }

  // Validate FPGA zone
  if (addr < ADDR_FPGA_ZONE || addr >= ADDR_USER_ZONE) {
    Serial.print(F("ERR:FPGA_ZONE ("));
    Serial.print(ADDR_FPGA_ZONE);
    Serial.print(F("-"));
    Serial.print(ADDR_USER_ZONE - 1);
    Serial.println(F(")"));
    return;
  }

  Serial.print(F("FPGA[0x"));
  Serial.print(addr, HEX);
  Serial.print(F("]: "));

  for (uint8_t i = 0; i < count && (addr + i) < ADDR_USER_ZONE; i++) {
    if (i > 0) Serial.print(F(" "));
    uint8_t value = EEPROM.read(addr + i);
    Serial.print(F("0x"));
    if (value < 16) Serial.print(F("0"));
    Serial.print(value, HEX);
  }
  Serial.println();
}
