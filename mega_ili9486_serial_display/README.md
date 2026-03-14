# Mega ILI9486/ILI9488 Sketch Notes

This folder contains the primary SAM firmware:

- `mega_ili9486_serial_display.ino` — main sketch (setup + loop + serialEvent only)

All subsystems are split into dedicated modules:

| File | Responsibility |
|---|---|
| `sam_config.h` | Constants, enums, structs |
| `sam_globals.h/.cpp` | Global variable declarations/definitions |
| `sam_commands.h/.cpp` | Command parser, #HELP, watch, uptime |
| `sam_display.h/.cpp` | LCD rendering, view mode, link indicators |
| `sam_touch.h/.cpp` | Touch calibration, button handling |
| `sam_gpio.h/.cpp` | GPIO modes, events, register control |
| `sam_analog.h/.cpp` | Analog read, stats, thresholds |
| `sam_fpga_serial.h/.cpp` | FPGA serial RX/TX, framing, bridge |
| `sam_logging.h/.cpp` | Data logger, event log, #WHY |
| `sam_eeprom.h/.cpp` | EEPROM read/write/config persist |
| `sam_i2c.h/.cpp` | I2C scan/read/write |
| `sam_spi.h/.cpp` | SPI transfer/settings |
| `sam_waveform.h/.cpp` | Waveform gen, PWM, freq monitor |
| `sam_rules.h/.cpp` | Smart rules engine |
| `sam_macros.h/.cpp` | Macro define/run |
| `sam_hooks.h/.cpp` | GPIO/threshold event hooks |
| `sam_failsafe.h/.cpp` | Escalation, failsafe, watchdog, reset cause |
| `MenuSystem.h/.cpp` | Generic touch menu engine |
| `MenuConfig.h/.cpp` | Menu tree + FPGA callbacks |

For complete setup, command reference, and troubleshooting, see the repository root `README.md` and `docs/manual/`.

## Build and Upload

From repo root:

```powershell
arduino-cli compile --fqbn arduino:avr:mega ".\mega_ili9486_serial_display"
arduino-cli upload -p COM3 --fqbn arduino:avr:mega ".\mega_ili9486_serial_display"
```

Replace `COM3` with your board port (`arduino-cli board list` to find it).

## Quick Smoke Test

1. Open serial monitor at `115200`
2. Send `#HELP`
3. Send `#STATUS`
4. Send `#VERSION`
5. Send `#WDT ?`
6. Send `#SAFE ?`
7. Send `#WHY`
