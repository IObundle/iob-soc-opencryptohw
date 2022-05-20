// Generator : SpinalHDL v1.7.0a    git head : 150a9b9067020722818dfb17df4a23ac712a7af8
// Component : SpinalxunitM
// Git hash  : e3db69359f553aef955eb4a49b293eb440752fbe

`timescale 1ns/1ps

module SpinalxunitM (
  input               io_cond0,
  input               io_cond1,
  output              io_flag,
  output     [7:0]    io_state,
  input               clk,
  input               reset
);

  reg        [7:0]    counter;

  assign io_state = counter;
  assign io_flag = ((counter == 8'h0) || io_cond1);
  always @(posedge clk or posedge reset) begin
    if(reset) begin
      counter <= 8'h0;
    end else begin
      if(io_cond0) begin
        counter <= (counter + 8'h01);
      end
    end
  end


endmodule
