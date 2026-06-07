set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets prueba_i/clk_wiz_1/inst/clk_out1]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets prueba_i/clk_wiz_1/inst/clk_out2]
# ── VGA ──────────────────────────────────────────────────────────────
set_property PACKAGE_PIN A3  [get_ports {VGA_R_0[0]}]
set_property PACKAGE_PIN B4  [get_ports {VGA_R_0[1]}]
set_property PACKAGE_PIN C5  [get_ports {VGA_R_0[2]}]
set_property PACKAGE_PIN A4  [get_ports {VGA_R_0[3]}]

set_property PACKAGE_PIN C6  [get_ports {VGA_G_0[0]}]
set_property PACKAGE_PIN A5  [get_ports {VGA_G_0[1]}]
set_property PACKAGE_PIN B6  [get_ports {VGA_G_0[2]}]
set_property PACKAGE_PIN A6  [get_ports {VGA_G_0[3]}]

set_property PACKAGE_PIN B7  [get_ports {VGA_B_0[0]}]
set_property PACKAGE_PIN C7  [get_ports {VGA_B_0[1]}]
set_property PACKAGE_PIN D7  [get_ports {VGA_B_0[2]}]
set_property PACKAGE_PIN D8  [get_ports {VGA_B_0[3]}]

set_property PACKAGE_PIN B11 [get_ports VGA_HS_0]
set_property PACKAGE_PIN B12 [get_ports VGA_VS_0]

set_property IOSTANDARD LVCMOS33 [get_ports {VGA_R_0[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {VGA_G_0[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {VGA_B_0[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports VGA_HS_0]
set_property IOSTANDARD LVCMOS33 [get_ports VGA_VS_0]