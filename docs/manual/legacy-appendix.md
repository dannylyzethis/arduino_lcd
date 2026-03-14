# Legacy Uno Appendix (Non-Primary Path)

## Authoritative Path

The authoritative firmware path is Mega SAM:
- `mega_ili9486_serial_display/mega_ili9486_split_text.ino`

Use legacy Uno content only when maintaining existing Uno deployments.

## Legacy Assets

Legacy folder:
- `ili9341_serial_display/`

Notable files:
- `ili9341_serial_display/ili9341_serial_display.ino`
- `ili9341_serial_display/wiring_guide.md`
- `ili9341_serial_display/serial_methods.md`

## When to Use Legacy

Use legacy Uno flow only if:
- Hardware is locked to Uno + ILI9341 stack.
- You need backward compatibility with an existing deployed legacy setup.
- Mega migration is not currently possible.

## Migration Notes (Legacy -> SAM Mega)

1. Hardware migration:
- Move to Mega 2560 and ILI9486/ILI9488 shield wiring profile.
2. Firmware migration:
- Adopt `mega_ili9486_split_text.ino`.
3. Command migration:
- Re-validate automation scripts against SAM command surface (`#HELP` + manual reference).
4. Feature migration:
- Reconfigure smart ops (`#PROFILE`, `#ESC`, `#SAFE`, `#WDT`) as needed.

## Legacy Scope Boundary

This appendix is intentionally concise.
Do not treat legacy Uno docs as co-equal with SAM Mega docs for new development.
