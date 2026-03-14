# Mega ILI9486/ILI9488 Sketch Notes

This folder contains the primary firmware used on this branch:

- `mega_ili9486_split_text.ino`

For complete setup, command reference, and troubleshooting, use the repository root `README.md`.

## Build and Upload

From repo root:

```powershell
arduino-cli compile --fqbn arduino:avr:mega --libraries ".\\Libraries" ".\\mega_ili9486_serial_display"
arduino-cli upload -p COM4 --fqbn arduino:avr:mega ".\\mega_ili9486_serial_display"
```

Replace `COM4` with your board port.

## Quick Smoke Test

1. Open serial monitor at `115200`
2. Send `#HELP`
3. Send `#STATUS`
4. Send `#MENU`
5. Send `#TOUCHCAL ?`
