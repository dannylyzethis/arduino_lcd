--------------------------------------------------------------------------------
-- Example Top-Level Design Using LCD Interface
-- Demonstrates how to integrate lcd_interface into your FPGA design
-- This is a complete, synthesizable example
--------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity lcd_interface_example is
    generic (
        CLK_FREQ    : integer := 50_000_000   -- 50 MHz system clock
    );
    port (
        -- System
        clk         : in  std_logic;          -- 50 MHz clock input
        rst_n       : in  std_logic;          -- Active-low reset button

        -- UART to Arduino
        arduino_rx  : out std_logic;          -- To Arduino pin 10
        arduino_tx  : in  std_logic;          -- From Arduino pin 11

        -- User inputs/outputs (example)
        switches    : in  std_logic_vector(7 downto 0);  -- 8 switches
        leds        : out std_logic_vector(7 downto 0);  -- 8 LEDs
        button      : in  std_logic           -- Push button to send status
    );
end lcd_interface_example;

architecture behavioral of lcd_interface_example is
    -- Component declaration
    component lcd_interface is
        generic (CLK_FREQ : integer; BAUD_RATE : integer);
        port (clk, rst : in std_logic;
              uart_rx, uart_tx : inout std_logic;
              status_in : in std_logic_vector(7 downto 0);
              counter_in : in std_logic_vector(15 downto 0);
              send_status : in std_logic;
              last_cmd : out std_logic_vector(31 downto 0);
              cmd_valid : out std_logic);
    end component;

    -- Internal signals
    signal rst          : std_logic;
    signal status_byte  : std_logic_vector(7 downto 0);
    signal counter      : unsigned(15 downto 0) := (others => '0');
    signal send_stat    : std_logic := '0';
    signal last_command : std_logic_vector(31 downto 0);
    signal cmd_received : std_logic;

    -- Button debouncing
    signal button_sync  : std_logic_vector(2 downto 0) := "000";
    signal button_pulse : std_logic := '0';

    -- Periodic status sender
    signal status_timer : integer range 0 to CLK_FREQ := 0;

begin
    -- Active-low to active-high reset conversion
    rst <= not rst_n;

    -- Connect switches to status byte
    status_byte <= switches;

    -- Simple counter (increments every clock cycle)
    process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                counter <= (others => '0');
            else
                counter <= counter + 1;
            end if;
        end if;
    end process;

    -- Button debouncing and edge detection
    process(clk)
    begin
        if rising_edge(clk) then
            button_sync <= button_sync(1 downto 0) & button;

            -- Detect rising edge
            if button_sync(2 downto 1) = "01" then
                button_pulse <= '1';
            else
                button_pulse <= '0';
            end if;
        end if;
    end process;

    -- Periodic status sender (every 1 second)
    process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                status_timer <= 0;
                send_stat <= '0';
            else
                send_stat <= '0';  -- Default

                if status_timer >= CLK_FREQ - 1 then
                    -- Send status every second
                    send_stat <= '1';
                    status_timer <= 0;
                elsif button_pulse = '1' then
                    -- Also send when button pressed
                    send_stat <= '1';
                    status_timer <= 0;
                else
                    status_timer <= status_timer + 1;
                end if;
            end if;
        end if;
    end process;

    -- Display last command on LEDs
    process(clk)
    begin
        if rising_edge(clk) then
            if cmd_received = '1' then
                leds <= last_command(7 downto 0);  -- Show lowest byte of command
            end if;
        end if;
    end process;

    -- Instantiate LCD interface
    lcd_if_inst : lcd_interface
        generic map (
            CLK_FREQ  => CLK_FREQ,
            BAUD_RATE => 9600
        )
        port map (
            clk         => clk,
            rst         => rst,
            uart_rx     => arduino_tx,    -- Receive from Arduino TX
            uart_tx     => arduino_rx,    -- Transmit to Arduino RX
            status_in   => status_byte,
            counter_in  => std_logic_vector(counter),
            send_status => send_stat,
            last_cmd    => last_command,
            cmd_valid   => cmd_received
        );

end behavioral;
