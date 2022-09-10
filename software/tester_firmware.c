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

// set pointer to DDR base
#if (RUN_EXTMEM==0)  //running firmware from SRAM
  #define DATA_BASE_ADDR (EXTRA_BASE)
#else //running firmware from DDR
  #define DATA_BASE_ADDR ((1<<(FIRM_ADDR_W)))
#endif

#define MAX_FILE_SIZE 100000

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

int main()
{
  uint32_t i, file_size;
  char msgBuffer[64];
  //Create pointers for two file buffers stored in DDR. Start first buffer leaving at least 2*MAX_FILE_SIZE of free space before it, since this space can be used by the SUT.
  char *file_buffer[2] = {(char *) DATA_BASE_ADDR + 2*MAX_FILE_SIZE, (char *) DATA_BASE_ADDR + 3*MAX_FILE_SIZE};

  //Init peripherals
  uart_init(UART0_BASE,FREQ/BAUD);   
  eth_init(ETHERNET0_BASE);

  uart_puts("\n\nHello from tester!\n\n\n");

  //Switch to instance 1 of UART (Connected to SUT)
  IOB_UART_INIT_BASEADDR(UART1_BASE);
  
  //Wait for ENQ signal from SUT
  while(uart_getc()!=ENQ);
  //Send ack to sut
  uart_putc(ACK);
  
  //Switch to instance 0 of UART (Connected to console)
  IOB_UART_INIT_BASEADDR(UART0_BASE);

  //Send file receive request by ethernet
  file_size = uart_recvfile_ethernet("sim_in.bin"); //FIXME: should this be another file when not in sim?
  //Receive requested file by ethernet
  eth_rcv_file(file_buffer[0],file_size);

  //Switch and init instance 1 of ETHERNET (Connected to SUT)
  eth_init(ETHERNET1_BASE);

  //Send input file by ethernet to SUT
  eth_send_variable_file(file_buffer[0], file_size);

  //Switch back to instance 1 of UART (Connected to SUT)
  IOB_UART_INIT_BASEADDR(UART1_BASE);

  //Store message received from SUT
  for(i=0; (c=uart_getc())!='\n'; i++)
	  msgBuffer[i] = c;
  msgBuffer[i] = '\n';

  //Switch to instance 0 of UART (Connected to console)
  IOB_UART_INIT_BASEADDR(UART0_BASE);

  //Print message previously received from SUT
  printf("[SUT] ")
  for(i=0; (c=msgBuffer[i])!='\n'; i++)
	  uart_putc(c);
  uart_putc('\n');

  //Receive output file by ethernet from SUT
  file_size = eth_rcv_variable_file(file_buffer[0]);
  
  //Switch to instance 0 of ETHERNET (Connected to Host machine)
  IOB_ETH_INIT_BASEADDR(ETHERNET0_BASE);

  //Request to receive expected output file by ethernet from Host machine
  file_size = uart_recvfile_ethernet("soc_out.bin"); //FIXME: should this be another file when not in sim?
  //Receive expected output file by ethernet from Host machine
  eth_rcv_file(file_buffer[1],file_size);

  //Check if received file is equal to expected file
  for(i=0; i<file_size; i++){
		if(file_buffer[0][i]!=file_buffer[1][i]){
			print("Test failed! Byte %d of received file with value 0x%x is different from byte in expected file with value 0x%x.", i, file_buffer[0][i], file_buffer[1][i])
			uart_finish();
			return -1;
		}
  }
  
  uart_puts ("\nTest complete! Output and expected files are equal!.\n\n");

  //End UART0 connection
  uart_finish();
}
