# SAM (Smart Arduino Monitor) - Arduino LCD + FPGA Control

This repository contains Arduino display + control firmware, with the **primary target** being:

- `mega_ili9486_serial_display/mega_ili9486_serial_display.ino`

SAM (Smart Arduino Monitor) is the Mega firmware in this repository: a split/full-screen control console for FPGA and peripherals, with serial commands and touch-driven navigation.

## Manual

Primary manual set:
- `docs/manual/index.md`
- Operator guide: `docs/manual/operator-manual.md`
- Full command reference: `docs/manual/command-reference.md`
- Legacy appendix: `docs/manual/legacy-appendix.md`

## Branch Scope

- Branch: `modularize/split-subsystems`
- Primary MCU: **Arduino Mega 2560**
- Primary display: **ILI9486/ILI9488 320x480** using `MCUFRIEND_kbv`
- Legacy path also exists: `ili9341_serial_display/` (older Uno/ILI9341 workflow)

## What This Firmware Does

- Split LCD into:
  - Top: command/status text
  - Bottom: FPGA/device output and interaction area
- Supports touch UI:
  - Quick touch buttons
  - Hierarchical `#MENU` system
- Manages up to 3 FPGA serial channels (Serial1/2/3)
- Supports protocol framing/termination and binary/text parsing modes
- Optional address-byte routing (`#ADDRMODE`, `#ADDR`, `#ADDRSEND`)
- Optional serial bridge/sniffer output (`#BRIDGE`) with channel/timestamp tags
- Watch mode + health telemetry (`#WATCH`, `#HEALTH`, `#HEALTHRESET`)
- Event hook automation to macros for GPIO and threshold transitions (`#HOOK ...`)
- Smart diagnostics and resilience features:
  - profile/summary/links (`#PROFILE`, `#SUMMARY`, `#LINK`)
  - escalation engine (`#ESC ...`)
  - failsafe auto-recovery (`#SAFE ...`)
  - last-trigger reason (`#WHY`)
  - hardware watchdog (`#WDT ...`)
- Split/fullscreen runtime view toggle (`#VIEW ...` + top-right touch corner)
- Includes onboard utility subsystems:
  - GPIO monitor/control
  - I2C tools
  - SPI tools
  - EEPROM tools
  - Analog acquisition tools
  - Data widgets, logging, waveform, PWM helpers

## Repository Layout

```
mega_ili9486_serial_display/
├── mega_ili9486_serial_display.ino  ← main sketch (setup + loop only)
├── sam_config.h                     ← constants, enums, structs
├── sam_globals.h / .cpp             ← all global variable declarations/definitions
├── sam_commands.h / .cpp            ← command parser, #HELP, watch, uptime
├── sam_display.h / .cpp             ← LCD rendering, view mode, link indicators
├── sam_touch.h / .cpp               ← touch calibration, button handling
├── sam_gpio.h / .cpp                ← GPIO modes, events, register control
├── sam_analog.h / .cpp              ← analog read, stats, thresholds
├── sam_fpga_serial.h / .cpp         ← FPGA serial RX/TX, framing, bridge
├── sam_logging.h / .cpp             ← data logger, event log, #WHY
├── sam_eeprom.h / .cpp              ← EEPROM read/write/config persist
├── sam_i2c.h / .cpp                 ← I2C scan/read/write
├── sam_spi.h / .cpp                 ← SPI transfer/settings
├── sam_waveform.h / .cpp            ← waveform gen, PWM, freq monitor
├── sam_rules.h / .cpp               ← smart rules engine
├── sam_macros.h / .cpp              ← macro define/run
├── sam_hooks.h / .cpp               ← GPIO/threshold event hooks
├── sam_failsafe.h / .cpp            ← escalation, failsafe, watchdog, reset cause
├── MenuSystem.h / .cpp              ← generic touch menu engine
└── MenuConfig.h / .cpp              ← menu tree + FPGA callbacks

ili9341_serial_display/              ← legacy Uno/ILI9341 workflow
Libraries/                           ← vendored display libraries
docs/manual/                         ← operator manual + command reference
```

## Hardware Mapping (Mega Path)

### Core serial channels

- USB/PC terminal: `Serial` at `115200`
- FPGA channel 1: `Serial1` (`TX1=18`, `RX1=19`)
- FPGA channel 2: `Serial2` (`TX2=16`, `RX2=17`)
- FPGA channel 3: `Serial3` (`TX3=14`, `RX3=15`)

### Touch + display related pins

- Touch pins (MCUFRIEND style):
  - `YP=A3`
  - `XM=A2`
  - `YM=9`
  - `XP=8`

### Peripheral buses

- I2C: `SDA=20`, `SCL=21`
- SPI: `MOSI=51`, `MISO=50`, `SCK=52`

### Extended I/O

- GPIO control block: pins `22-45` (24 GPIO)
- Analog block: `A8-A15`

## Prerequisites

1. Arduino CLI installed and on PATH (or call full executable path).
2. AVR core installed (`arduino:avr`).
3. Board connected over USB and visible via `arduino-cli board list`.

## Build and Upload (arduino-cli)

Run from repo root:

```powershell
arduino-cli core update-index
arduino-cli core install arduino:avr

arduino-cli compile --fqbn arduino:avr:mega ".\mega_ili9486_serial_display"
arduino-cli upload -p COM3 --fqbn arduino:avr:mega ".\mega_ili9486_serial_display"
```

Tips:

- Replace `COM3` with your actual board port.
- Use `arduino-cli board list` to discover the port.
- If upload fails because the serial port is busy, close Serial Monitor first.

## First Boot Checklist

1. Open serial monitor/terminal at **115200 baud**.
2. Send `#HELP` and confirm command help prints.
3. Send `#STATUS` and verify current FPGA/display settings.
4. Send `#VERSION` and confirm firmware/build string.
5. Send `#MENU` and test one touch selection.
6. If touch is offset, run:
   - `#TOUCHTEST` — touch corners to find raw values
   - `#TOUCHCAL <minx> <maxx> <miny> <maxy>` — apply calibration
   - Touch top-right corner toggles split/full view

## Command Model

Commands are line-based text over USB serial.

- `#...` commands: control/configure/display/peripherals
- `>>>text`: passthrough text send to active FPGA channel
- Query style: many commands accept `?` (example: `#FPGA1BAUD ?`)

## Quick Command Reference

### Display + UI

- `#HELP`
- `#STATUS`
- `#VERSION`
- `#VIEW <SPLIT|FULL|?>`
- `#CLR` / `#CLEAR`
- `#CLRBOT`
- `#CLRALL`
- `#SIZE <1-10>`
- `#COLOR <name>`
- `#BGCOLOR <name|NONE>`
- `#BOTSIZE <1-5>`
- `#BOTCOLOR <name>`
- `#POS <x> <y>`

### Touch + Menu

- `#SHOWBTNS`
- `#HIDEBTNS`
- `#MENU`
- `#MENUHIDE`
- `#MENUBACK`
- `#TOUCHCAL ?`
- `#TOUCHCAL <minx> <maxx> <miny> <maxy>`
- `#TOUCHTEST`

### FPGA serial control

- `#FPGASEL <1|2|3|?>`
- `#FPGA1BAUD <baud|?>` (same for 2/3)
- `#FPGA1STOP <1|2|?>` (same for 2/3)
- `#FPGA1RX <TEXT|BINARY|?>` (same for 2/3)
- `#FPGA1PARSE <NONE|ASCII|DEC|MIXED|?>` (same for 2/3)
- `#TERM <NONE|LF|CR|CRLF|0xFF|255|?>`
- `#FPGASEND <text>`
- `#FPGABYTES <hex bytes>`
- `#FPGAREAD <num_bytes> [timeout_ms]`
- `#FPGAQUERY <send_hex...> <expect_bytes> [timeout_ms]`
- `#FPGAPING`
- `#ADDRMODE <ON|OFF|?>`
- `#ADDR <value|?>`
- `#ADDRSEND <hex bytes...>`
- `#FRAME <ON|OFF|?>`
- `#BRIDGE <ON|OFF|?>`
- `>>>text`

### GPIO

- `#GPIOMODE <pin> <IN|INPU|OUT>`
- `#GPIOWRITE <pin> <0|1|HIGH|LOW>`
- `#GPIOREAD <pin>`
- `#GPIOREADALL`
- `#GPIOEVENT <pin> <NONE|RISING|FALLING|BOTH>`
- `#GPIOREG`
- `#GPIOSET <0x000000-0xFFFFFF>`
- `#HOOK GPIO <pin> <RISE|FALL|BOTH|NONE> <macro> [cooldown_ms]`

### Smart Ops

- `#PROFILE <SAFE|BALANCED|PERF|CUSTOM|?>`
- `#SUMMARY`
- `#WHY`
- `#LINK <ON|OFF|?|REPORT>` (`#LINKS` alias)
- `#ESC <ON|OFF|?|RESET>`
- `#ESC SET <ERR|TIMEOUT|RULE|ALL> <threshold> <window_ms> <LOG|ALERT|MACRO n>`
- `#SAFE <ON|OFF|?|CLEAR>`
- `#SAFE SET <threshold> <window_ms> <hold_ms>`
- `#WDT <ON ms|OFF|?>`

### I2C

- `#I2CSCAN`
- `#I2CREAD <addr> <reg> <bytes>`
- `#I2CWRITE <addr> <reg> <data...>`
- `#I2CWRITE <addr> <data...>`

### SPI

- `#SPIBEGIN <cs_pin>`
- `#SPITRANSFER <byte...>`
- `#SPISETTINGS <speed_hz> <mode_0_to_3>`
- `#SPIEND`

### EEPROM

- `#EEPROMREAD <addr>`
- `#EEPROMWRITE <addr> <value>`
- `#EEPROMDUMP <start> <end>`
- `#EEPROMCLEAR`
- `#EEPROMPROTECT <zone> <start> <end>`
- `#EEPROMPROTECT <zone> OFF`
- `#EEPROMPROTECT?`

### Analog

- `#ANALOGREAD <pin>`
- `#ANALOGREADALL`
- `#ANALOGREF <DEFAULT|INTERNAL>`
- `#ANALOGAVG <1-16>`
- `#HOOK THR <0-7> <ENTER|EXIT|BOTH|NONE> <macro> [cooldown_ms]`

## Menu System Details

`#MENU` opens touch navigation with this structure:

- Control Register
  - 8 entries (`Control Reg 0..7`) mapped to FPGA 64-bit reads
- Status Register
  - 8 entries (`Status Reg 0..7`) mapped to FPGA 64-bit reads
- Temperature
  - Single read + all sensors path
- Firmware Info
  - Version, build number, firmware ID
- Custom Bytes
  - directs to raw byte workflow

Menu implementation is split across:

- `MenuSystem.*` for rendering + navigation
- `MenuConfig.*` for command/handler definitions

## Example Session

```text
#STATUS
#FPGASEL 1
#FPGA1BAUD 115200
#FPGA1RX BINARY
#FPGA1PARSE MIXED
#TERM CRLF
#FPGABYTES 52 45 47
#FPGAREAD 8 1000
#MENU
```

## Troubleshooting

### 1) Compile succeeds but upload fails

- Check COM port (`arduino-cli board list`).
- Close any app using the port.
- Retry upload.

### 2) Touch coordinates are wrong

- Run `#TOUCHTEST`.
- Tap screen corners and note values.
- Set new calibration with `#TOUCHCAL minx maxx miny maxy`.

### 3) FPGA replies look garbled

- Confirm selected channel (`#FPGASEL ?` / `#STATUS`).
- Match baud and stop bits on both ends.
- Try `#FPGAxRX TEXT` first, then binary modes if needed.
- Adjust `#TERM` to match FPGA framing.

### 4) I2C/SPI commands return nothing

- Verify wiring and voltage levels.
- Confirm chip select pin for SPI peripherals.
- Use `#I2CSCAN` first to verify device presence.

## Legacy Documentation

The files in `ili9341_serial_display/` are retained for older workflows and are not the authoritative docs for the Mega feature set.

For this branch, use this `README.md` + runtime `#HELP` output as primary reference.

## Current Build Snapshot (SAM on Mega 2560)

Validated via `arduino-cli compile` on March 14, 2026:

- Flash: `144,536 / 253,952` bytes (`56%`)
- RAM: `6,410 / 8,192` bytes (`78%`)
- Estimated free RAM for locals/stack: `1,782` bytes

## Firmware Versioning

- Product name: `SAM (Smart Arduino Monitor)`
- Runtime query command: `#VERSION`
- Current firmware constant: `SAM_FIRMWARE_VERSION = 1.2.0`
