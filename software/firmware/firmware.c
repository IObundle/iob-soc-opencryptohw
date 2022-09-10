#include <stdio.h>
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"

#include "iob-timer.h"
#include "iob-eth.h"

#ifdef PROFILE
#include "profile.h"
#endif

#include "crypto/sha2.h"

#define HASH_SIZE (256/8)

// Pointer to DDR_MEM
#ifdef PC
    char ddr_mem[100000] = {0};
#else
    char *ddr_mem = (char*) EXTRA_BASE;
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
  // input file points to ddr_mem start
  char *din_fp = (char*) ddr_mem;

  int dout_size = 0, dout_ptr = 0;
  char *dout_fp = NULL;

  //init uart
  uart_init(UART_BASE,FREQ/BAUD);   

  //init timer
  timer_init(TIMER_BASE);

  //init ethernet
  eth_init(ETHERNET_BASE);

  //Receive input data from ethernet
  din_size = eth_rcv_variable_file(din_fp);

  printf("ETHERNET: Received file with %d bytes\n", din_size);
#ifdef PROFILE
  PROF_START(global)
  PROF_START(mem)
#endif

  // Calculate output size and allocate output memory
  din_ptr += get_int(din_fp + din_ptr, &num_msgs);
  dout_size = num_msgs*HASH_SIZE;
  // output file starts after input file
  dout_fp = din_fp + din_size;

  char *msg = NULL;

#ifdef PROFILE
  PROF_STOP(mem)
#endif
  //Message test loop
  for(i=0; i< num_msgs; i++){
#ifdef PROFILE
    PROF_START(mem)
#endif
    // Parse message and length
    din_ptr += get_int(din_fp + din_ptr, &msg_len);
    din_ptr += get_msg(&(din_fp[din_ptr]), &msg, ((msg_len) ? msg_len : 1) );

#ifdef PROFILE
    PROF_STOP(mem)
    PROF_START(sha256)
#endif
    sha256(digest,msg,msg_len);
#ifdef PROFILE
    PROF_STOP(sha256)
    PROF_START(mem)
#endif

    //save to memory
    dout_ptr += save_msg(&(dout_fp[dout_ptr]), digest, HASH_SIZE);
#ifdef PROFILE
    PROF_STOP(mem)
#endif
  }

#ifdef PROFILE
  // Finish profile
  PROF_STOP(global)
#endif

  // send message digests via ethernet
  eth_send_variable_file(dout_fp, dout_size);
  printf("ETHERNET: Sent file with %d bytes\n", dout_size);

#ifdef PROFILE
  // report execution times
  profile_report();
#endif

  uart_finish();
}
