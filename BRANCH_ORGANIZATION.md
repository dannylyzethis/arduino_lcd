# S.A.M. Branch Organization Guide

## Current Repository State

The S.A.M. (Serial Arduino Monitor) project has been fully documented with three distinct versions organized across different branches.

## Branch Structure

### 1. Main Branch (`main`)
**Version**: Original / Basic Display
**Startup Message**: `ILI9341 Ready`
**Status**: ✅ Documentation added (committed locally)
**Features**:
- Simple serial text display
- Basic graphics primitives
- No FPGA support
- No split-screen

**Note**: Main branch has documentation committed locally but **not pushed to remote** due to branch naming restrictions (branches must start with 'claude/' to push).

**Action Required**: You'll need to manually merge the main branch changes to remote via GitHub web UI or push it yourself.

---

### 2. Text-Based FPGA Version (`claude/review-codebase-01BRB8ZApEM6KTx3Srfihhgh`)
**Version**: Text-Based FPGA
**Startup Message**: `READY`
**Status**: ✅ Fully documented and pushed to remote
**Features**:
- Split-screen display
- FPGA communication (pins 12/13)
- Touch buttons (T/S/C/D)
- Text-based commands (#COLOR, #SIZE, etc.)
- FPGA passthrough (>>>)
- 99% memory usage

**Current State**: This branch contains the text-based FPGA version with all documentation pushed successfully.

---

### 3. Register-Based Version (`claude/register-based-commands-01BRB8ZApEM6KTx3Srfihhgh`)
**Version**: Register-Based FPGA
**Startup Message**: `S.A.M. REG-v1.0`
**Status**: ✅ Fully documented and pushed to remote (current branch)
**Features**:
- All text-based version features
- 16x 32-bit register architecture
- FPGA framing modes (Raw, Length-Prefix, Terminated, Both)
- 8 user FPGA registers (R08-R0F)
- Memory optimized to 95%
- Hex command interface (#W, #R)
- Comprehensive help system

**Current State**: This is the production-ready version with all features and optimizations.

---

## Documentation Files

All branches now contain the following documentation:

```
arduino_lcd/
├── README.md                    # Main project overview
└── docs/
    ├── VERSION_ORIGINAL.md      # Original version docs
    ├── VERSION_TEXT.md          # Text-based version docs
    └── VERSION_REGISTER.md      # Register-based version docs
```

## Renaming the Repository to "S.A.M."

To rename this repository to match the project name:

### Method 1: GitHub Web UI (Recommended)

1. Go to your repository on GitHub: `https://github.com/dannylyzethis/arduino_lcd`
2. Click on **Settings** (top right tab)
3. Scroll down to **Repository name** section
4. Change name from `arduino_lcd` to one of:
   - `SAM` (short and clean)
   - `S.A.M` (with periods)
   - `serial-arduino-monitor` (descriptive)
   - `sam-serial-monitor` (combination)

5. Click **Rename**

**Note**: GitHub automatically creates redirects, so old URLs will still work.

### Method 2: Recommended Repository Name

Based on GitHub naming conventions:
- **Best**: `sam-serial-monitor`
- **Alternative**: `SAM` (short and memorable)
- **Descriptive**: `serial-arduino-monitor`

Avoid using periods in repository names as they can cause issues with some tools.

---

## Organizing Branches (Optional Cleanup)

Once you've renamed the repository, you may want to create cleaner branch names:

### Option A: Keep Current Branches (Easiest)

Just update the README.md to reference the current branch names:
- `main` - Original version
- `claude/review-codebase-01BRB8ZApEM6KTx3Srfihhgh` - Text-based version
- `claude/register-based-commands-01BRB8ZApEM6KTx3Srfihhgh` - Register-based version

### Option B: Create Clean Branch Names (Advanced)

Create new branches with clean names and delete old ones:

```bash
# From main
git checkout main
git branch version/original main  # Keep original as separate branch name
git push origin version/original

# From text-based
git checkout claude/review-codebase-01BRB8ZApEM6KTx3Srfihhgh
git branch version/text-based
git push origin version/text-based

# From register-based
git checkout claude/register-based-commands-01BRB8ZApEM6KTx3Srfihhgh
git branch version/register-based
git push origin version/register-based
```

Then update README.md to point to new branches.

### Option C: Use Tags Instead (Recommended for Releases)

Create release tags for each version:

```bash
# Tag original version
git checkout main
git tag -a v1.0-original -m "S.A.M. Original Version - Basic Display"
git push origin v1.0-original

# Tag text-based version
git checkout claude/review-codebase-01BRB8ZApEM6KTx3Srfihhgh
git tag -a v1.0-text -m "S.A.M. Text-Based FPGA Version"
git push origin v1.0-text

# Tag register-based version
git checkout claude/register-based-commands-01BRB8ZApEM6KTx3Srfihhgh
git tag -a v1.0-register -m "S.A.M. REG-v1.0 - Register-Based Version"
git push origin v1.0-register
```

Tags appear in GitHub Releases section and are easier for users to find.

---

## Main Branch Update Required

**Important**: The `main` branch has documentation committed locally but could not be pushed due to branch naming restrictions.

### To Update Main Branch on GitHub:

1. Manually push from your local machine:
```bash
git checkout main
git push origin main
```

2. Or merge via GitHub web UI:
   - The documentation is identical across all branches
   - You can manually copy files if needed

---

## Recommended README.md Updates

After repository rename, update the clone commands in README.md:

**Find**: `git clone https://github.com/dannylyzethis/arduino_lcd.git`
**Replace with**: `git clone https://github.com/dannylyzethis/SAM.git` (or your chosen name)

Also update branch checkout commands to use final branch names if you create clean versions.

---

## Version Selection Chart

| Need | Use This Version |
|------|------------------|
| Simple display only | `main` (Original) |
| FPGA monitoring, easy commands | Text-based |
| Production FPGA, flexible config | Register-based (REG-v1.0) |

---

## Summary of Completed Work

✅ **All three versions fully documented**
✅ **Main README.md created** with project overview
✅ **Individual version guides** created (ORIGINAL, TEXT, REGISTER)
✅ **Register-based and text-based branches** pushed to remote
✅ **Main branch** documented locally (needs manual push)
✅ **Branch organization guide** created (this file)

---

## Next Steps (Your Action Items)

1. **Rename repository** to "S.A.M." or "sam-serial-monitor" via GitHub Settings

2. **Push main branch** documentation:
   ```bash
   git checkout main
   git push origin main
   ```

3. **(Optional) Create clean branch names** like `version/register-based`

4. **(Optional) Create release tags** for each version

5. **Update README.md** clone URLs after repository rename

6. **Share repository** with others - it's fully documented!

---

## Questions or Issues?

If you need to make changes:

**To modify documentation**: Edit files in `/docs/` directory
**To update version info**: Edit README.md
**To add features**: See VERSION_REGISTER.md for available flash memory (~1.5KB available)

All documentation is written in Markdown and can be edited with any text editor.

---

Created: 2025-11-22
Project: S.A.M. (Serial Arduino Monitor)
Dedicated to the memory of Sammy
