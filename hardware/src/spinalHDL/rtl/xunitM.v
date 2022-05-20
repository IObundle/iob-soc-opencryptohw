// Generator : SpinalHDL v1.7.0a    git head : 150a9b9067020722818dfb17df4a23ac712a7af8
// Component : xunitM
// Git hash  : 1d673651fb49ca91712670e3c33b4781f4ec47b0

`timescale 1ns/1ps

module xunitM (
  input               clk,
  input               rst,
  input               run,
  input      [7:0]    delay0,
  input      [31:0]   in0,
  output     [31:0]   out0
);

  wire       [31:0]   _zz_coreArea_next_w_16;
  wire       [31:0]   _zz_coreArea_next_w_17;
  wire       [31:0]   _zz_coreArea_next_w_18;
  wire       [31:0]   _zz_coreArea_next_w_19;
  reg        [31:0]   _zz_coreArea_next_w;
  reg        [31:0]   _zz_coreArea_next_w_1;
  reg        [31:0]   _zz_coreArea_next_w_2;
  reg        [31:0]   _zz_coreArea_next_w_3;
  reg        [31:0]   _zz_coreArea_next_w_4;
  reg        [31:0]   _zz_coreArea_next_w_5;
  reg        [31:0]   _zz_coreArea_next_w_6;
  reg        [31:0]   _zz_coreArea_next_w_7;
  reg        [31:0]   _zz_coreArea_next_w_8;
  reg        [31:0]   _zz_coreArea_next_w_9;
  reg        [31:0]   _zz_coreArea_next_w_10;
  reg        [31:0]   _zz_coreArea_next_w_11;
  reg        [31:0]   _zz_coreArea_next_w_12;
  reg        [31:0]   _zz_coreArea_next_w_13;
  reg        [31:0]   _zz_coreArea_next_w_14;
  reg        [31:0]   _zz_coreArea_next_w_15;
  wire       [31:0]   coreArea_next_w;
  reg        [7:0]    coreArea_delay;
  reg        [4:0]    coreArea_latency;
  reg        [31:0]   coreArea_out0_reg;
  wire                when_xunitM_l52;
  wire                when_xunitM_l60;
  wire                when_xunitM_l49;

  assign _zz_coreArea_next_w_16 = (_zz_coreArea_next_w_17 + (({_zz_coreArea_next_w_1[6 : 0],_zz_coreArea_next_w_1[31 : 7]} ^ {_zz_coreArea_next_w_1[17 : 0],_zz_coreArea_next_w_1[31 : 18]}) ^ _zz_coreArea_next_w_19));
  assign _zz_coreArea_next_w_17 = ((({_zz_coreArea_next_w_14[16 : 0],_zz_coreArea_next_w_14[31 : 17]} ^ {_zz_coreArea_next_w_14[18 : 0],_zz_coreArea_next_w_14[31 : 19]}) ^ _zz_coreArea_next_w_18) + _zz_coreArea_next_w_9);
  assign _zz_coreArea_next_w_18 = (_zz_coreArea_next_w_14 >>> 10);
  assign _zz_coreArea_next_w_19 = (_zz_coreArea_next_w_1 >>> 3);
  assign coreArea_next_w = (_zz_coreArea_next_w_16 + _zz_coreArea_next_w);
  assign when_xunitM_l52 = (5'h0 < coreArea_latency);
  assign when_xunitM_l60 = (5'h01 < coreArea_latency);
  assign when_xunitM_l49 = (8'h0 < coreArea_delay);
  assign out0 = coreArea_out0_reg;
  always @(posedge clk or posedge rst) begin
    if(rst) begin
      _zz_coreArea_next_w <= 32'h0;
      _zz_coreArea_next_w_1 <= 32'h0;
      _zz_coreArea_next_w_2 <= 32'h0;
      _zz_coreArea_next_w_3 <= 32'h0;
      _zz_coreArea_next_w_4 <= 32'h0;
      _zz_coreArea_next_w_5 <= 32'h0;
      _zz_coreArea_next_w_6 <= 32'h0;
      _zz_coreArea_next_w_7 <= 32'h0;
      _zz_coreArea_next_w_8 <= 32'h0;
      _zz_coreArea_next_w_9 <= 32'h0;
      _zz_coreArea_next_w_10 <= 32'h0;
      _zz_coreArea_next_w_11 <= 32'h0;
      _zz_coreArea_next_w_12 <= 32'h0;
      _zz_coreArea_next_w_13 <= 32'h0;
      _zz_coreArea_next_w_14 <= 32'h0;
      _zz_coreArea_next_w_15 <= 32'h0;
      coreArea_delay <= 8'h0;
      coreArea_latency <= 5'h0;
    end else begin
      if(run) begin
        coreArea_delay <= delay0;
        coreArea_latency <= 5'h11;
      end else begin
        if(when_xunitM_l49) begin
          coreArea_delay <= (coreArea_delay - 8'h01);
        end else begin
          if(when_xunitM_l52) begin
            coreArea_latency <= (coreArea_latency - 5'h01);
          end
          _zz_coreArea_next_w <= _zz_coreArea_next_w_1;
          _zz_coreArea_next_w_1 <= _zz_coreArea_next_w_2;
          _zz_coreArea_next_w_2 <= _zz_coreArea_next_w_3;
          _zz_coreArea_next_w_3 <= _zz_coreArea_next_w_4;
          _zz_coreArea_next_w_4 <= _zz_coreArea_next_w_5;
          _zz_coreArea_next_w_5 <= _zz_coreArea_next_w_6;
          _zz_coreArea_next_w_6 <= _zz_coreArea_next_w_7;
          _zz_coreArea_next_w_7 <= _zz_coreArea_next_w_8;
          _zz_coreArea_next_w_8 <= _zz_coreArea_next_w_9;
          _zz_coreArea_next_w_9 <= _zz_coreArea_next_w_10;
          _zz_coreArea_next_w_10 <= _zz_coreArea_next_w_11;
          _zz_coreArea_next_w_11 <= _zz_coreArea_next_w_12;
          _zz_coreArea_next_w_12 <= _zz_coreArea_next_w_13;
          _zz_coreArea_next_w_13 <= _zz_coreArea_next_w_14;
          _zz_coreArea_next_w_14 <= _zz_coreArea_next_w_15;
          if(when_xunitM_l60) begin
            _zz_coreArea_next_w_15 <= in0;
          end else begin
            _zz_coreArea_next_w_15 <= coreArea_next_w;
          end
        end
      end
    end
  end

  always @(posedge clk) begin
    coreArea_out0_reg <= coreArea_next_w;
  end


endmodule
