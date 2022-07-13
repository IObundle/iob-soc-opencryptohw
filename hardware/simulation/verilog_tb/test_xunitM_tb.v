`timescale 1ns / 1ps

`include "system.vh"


`define INPUT_SIZE (64/4)
`define OUTPUT_SIZE (64/4)

// Implicitly know FU latency
`define XUNITM_LATENCY (17)

module xunitM_tb;

  parameter realtime clk_per = 1s/`FREQ;

  //clock
  reg clk = 1;
  always #(clk_per/2) clk = ~clk;

  //reset
  reg reset = 0;

  // run
  reg run = 0;

  // in/out
  reg [31:0] in;
  wire [31:0] out;

  // configurations
  reg [7:0] delay_cfg = 8'h00;

  //iterator
  integer                i = 0;

  //test vectors
  reg [31:0] test_data_in[`INPUT_SIZE-1:0];
  reg [31:0] test_data_out[`OUTPUT_SIZE-1:0];
  integer fp_in = 0;
  integer fp_out = 0;
  integer tmp = 0;

  // validation
  reg check_done = 0;
  reg valid_out = 0;
  reg [7:0] latency_cnt;
  integer nerrors = 0;
  reg [4:0] result_addr = 0;
  reg [31:0] expected;

  initial begin
`ifdef VCD
    $dumpfile("xunitM_tb.vcd");
    $dumpvars;
`endif
    run = 0;
    result_addr = 0;

    // read input and validation .bin files
    fp_in = $fopen("xunitM_in.bin", "rb");
    tmp = $fread(test_data_in, fp_in);
    $fclose(fp_in);
    fp_out = $fopen("xunitM_out.bin", "rb");
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

    for(i=0; i < `INPUT_SIZE; i = i+1) begin
        in = test_data_in[i];
        @(posedge clk) #1;
    end

    // wait for all output data
    while(check_done)
        @(posedge clk);

    // wait some more cycles
    repeat (10) @(posedge clk) #1;

    // write test results
    if(nerrors) begin
        $display("xunitM test FAILED with %d errors out of %d\n", nerrors, `OUTPUT_SIZE);
        $fatal; // return non-zero exit status
    end else begin
        $display("xunitM test PASSED with %d errors out of %d\n", nerrors, `OUTPUT_SIZE);
    end

    $finish;

  end

    xunitM uut(
        .clk(clk),
        .rst(reset),

        .run(run),

        .in0(in),
        .out0(out),

        .done(),

        .delay0(delay_cfg)
    );

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
            latency_cnt <= delay_cfg + `XUNITM_LATENCY;
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
