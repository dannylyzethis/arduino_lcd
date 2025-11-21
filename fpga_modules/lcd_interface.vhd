--------------------------------------------------------------------------------
-- Arduino LCD Interface Module (FPGA Side)
-- Bidirectional communication with Arduino LCD display
-- Receives commands from Arduino, sends status/data back
-- Compatible with Arduino split-screen display controller
--------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity lcd_interface is
    generic (
        CLK_FREQ    : integer := 50_000_000;  -- System clock frequency in Hz
        BAUD_RATE   : integer := 9600          -- UART baud rate (match Arduino)
    );
    port (
        clk         : in  std_logic;
        rst         : in  std_logic;

        -- UART connections to Arduino
        uart_rx     : in  std_logic;           -- From Arduino TX (pin 13)
        uart_tx     : out std_logic;           -- To Arduino RX (pin 12)

        -- User interface (example signals)
        status_in   : in  std_logic_vector(7 downto 0);  -- System status byte
        counter_in  : in  std_logic_vector(15 downto 0); -- Counter value
        send_status : in  std_logic;           -- Pulse to send status update

        -- Debug outputs
        last_cmd    : out std_logic_vector(31 downto 0);  -- Last command received
        cmd_valid   : out std_logic            -- Pulse when command received
    );
end lcd_interface;

architecture behavioral of lcd_interface is
    -- Component declarations
    component uart_rx is
        generic (CLK_FREQ : integer; BAUD_RATE : integer);
        port (clk, rst, rx : in std_logic;
              data_out : out std_logic_vector(7 downto 0);
              data_valid : out std_logic);
    end component;

    component uart_tx is
        generic (CLK_FREQ : integer; BAUD_RATE : integer);
        port (clk, rst : in std_logic; tx : out std_logic;
              data_in : in std_logic_vector(7 downto 0);
              send : in std_logic; busy : out std_logic);
    end component;

    -- UART signals
    signal rx_data      : std_logic_vector(7 downto 0);
    signal rx_valid     : std_logic;
    signal tx_data      : std_logic_vector(7 downto 0);
    signal tx_send      : std_logic;
    signal tx_busy      : std_logic;

    -- Command parsing
    signal cmd_buffer   : std_logic_vector(31 downto 0) := (others => '0');
    signal cmd_length   : integer range 0 to 4 := 0;

    -- Transmit state machine
    type tx_state_type is (TX_IDLE, TX_SENDING, TX_WAIT);
    signal tx_state     : tx_state_type := TX_IDLE;

    -- Response buffer (for sending multi-byte messages)
    type byte_array is array (0 to 31) of std_logic_vector(7 downto 0);
    signal response_buffer : byte_array := (others => (others => '0'));
    signal response_length : integer range 0 to 32 := 0;
    signal response_index  : integer range 0 to 32 := 0;

    -- ASCII constants
    constant ASCII_LF   : std_logic_vector(7 downto 0) := x"0A";  -- '\n'
    constant ASCII_CR   : std_logic_vector(7 downto 0) := x"0D";  -- '\r'
    constant ASCII_P    : std_logic_vector(7 downto 0) := x"50";  -- 'P'
    constant ASCII_I    : std_logic_vector(7 downto 0) := x"49";  -- 'I'
    constant ASCII_N    : std_logic_vector(7 downto 0) := x"4E";  -- 'N'
    constant ASCII_G    : std_logic_vector(7 downto 0) := x"47";  -- 'G'
    constant ASCII_O    : std_logic_vector(7 downto 0) := x"4F";  -- 'O'

begin
    -- Instantiate UART RX
    uart_rx_inst: uart_rx
        generic map (CLK_FREQ => CLK_FREQ, BAUD_RATE => BAUD_RATE)
        port map (clk => clk, rst => rst, rx => uart_rx,
                  data_out => rx_data, data_valid => rx_valid);

    -- Instantiate UART TX
    uart_tx_inst: uart_tx
        generic map (CLK_FREQ => CLK_FREQ, BAUD_RATE => BAUD_RATE)
        port map (clk => clk, rst => rst, tx => uart_tx,
                  data_in => tx_data, send => tx_send, busy => tx_busy);

    -- Command reception and parsing
    process(clk, rst)
    begin
        if rst = '1' then
            cmd_buffer <= (others => '0');
            cmd_length <= 0;
            cmd_valid <= '0';
            last_cmd <= (others => '0');
            response_length <= 0;

        elsif rising_edge(clk) then
            cmd_valid <= '0';  -- Pulse

            -- Clear response length when transmission starts
            if tx_state = TX_SENDING and response_index = 0 then
                response_length <= 0;
            end if;

            if rx_valid = '1' then
                if rx_data = ASCII_LF or rx_data = ASCII_CR then
                    -- End of command
                    if cmd_length > 0 then
                        last_cmd <= cmd_buffer;
                        cmd_valid <= '1';

                        -- Parse command and prepare response
                        -- Check for PING command (4 bytes: 'P','I','N','G')
                        if cmd_length = 4 and cmd_buffer(31 downto 24) = ASCII_P and
                           cmd_buffer(23 downto 16) = ASCII_I and
                           cmd_buffer(15 downto 8) = ASCII_N and
                           cmd_buffer(7 downto 0) = ASCII_G then
                            -- Respond with "PONG\n"
                            response_buffer(0) <= ASCII_P;  -- 'P'
                            response_buffer(1) <= ASCII_O;  -- 'O'
                            response_buffer(2) <= ASCII_N;  -- 'N'
                            response_buffer(3) <= ASCII_G;  -- 'G'
                            response_buffer(4) <= ASCII_LF; -- '\n'
                            response_length <= 5;
                        else
                            -- Unknown command, send generic acknowledgment
                            response_buffer(0) <= x"4F";    -- 'O'
                            response_buffer(1) <= x"4B";    -- 'K'
                            response_buffer(2) <= ASCII_LF; -- '\n'
                            response_length <= 3;
                        end if;

                        cmd_length <= 0;
                        cmd_buffer <= (others => '0');
                    end if;
                else
                    -- Accumulate command bytes
                    if cmd_length < 4 then
                        cmd_buffer <= cmd_buffer(23 downto 0) & rx_data;
                        cmd_length <= cmd_length + 1;
                    end if;
                end if;
            end if;
        end if;
    end process;

    -- Transmit state machine
    process(clk, rst)
        variable nibble : unsigned(3 downto 0);
        variable hex_char : std_logic_vector(7 downto 0);
    begin
        if rst = '1' then
            tx_state <= TX_IDLE;
            tx_send <= '0';
            response_index <= 0;

        elsif rising_edge(clk) then
            tx_send <= '0';  -- Default

            case tx_state is
                when TX_IDLE =>
                    response_index <= 0;

                    -- Check if we have a response to send
                    if response_length > 0 then
                        tx_state <= TX_SENDING;
                    elsif send_status = '1' then
                        -- User requested status update - format message
                        response_buffer(0) <= x"53";  -- 'S'
                        response_buffer(1) <= x"54";  -- 'T'
                        response_buffer(2) <= x"41";  -- 'A'
                        response_buffer(3) <= x"54";  -- 'T'
                        response_buffer(4) <= x"3A";  -- ':'
                        response_buffer(5) <= x"20";  -- ' '

                        -- Convert status high nibble to hex
                        nibble := unsigned(status_in(7 downto 4));
                        if nibble < 10 then
                            hex_char := std_logic_vector(x"30" + ("0000" & nibble));
                        else
                            hex_char := std_logic_vector(x"41" + ("0000" & (nibble - 10)));
                        end if;
                        response_buffer(6) <= hex_char;

                        -- Convert status low nibble to hex
                        nibble := unsigned(status_in(3 downto 0));
                        if nibble < 10 then
                            hex_char := std_logic_vector(x"30" + ("0000" & nibble));
                        else
                            hex_char := std_logic_vector(x"41" + ("0000" & (nibble - 10)));
                        end if;
                        response_buffer(7) <= hex_char;

                        response_buffer(8) <= ASCII_LF;
                        response_length <= 9;
                        tx_state <= TX_SENDING;
                    end if;

                when TX_SENDING =>
                    if tx_busy = '0' and response_index < response_length then
                        tx_data <= response_buffer(response_index);
                        tx_send <= '1';
                        response_index <= response_index + 1;
                        tx_state <= TX_WAIT;
                    elsif response_index >= response_length then
                        tx_state <= TX_IDLE;
                    end if;

                when TX_WAIT =>
                    -- Wait for transmitter to become busy
                    if tx_busy = '1' then
                        tx_state <= TX_SENDING;
                    end if;

            end case;
        end if;
    end process;

end behavioral;
