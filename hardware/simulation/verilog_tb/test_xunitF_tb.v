`timescale 1ns / 1ps

`include "system.vh"

`define INPUT_SIZE (8+(2*16))
`define OUTPUT_SIZE (16)

// Implicitly know FU latency
`define XUNITF_LATENCY (1)

module xunitF_tb;

  parameter realtime clk_per = 1s/`FREQ;

  //clock
  reg clk = 1;
  always #(clk_per/2) clk = ~clk;

  //reset
  reg reset = 0;

  // run
  reg run = 0;

  // in/out
  reg [31:0] in0;
  reg [31:0] in1;
  reg [31:0] in2;
  reg [31:0] in3;
  reg [31:0] in4;
  reg [31:0] in5;
  reg [31:0] in6;
  reg [31:0] in7;
  reg [31:0] w_in;
  reg [31:0] k_in;

  wire [8*32-1:0] out;
  wire [31:0] out0;
  wire [31:0] out1;
  wire [31:0] out2;
  wire [31:0] out3;
  wire [31:0] out4;
  wire [31:0] out5;
  wire [31:0] out6;
  wire [31:0] out7;

  // configurations
  reg [7:0] delay_cfg = 8'h00;

  //iterator
  integer                i = 0;

  //test vectors
  reg [31:0] test_data_in[`INPUT_SIZE-1:0];
  reg [8*32-1:0] test_data_out[`OUTPUT_SIZE-1:0];
  integer fp_in = 0;
  integer fp_out = 0;
  integer tmp = 0;

  // validation
  reg check_done = 0;
  reg valid_out = 0;
  reg [7:0] latency_cnt;
  integer nerrors = 0;
  reg [4:0] result_addr = 0;
  reg [8*32-1:0] expected;

  initial begin
`ifdef VCD
    $dumpfile("xunitF_tb.vcd");
    $dumpvars;
`endif
    run = 0;
    result_addr = 0;

    // read input and validation .bin files
    fp_in = $fopen("xunitF_in.bin", "rb");
    tmp = $fread(test_data_in, fp_in);
    $fclose(fp_in);
    fp_out = $fopen("xunitF_out.bin", "rb");
    tmp = $fread(test_data_out, fp_out);
    $fclose(fp_out);

    //assert reset
    #100 reset = 1;

    // deassert rst
    repeat (100) @(posedge clk) #1;
    reset = 0;

    //wait an arbitray (10) number of cycles
    repeat (10) @(posedge clk) #1;

    // start execution
    run = 1;
    @(posedge clk) #1; run = 0;
    check_done = 1;
    
    // initial state regs
    in0 = test_data_in[0];
    in1 = test_data_in[1];
    in2 = test_data_in[2];
    in3 = test_data_in[3];
    in4 = test_data_in[4];
    in5 = test_data_in[5];
    in6 = test_data_in[6];
    in7 = test_data_in[7];

    // word and constant values
    for(i=8; i < `INPUT_SIZE; i = i+2) begin
        w_in = test_data_in[i];
        k_in = test_data_in[i+1];
        @(posedge clk) #1;
    end

    // wait for all output data
    while(check_done)
        @(posedge clk);

    // wait some more cycles
    repeat (10) @(posedge clk) #1;

    // write test results
    if(nerrors) begin
        $display("xunitF test FAILED with %d errors out of %d\n", nerrors, `OUTPUT_SIZE);
        $fatal; // return non-zero exit status
    end else begin
        $display("xunitF test PASSED with %d errors out of %d\n", nerrors, `OUTPUT_SIZE);
    end

    $finish;

  end

    xunitF uut(
        .clk(clk),
        .rst(reset),

        .run(run),

        .in0(in0),
        .in1(in1),
        .in2(in2),
        .in3(in3),
        .in4(in4),
        .in5(in5),
        .in6(in6),
        .in7(in7),

        .in8(w_in),
        .in9(k_in),

        .out0(out0),
        .out1(out1),
        .out2(out2),
        .out3(out3),
        .out4(out4),
        .out5(out5),
        .out6(out6),
        .out7(out7),

        .delay0(delay_cfg)
    );

    assign out = { out0, out1, out2, out3, out4, out5, out6, out7 };

    // verification process
    always @(posedge clk) begin
        expected = test_data_out[result_addr];
        if(valid_out & check_done) begin
            expected = test_data_out[result_addr];
            if(expected != out) begin
                nerrors = nerrors + 1;
            end
            result_addr = result_addr + 1;
            if (result_addr == `OUTPUT_SIZE) begin
                check_done = 0;
            end
        end
    end

    // valid_out state machine
    always @(posedge clk) begin
        if (reset) begin
            valid_out <= 0;
            latency_cnt <= 1;
        end else if (run) begin
            latency_cnt <= delay_cfg + `XUNITF_LATENCY;
            valid_out <= 0;
        end else begin
            if (|latency_cnt) begin
                latency_cnt <= latency_cnt - 1;
            end else begin
                latency_cnt <= latency_cnt;
            end
            valid_out <= (latency_cnt <= 1);
        end
    end

endmodule
