extern "C"{
#include <stdio.h>
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"

#include "iob-timer.h"
#include "iob-eth.h"

}

#include "versat_sha.hpp"

#define HASH_SIZE (256/8)

// Pointer to DDR_MEM
#ifdef PC
    char ddr_mem[100000] = {0};
#else
#ifndef RUN_EXTMEM
    char *ddr_mem = (char*) (EXTRA_BASE);
#else
    char *ddr_mem = (char*) ((1<<(FIRM_ADDR_W)));
#endif
#endif

/* read integer value
 * return number of bytes read */
int get_int(char* ptr, unsigned int *i_val){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_int: invalid pointer\n");
        return -1;
    }
    /* read 1 byte at a time
     * write to int */
    *i_val = (unsigned char) ptr[3];
    *i_val <<= 8;
    *i_val += (unsigned char) ptr[2];
    *i_val <<= 8;
    *i_val += (unsigned char) ptr[1];
    *i_val <<= 8;
    *i_val += (unsigned char) ptr[0];
    return sizeof(int);
}

/* get pointer to message and increment pointer */
/* get pointer to message
 * return size of message */
int get_msg(char *ptr, uint8_t **msg_ptr, int size){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_msg: invalid pointer\n");
        return -1;
    }
    /* update message pointer */
    *msg_ptr = (uint8_t*) ptr;
    return sizeof(uint8_t)*size;
}

/* copy memory from pointer to another */
void mem_copy(uint8_t *src_buf, char *dst_buf, int size){
    if(src_buf == NULL || dst_buf == NULL){
        printf("mem_copy: invalid pointer\n");
        return;
    }
    int i = 0;
    while(i<size){
        dst_buf[i] = src_buf[i];
        i++;
    }
    return;
}

/* copy mesage to pointer, update pointer to after message
 * returns number of chars written
 */
int save_msg(char *ptr, uint8_t* msg, int size){
    if(ptr == NULL || msg == NULL || size < 0 ){
        printf("save_msg: invalid inputs\n");
        return -1;
    }

    // copy message to pointer
    mem_copy(msg, ptr, size); 
    return sizeof(uint8_t)*size;
}


int main(int argc, const char* argv[])
{
  int INPUT_FILE_SIZE = 4096;
  uint8_t digest[256] = {0};
  unsigned int num_msgs = 0;
  unsigned int msg_len = 0;
  int i = 0;

  int din_size = 0, din_ptr = 0;
  // input file points after versat sha space
  char *din_fp = (char*) (ddr_mem + VERSAT_SHA_W_PTR_NBYTES);

  int dout_size = 0, dout_ptr = 0;
  char *dout_fp = NULL;

  //init uart
  uart_init(UART_BASE,FREQ/BAUD);   

  //init timer
  timer_init(TIMER_BASE);

  //init ethernet
  eth_init(ETHERNET_BASE);

  //instantiate versat 
  Versat versatInst = {};
  Versat* versat = &versatInst;
  InitVersat(versat,VERSAT_BASE,1); 

  // Sha specific units
  // Need to RegisterFU, can ignore return value 
#ifdef PC
  RegisterUnitF(versat);
  RegisterUnitM(versat);

  ParseVersatSpecification(versat,"testVersatSpecification.txt");
#endif

  InstantiateSHA(versat);
  printf("After Instantiation SHA\n");

#ifdef SIM
  //Receive input data from uart
  char input_file_name[] = "sim_in.bin";
  din_size = uart_recvfile(input_file_name, &din_fp);
#else
  //Receive input data from ethernet
  din_size = eth_rcv_variable_file(din_fp);
#endif
  printf("ETHERNET: Received file with %d bytes\n", din_size);

  // Calculate output size and allocate output memory
  din_ptr += get_int(din_fp + din_ptr, &num_msgs);
  dout_size = num_msgs*HASH_SIZE;
  // output file starts after input file
  dout_fp = din_fp + din_size;

  uint8_t *msg = NULL;

  //Message test loop
  for(i=0; i< num_msgs; i++){
    // Parse message and length
    din_ptr += get_int(din_fp + din_ptr, &msg_len);
    din_ptr += get_msg(&(din_fp[din_ptr]), &msg, ((msg_len) ? msg_len : 1) );

    versat_sha256(digest,msg,msg_len);

    //save to memory
    dout_ptr += save_msg(&(dout_fp[dout_ptr]), digest, HASH_SIZE);
  }

#ifdef SIM
  // send message digests via uart
  char output_file_name[] = "soc-out.bin";
  uart_sendfile(output_file_name, dout_size, dout_fp); 
#else
  // send message digests via ethernet
  eth_send_variable_file(dout_fp, dout_size);
#endif
  printf("ETHERNET: Sent file with %d bytes\n", dout_size);

  uart_finish();
}
