#include "stdlib.h"
#include <stdio.h>
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"

#include "crypto/sha2.h"

#include "test_vectors.h"

#define HASH_SIZE (256/8)

char GetHexadecimalChar(int value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'a' + (value - 10);
  }
}

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

    buffer[i*2] = GetHexadecimalChar(ch / 16);
    buffer[i*2+1] = GetHexadecimalChar(ch % 16);
  }

  buffer[(i)*2] = '\0';

  return buffer;
}

static char testBuffer[10000];

int main()
{
  char digest[256];

  int i = 0;
  //init uart
  uart_init(UART_BASE,FREQ/BAUD);   

  printf("[L = %d]\n", HASH_SIZE);

  //Message test loop
  for(i=0; i< NUM_MSGS; i++){
    sha256(digest,msg_array[i],msg_len[i]);
    printf("\nLen = %d\n", msg_len[i]*8);
    printf("Msg = %s\n", GetHexadecimal(msg_array[i], (msg_len[i]) ? msg_len[i] : 1));
    printf("MD = %s\n",GetHexadecimal(digest, HASH_SIZE));
  }
  printf("\n");
  uart_finish();
}
