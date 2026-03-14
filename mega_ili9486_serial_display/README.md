# Mega ILI9486/ILI9488 Sketch Notes

This folder contains the primary firmware used on this branch:

- `mega_ili9486_split_text.ino`

For complete setup, command reference, and troubleshooting, use the repository root `README.md`.

Manual set:
- `docs/manual/index.md`
- `docs/manual/operator-manual.md`
- `docs/manual/command-reference.md`

## Build and Upload

From repo root:

```powershell
arduino-cli compile --fqbn arduino:avr:mega --libraries ".\\Libraries" ".\\_compile_stage_mega"
arduino-cli upload -p COM4 --fqbn arduino:avr:mega ".\\_compile_stage_mega"
```

Replace `COM4` with your board port.

### Staging note

`arduino-cli` expects the sketch file name to match the folder name.  
This firmware file is `mega_ili9486_split_text.ino`, so use a temporary staging folder
with a renamed main sketch file (for example `_compile_stage_mega/_compile_stage_mega.ino`).

## Quick Smoke Test

1. Open serial monitor at `115200`
2. Send `#HELP`
3. Send `#STATUS`
4. Send `#VERSION`
5. Send `#WDT ?`
6. Send `#SAFE ?`
7. Send `#WHY`
