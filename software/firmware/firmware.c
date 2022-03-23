#include "stdlib.h"
#include <stdio.h>
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"

#include "iob-eth.h"

#include "crypto/sha2.h"

#define HASH_SIZE (256/8)

/* read integer value
 * return number of bytes read */
int get_int(char* ptr, int *i_val){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_int: invalid pointer\n");
        return -1;
    }
    /* read 1 byte at a time
     * write to int */
    *i_val = ptr[3];
    *i_val <<= 4;
    *i_val = ptr[2];
    *i_val <<= 4;
    *i_val = ptr[1];
    *i_val <<= 4;
    *i_val = ptr[0];

    return sizeof(int);
}

/* get pointer to message and increment pointer */
/* get pointer to message
 * return size of message */
int get_msg(char *ptr, char **msg_ptr, int size){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_msg: invalid pointer\n");
        return -1;
    }
    /* update message pointer */
    *msg_ptr = ptr;
    return sizeof(char)*size;
}

/* copy memory from pointer to another */
void mem_copy(char *src_buf, char *dst_buf, int size){
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
int save_msg(char *ptr, char* msg, int size){
    if(ptr == NULL || msg == NULL || size < 0 ){
        printf("save_msg: invalid inputs\n");
        return -1;
    }

    // copy message to pointer
    mem_copy(msg, ptr, size); 
    return sizeof(char)*size;
}

int main()
{
  int INPUT_FILE_SIZE = 4096;
  char digest[256];
  int num_msgs = 0;
  int msg_len = 0;
  int i = 0;

  int din_size = 0, din_ptr = 0;
  char *din_fp = (char*) malloc(sizeof(char)*INPUT_FILE_SIZE);
  if(din_fp == NULL){
      printf("Failed to allocate input file memory\n");
      return -1;
  }

  int dout_size = 0, dout_ptr = 0;
  char *dout_fp = NULL;

  //init uart
  uart_init(UART_BASE,FREQ/BAUD);   

  //init ethernet
  eth_init(ETHERNET_BASE);

  //Receive input data from ethernet
  din_size = eth_rcv_variable_file(din_fp);
  printf("ETHERNET: Received file with %d bytes\n", din_size);

  // Calculate output size and allocate output memory
  num_msgs = *( (int*) &(din_fp[din_ptr]));
  din_ptr += sizeof(int);
  dout_size = num_msgs*HASH_SIZE;
  dout_fp = (char*) malloc(sizeof(char)*dout_size);
  if(dout_fp == NULL){
      printf("Failed to allocate output file memory\n");
      free(din_fp);
      return -1;
  }

  char *msg = NULL;

  //Message test loop
  for(i=0; i< num_msgs; i++){
    printf("Msg #%d\n", i);
    // Parse message and length
    din_ptr += get_int(din_fp + din_ptr, &msg_len);

    printf("Msglen: %d\n", msg_len);
    din_ptr += get_msg(&(din_fp[din_ptr]), &msg, ((msg_len) ? msg_len : 1) );

    sha256(digest,msg,msg_len);

    //save to memory
    dout_ptr += save_msg(&(dout_fp[dout_ptr]), digest, HASH_SIZE);
    printf("saved msg\t acc_ptr: %d/%d\n", dout_ptr, HASH_SIZE*num_msgs);
  }

  // send message digests via ethernet
  eth_send_variable_file(dout_fp, dout_size);

  // free allocated memory
  free(din_fp);
  free(dout_fp);

  uart_finish();
}
