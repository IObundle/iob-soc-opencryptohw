#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"
#include "string.h"

#include "versat.h"
#include "iob_timer.h"

#include "crypto/sha2.h"

#include "../test_vectors.h"
#include "unitWrapper.h"
#include "unitVerilogWrappers.h"

#define HASH_SIZE (256/8)

void versat_sha256(uint8_t *out, const uint8_t *in, size_t inlen);

static void store_bigendian_32(uint8_t *x, uint64_t u) {
    x[3] = (uint8_t) u;
    u >>= 8;
    x[2] = (uint8_t) u;
    u >>= 8;
    x[1] = (uint8_t) u;
    u >>= 8;
    x[0] = (uint8_t) u;
}

static uint32_t initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint32_t kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint32_t kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint32_t kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint32_t kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint32_t* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

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

int32_t* TerminateFunction(FUInstance* inst){
   static int delay = 80;

   delay -= 1;

   if(delay == 0){
      delay = 80;
      return (int32_t*) 1;
   }
   else{
      return 0;
   }
}

static int unitFBits[] = {8};
static int readMemory_[64];
static int* readMemory;
static int writeMemory_[64];
static int* writeMemory;

// GLOBALS
Versat* versat;
Accelerator* accel;
FUInstance* wMem;
FUInstance* read;
FUInstance* stateReg[8];
bool initVersat = false;

void ClearCache(){
   int count = 0;

   for(int i = 0; i < 1024; i += 16){
      count += writeMemory[i];
   }

   printf("Clear cache: %d [ignore]\n\n",count);
}

int main()
{
   //init uart
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);

   // Force alignment on a 64 byte boundary
   readMemory = readMemory_;
   writeMemory = writeMemory_;
   while((((int)readMemory) & (16 * sizeof(int) - 1)) != 0)
      readMemory += 1;

   while((((int)writeMemory) & (16 * sizeof(int) - 1)) != 0)
      writeMemory += 1;

   Versat versatInstance = {};
   versat = &versatInstance;

   InitVersat(versat,VERSAT_BASE);

   // Versat specific units
   FU_Type ADD = RegisterAdd(versat);
   FU_Type REG = RegisterReg(versat);
   FU_Type VREAD = RegisterVRead(versat);
   FU_Type VWRITE = RegisterVWrite(versat);
   FU_Type MEM = RegisterMem(versat,10);
   FU_Type DEBUG = RegisterDebug(versat);

   // Sha specific units
   FU_Type UNIT_F = RegisterUnitF(versat);
   FU_Type UNIT_M = RegisterUnitM(versat);

   accel = CreateAccelerator(versat);

   read = CreateFUInstance(accel,VREAD);
   {
      volatile VReadConfig* c = (volatile VReadConfig*) read->config;

      // Versat side
      c->iterB = 1;
      c->incrB = 1;
      c->perB = 16;
      c->dutyB = 16;
      c->delayB = 2;

      // Memory side
      c->incrA = 1;
      c->iterA = 1;
      c->perA = 16;
      c->dutyA = 16;
      c->size = 8;
      c->int_addr = 0;
      c->pingPong = 1;
      c->ext_addr = (int) readMemory; // Some place so no segfault if left unconfigured
   }

   #if 1
   FUInstance* unitF[4];
   for(int i = 0; i < 4; i++){
      unitF[i] = CreateFUInstance(accel,UNIT_F);
      volatile UnitFConfig* c = (volatile UnitFConfig*) unitF[i]->config;

      c->configDelay = 3 + (i * 17);

      if(i > 0){
         for(int ii = 0; ii < 8; ii++){
            ConnectUnits(versat,unitF[i-1],ii,unitF[i],ii);
         }
      }
   }

   FUInstance* kMem[4]; // Could be done by using 1 memory, change later 
   for(int i = 0; i < 4; i++){
      kMem[i] = CreateFUInstance(accel,MEM);
      volatile MemConfig* c = (volatile MemConfig*) kMem[i]->config;
      
      c->iterA = 1;
      c->incrA = 1;
      c->delayA = (17*i);
      c->perA = 16;
      c->dutyA = 16;

      for (int ii = 0; ii < 16; ii++)
      {
         VersatUnitWrite(versat,kMem[i],ii,kConstants[i][ii]);
      }
   }
   
   for(int i = 0; i < 8; i++){
      stateReg[i] = CreateFUInstance(accel,REG);

      {
         volatile RegConfig* config = (volatile RegConfig*) stateReg[i]->config; 

         config->writeDelay = 3 + (16*4) + 5;
         VersatUnitWrite(versat,stateReg[i],0,initialStateValues[i]);
      }

      ConnectUnits(versat,stateReg[i],0,unitF[0],i);
   }
   
   for(int i = 0; i < 8; i++){
      FUInstance* add = CreateFUInstance(accel,ADD);

      ConnectUnits(versat,stateReg[i],0,add,0);
      ConnectUnits(versat,unitF[3],i,add,1);
      ConnectUnits(versat,add,0,stateReg[i],0);
   }

   ConnectUnits(versat,read,0,unitF[0],8);

   FUInstance* unitM[3];
   for(int i = 0; i < 3; i++){
      unitM[i] = CreateFUInstance(accel,UNIT_M);

      volatile UnitMConfig* c = (volatile UnitMConfig*) unitM[i]->config;

      c->configDelay = 3 + 16 * (i+1) + i;

      ConnectUnits(versat,unitM[i],0,unitF[i+1],8);
      if(i != 0){
         ConnectUnits(versat,unitM[i-1],0,unitM[i],0);
      }
   }
   ConnectUnits(versat,read,0,unitM[0],0);

   for(int i = 0; i < 4; i++){
      ConnectUnits(versat,kMem[i],0,unitF[i],9);
   }
   #endif

   // Gera o versat.
   OutputVersatSource(versat,"versat_defs.vh","versat_instance.v");

   char digest[256];

   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

#ifdef AUTOMATIC_TEST
   int i = 0;

   printf("[L = %d]\n", HASH_SIZE);

   //Message test loop
   for(i=0; i< NUM_MSGS; i++){
      versat_sha256(digest,msg_array[i],msg_len[i]);
      printf("\nLen = %d\n", msg_len[i]*8);
      printf("Msg = %s\n", GetHexadecimal(msg_array[i], (msg_len[i]) ? msg_len[i] : 1));
      printf("MD = %s\n",GetHexadecimal(digest, HASH_SIZE));
   }
   printf("\n");
#else
   printf("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa\n");
   
   timer_start();
   timer_reset();
   
   versat_sha256(digest,msg_64,64);
   
   unsigned int count = timer_time_us();
   printf("%s\n",GetHexadecimal(digest, HASH_SIZE));
   printf("Took %d us\n",count);

   //OutputMemoryMap(versat);
#endif
   
   uart_finish();

   return 0;
}

static uint32_t load_bigendian_32(const uint8_t *x) {
    return (uint32_t)(x[3]) | (((uint32_t)(x[2])) << 8) |
           (((uint32_t)(x[1])) << 16) | (((uint32_t)(x[0])) << 24);
}

static size_t versat_crypto_hashblocks_sha256(const uint8_t *in, size_t inlen) {
    uint32_t w[16];

   {
      volatile VReadConfig* c = (volatile VReadConfig*) read->config;
      c->ext_addr = (int) w;
   }

    while (inlen >= 64) {
         w[0]  = load_bigendian_32(in + 0);
         w[1]  = load_bigendian_32(in + 4);
         w[2]  = load_bigendian_32(in + 8);
         w[3]  = load_bigendian_32(in + 12);
         w[4]  = load_bigendian_32(in + 16);
         w[5]  = load_bigendian_32(in + 20);
         w[6]  = load_bigendian_32(in + 24);
         w[7]  = load_bigendian_32(in + 28);
         w[8]  = load_bigendian_32(in + 32);
         w[9]  = load_bigendian_32(in + 36);
         w[10] = load_bigendian_32(in + 40);
         w[11] = load_bigendian_32(in + 44);
         w[12] = load_bigendian_32(in + 48);
         w[13] = load_bigendian_32(in + 52);
         w[14] = load_bigendian_32(in + 56);
         w[15] = load_bigendian_32(in + 60);

         // Loads data + performs work 
         AcceleratorRun(versat,accel,NULL,TerminateFunction);
         
         // Since vread currently reads before outputing, this piece of code is set before aceleratorRun
         // Eventually it will need to be moved to after
         #if 1
         if(!initVersat){
            for(int i = 0; i < 8; i++){
               VersatUnitWrite(versat,stateReg[i],0,initialStateValues[i]);
            }
            initVersat = true;
         }
         #endif

         in += 64;
         inlen -= 64;
    }

    return inlen;
}

void versat_sha256(uint8_t *out, const uint8_t *in, size_t inlen) {
    uint8_t padded[128];
    uint64_t bytes = 0 + inlen;

    versat_crypto_hashblocks_sha256(in, inlen);
    in += inlen;
    inlen &= 63;
    in -= inlen;

    for (size_t i = 0; i < inlen; ++i) {
        padded[i] = in[i];
    }
    padded[inlen] = 0x80;

    if (inlen < 56) {
        for (size_t i = inlen + 1; i < 56; ++i) {
            padded[i] = 0;
        }
        padded[56] = (uint8_t) (bytes >> 53);
        padded[57] = (uint8_t) (bytes >> 45);
        padded[58] = (uint8_t) (bytes >> 37);
        padded[59] = (uint8_t) (bytes >> 29);
        padded[60] = (uint8_t) (bytes >> 21);
        padded[61] = (uint8_t) (bytes >> 13);
        padded[62] = (uint8_t) (bytes >> 5);
        padded[63] = (uint8_t) (bytes << 3);
        versat_crypto_hashblocks_sha256(padded, 64);
    } else {
        for (size_t i = inlen + 1; i < 120; ++i) {
            padded[i] = 0;
        }
        padded[120] = (uint8_t) (bytes >> 53);
        padded[121] = (uint8_t) (bytes >> 45);
        padded[122] = (uint8_t) (bytes >> 37);
        padded[123] = (uint8_t) (bytes >> 29);
        padded[124] = (uint8_t) (bytes >> 21);
        padded[125] = (uint8_t) (bytes >> 13);
        padded[126] = (uint8_t) (bytes >> 5);
        padded[127] = (uint8_t) (bytes << 3);
        versat_crypto_hashblocks_sha256(padded, 128);
    }
    
    // Does the last run with valid data
    AcceleratorRun(versat,accel,NULL,TerminateFunction);

    for (size_t i = 0; i < 8; ++i) {
        uint32_t val = *stateReg[i]->state;

        store_bigendian_32(&out[i*4],val);
    }

    initVersat = false; // At the end of each run
}

/*

Things to do:

Move non specific versat_instance code upwards
Implement delay as a standard unit connection
Implement done as a standard unit connection
Implement valid data time computation
   - Need to provide a range of valid data (Meaning that, after delay, the unit will produce a constant stream of valid data for N cycles)
   - Need to differenciate between units that produce data, process data and consume data (produce and consume have a range of time values, the process do not)
   - This does not take into account IO operations.

Cleanup the versat value computations and code generation
   - Add the versat computation function to embedded as well
   - Refactor the code generation portion into something that is easier to see what it is producing.

*/
