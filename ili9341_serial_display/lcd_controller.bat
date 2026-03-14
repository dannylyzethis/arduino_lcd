@echo off
REM LEGACY UNO PATH: This script targets ili9341_serial_display.
REM For Mega + ILI9486/9488, use the root README and mega_ili9486_serial_display.
REM ==============================================
REM  ILI9341 LCD Serial Controller - Windows Batch
REM ==============================================
REM  Change COM3 to your Arduino's COM port
REM ==============================================

SET COMPORT=COM3
SET BAUDRATE=9600

echo ==========================================
echo   ILI9341 LCD Serial Controller
echo ==========================================
echo.
echo Using %COMPORT% at %BAUDRATE% baud
echo.

:CHECK_PORT
echo Configuring serial port...
mode %COMPORT%:%BAUDRATE%,n,8,1 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Cannot open %COMPORT%
    echo.
    echo Please check:
    echo 1. Arduino is connected
    echo 2. Correct COM port in this script
    echo 3. No other program is using the port
    echo.
    set /p COMPORT="Enter COM port (e.g., COM3): "
    goto CHECK_PORT
)

echo Port configured successfully!
echo.
echo ==========================================
echo   COMMANDS:
echo ==========================================
echo   Text message - Display text on LCD
echo   #CLEAR      - Clear screen
echo   #CLR        - Clear screen (short)
echo   #COLOR name - Set text color
echo   #SIZE n     - Set text size (1-5)
echo   #POS x y    - Set position
echo   #RECT x y w h - Draw rectangle
echo   #CIRCLE x y r - Draw circle
echo   #LINE x1 y1 x2 y2 - Draw line
echo   #INFO       - Show display info
echo   #HELP       - Show help on Arduino
echo   MENU        - Show this menu again
echo   EXIT        - Quit program
echo ==========================================
echo.

:INPUT_LOOP
set /p CMD="LCD> "

if /i "%CMD%"=="EXIT" goto END
if /i "%CMD%"=="QUIT" goto END
if /i "%CMD%"=="MENU" goto SHOW_MENU

echo %CMD% > %COMPORT%
goto INPUT_LOOP

:SHOW_MENU
echo.
echo Commands: Text, #CLEAR, #COLOR, #SIZE, #POS, #RECT, #CIRCLE, #LINE, #INFO, #HELP
echo.
goto INPUT_LOOP

:END
echo.
echo Goodbye!
pause
