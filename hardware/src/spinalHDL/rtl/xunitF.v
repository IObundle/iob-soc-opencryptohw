// Generator : SpinalHDL v1.7.0a    git head : 150a9b9067020722818dfb17df4a23ac712a7af8
// Component : xunitF
// Git hash  : 1d673651fb49ca91712670e3c33b4781f4ec47b0

`timescale 1ns/1ps

module xunitF (
  input               clk,
  input               rst,
  input               run,
  input      [7:0]    delay0,
  input      [31:0]   in0,
  input      [31:0]   in1,
  input      [31:0]   in2,
  input      [31:0]   in3,
  input      [31:0]   in4,
  input      [31:0]   in5,
  input      [31:0]   in6,
  input      [31:0]   in7,
  input      [31:0]   in8,
  input      [31:0]   in9,
  output     [31:0]   out0,
  output     [31:0]   out1,
  output     [31:0]   out2,
  output     [31:0]   out3,
  output     [31:0]   out4,
  output     [31:0]   out5,
  output     [31:0]   out6,
  output     [31:0]   out7
);

  wire       [31:0]   _zz_coreArea_T1;
  wire       [31:0]   _zz_coreArea_T1_1;
  wire       [31:0]   _zz_coreArea_T1_2;
  reg        [31:0]   coreArea_a;
  reg        [31:0]   coreArea_b;
  reg        [31:0]   coreArea_c;
  reg        [31:0]   coreArea_d;
  reg        [31:0]   coreArea_e;
  reg        [31:0]   coreArea_f;
  reg        [31:0]   coreArea_g;
  reg        [31:0]   coreArea_h;
  wire       [31:0]   coreArea_w;
  wire       [31:0]   coreArea_k;
  wire       [31:0]   coreArea_T1;
  wire       [31:0]   coreArea_T2;
  reg        [7:0]    coreArea_delay;
  reg        [4:0]    coreArea_latency;
  wire                when_xunitF_l71;
  wire                when_xunitF_l75;
  wire                when_xunitF_l68;

  assign _zz_coreArea_T1 = (_zz_coreArea_T1_1 + coreArea_k);
  assign _zz_coreArea_T1_1 = (_zz_coreArea_T1_2 + ((coreArea_e & coreArea_f) ^ ((~ coreArea_e) & coreArea_g)));
  assign _zz_coreArea_T1_2 = (coreArea_h + (({coreArea_e[5 : 0],coreArea_e[31 : 6]} ^ {coreArea_e[10 : 0],coreArea_e[31 : 11]}) ^ {coreArea_e[24 : 0],coreArea_e[31 : 25]}));
  assign coreArea_w = in8;
  assign coreArea_k = in9;
  assign coreArea_T1 = (_zz_coreArea_T1 + coreArea_w);
  assign coreArea_T2 = ((({coreArea_a[1 : 0],coreArea_a[31 : 2]} ^ {coreArea_a[12 : 0],coreArea_a[31 : 13]}) ^ {coreArea_a[21 : 0],coreArea_a[31 : 22]}) + (((coreArea_a & coreArea_b) ^ (coreArea_a & coreArea_c)) ^ (coreArea_b & coreArea_c)));
  assign when_xunitF_l71 = (5'h0 < coreArea_latency);
  assign when_xunitF_l75 = (coreArea_latency == 5'h02);
  assign when_xunitF_l68 = (8'h0 < coreArea_delay);
  assign out0 = coreArea_a;
  assign out1 = coreArea_b;
  assign out2 = coreArea_c;
  assign out3 = coreArea_d;
  assign out4 = coreArea_e;
  assign out5 = coreArea_f;
  assign out6 = coreArea_g;
  assign out7 = coreArea_h;
  always @(posedge clk or posedge rst) begin
    if(rst) begin
      coreArea_a <= 32'h0;
      coreArea_b <= 32'h0;
      coreArea_c <= 32'h0;
      coreArea_d <= 32'h0;
      coreArea_e <= 32'h0;
      coreArea_f <= 32'h0;
      coreArea_g <= 32'h0;
      coreArea_h <= 32'h0;
      coreArea_delay <= 8'h0;
      coreArea_latency <= 5'h0;
    end else begin
      if(run) begin
        coreArea_delay <= delay0;
        coreArea_latency <= 5'h02;
      end else begin
        if(when_xunitF_l68) begin
          coreArea_delay <= (coreArea_delay - 8'h01);
        end else begin
          if(when_xunitF_l71) begin
            coreArea_latency <= (coreArea_latency - 5'h01);
          end
          if(when_xunitF_l75) begin
            coreArea_a <= in0;
            coreArea_b <= in1;
            coreArea_c <= in2;
            coreArea_d <= in3;
            coreArea_e <= in4;
            coreArea_f <= in5;
            coreArea_g <= in6;
            coreArea_h <= in7;
          end else begin
            coreArea_a <= (coreArea_T1 + coreArea_T2);
            coreArea_b <= coreArea_a;
            coreArea_c <= coreArea_b;
            coreArea_d <= coreArea_c;
            coreArea_e <= (coreArea_d + coreArea_T1);
            coreArea_f <= coreArea_e;
            coreArea_g <= coreArea_f;
            coreArea_h <= coreArea_g;
          end
        end
      end
    end
  end


endmodule
