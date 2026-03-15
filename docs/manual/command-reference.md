# SAM Full Command Reference (Mega)

Canonical target:
- `mega_ili9486_serial_display/mega_ili9486_split_text.ino`

Notes:
- Commands are line-based over USB serial (`115200`).
- Many commands support query via `?`.
- `#LINKS` is an alias for `#LINK`.
- `>>>text` sends passthrough text to active FPGA.

## Core Display/UI

- `#CLR`
- `#CLRBOT`
- `#CLRALL`
- `#SIZE <1-10>`
- `#COLOR <name>`
- `#BGCOLOR <name|NONE>`
- `#POS <x> <y>`
- `#BOTSIZE <1-5>`
- `#BOTCOLOR <name>`
- `#ROT <0-3>`
- `#VIEW <SPLIT|FULL|?>`

## Device and Metadata

- `#STATUS`
- `#ID`
- `#VERSION`
- `#HELP`

## Touch and Menu

- `#SHOWBTNS`
- `#HIDEBTNS`
- `#MENU`
- `#MENUHIDE`
- `#MENUBACK`
- `#TOUCHCAL ?`
- `#TOUCHCAL <minx> <maxx> <miny> <maxy>`
- `#TOUCHTEST`

## FPGA Serial and Protocol

- `#FPGASEL <1|2|3|?>`
- `#FPGA1BAUD <baud|?>`
- `#FPGA2BAUD <baud|?>`
- `#FPGA3BAUD <baud|?>`
- `#FPGA1STOP <1|2|?>`
- `#FPGA2STOP <1|2|?>`
- `#FPGA3STOP <1|2|?>`
- `#FPGA1RX <TEXT|BINARY|?>`
- `#FPGA2RX <TEXT|BINARY|?>`
- `#FPGA3RX <TEXT|BINARY|?>`
- `#FPGA1PARSE <NONE|ASCII|DEC|MIXED|?>`
- `#FPGA2PARSE <mode|?>`
- `#FPGA3PARSE <mode|?>`
- `#FPGASEND <text>`
- `#FPGAPING`
- `#FPGABYTES <hex>`
- `#FPGAREAD <bytes> [timeout]`
- `#FPGAQUERY <hex> <expect> [timeout]`
- `#TERM <NONE|LF|CR|CRLF|0xFF|255|?>`
- `#ADDRMODE <ON|OFF|?>`
- `#ADDR <byte|?>`
- `#ADDRSEND <hex bytes>`
- `#FRAME <ON|OFF|?>`
- `#BRIDGE <ON|OFF|?>`
- `>>>text`

## Smart Operations

- `#WATCH START <A8..A15|GPIO> <ms>`
- `#WATCH STOP`
- `#WATCH ?`
- `#WDT <ON ms|OFF|?>`
- `#LINK <ON|OFF|?|REPORT>`
- `#LINKS <ON|OFF|?>` (alias)
- `#PROFILE <SAFE|BALANCED|PERF|CUSTOM|?>`
- `#SUMMARY`
- `#WHY`
- `#ESC <ON|OFF|?|RESET>`
- `#ESC SET <ERR|TIMEOUT|RULE|ALL> <threshold> <window_ms> <LOG|ALERT|MACRO n>`
- `#SAFE <ON|OFF|?|CLEAR>`
- `#SAFE SET <threshold> <window_ms> <hold_ms>`
- `#RULE ?|ON|OFF|LIST`
- `#RULE ADD <expr> [FOR <ms>] THEN <MACRO id|LOG|ALERT>`
- `#RULE CLEAR <id|ALL>`
- `#HEALTH`
- `#HEALTHRESET`
- `#LOG ?`
- `#LOG LAST <n> [ALL|RULE|GPIO|THR|ERR|INFO]`
- `#LOG CLEAR`

## GPIO and Hooks

- `#GPIOMODE <pin> <IN|INPU|OUT>`
- `#GPIOWRITE <pin> <0|1>`
- `#GPIOREAD <pin>`
- `#GPIOREADALL`
- `#GPIOEVENT <pin> <NONE|RISING|FALLING|BOTH>`
- `#HOOK GPIO <pin> <RISE|FALL|BOTH|NONE> <macro> [cooldown]`
- `#HOOK THR <0-7> <ENTER|EXIT|BOTH|NONE> <macro> [cooldown]`
- `#HOOK LIST`
- `#GPIOREG`
- `#GPIOSET <0x000000-0xFFFFFF>`

## I2C

- `#I2CSCAN`
- `#I2CREAD <addr> <reg> <bytes>`
- `#I2CWRITE <addr> <reg> <data...>`
- `#I2CWRITE <addr> <data...>`

## SPI

- `#SPIBEGIN <cs_pin>`
- `#SPIEND`
- `#SPITRANSFER <byte...>`
- `#SPISETTINGS <speed> <mode>`

## EEPROM

- `#EEPROMREAD <addr>`
- `#EEPROMWRITE <addr> <value>`
- `#EEPROMDUMP <start> <end>`
- `#EEPROMCLEAR`
- `#EEPROMPROTECT <zone> <start> <end>`
- `#EEPROMPROTECT <zone> OFF`
- `#EEPROMPROTECT?`

## Analog, Thresholds, Stats

- `#ANALOGREAD <pin>`
- `#ANALOGREADALL`
- `#ANALOGREF <DEFAULT|INTERNAL>`
- `#ANALOGAVG <1-16>`
- `#THRESHOLD <pin> <low> <high>`
- `#THRESHOLDCLEAR <pin>`
- `#THRESHOLDSTATUS`
- `#STATSSTART <pin>`
- `#STATSSTOP <pin>`
- `#STATSSHOW <pin>`
- `#STATSRESET`

## Visual Widgets and Drawing

- `#RECT <x y w h>`
- `#FILL <x y w h>`
- `#CIRCLE <x y r>`
- `#LINE <x1 y1 x2 y2>`
- `#PROG <x y w h %>`
- `#GAUGE <x y r val max>`
- `#BARGRAPH <x y w h val1 val2...>`
- `#NUMBOX <x y value>`
- `#TREND <x y w h val1 val2...>`
- `#XYPLOT <x y w h x1,y1 x2,y2...>`
- `#SCROLL <y> <text>`

## Logging, Waveform, PWM, Timing

- `#LOGSTART <ms> [source]`
- `#LOGSTOP`
- `#LOGREAD [count]`
- `#LOGCLEAR`
- `#LOGSTATUS`
- `#LOGCONFIG <source>`
- `#LOGZONE <start> <end>`
- `#LOGZONE?`
- `#WAVEGEN <pin> <type> <freq>`
- `#WAVESTOP`
- `#PWMSET <pin> <duty>`
- `#PWMSTOP <pin>`
- `#PWMFREQ <pin> <freq>`
- `#PULSEIN <pin> <state> <timeout>`
- `#FREQCOUNT <pin> <duration>`
- `#FREQMON <pin> <duration>`
- `#FREQSTOP`

## Macro, Config, Misc Utilities

- `#MACRODEF <id> <cmd1>;<cmd2>...`
- `#MACRORUN <id>`
- `#MACROLIST`
- `#MACROCLEAR <id>`
- `#CONFIGSAVE`
- `#CONFIGLOAD`
- `#CONFIGRESET`
- `#CONFIGSHOW`
- `#BTNCONFIG <btn> <len> <bytes...>`
- `#BTNCONFIG <btn> ?`
- `#FPGAZWRITE <addr> <bytes...>` (EEPROM FPGA zone 100-299)
- `#FPGAZREAD <addr> [count]` (EEPROM FPGA zone 100-299)
- `#MEMINFO`
- `#UPTIME`
- `#BENCHMARK`

## Compatibility Notes

- New smart/protocol features default to conservative behavior where applicable.
- `#FPGAREAD` is reserved for live serial reads (`<bytes> [timeout]`).
- EEPROM FPGA-zone access uses `#FPGAZREAD/#FPGAZWRITE`.
- Legacy EEPROM-form `#FPGAREAD <addr> [count]` is deprecated.
- Command outputs include `OK:` and `ERR:` prefixed statuses used by scripts/automation.
