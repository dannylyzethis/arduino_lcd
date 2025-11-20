--------------------------------------------------------------------------------
-- UART Transmitter Module
-- Configurable baud rate (default 9600)
-- 8 data bits, 1 stop bit, no parity
-- Compatible with Arduino SoftwareSerial
--------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity uart_tx is
    generic (
        CLK_FREQ    : integer := 50_000_000;  -- System clock frequency in Hz
        BAUD_RATE   : integer := 9600          -- UART baud rate
    );
    port (
        clk         : in  std_logic;
        rst         : in  std_logic;
        tx          : out std_logic;           -- Serial output to Arduino RX
        data_in     : in  std_logic_vector(7 downto 0);
        send        : in  std_logic;           -- Pulse high to send byte
        busy        : out std_logic            -- High when transmitting
    );
end uart_tx;

architecture behavioral of uart_tx is
    constant CLKS_PER_BIT : integer := CLK_FREQ / BAUD_RATE;

    type state_type is (IDLE, START_BIT, DATA_BITS, STOP_BIT);
    signal state : state_type := IDLE;

    signal clk_count    : integer range 0 to CLKS_PER_BIT-1 := 0;
    signal bit_index    : integer range 0 to 7 := 0;
    signal tx_data      : std_logic_vector(7 downto 0) := (others => '0');

begin
    process(clk, rst)
    begin
        if rst = '1' then
            state <= IDLE;
            clk_count <= 0;
            bit_index <= 0;
            tx <= '1';  -- Idle high
            busy <= '0';

        elsif rising_edge(clk) then
            case state is
                when IDLE =>
                    tx <= '1';
                    clk_count <= 0;
                    bit_index <= 0;
                    busy <= '0';

                    if send = '1' then
                        tx_data <= data_in;
                        state <= START_BIT;
                        busy <= '1';
                    end if;

                when START_BIT =>
                    tx <= '0';  -- Start bit
                    if clk_count < CLKS_PER_BIT-1 then
                        clk_count <= clk_count + 1;
                    else
                        clk_count <= 0;
                        state <= DATA_BITS;
                    end if;

                when DATA_BITS =>
                    tx <= tx_data(bit_index);
                    if clk_count < CLKS_PER_BIT-1 then
                        clk_count <= clk_count + 1;
                    else
                        clk_count <= 0;
                        if bit_index < 7 then
                            bit_index <= bit_index + 1;
                        else
                            bit_index <= 0;
                            state <= STOP_BIT;
                        end if;
                    end if;

                when STOP_BIT =>
                    tx <= '1';  -- Stop bit
                    if clk_count < CLKS_PER_BIT-1 then
                        clk_count <= clk_count + 1;
                    else
                        clk_count <= 0;
                        state <= IDLE;
                    end if;

            end case;
        end if;
    end process;

end behavioral;
