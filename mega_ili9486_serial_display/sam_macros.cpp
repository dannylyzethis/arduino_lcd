#include "sam_macros.h"
#include "sam_globals.h"
#include "sam_commands.h"

void runMacroSafe(uint8_t id, const __FlashStringHelper* sourceTag) {
  if (id >= MAX_MACROS || !macros[id].defined) return;
  if (macroRunDepth >= MAX_MACRO_DEPTH) {
    Serial.println(F("ERR:MACRO_DEPTH"));
    return;
  }
  macroRunDepth++;
  Serial.print(F("HOOK_RUN: "));
  Serial.print(sourceTag);
  Serial.print(F(" -> MACRO "));
  Serial.println(id);
  for (uint8_t i = 0; i < macros[id].cmdCount; i++) {
    processCmd(macros[id].commands[i]);
    delay(30);
  }
  macroRunDepth--;
}

void handleMacroDef(String params) {
  // Parse: <id> <cmd1>;<cmd2>;<cmd3>...
  // id: 0-2 (MAX_MACROS-1)
  int sp = params.indexOf(' ');
  if (sp == -1) {
    Serial.println(F("ERR:FORMAT #MACRODEF <id> <cmd1>;<cmd2>..."));
    Serial.println(F("  id: 0-2, max 5 commands per macro"));
    return;
  }

  int id = params.substring(0, sp).toInt();
  String cmdStr = params.substring(sp + 1);

  if (id < 0 || id >= MAX_MACROS) {
    Serial.print(F("ERR:ID_0_TO_"));
    Serial.println(MAX_MACROS - 1);
    return;
  }

  // Clear existing macro
  macros[id].cmdCount = 0;
  macros[id].defined = false;

  // Parse commands separated by semicolons
  int lastPos = 0;
  for (int i = 0; i <= cmdStr.length() && macros[id].cmdCount < MAX_MACRO_CMDS; i++) {
    if (i == cmdStr.length() || cmdStr[i] == ';') {
      if (i > lastPos) {
        String cmd = cmdStr.substring(lastPos, i);
        cmd.trim();
        if (cmd.length() > 0) {
          macros[id].commands[macros[id].cmdCount++] = cmd;
        }
      }
      lastPos = i + 1;
    }
  }

  if (macros[id].cmdCount > 0) {
    macros[id].defined = true;
    Serial.print(F("MACRO_DEF: ID="));
    Serial.print(id);
    Serial.print(F(", Commands="));
    Serial.println(macros[id].cmdCount);
  } else {
    Serial.println(F("ERR:NO_COMMANDS"));
  }
}

void handleMacroRun(String params) {
  int id = params.toInt();

  if (id < 0 || id >= MAX_MACROS) {
    Serial.print(F("ERR:ID_0_TO_"));
    Serial.println(MAX_MACROS - 1);
    return;
  }

  if (!macros[id].defined) {
    Serial.print(F("ERR:MACRO_"));
    Serial.print(id);
    Serial.println(F("_NOT_DEFINED"));
    return;
  }

  Serial.print(F("MACRO_RUN: ID="));
  Serial.println(id);

  runMacroSafe((uint8_t)id, F("MANUAL"));
  Serial.println(F("MACRO_DONE"));
}

void handleMacroList() {
  Serial.println(F("=== MACROS ==="));
  for (uint8_t i = 0; i < MAX_MACROS; i++) {
    Serial.print(F("Macro "));
    Serial.print(i);
    Serial.print(F(": "));
    if (macros[i].defined) {
      Serial.print(macros[i].cmdCount);
      Serial.println(F(" commands"));
      for (uint8_t j = 0; j < macros[i].cmdCount; j++) {
        Serial.print(F("  "));
        Serial.print(j + 1);
        Serial.print(F(". "));
        Serial.println(macros[i].commands[j]);
      }
    } else {
      Serial.println(F("Not defined"));
    }
  }
  Serial.println(F("=============="));
}

void handleMacroClear(String params) {
  int id = params.toInt();

  if (id < 0 || id >= MAX_MACROS) {
    Serial.print(F("ERR:ID_0_TO_"));
    Serial.println(MAX_MACROS - 1);
    return;
  }

  macros[id].cmdCount = 0;
  macros[id].defined = false;

  Serial.print(F("MACRO_CLEAR: ID="));
  Serial.println(id);
}
