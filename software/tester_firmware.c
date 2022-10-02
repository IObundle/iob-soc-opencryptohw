/*********************************************************
 *                    Tester Firmware                    *
 *********************************************************/
#include "stdlib.h"
#include <stdio.h>
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "iob-eth.h"
#include "printf.h"
#include "string.h"

// set pointer to DDR base
#if (RUN_EXTMEM==0)  //running firmware from SRAM
  #define DATA_BASE_ADDR (EXTRA_BASE)
#else //running firmware from DDR
  #define DATA_BASE_ADDR ((1<<(FIRM_ADDR_W)))
#endif

#define MAX_FILE_SIZE 100000

#define INPUT_FILENAME "sim_in.bin"
#define OUTPUT_FILENAME "soc_out.bin"

//Global buffers for messages
char c, msgBuffer[512];

// Send signal by uart to receive file by ethernet
int uart_recvfile_ethernet(char* file_name) {
  uart_puts(UART_PROGNAME);
  uart_puts (": requesting to receive by ethernet file\n");

  //send file receive by ethernet request
  uart_putc (0x12);

  //send file name (including end of string)
  uart_puts(file_name); uart_putc(0);

  //receive file size
  int file_size = (unsigned int) uart_getc();
  file_size |= ((unsigned int) uart_getc()) << 8;
  file_size |= ((unsigned int) uart_getc()) << 16;
  file_size |= ((unsigned int) uart_getc()) << 24;

  //send ACK before receiving file
  uart_putc(ACK);

  return file_size;
}

//This function receives lines from UART connected to UUT, and relays those messages to the UART connected to console.
//It adds the prefix "[SUT] " to every line relayed
//The finalMsg argument is the last string expected to be received by the UUT, terminating this function
void relayUUTMessagesUntil(char finalMsg[]){
  uint32_t i, startOfLine;
  IOB_UART_INIT_BASEADDR(UART1_BASE); //Switch to instance 1 of UART (Connected to SUT)

  //Store line message received from SUT
  i=0;
  do{
	  startOfLine=i;
	  for(; (c=uart_getc())!='\n'; i++) msgBuffer[i] = c;
	  msgBuffer[i++] = '\n'; //End line in buffer
  }while(strcmp(msgBuffer+startOfLine,finalMsg)); //Break after final boot message
  msgBuffer[i] = '\0'; //End of message

  IOB_UART_INIT_BASEADDR(UART0_BASE);//Switch to instance 0 of UART (Connected to console)
  //Print message previously received from SUT
  i=0;
  while(msgBuffer[i]!='\0'){
	  printf("[SUT] ");
	  while((c=msgBuffer[i++])!='\n')uart_putc(c);
	  uart_putc('\n');
  }
}

int main()
{
  uint32_t i, file_size[2];
  //Create pointers for two file buffers stored in DDR. Start first buffer leaving at least 2*MAX_FILE_SIZE of free space before it, since this space can be used by the SUT.
  char *file_buffer[2] = {(char *) DATA_BASE_ADDR + 2*MAX_FILE_SIZE, (char *) DATA_BASE_ADDR + 3*MAX_FILE_SIZE};

  //Init peripherals
  uart_init(UART0_BASE,FREQ/BAUD);   
  eth_init(ETHERNET0_BASE);

  uart_puts("\n\nHello from tester!\n\n\n");

  uart_init(UART1_BASE,FREQ/BAUD); //Init and switch to instance 1 of UART (Connected to SUT)

  //Wait for ENQ signal from SUT
  while(uart_getc()!=ENQ);
  //Send ack to sut
  uart_putc(ACK);
  
  //Receive and print SUT boot messages
  relayUUTMessagesUntil("Boot complete!\n");

  //Send file receive request by ethernet
  file_size[0] = uart_recvfile_ethernet(INPUT_FILENAME);
  //Receive requested file by ethernet
  eth_rcv_file(file_buffer[0],file_size[0]);

  //Switch and init instance 1 of ETHERNET (Connected to SUT)
  eth_init(ETHERNET1_BASE);

  //Send input file by ethernet to SUT
  eth_send_variable_file(file_buffer[0], file_size[0]);

  //Switch back to instance 1 of UART (Connected to SUT)
  IOB_UART_INIT_BASEADDR(UART1_BASE);

  //Store message received from SUT
  for(i=0; (c=uart_getc())!='\n'; i++)
	  msgBuffer[i] = c;
  msgBuffer[i] = '\n';

  //Switch to instance 0 of UART (Connected to console)
  IOB_UART_INIT_BASEADDR(UART0_BASE);

  //Print message previously received from SUT
  printf("[SUT] ");
  for(i=0; (c=msgBuffer[i])!='\n'; i++)
	  uart_putc(c);
  uart_putc('\n');

  //Receive output file by ethernet from SUT
  file_size[0] = eth_rcv_variable_file(file_buffer[0]);
  
  //Switch to instance 0 of ETHERNET (Connected to Host machine)
  IOB_ETH_INIT_BASEADDR(ETHERNET0_BASE);

  //Request to receive expected output file by ethernet from Host machine
  file_size[1] = uart_recvfile_ethernet(OUTPUT_FILENAME);
  //Check if file sizes are different
  if(file_size[0]!=file_size[1]){
    printf("Test failed! Output file size (%d) and expected file size (%d) are different.", file_size[0], file_size[1]);
    uart_finish();
    return -1;
  }
  //Receive expected output file by ethernet from Host machine
  eth_rcv_file(file_buffer[1],file_size[1]);

  //Check if received file is equal to expected file
  for(i=0; i<file_size[0]; i++){
		if(file_buffer[0][i]!=file_buffer[1][i]){
			printf("Test failed! Byte %d of received file with value 0x%x is different from byte in expected file with value 0x%x.", i, file_buffer[0][i], file_buffer[1][i]);
			uart_finish();
			return -1;
		}
  }
  
  uart_puts ("\nTest complete! Output and expected files are equal!.\n\n");

  //End UART0 connection
  uart_finish();
}
