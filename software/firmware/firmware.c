#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"

#include "board_crypto/sha2.h"

char GetHexadecimalChar(int value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'A' + (value - 10);
  }
}

char* GetHexadecimal(const char* text){
  static char buffer[2048+1];
  int i;

  for(i = 0; text[i] != '\0'; i++){
    if(i * 2 > 2048){
      printf("\n\n<GetHexadecimal> Maximum size reached\n\n");
      buffer[i*2] = '\0';
      return buffer;
    }

    int ch = (int) ((unsigned char) text[i]);

    buffer[i*2] = GetHexadecimalChar(ch / 16);
    buffer[i*2+1] = GetHexadecimalChar(ch % 16);
  }

  buffer[(i+1)*2] = '\0';

  return buffer;
}

int main()
{
  char digest[256];
  //init uart
  uart_init(UART_BASE,FREQ/BAUD);   

  sha256(digest,"",0);
  
  printf("\n\n%s\n\n",GetHexadecimal(digest));

  uart_finish();
}
