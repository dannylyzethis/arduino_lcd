#!/bin/bash
# LEGACY UNO PATH: This script targets ili9341_serial_display.
# For Mega + ILI9486/9488, use the root README and mega_ili9486_serial_display.


# ==============================================
#  ILI9341 LCD Serial Controller - Linux/Mac
# ==============================================

# Default serial port - change this to match your system
# Linux: usually /dev/ttyACM0 or /dev/ttyUSB0
# Mac: usually /dev/cu.usbmodem* or /dev/tty.usbserial*
SERIAL_PORT="/dev/ttyACM0"
BAUDRATE=9600

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "  ILI9341 LCD Serial Controller"
echo "=========================================="
echo ""

# Function to find Arduino port
find_arduino() {
    echo "Looking for Arduino..."
    
    # Check common Arduino ports
    for port in /dev/ttyACM* /dev/ttyUSB* /dev/cu.usbmodem* /dev/tty.usbserial*; do
        if [ -c "$port" ]; then
            echo "Found potential Arduino at: $port"
            read -p "Use this port? (y/n): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                SERIAL_PORT=$port
                return 0
            fi
        fi
    done
    
    echo "No Arduino found automatically."
    read -p "Enter serial port manually: " SERIAL_PORT
}

# Check if serial port exists
if [ ! -c "$SERIAL_PORT" ]; then
    echo -e "${RED}Error: Serial port $SERIAL_PORT not found${NC}"
    find_arduino
fi

# Configure serial port
echo "Configuring $SERIAL_PORT at $BAUDRATE baud..."
stty -F $SERIAL_PORT $BAUDRATE cs8 -cstopb -parenb 2>/dev/null

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Could not configure serial port${NC}"
    echo "Try running with sudo: sudo $0"
    exit 1
fi

echo -e "${GREEN}Port configured successfully!${NC}"
echo ""

# Show menu
show_menu() {
    echo "=========================================="
    echo -e "${BLUE}COMMANDS:${NC}"
    echo "=========================================="
    echo "  Text message - Display text on LCD"
    echo "  #CLEAR      - Clear screen"
    echo "  #COLOR name - Set text color"
    echo "    (RED, GREEN, BLUE, YELLOW, CYAN,"
    echo "     MAGENTA, WHITE, BLACK, ORANGE, PINK)"
    echo "  #SIZE n     - Set text size (1-5)"
    echo "  #POS x y    - Set position"
    echo "  #RECT x y w h - Draw rectangle"
    echo "  #CIRCLE x y r - Draw circle"
    echo "  #LINE x1 y1 x2 y2 - Draw line"
    echo "  #INFO       - Show display info"
    echo "  #HELP       - Show Arduino help"
    echo "  menu        - Show this menu"
    echo "  exit        - Quit program"
    echo "=========================================="
    echo ""
}

# Send command to LCD
send_command() {
    echo "$1" > $SERIAL_PORT
    echo -e "${GREEN}Sent:${NC} $1"
}

show_menu

# Main loop
while true; do
    read -p "LCD> " cmd
    
    case "$cmd" in
        exit|quit|EXIT|QUIT)
            echo "Goodbye!"
            break
            ;;
        menu|MENU)
            show_menu
            ;;
        "")
            # Empty line, do nothing
            ;;
        *)
            send_command "$cmd"
            ;;
    esac
done
