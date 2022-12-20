curent_design system

# Timing Constraints
create_clock -name clk -period 10.000 [get_ports {clk}]
create_clock -name RX_CLK -period 40.000 [get_ports {RX_CLK}]
create_clock -name TX_CLK -period 40.000 [get_ports {TX_CLK}]

set_max_delay 10.000 -from [get_clocks {RX_CLK}] -to [get_clocks {clk}]
