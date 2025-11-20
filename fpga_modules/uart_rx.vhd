--------------------------------------------------------------------------------
-- UART Receiver Module
-- Configurable baud rate (default 9600)
-- 8 data bits, 1 stop bit, no parity
-- Compatible with Arduino SoftwareSerial
--------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity uart_rx is
    generic (
        CLK_FREQ    : integer := 50_000_000;  -- System clock frequency in Hz
        BAUD_RATE   : integer := 9600          -- UART baud rate
    );
    port (
        clk         : in  std_logic;
        rst         : in  std_logic;
        rx          : in  std_logic;           -- Serial input from Arduino TX
        data_out    : out std_logic_vector(7 downto 0);
        data_valid  : out std_logic            -- Pulses high when byte received
    );
end uart_rx;

architecture behavioral of uart_rx is
    constant CLKS_PER_BIT : integer := CLK_FREQ / BAUD_RATE;

    type state_type is (IDLE, START_BIT, DATA_BITS, STOP_BIT);
    signal state : state_type := IDLE;

    signal clk_count    : integer range 0 to CLKS_PER_BIT-1 := 0;
    signal bit_index    : integer range 0 to 7 := 0;
    signal rx_data      : std_logic_vector(7 downto 0) := (others => '0');
    signal rx_sync      : std_logic_vector(1 downto 0) := "11";

begin
    -- Synchronize input to avoid metastability
    process(clk)
    begin
        if rising_edge(clk) then
            rx_sync <= rx_sync(0) & rx;
        end if;
    end process;

    -- Main UART RX state machine
    process(clk, rst)
    begin
        if rst = '1' then
            state <= IDLE;
            clk_count <= 0;
            bit_index <= 0;
            rx_data <= (others => '0');
            data_valid <= '0';

        elsif rising_edge(clk) then
            data_valid <= '0';  -- Default, pulse for one cycle

            case state is
                when IDLE =>
                    clk_count <= 0;
                    bit_index <= 0;
                    -- Wait for start bit (falling edge)
                    if rx_sync(1) = '0' then
                        state <= START_BIT;
                    end if;

                when START_BIT =>
                    if clk_count = (CLKS_PER_BIT-1)/2 then
                        -- Sample in middle of start bit
                        if rx_sync(1) = '0' then
                            clk_count <= 0;
                            state <= DATA_BITS;
                        else
                            state <= IDLE;  -- False start
                        end if;
                    else
                        clk_count <= clk_count + 1;
                    end if;

                when DATA_BITS =>
                    if clk_count < CLKS_PER_BIT-1 then
                        clk_count <= clk_count + 1;
                    else
                        clk_count <= 0;
                        rx_data(bit_index) <= rx_sync(1);

                        if bit_index < 7 then
                            bit_index <= bit_index + 1;
                        else
                            bit_index <= 0;
                            state <= STOP_BIT;
                        end if;
                    end if;

                when STOP_BIT =>
                    if clk_count < CLKS_PER_BIT-1 then
                        clk_count <= clk_count + 1;
                    else
                        clk_count <= 0;
                        data_valid <= '1';
                        data_out <= rx_data;
                        state <= IDLE;
                    end if;

            end case;
        end if;
    end process;

end behavioral;
