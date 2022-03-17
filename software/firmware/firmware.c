#include "stdlib.h"
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

/* read integer value and increment pointer */
int get_int(char** ptr){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_int: invalid pointer\n");
        return -1;
    }
    /* get int and update pointer */
    int i_val = *( (int*) *ptr);
    *ptr += sizeof(int);
    return i_val;
}

/* get pointer to message and increment pointer */
char *get_msg(char **ptr, int size){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_msg: invalid pointer\n");
        return NULL;
    }
    /* get int and update pointer */
    char *msg = *ptr;
    *ptr += sizeof(char)*size;
    return msg;
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
int save_msg(char **ptr, char* msg, int size){
    if(ptr == NULL || msg == NULL || size < 0 ){
        printf("save_msg: invalid inputs\n");
        return -1;
    }

    // copy message to pointer
    mem_copy(msg, *ptr, size); 
    // update pointer to position after message
    *ptr += sizeof(char)*size;
    return size;
}

char HexTable[16] = "0123456789abcdef";

char* GetHexadecimal(const char* text, int str_size){
  static char buffer[2048+1];
  int i;

  for(i = 0; i< str_size; i++){
    if(i * 2 > 2048){
      printf("\n\n<GetHexadecimal> Maximum size reached\n\n");
      buffer[i*2] = '\0';
      return buffer;
    }

    int ch = (int) ((unsigned char) text[i]);

    buffer[i*2] = HexTable[ch >> 4]; // ch/16
    buffer[i*2+1] = HexTable[ch & 0xF]; // ch % 16
  }

  buffer[(i)*2] = '\0';

  return buffer;
}

int main()
{
  char digest[256];
  char *hex_string;

#ifdef PC
  char *ddr_mem = (char*) malloc(sizeof(char)*10000);
  if(ddr_mem == NULL){
      printf("Failed to allocate PC-emul DDR_MEM\n");
      return -1;
  }
#else
  /* char static_mem[10000] = {0}; */
  char *ddr_mem = (char*) DDR_MEM;
  /* char *ddr_mem = (char*) static_mem; */
#endif // ifndef PC

  char *din_fp = ddr_mem; /* input data at ddr memory start */
  char *dout_fp = NULL, *dout_fp_start = NULL;
  int din_size = 0;
  int dout_size = 0;

  int i = 0;
  //init uart
  uart_init(UART_BASE,FREQ/BAUD);   

  //init timer
  timer_init(TIMER_BASE);

  //init ethernet
  eth_init(ETHERNET_BASE);

  //Receive input data from ethernet
  din_size = eth_rcv_variable_file(din_fp);
  printf("ETHERNET: Received file with %d bytes\n", din_size);
  // Set output data pointer after data in
  dout_fp_start = dout_fp = ddr_mem + din_size;

  //Parse input data
  int num_msgs = get_int(&din_fp);
  int msg_len = 0;
  char *msg = NULL;

#ifdef PROFILE
  PROF_START(global)
  PROF_START(printf)
#endif
  /* printf("[L = %d]\n", HASH_SIZE); */
#ifdef PROFILE
  PROF_STOP(printf)
#endif

  //Message test loop
  for(i=0; i< num_msgs; i++){
    printf("Msg #%d\n", i);
    // Parse message and length
    msg_len = get_int(&din_fp);
    msg = get_msg(&din_fp, (msg_len) ? msg_len : 1);

#ifdef PROFILE
    PROF_START(sha256)
#endif
    sha256(digest,msg,msg_len);
    printf("sha256 done\n");
#ifdef PROFILE
    PROF_STOP(sha256)
    PROF_START(GetHexadecimal)
#endif
    /* hex_string = GetHexadecimal(msg, (msg_len) ? msg_len : 1); */
#ifdef PROFILE
    PROF_STOP(GetHexadecimal)
    PROF_START(printf)
#endif
    /* printf("\nLen = %d\n", msg_len*8); */
    /* printf("Msg = %s\n", hex_string); */
#ifdef PROFILE
    PROF_STOP(printf)
    PROF_START(GetHexadecimal)
#endif
    /* hex_string = GetHexadecimal(digest, HASH_SIZE); */
#ifdef PROFILE
    PROF_STOP(GetHexadecimal)
    PROF_START(printf)
#endif
    /* printf("MD = %s\n", GetHexadecimal(digest, HASH_SIZE)); */
#ifdef PROFILE
    PROF_STOP(printf)
#endif
    //save to memory
    dout_size += save_msg(&dout_fp, digest, HASH_SIZE);
    printf("saved msg\t acc_ptr: %d/%d\n", dout_size, 32*num_msgs);
  }

  // send message digests via ethernet
  eth_send_variable_file(dout_fp_start, dout_size);

#ifdef PROFILE
    PROF_START(printf)
#endif
  printf("\n");
#ifdef PROFILE
    PROF_STOP(printf)
  // Finish profile and report execution times
  PROF_STOP(global)
  profile_report();
#endif

#ifdef PC
    free(ddr_mem);
#endif
  uart_finish();
}
