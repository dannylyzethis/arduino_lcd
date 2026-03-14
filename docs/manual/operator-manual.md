# SAM Operator Manual (Mega Primary)

## 1) Boot and Connect Workflow

Target firmware:
- `mega_ili9486_serial_display/mega_ili9486_split_text.ino`

Serial baseline:
- USB serial: `115200`
- Typical board port: `COM4` (verify with `arduino-cli board list`)

Connect sequence:
1. Connect Mega over USB.
2. Open terminal/monitor at `115200`.
3. Wait about `2.5-3.0s` after connect/reset before first command.
4. Send:
- `#VERSION`
- `#STATUS`
- `#HEALTH`
5. Confirm expected readiness:
- `#WHY` should report a current reason string.

## 2) First 5-Minute Setup (Playbook)

1. Confirm command surface:
- `#HELP`
2. Confirm device identity:
- `#ID`
- `#VERSION`
3. Confirm serial channel config:
- `#FPGASEL ?`
- `#FPGA1BAUD ?`
- `#TERM ?`
4. Confirm smart ops baseline:
- `#PROFILE ?`
- `#ESC ?`
- `#SAFE ?`
- `#WDT ?`
5. Confirm memory headroom:
- `#MEMINFO`
- `#HEALTH`

## 3) Build/Upload Workflow (Staged Sketch)

Because `arduino-cli` expects sketch file and folder names to match, use staged compile:

```powershell
$src='C:\Users\danny\Documents\Arduino\Arduino LCD\mega_ili9486_serial_display'
$dst='C:\Users\danny\Documents\Arduino\Arduino LCD\_compile_stage_mega'
if(Test-Path $dst){Remove-Item -Recurse -Force $dst}
Copy-Item -Recurse $src $dst
Copy-Item "$dst\mega_ili9486_split_text.ino" "$dst\_compile_stage_mega.ino"
Remove-Item "$dst\mega_ili9486_split_text.ino"
arduino-cli compile --fqbn arduino:avr:mega $dst
arduino-cli upload -p COM4 --fqbn arduino:avr:mega $dst
```

Cleanup:

```powershell
if(Test-Path $dst){Remove-Item -Recurse -Force $dst}
```

## 4) Daily Operations Playbooks

### A) FPGA Routing and Link Check

1. Select target channel:
- `#FPGASEL 1`
2. Verify link and counters:
- `#LINK ?`
- `#STATUS`
3. Check health counters:
- `#HEALTH`
4. Quick traffic probe:
- `#FPGAPING`
- `#FPGABYTES 23 48 45 4C 50`

### B) Watch + Health Triage

1. Start watch:
- `#WATCH START GPIO 1000`
2. Observe short interval behavior.
3. Snapshot health:
- `#HEALTH`
- `#SUMMARY`
- `#WHY`
4. Stop watch:
- `#WATCH STOP`

### C) Safe Recovery Under Faults

1. Enable failsafe policy:
- `#SAFE SET 4 2000 8000`
- `#SAFE ON`
2. Enable escalation policy:
- `#ESC SET ERR 2 1000 LOG`
- `#ESC ON`
3. If unstable, check:
- `#WHY`
- `#LOG LAST 10 ERR`
- `#HEALTH`
4. Recover:
- `#SAFE CLEAR`
- `#ESC RESET`
- `#HEALTHRESET`

## 5) Smart Feature Operations

### Profiles and Summary
- `#PROFILE SAFE`
- `#PROFILE BALANCED`
- `#PROFILE PERF`
- `#PROFILE CUSTOM`
- `#SUMMARY`

### Last Trigger Reason
- `#WHY`

### Escalation
- `#ESC ?`
- `#ESC SET <ERR|TIMEOUT|RULE|ALL> <threshold> <window_ms> <LOG|ALERT|MACRO n>`
- `#ESC ON`
- `#ESC OFF`
- `#ESC RESET`

### Failsafe
- `#SAFE ?`
- `#SAFE SET <threshold> <window_ms> <hold_ms>`
- `#SAFE ON`
- `#SAFE OFF`
- `#SAFE CLEAR`

### Watchdog
- `#WDT ?`
- `#WDT ON 1000`
- `#WDT OFF`

## 6) Reliability Notes

- First command reliability:
- Parser tolerates leading noise before `#` or `>>>`.
- Still prefer sending clean commands.
- Serial settle:
- After opening port, wait around `2.5-3.0s` before first command.
- RAM caution zone:
- Treat `~78%` global RAM usage as high caution.
- Prefer bugfixes/docs/tuning over large new runtime features.

## 7) Troubleshooting Matrix

| Symptom | Fastest Checks | Typical Fix |
|---|---|---|
| No command response | `#VERSION`, `#STATUS` | Verify baud `115200`, wait 3s after connect, retry plain `#` command |
| FPGA timeout | `#HEALTH`, `#WHY`, `#LOG LAST 10 ERR` | Confirm `#FPGASEL`, baud/term settings, wiring |
| Repeated safe-mode entry | `#SAFE ?`, `#ESC ?`, `#WHY` | Raise SAFE threshold/window or clear underlying link errors |
| Unexpected resets | `#HEALTH` (`LastReset`), `#WDT ?` | Adjust or disable WDT while debugging blocking paths |
| Noisy command behavior | `#WHY`, `#STATUS` | Use clean line endings, avoid extra terminal prefixes |
| Memory pressure | `#MEMINFO`, `#HEALTH` | Disable heavy modes, reduce watch/log intensity |

## 8) Documentation Maintenance Checklist

When firmware behavior changes, update:
1. `docs/manual/operator-manual.md` playbooks impacted by new behavior.
2. `docs/manual/command-reference.md` command syntax/defaults/errors.
3. `docs/manual/index.md` navigation links if new docs are added.
4. Root and Mega README discovery links and version/memory snapshot where applicable.
