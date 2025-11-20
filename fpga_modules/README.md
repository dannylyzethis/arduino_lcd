# FPGA LCD Interface Modules

VHDL modules for bidirectional communication with Arduino LCD split-screen display.

## Overview

These modules enable an FPGA to communicate with the Arduino LCD controller via UART, allowing:
- Receive commands from Arduino
- Send data to be displayed on LCD bottom section
- Respond to queries (e.g., PING/PONG)
- Stream status updates and debug information

## Module Descriptions

### uart_rx.vhd
**UART Receiver**
- Receives serial data from Arduino TX (pin 11)
- 8 data bits, 1 stop bit, no parity
- Configurable baud rate (default 9600)
- Synchronizes input to prevent metastability
- Pulses `data_valid` when complete byte received

**Generics:**
- `CLK_FREQ`: System clock frequency in Hz (default: 50 MHz)
- `BAUD_RATE`: UART baud rate (default: 9600, must match Arduino)

**Ports:**
- `clk`: System clock
- `rst`: Active high reset
- `rx`: Serial input from Arduino
- `data_out`: Received byte (8 bits)
- `data_valid`: Pulses high for one clock when byte received

### uart_tx.vhd
**UART Transmitter**
- Sends serial data to Arduino RX (pin 10)
- 8 data bits, 1 stop bit, no parity
- Configurable baud rate (default 9600)
- Indicates busy status during transmission

**Generics:**
- `CLK_FREQ`: System clock frequency in Hz (default: 50 MHz)
- `BAUD_RATE`: UART baud rate (default: 9600, must match Arduino)

**Ports:**
- `clk`: System clock
- `rst`: Active high reset
- `tx`: Serial output to Arduino
- `data_in`: Byte to transmit (8 bits)
- `send`: Pulse high to initiate transmission
- `busy`: High when transmitting

### lcd_interface.vhd
**Top-Level LCD Interface**
- Integrates UART RX and TX modules
- Parses incoming commands
- Generates responses
- Handles PING/PONG protocol
- Formats and sends status messages

**Generics:**
- `CLK_FREQ`: System clock frequency in Hz (default: 50 MHz)
- `BAUD_RATE`: UART baud rate (default: 9600)

**Ports:**
- `clk`: System clock
- `rst`: Active high reset
- `uart_rx`: UART receive line (from Arduino TX, pin 11)
- `uart_tx`: UART transmit line (to Arduino RX, pin 10)
- `status_in`: 8-bit status value to report
- `counter_in`: 16-bit counter value (reserved for future use)
- `send_status`: Pulse high to send status message to LCD
- `last_cmd`: Debug output - last received command (32 bits)
- `cmd_valid`: Debug output - pulses when command received

## Hardware Connections

### Wiring Diagram
```
┌─────────────┐              ┌──────────────┐
│   Arduino   │              │     FPGA     │
│  (Uno R3)   │              │              │
├─────────────┤              ├──────────────┤
│ Pin 10      │◄─────TX──────│ uart_tx      │
│ Pin 11      │──────RX─────►│ uart_rx      │
│ GND         │──────────────│ GND          │
└─────────────┘              └──────────────┘
```

### Pin Connections
| Arduino  | FPGA Signal | Direction | Description |
|----------|-------------|-----------|-------------|
| Pin 10   | uart_tx     | ← FPGA    | Arduino receives data |
| Pin 11   | uart_rx     | → FPGA    | FPGA receives commands |
| GND      | GND         | -         | Common ground (required) |

**Note:** Pins 10 and 11 are free when the LCD shield uses 8-bit parallel mode (D2-D9 for data, A0-A4 for control). This avoids conflicts with the LCD shield control pins.

## Operating Modes

The Arduino LCD system operates in two modes:

### Normal Mode (Default)
- Arduino interprets commands from USB serial
- FPGA output automatically displays on LCD bottom section
- Full command set available (#CLR, #COLOR, #FPGASEND, etc.)

### Bypass Mode (100% Transparent Pass-Through)
- Direct USB ↔ FPGA communication with ZERO processing
- Raw byte forwarding in both directions
- Arduino becomes completely invisible bridge
- No formatting, no buffering, no LCD updates during operation
- Perfect for FPGA programming, direct debugging, or any external device
- Works with binary protocols, not just text

**Enter bypass mode:** Send `#BYPASS` command
**Exit bypass mode:** Send `###` (three hash symbols within 500ms)
**Note:** The ### escape sequence is the ONLY processing done in bypass mode

## Command Protocol

### Commands Received (Arduino → FPGA)
- **PING**: Test command, FPGA responds with "PONG"
- **Custom**: Add your own commands by modifying the command parser

### Responses Sent (FPGA → Arduino)
- **PONG**: Response to PING command
- **OK**: Generic acknowledgment for unknown commands
- **STATUS: XX**: Status report (sent when `send_status` pulsed)

## Usage Example

### Basic Instantiation
```vhdl
-- In your top-level FPGA design
signal fpga_status : std_logic_vector(7 downto 0);
signal send_stat   : std_logic;

lcd_if : entity work.lcd_interface
    generic map (
        CLK_FREQ  => 50_000_000,  -- 50 MHz clock
        BAUD_RATE => 9600          -- Match Arduino
    )
    port map (
        clk         => sys_clk,
        rst         => sys_rst,
        uart_rx     => fpga_rx_pin,  -- Connect to Arduino TX
        uart_tx     => fpga_tx_pin,  -- Connect to Arduino RX
        status_in   => fpga_status,
        counter_in  => x"0000",
        send_status => send_stat,
        last_cmd    => open,
        cmd_valid   => open
    );
```

### Sending Status Updates
```vhdl
-- Send status to LCD periodically
process(sys_clk)
    variable count : integer := 0;
begin
    if rising_edge(sys_clk) then
        send_stat <= '0';  -- Default

        if count = 50_000_000 then  -- Every 1 second @ 50MHz
            fpga_status <= some_status_value;
            send_stat <= '1';  -- Trigger send
            count := 0;
        else
            count := count + 1;
        end if;
    end if;
end process;
```

### Testing Communication

**Normal Mode:**
1. Upload Arduino sketch to Arduino Uno
2. Program FPGA with lcd_interface module
3. Connect wiring as shown above
4. Power both devices
5. From Arduino Serial Monitor, send: `#FPGAPING`
6. LCD bottom section should display: `PONG`
7. FPGA can send status by pulsing `send_status` signal

**Bypass Mode (100% Transparent - Direct FPGA/Device Access):**
1. From Serial Monitor, send: `#BYPASS`
2. Arduino responds: `[BYPASS MODE ACTIVE]` then goes silent
3. Now you're talking DIRECTLY to FPGA with zero Arduino interference
4. Send: `PING`
5. FPGA responds: `PONG` (visible ONLY in Serial Monitor, raw bytes)
6. LCD shows static message: "Transparent Mode" (no updates during operation)
7. All bytes pass through transparently - text, binary, any protocol
8. To exit, send: `###`
9. Arduino responds: `[EXIT BYPASS MODE]` and returns to normal mode

**Use cases for bypass mode:**
- FPGA firmware upload
- Direct UART communication with any device
- Binary protocol testing
- Maximum performance (zero Arduino overhead)
- Connecting to non-FPGA devices on pins 10/11

## Customization

### Adding New Commands
Edit `lcd_interface.vhd` around line 120:

```vhdl
-- Example: Add "STATUS" command handler
if cmd_length = 6 and
   cmd_buffer(31 downto 24) = x"53" and  -- 'S'
   cmd_buffer(23 downto 16) = x"54" and  -- 'T'
   -- ... check remaining bytes
   then
    -- Prepare response
    response_buffer(0) <= x"...";
    response_length <= ...;
end if;
```

### Changing Baud Rate
Ensure both Arduino and FPGA use same baud rate:

**Arduino:** `#FPGABAUD 19200`
**FPGA:** Set generic `BAUD_RATE => 19200` when instantiating

Supported rates: 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200

### Changing Clock Frequency
If your FPGA runs at different frequency, update the generic:

```vhdl
generic map (
    CLK_FREQ => 100_000_000,  -- 100 MHz instead of 50 MHz
    BAUD_RATE => 9600
)
```

## Synthesis Notes

- **Target**: Tested concept for generic FPGA (Xilinx/Intel/Lattice)
- **Resources**: Minimal (~50 LUTs, ~30 FFs per UART)
- **Timing**: Easily meets timing at 50 MHz on most FPGAs
- **Clock**: Requires stable clock source for accurate baud rate

## File List
```
fpga_modules/
├── README.md           (this file)
├── uart_rx.vhd         (UART receiver)
├── uart_tx.vhd         (UART transmitter)
└── lcd_interface.vhd   (Top-level interface)
```

## License
Same as parent project.

## Support
For issues or questions, see main project README.
