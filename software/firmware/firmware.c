#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"
#include "versat.h"
#include "FU_Defs.h"
#include "string.h"

//#include "crypto/sha2.h"

#include "../test_vectors.h"
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

int32_t* MemTerminateFunction(FUInstance* inst){
   MemExtra* e = (MemExtra*) inst->extraData;
   if(e->done)
      return (int32_t*) 1;

   return (int32_t*) 0;
}

int32_t* CycleTerminateFunction(FUInstance* inst){
   static int cycles = 60;

   cycles -= 1;

   if(cycles == 0){
      cycles = 60;
      return (int32_t*) 1;
   }

   return (int32_t*) 0;
}

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

#if 1
#define SHR(x, c) ((x) >> (c))
#define ROTR_32(x, c) (((x) >> (c)) | ((x) << (32 - (c))))
#define ROTR_64(x, c) (((x) >> (c)) | ((x) << (64 - (c))))

#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define Sigma0_32(x) (ROTR_32(x, 2) ^ ROTR_32(x,13) ^ ROTR_32(x,22))
#define Sigma1_32(x) (ROTR_32(x, 6) ^ ROTR_32(x,11) ^ ROTR_32(x,25))
#define sigma0_32(x) (ROTR_32(x, 7) ^ ROTR_32(x,18) ^ SHR(x, 3))
#define sigma1_32(x) (ROTR_32(x,17) ^ ROTR_32(x,19) ^ SHR(x,10))

#define M_32(w0, w14, w9, w1) w0 = sigma1_32(w14) + (w9) + sigma0_32(w1) + (w0);

#define F_32(w, k)                                   \
    T1 = h + Sigma1_32(e) + Ch(e, f, g) + (k) + (w); \
    T2 = Sigma0_32(a) + Maj(a, b, c);                \
    h = g;                                           \
    g = f;                                           \
    f = e;                                           \
    e = d + T1;                                      \
    d = c;                                           \
    c = b;                                           \
    b = a;                                           \
    a = T1 + T2;

typedef struct {
   int delay;
} UnitFConfig;

typedef struct {
   uint32_t a,b,c,d,e,f,g,h;
   uint32_t delay;
} UnitFExtra;

int32_t* UnitFStartFunction(FUInstance* instance){
   UnitFConfig* config = (UnitFConfig*) instance->config;
   UnitFExtra* extra = (UnitFExtra*) instance->extraData;   

   extra->delay = config->delay;

   return NULL;
}

static uint32_t initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint32_t kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint32_t kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint32_t kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint32_t kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint32_t* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

int32_t* UnitFUpdateFunction(FUInstance* instance){
   static int32_t results[8];

   UnitFConfig* config = (UnitFConfig*) instance->config;
   UnitFExtra* extra = (UnitFExtra*) instance->extraData;

   memset(results,0,sizeof(int32_t) * 8);

   if(extra->delay == 1){
      extra->a = (uint32_t) GetInputValue(instance,0);
      extra->b = (uint32_t) GetInputValue(instance,1);
      extra->c = (uint32_t) GetInputValue(instance,2);
      extra->d = (uint32_t) GetInputValue(instance,3);
      extra->e = (uint32_t) GetInputValue(instance,4);
      extra->f = (uint32_t) GetInputValue(instance,5);
      extra->g = (uint32_t) GetInputValue(instance,6);
      extra->h = (uint32_t) GetInputValue(instance,7);
      
      extra->delay = 0;

      return results;
   } else if(extra->delay > 0){
      extra->delay -= 1;
      return results;
   }

   uint32_t a = (uint32_t) extra->a;
   uint32_t b = (uint32_t) extra->b;
   uint32_t c = (uint32_t) extra->c;
   uint32_t d = (uint32_t) extra->d;
   uint32_t e = (uint32_t) extra->e;
   uint32_t f = (uint32_t) extra->f;
   uint32_t g = (uint32_t) extra->g;
   uint32_t h = (uint32_t) extra->h;

   uint32_t w = (uint32_t) GetInputValue(instance,8);
   uint32_t k = (uint32_t) GetInputValue(instance,9);

   uint32_t T1,T2;

   F_32(w,k)

   extra->a = results[0] = a;
   extra->b = results[1] = b;
   extra->c = results[2] = c;
   extra->d = results[3] = d;
   extra->e = results[4] = e;
   extra->f = results[5] = f;
   extra->g = results[6] = g;
   extra->h = results[7] = h;

   return results;
}

typedef struct{
   int delay;
} UnitMConfig;


typedef struct{
   uint32_t w[16];
   int delay;
} UnitMExtra;

int32_t* UnitMStartFunction(FUInstance* instance){
   UnitMConfig* config = (UnitMConfig*) instance->config;
   UnitMExtra* extra = (UnitMExtra*) instance->extraData;   

   extra->delay = config->delay;

   return NULL;
}

int32_t* UnitMUpdateFunction(FUInstance* instance){
   static int32_t result;

   UnitMConfig* config = (UnitMConfig*) instance->config;
   UnitMExtra* extra = (UnitMExtra*) instance->extraData;

   uint32_t w0 = extra->w[0];

   M_32(w0,extra->w[14],extra->w[9],extra->w[1])

   for(int i = 0; i < (16-1); i++){
      extra->w[i] = extra->w[i+1];
   }

   result = w0;

   if(extra->delay > 0){      
      extra->w[15] = GetInputValue(instance,0);
      extra->delay -= 1;
   } else {
      extra->w[15] = result;
   }

   return &result;
}
#endif 

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

#ifdef PC
#define SI 400000
#else
#define SI 32
#endif

static int unitFBits[] = {8};
static int readMemory_[64];
static int* readMemory;
static int writeMemory_[SI];
static int* writeMemory;

// GLOBALS
Versat* versat;
Accelerator* accel;
FUInstance* wMem;
FUInstance* stateReg[8];

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

   readMemory = readMemory_;
   writeMemory = writeMemory_;

   while((((int)readMemory) & (16 * sizeof(int) - 1)) != 0)
      readMemory += 1;

   while((((int)writeMemory) & (16 * sizeof(int) - 1)) != 0)
      writeMemory += 1;

   Versat versatInstance = {};
   versat = &versatInstance;

   InitVersat(versat,VERSAT_BASE);

   versat->useShadowRegisters = 0;

   FU_Type ADD = RegisterFU(versat,"xadd",
                                    2, // n inputs
                                    1, // n outputs
                                    0, // Config
                                    NULL,
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    false, // IO
                                    AddExtraSize(), // Extra memory
                                    AddInitializeFunction,
                                    AddStartFunction,
                                    AddUpdateFunction);

   #if 1
   FU_Type REG = RegisterFU(versat,"xreg",
                                    1, // n inputs
                                    1, // n outputs
                                    ARRAY_SIZE(regBits), // Config
                                    regBits,
                                    ARRAY_SIZE(regStates), // State
                                    regStates,
                                    0, // MemoryMapped
                                    false, // IO
                                    sizeof(int32_t) * 2, // Extra memory
                                    NULL,
                                    RegStartFunction,
                                    RegUpdateFunction);
   #endif

   FU_Type VREAD = RegisterFU(versat,"vread",
                                    0, // n inputs
                                    1, // n outputs
                                    ARRAY_SIZE(vreadBits), // Config
                                    vreadBits, 
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    true, // IO
                                    sizeof(VReadExtra), // Extra memory
                                    NULL,
                                    VReadStartFunction,
                                    VReadUpdateFunction);

   FU_Type VWRITE = RegisterFU(versat,"vwrite",
                                    1, // n inputs
                                    0, // n outputs
                                    ARRAY_SIZE(vwriteBits), // Config
                                    vwriteBits, 
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    true, // IO
                                    sizeof(VWriteExtra), // Extra memory
                                    NULL,
                                    VWriteStartFunction,
                                    VWriteUpdateFunction);

   FU_Type MEM = RegisterFU(versat,"xmem #(.ADDR_W(10))",
                                    2, // n inputs
                                    2, // n outputs
                                    ARRAY_SIZE(memBits), // Config
                                    memBits, 
                                    0, // State
                                    NULL,
                                    1024 * 4, // MemoryMapped
                                    false, // IO
                                    sizeof(MemExtra), // Extra memory
                                    NULL,
                                    MemStartFunction,
                                    MemUpdateFunction);

   #if 1
   FU_Type UNIT_F = RegisterFU(versat,"xunitF",
                                    10, // n inputs
                                    8, // n outputs
                                    ARRAY_SIZE(unitFBits), // Config
                                    unitFBits,
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    false, // IO
                                    sizeof(UnitFExtra), // Extra memory
                                    NULL,
                                    UnitFStartFunction,
                                    UnitFUpdateFunction);

   FU_Type UNIT_M = RegisterFU(versat,"xunitM",
                                    1, // n inputs
                                    1, // n outputs
                                    ARRAY_SIZE(unitFBits), // Config
                                    unitFBits,
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    false, // IO
                                    sizeof(UnitMExtra), // Extra memory
                                    NULL,
                                    UnitMStartFunction,
                                    UnitMUpdateFunction);
   #endif

   accel = CreateAccelerator(versat);

   #if 0   
   //FUInstance* mem = CreateFUInstance(accel,MEM);
   FUInstance* read = CreateFUInstance(accel,VREAD);
   FUInstance* write = CreateFUInstance(accel,VWRITE);

   union {
      void* ptr;
      int arr[2];
   } val;

   union {
      void* ptr;
      int arr[2];
   } val2;

   val.ptr = (void*) readMemory;
   val2.ptr = (void*) writeMemory;

   printf("Read:%x Write:%x\n",val.arr[0],val2.arr[0]);

   #if 0
   for(int i = 0; i < 32; i++){
      readMemory[i] = i + 1;
   }
   #endif

   {
      volatile MemConfig* c = (volatile MemConfig*) mem->config;

      c->A.iter = 1;
      c->A.incr = 1;
      c->A.delay = 0;
      c->A.per = 16 + 16; // Add a few more cycles due to terminate function
      c->A.duty = 16 + 16;

      // Initialize memory to known value
      for(int i = 0; i < 32; i++){
         mem->memMapped[i] = i + 1;
      }
   }

   {
      volatile VReadConfig* c = (volatile VReadConfig*) read->config;
      read->state = readMemory; // Temporary hack
      val.ptr = (void*) readMemory;

      // Versat side
      c->B.iter = 1;
      c->B.incr = 1;
      c->B.per = 8;
      c->B.duty = 8;

      // Memory side
      c->incrA = 1;
      c->iterA = 1;
      c->perA = 8;
      c->dutyA = 8;
      c->size = 8;
      c->int_addr = 0;
      c->ext_addr = val.arr[0];
   }

   {
      volatile VWriteConfig* c = (volatile VWriteConfig*) write->config;
      write->state = writeMemory; // Temporary hack
      val.ptr = (void*) writeMemory;

      // Versat side
      c->B.iter = 1;
      c->B.incr = 1;
      c->B.per = 16;
      c->B.duty = 16;
      c->B.delay = 1;

      // Memory side
      c->incrA = 1;
      c->iterA = 1;
      c->perA = 16;
      c->dutyA = 16;
      c->size = 16;
      c->int_addr = 0;
      c->ext_addr = val.arr[0];
   }

   ConnectUnits(versat,read,0,write,0);

   AcceleratorRun(versat,accel,NULL,CycleTerminateFunction);

   ClearCache();

   for(int i = 0; i < 20; i++){
      printf("%d ",writeMemory[i]);
   }
   printf("\n");

   ClearCache();

   AcceleratorRun(versat,accel,NULL,CycleTerminateFunction);
   
   ClearCache();

   for(int i = 0; i < 20; i++){
      printf("%d ",writeMemory[i]);
   }
   printf("\n");

   ClearCache();

   AcceleratorRun(versat,accel,NULL,CycleTerminateFunction);
 
   ClearCache();

   for(int i = 0; i < 20; i++){
      printf("%d ",writeMemory[i]);
   }
   printf("\n");

   ClearCache();

   //OutputMemoryMap(versat);

   OutputVersatSource(versat,"versat_defs.vh","versat_instance.v");

   uart_finish();

   return 0;
   #endif

   FUInstance* unitF[4];
   for(int i = 0; i < 4; i++){
      unitF[i] = CreateFUInstance(accel,UNIT_F);
      volatile UnitFConfig* c = (volatile UnitFConfig*) unitF[i]->config;

      c->delay = 3 + (i * 17);

      if(i > 0){
         for(int ii = 0; ii < 8; ii++){
            ConnectUnits(versat,unitF[i-1],ii,unitF[i],ii);
         }
      }
   }
   
   wMem = CreateFUInstance(accel,MEM);   
   {
      volatile MemConfig* c = (volatile MemConfig*) wMem->config;
      
      c->A.iter = 1;
      c->A.incr = 1;
      c->A.delay = 0;
      c->A.per = 16;
      c->A.duty = 16;
   }

   FUInstance* kMem[4]; // Could be done by using 1 memory, change later 
   for(int i = 0; i < 4; i++){
      kMem[i] = CreateFUInstance(accel,MEM);
      volatile MemConfig* c = (volatile MemConfig*) kMem[i]->config;
      
      c->A.iter = 1;
      c->A.incr = 1;
      c->A.delay = (17*i);
      c->A.per = 16;
      c->A.duty = 16;

      for (int ii = 0; ii < 16; ii++)
      {
         kMem[i]->memMapped[ii] = kConstants[i][ii];
      }
   }

   for(int i = 0; i < 8; i++){
      stateReg[i] = CreateFUInstance(accel,REG);

      {
         volatile RegConfig* config = (volatile RegConfig*) stateReg[i]->config; 

         config->writeDelay = 3 + (16*4) + 5;
         config->initialValue = (int32_t) initialStateValues[i];
      }

      ConnectUnits(versat,stateReg[i],0,unitF[0],i);
   }
   
   for(int i = 0; i < 8; i++){
      FUInstance* add = CreateFUInstance(accel,ADD);

      ConnectUnits(versat,stateReg[i],0,add,0);
      ConnectUnits(versat,unitF[3],i,add,1);
      ConnectUnits(versat,add,0,stateReg[i],0);
   }

   ConnectUnits(versat,wMem,0,unitF[0],8);

   FUInstance* unitM[3];
   for(int i = 0; i < 3; i++){
      unitM[i] = CreateFUInstance(accel,UNIT_M);

      volatile UnitMConfig* c = (volatile UnitMConfig*) unitM[i]->config;

      c->delay = 3 + 16 * (i+1) + i;

      ConnectUnits(versat,unitM[i],0,unitF[i+1],8);
      if(i != 0){
         ConnectUnits(versat,unitM[i-1],0,unitM[i],0);
      }
   }
   ConnectUnits(versat,wMem,0,unitM[0],0);

   for(int i = 0; i < 4; i++){
      ConnectUnits(versat,kMem[i],0,unitF[i],9);
   }

   char digest[256];

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
   versat_sha256(digest,"",0);
   printf("%s\n",GetHexadecimal(digest, HASH_SIZE));
#endif

   OutputVersatSource(versat,"versat_defs.vh","versat_instance.v");

   uart_finish();

   return 0;
}

static uint32_t load_bigendian_32(const uint8_t *x) {
    return (uint32_t)(x[3]) | (((uint32_t)(x[2])) << 8) |
           (((uint32_t)(x[1])) << 16) | (((uint32_t)(x[0])) << 24);
}


static size_t versat_crypto_hashblocks_sha256(const uint8_t *in, size_t inlen) {
    uint32_t w[16];

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

        {
            volatile MemConfig* c = (volatile MemConfig*) wMem->config;
            
            for (int i = 0; i < 16; i++)
            {
               wMem->memMapped[i] = w[i];
            }
        }

        AcceleratorRun(versat,accel,NULL,TerminateFunction);

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

    for (size_t i = 0; i < 8; ++i) {
        uint32_t val = stateReg[i]->state[0];

        store_bigendian_32(&out[i*4],val);
    }
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
