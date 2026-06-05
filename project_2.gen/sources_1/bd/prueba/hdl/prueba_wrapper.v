//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2025.1 (win64) Build 6140274 Thu May 22 00:12:29 MDT 2025
//Date        : Fri Jun  5 09:29:54 2026
//Host        : jesus running 64-bit major release  (build 9200)
//Command     : generate_target prueba_wrapper.bd
//Design      : prueba_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module prueba_wrapper
   (dip_switches_16bits_tri_i,
    led_16bits_tri_o,
    reset,
    sys_clock);
  input [15:0]dip_switches_16bits_tri_i;
  output [15:0]led_16bits_tri_o;
  input reset;
  input sys_clock;

  wire [15:0]dip_switches_16bits_tri_i;
  wire [15:0]led_16bits_tri_o;
  wire reset;
  wire sys_clock;

  prueba prueba_i
       (.dip_switches_16bits_tri_i(dip_switches_16bits_tri_i),
        .led_16bits_tri_o(led_16bits_tri_o),
        .reset(reset),
        .sys_clock(sys_clock));
endmodule
