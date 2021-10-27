#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"
#include "versat.h"
#include "FU_Defs.h"
#include "string.h"

//#include "crypto/sha2.h"

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

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

static int aluliteBits[] = {4};
static int memBits[] = {10,10,10,10,1,1,1,10,10,10,10,10,10,10,10,10,10,10,1,1,1,10,10,10,10,10,10,10};
static int muladdBits[] = {5,10,10,10,1};

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
static uint32_t wConstants[] = {0x30313233,0x34353637,0x38393031,0x32333435,0x36373839,0x30313233,0x34353637,0x38398000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf0};
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
    return 'A' + (value - 10);
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

static unsigned char testBuffer[10000];

static int regBits[] = {32,10};
static int regStates[] = {32};
static int unitFBits[] = {8};

typedef struct{
   int initialValue,writeDelay;
} RegConfig;

int32_t* RegStartFunction(FUInstance* inst){
   static int32_t startValue;

   RegConfig* config = (RegConfig*) inst->config;
   int32_t* view = (int32_t*) inst->extraData;

   view[0] = config->writeDelay;
   
   startValue = inst->outputs[0]; // Kinda sketchy

   if(!view[1]){
      startValue = config->initialValue;
      inst->state[0] = startValue;
      view[1] = 1;
   }

   return &startValue;
}

int32_t* RegUpdateFunction(FUInstance* inst){
   static int32_t out;

   out = inst->outputs[0]; // By default, keep same output
   int32_t* delayView = (int32_t*) inst->extraData;

   if((*delayView) > 0){
      if(*delayView == 1){
         out = GetInputValue(inst,0);
      }
      *delayView -= 1;
   }

   inst->state[0] = out;

   return &out;
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

int32_t* AddFunction(FUInstance* inst){
   static int32_t out;

   out = (GetInputValue(inst,0)) + (GetInputValue(inst,1));

   return &out;
}

// GLOBALS
Versat* versat;
Accelerator* accel;
FUInstance* wMem;
FUInstance* stateReg[8];

int main()
{
   unsigned char digest[256];
   //init uart
   uart_init(UART_BASE,FREQ/BAUD);

   #if 0
   printf("here\n");
   uart_finish();
   return 0;
   #endif

   Versat versatInstance = {};
   versat = &versatInstance;

   InitVersat(versat,VERSAT_BASE);

   FU_Type ADD = RegisterFU(versat,"xadd",
                                    2, // n inputs
                                    1, // n outputs
                                    0, // Config
                                    NULL,
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    false, // IO
                                    0, // Extra memory
                                    NULL,
                                    AddFunction);

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
                                    RegStartFunction,
                                    RegUpdateFunction);

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
                                    MemStartFunction,
                                    MemUpdateFunction);

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
                                    UnitMStartFunction,
                                    UnitMUpdateFunction);

   printf("Create Accel\n");
   accel = CreateAccelerator(versat);
  
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

   printf("Connected units\n");

   char* testStr = "012345678901234274193791238915880578124487349182739812748903468028748237489068027481258901784901875809237480875678901234567891231312312012345678901234274193791238915880578124487349182739812748903468028748237489068027481258901784901875809237480875678901234567891231312312";
   int testSize = 200;

   //char* testStr = "";
   //int testSize = 0;

   //sha256(digest,testStr,testSize);
   //printf("\n%s\n",GetHexadecimal(digest,(256 / 8)));

   //for(int i = 0; i < 8 * 8; i++){
   //   digest[i] = 0;
   //}

   versat_sha256(digest,testStr,testSize);

   printf("Done versat\n");
   printf("\n%s\n",GetHexadecimal(digest,(256 / 8)));

   // 87B0AA2EEFC1A296A09A145C7EDB44DF6A0D693082B107827C235693E675E59B
   //IterativeAcceleratorRun(versat,accel,NULL,TerminateFunction);

   printf("\n");

   OutputMemoryMap(versat);
   OutputVersatSource(versat,"versat_defs.vh","versat_instance.v");

   /*
   AcceleratorRun(versat,accel,NULL,TerminateFunction);

   //IterativeAcceleratorRun(versat,accel,NULL,TerminateFunction);

   printf("      ");
   for(int i = 0; i < 8; i++){
      printf("0x%x ",stateReg[i]->state[0]);
   }
   printf("\n");

   AcceleratorRun(versat,accel,NULL,TerminateFunction);

   //IterativeAcceleratorRun(versat,accel,NULL,TerminateFunction);

   printf("      ");
   for(int i = 0; i < 8; i++){
      printf("0x%x ",stateReg[i]->state[0]);
   }
   printf("\n");
   */

   //printf("\n%s\n\n",GetHexadecimal(digest));

   //printf("6a09e667 bb67ae85 3c6ef372 a54ff53a 510e527f 9b05688c 1f83d9ab 5be0cd19\n");

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

        #if 0
        printf("V:");
        for(int i = 0; i < 8; i++){
            printf("%x ",stateReg[i]->state[0]);
        }
        printf("\n");
        printf("W:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",w[0],w[1],w[2],w[3],w[4],w[5],w[6],w[7],w[8],w[9],w[10],w[11],w[12],w[13],w[14],w[15]);
        #endif

        {
            volatile MemConfig* c = (volatile MemConfig*) wMem->config;
            
            for (int i = 0; i < 16; i++)
            {
               wMem->memMapped[i] = w[i];
            }
        }

        AcceleratorRun(versat,accel,NULL,TerminateFunction);

        #if 0
        printf("V:");
        for(int i = 0; i < 8; i++){
            printf("%x ",stateReg[i]->state[0]);
        }
        printf("\n");
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

    for (size_t i = 0; i < 8; ++i) {
        uint32_t val = stateReg[i]->state[0];

        store_bigendian_32(&out[i*4],val);
    }
}

#if 0
#define nSTAGE 5

int MyRand(){
   static int index = 0;
   static int randomValues[] = {1804289383,846930886,1681692777,1714636915,1957747793,424238335,719885386,1649760492,596516649,1189641421,1025202362,1350490027,783368690,1102520059,2044897763,1967513926,1365180540,1540383426,304089172,1303455736,35005211,521595368,294702567,1726956429,336465782,861021530,278722862,233665123,2145174067,468703135,1101513929,1801979802,1315634022,635723058,1369133069,1125898167,1059961393,2089018456,628175011,1656478042,1131176229,1653377373,859484421,1914544919,608413784,756898537,1734575198,1973594324,149798315,2038664370,1129566413,184803526,412776091,1424268980,1911759956,749241873,137806862,42999170,982906996,135497281,511702305,2084420925,1937477084,1827336327,572660336,1159126505,805750846,1632621729,1100661313,1433925857,1141616124,84353895,939819582,2001100545,1998898814,1548233367,610515434,1585990364,1374344043,760313750,1477171087,356426808,945117276,1889947178,1780695788,709393584,491705403,1918502651,752392754,1474612399,2053999932,1264095060,1411549676,1843993368,943947739,1984210012,855636226,1749698586,1469348094,1956297539,1036140795,463480570,2040651434,1975960378,317097467,1892066601,1376710097,927612902,1330573317,603570492,1687926652,660260756,959997301,485560280,402724286,593209441,1194953865,894429689,364228444,1947346619,221558440,270744729,1063958031,1633108117,2114738097,2007905771,1469834481,822890675,1610120709,791698927,631704567,498777856,1255179497,524872353,327254586,1572276965,269455306,1703964683,352406219,1600028624,160051528,2040332871,112805732,1120048829,378409503,515530019,1713258270,1573363368,1409959708,2077486715,1373226340,1631518149,200747796,289700723,1117142618,168002245,150122846,439493451,990892921,1760243555,1231192379,1622597488,111537764,338888228,2147469841,438792350,1911165193,269441500,2142757034,116087764,1869470124,155324914,8936987,1982275856,1275373743,387346491,350322227,841148365,1960709859,1760281936,771151432,1186452551,1244316437,971899228,1476153275,213975407,1139901474,1626276121,653468858,2130794395,1239036029,1884661237,1605908235,1350573793,76065818,1605894428,1789366143,1987231011,1875335928,1784639529,2103318776,1597322404,1939964443,2112255763,1432114613,1067854538,352118606,1782436840,1909002904,165344818,1395235128,532670688,1351797369,492067917,1504569917,680466996,706043324,496987743,159259470,1359512183,480298490,1398295499,1096689772,2086206725,601385644,1172755590,1544617505,243268139,1012502954,1272469786,2027907669,968338082,722308542,1820388464,933110197,6939507,740759355,1285228804,1789376348,502278611,1450573622,1037127828,1034949299,654887343,1529195746,392035568,1335354340,87755422,889023311,1494613810,1447267605,1369321801,745425661,396473730,1308044878,1346811305,1569229320,705178736,1590079444,434248626,1977648522,1470503465,1402586708,552473416,1143408282,188213258,559412924,1884167637,1473442062,201305624,238962600,776532036,1238433452,1273911899,1431419379,620145550,1665947468,619290071,707900973,407487131,2113903881,7684930,1776808933,711845894,404158660,937370163,2058657199,1973387981,1642548899,1501252996,260152959,1472713773,824272813,1662739668,2025187190,1967681095,1850952926,437116466,1704365084,1176911340,638422090,1943327684,1953443376,1876855542,1069755936,1237379107,349517445,588219756,1856669179,1057418418,995706887,1823089412,1065103348,625032172,387451659,1469262009,1562402336,298625210,1295166342,1057467587,1799878206,1555319301,382697713,476667372,1070575321,260401255,296864819,774044599,697517721,2001229904,1950955939,1335939811,1797073940,1756915667,1065311705,719346228,846811127,1414829150,1307565984,555996658,324763920,155789224,231602422,1389867269,780821396,619054081,711645630,195740084,917679292,2006811972,1253207672,570073850,1414647625,1635905385,1046741222,337739299,1896306640,1343606042,1111783898,446340713,1197352298,915256190,1782280524,846942590,524688209,700108581,1566288819,1371499336,2114937732,726371155,1927495994,292218004,882160379,11614769,1682085273,1662981776,630668850,246247255,1858721860,1548348142,105575579,964445884,2118421993,1520223205,452867621,1017679567,1857962504,201690613,213801961,822262754,648031326,1411154259,1737518944,282828202,110613202,114723506,982936784,1676902021,1486222842,950390868,255789528,1266235189,1242608872,1137949908,1277849958,777210498,653448036,1908518808,1023457753,364686248,1309383303,1129033333,1329132133,1280321648,501772890,1781999754,150517567,212251746,1983690368,364319529,1034514500,484238046,1775473788,624549797,767066249,1886086990,739273303,1750003033,1415505363,78012497,552910253,1671294892,1344247686,1795519125,661761152,474613996,425245975,1315209188,235649157,1448703729,1679895436,1545032460,430253414,861543921,677870460,932026304,496060028,828388027,1144278050,332266748,1192707556,31308902,816504794,820697697,655858699,1583571043,559301039,1395132002,1186090428,1974806403,1473144500,1739000681,1498617647,669908538,1387036159,12895151,1144522535,1812282134,1328104339,1380171692,1113502215,860516127,777720504,1543755629,1722060049,1455590964,328298285,70636429,136495343,1472576335,402903177,1329202900,1503885238,1219407971,2416949,12260289,655495367,561717988,1407392292,1841585795,389040743,733053144,1433102829,1887658390,1402961682,672655340,1900553541,400000569,337453826,1081174232,1780172261,1450956042,1941690360,410409117,847228023,1516266761,1866000081,1175526309,1586903190,2002495425,500618996,1989806367,1184214677,2004504234,1061730690,1186631626,2016764524,1717226057,1748349614,1276673168,1411328205,2137390358,2009726312,696947386,1877565100,1265204346,1369602726,1630634994,1665204916,1707056552,564325578,1297893529,1010528946,358532290,1708302647,1857756970,1874799051,1426819080,885799631,1314218593,1281830857,1386418627,1156541312,318561886,1243439214,70788355,1505193512,1112720090,1788014412,1106059479,241909610,1051858969,1095966189,104152274,1748806355,826047641,1369356620,970925433,309198987,887077888,530498338,873524566,37487770,1541027284,1232056856,1745790417,1251300606,959372260,1025125849,2137100237,126107205,159473059,1376035217,1282648518,478034945,471990783,1353436873,1983228458,1584710873,993967637,941804289,1826620483,2045826607,2037770478,1930772757,1647149314,716334471,1152645729,470591100,1025533459,2039723618,1001089438,1899058025,2077211388,394633074,983631233,1675518157,1645933681,1943003493,553160358,1635550270,2069110699,712633417,864101839,1204275569,1190668363,1336092622,410228794,1026413173,773319847,1404196431,1968217462,452456682,1302539390,1858504292,235745791,802205057,427355115,1388391521,1272796157,1452888574,1280631491,126401947,1204462951,1210359231,521035021,40610537,738393740,19485054,1983614030,1291554098,1655035325,1905241081,2004187516,371653516,962033002,1047372231,1707746139,1372261796,2073785404,333582338,628974580,1894519218,786039021,1931513970,1605539862,1021784812,586235379,2032894977,262692685,1859031536,1338299904,1543324176,1985433483,395279207,606199759,358984857,435889744,1344593499,378469911,272020127,488663950,2033505236,29777560,345367818,257675105,991810563,1392740049,1965421244,216588711,1319041805,151519934,845563291,1066077375,937558955,629593614,524133589,1959343768,1215828993,409544918,74552805,927376882,1747844822,1617876982,765326717,2143124030,76593093,1124311574,431530126,1421186593,1502781486,703550253,1909850543,1388803074,733327814,107734713,1646478179,1725138377,1500474762,1464415775,1941727088,672032919,1615935710,639806732,1738110294,406011017,1269400346,114760235,217871137,337745691,524305153,292423943,1265122573,124666328,1910300925,2030449291,120306710,1986894018,1007277217,551836836,1260596963,362575055,1255387090,1022963858,1751378130,1988714904,1130698571,1250372661,1566369633,483689685,567304789,1360613073,1155722604,35756851,2000419805,746349250,441767868,1122336503,861109485,659639006,1460082195,1385414639,952062949,577721120,1510080967,714880226,460686763,1630387677,554290596,1467963981,34740865,1814887560,1830539036,1290127955,690367770,1434433518,1131359211,1821066342,537322532,550245196,157272379,1104627321,1910858270,1312994984,1140384172,1763794427,2059344234,1582152040,738647283,772970072,94307398,51245830,10901063,1046370347,628966950,1520982030,1761250573,1089653714,1003886059,168057522,410134047,1038626924,1982945082,93189435,181271232,525829204,1527622954,1312630443,199411898,2064945486,1862875640,356684278,1022089159,1626250262,1669679262,14989683,1242561041,1581539848,1597141723,1981208324,207026272,1691449122,2032454154,217927335,590335821,513937457,1738909365,204102747,1603591171,595311776,372160269,2013725218,1633938701,207621703,2106914653,1815209933,733450907,1487053959,980356728,932862806,1404515797,695748720,1289547084,279121308,174515334,811742698,294110991,1417076376,245798898,1891252715,1250801052,452825171,1435218189,1135771559,670752506,2025554010,1649709016,262178224,82173109,1105816539,857490000,454333378,972058109,343945053,661955081,931489114,11671338,1395405989,271059426,992028067,180785147,1675575223,1687776787,1470332231,1954696532,1862292122,134591281,101323875,1131884850,380390179,1992576590,235202254,833215350,1280311131,1370973813,1503967857,1158381494,873199181,1766146081,1240554603,1979015720,476152433,1694887982,803590181,820097487,209359415,1735079296,831768825,1604765404,2006138722,1823796892,1785550551,1534230297,1364090032,1108399134,1341443181,1078898506,1242990415,1442767057,63299708,1623380595,1287859999,298501962,309112297,420687483,1669475776,1813080154,1579068977,395191309,1431742587,672139932,226723382,1907895021,219544266,1030313563,580508860,428903682,617909211,1412277685,2033669086,476564285,1088590930,1671735990,2010794583,305197314,632651476,1204754116,1384095820,1875641892,500037525,1447395528,1351538839,1787897525,1745897490,1660651136,61101360,1267889618,1326247643,1640170337,1663080928,610506582,164826621,1889804310,370917955,384370888,772634225,951426815,813274570,1390543437,216220853,699460008,1867107722,1304811783,223712350,1730418657,1610009097,856363827,787689126,846621269,584522071,1287726651,146533149,1936060910,928140528,1892430639,1449228398,989241888,1012836610,627992393,481928577,528433890,1238498976,646755199,270754552,1609416931,1031126087,1043388777,413360099,1844400657,286448566,629580952,396377017,6072641,1934392735,620089368,1736491298,1396918184,1476453195,376696776,96055805,2060975266,1664423428,242588954,1849552528,445080308,2135019593,1151297278,1434322197,1000372555,1779289672,1916250774,1528806445,870305000,415522325,1799560997,332238283,1446648412,695466127,745598382,1143565421};

   return randomValues[index++];
}

int main(int argc,const char* argv[]){
   int i,j;
   int16_t pixels[25 * nSTAGE], weights[9 * nSTAGE], bias = 0;
   Versat versatInstance = {};
   Versat* versat = &versatInstance;

   //init uart
   uart_init(UART_BASE,FREQ/BAUD);

   InitVersat(versat,VERSAT_BASE);

   FU_Type ALULITE = RegisterFU(versat,"xalulite",
                                    2, // n inputs
                                    1, // n outputs
                                    ARRAY_SIZE(aluliteBits), // Config
                                    aluliteBits,
                                    0, // State
                                    NULL, 
                                    0, // MemoryMapped
                                    false, // IO
                                    sizeof(int32_t), // Extra memory
                                    NULL,
                                    AluliteUpdateFunction);

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
                                    MemStartFunction,
                                    MemUpdateFunction);

   FU_Type MULADD = RegisterFU(versat,"xmuladd",
                                    2, // n inputs
                                    1, // n outputs
                                    ARRAY_SIZE(muladdBits), // Config
                                    muladdBits,
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    false, // IO
                                    sizeof(MuladdExtra), // Extra memory
                                    MulAddStartFunction,
                                    MulAddUpdateFunction);

   Accelerator* accel = CreateAccelerator(versat);

   for (j = 0; j < nSTAGE; j++)
   {
      //write 5x5 feature map in mem0
      for (i = 0; i < 25; i++)
      {
        pixels[25 * j + i] = MyRand() % 50 - 25;
      }

      //write 3x3 kernel and bias in mem1
      for (i = 0; i < 9; i++)
      {
        weights[9 * j + i] = MyRand() % 10 - 5;
      }

      //write bias after weights of VERSAT 0
      if (j == 0)
      {
        bias = MyRand() % 20 - 10;
      }
   }

   #if 0
   printf("\nExpected result of 3D convolution\n");
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         int res = bias;
         for (int k = 0; k < nSTAGE; k++)
         {
            for (int l = 0; l < 3; l++)
            {
               for (int m = 0; m < 3; m++)
               {
                  res += pixels[i * 5 + j + k * 25 + l * 5 + m] * weights[9 * k + l * 3 + m];
               }
            }
         }
         printf("%d\t", res);
      }
      printf("\n");
   }
   printf("\n");
   #endif

   int delay = 0;
   FUInstance* in_1_alulite = NULL;
   FUInstance* mem0[nSTAGE]; 
   FUInstance* mem2[nSTAGE];
   for(j = 0; j < nSTAGE; j++){
      mem0[j] = CreateFUInstance(accel,MEM);
      FUInstance* mem1 = CreateFUInstance(accel,MEM);
      FUInstance* muladd0 = CreateFUInstance(accel,MULADD);
      FUInstance* add0 = CreateFUInstance(accel,ALULITE);
      mem2[j] = CreateFUInstance(accel,MEM);

       //configure mem0A to read 3x3 block from feature map
      {
         volatile MemConfig* c = (volatile MemConfig*) mem0[j]->config;

         c->A.iter = 3;
         c->A.incr = 1;
         c->A.delay = delay;
         c->A.per = 3;
         c->A.duty = 3;
         c->A.shift = 5-3;

         for (i = 0; i < 25; i++)
         {
            mem0[j]->memMapped[i] = pixels[25 * j + i];
         }
      }

      //configure mem1A to read kernel
      {
         volatile MemConfig* c = (volatile MemConfig*) mem1->config;
         c->A.iter = 1;
         c->A.incr = 1;
         c->A.delay = delay;
         c->A.per = 10;
         c->A.duty = 10;

         for (i = 0; i < 9; i++)
         {
            mem1->memMapped[i] = weights[9 * j + i];
         }
         if(j == 0){
            mem1->memMapped[9] = bias;
            mem1->memMapped[10] = bias;
            mem1->memMapped[11] = bias;
         }
      }

      //configure muladd0
      {
         volatile MuladdConfig* c = (volatile MuladdConfig*) muladd0->config;
         c->iter = 1;
         c->per = 9;
         c->delay = delay + MEMP_LAT;
      }

      //configure alulite
      {
         volatile AluliteConfig* c = (volatile AluliteConfig*) add0->config;
         c->fns = ALULITE_ADD;
      }

      // Connect units
      ConnectUnits(versat,mem0[j],0,muladd0,0);
      ConnectUnits(versat,mem1,0,muladd0,1);
      ConnectUnits(versat,add0,0,mem2[j],0);
      ConnectUnits(versat,muladd0,0,add0,0);

      #if 0
      muladd0->inputs[0] = &mem0[j]->outputs[0];
      muladd0->inputs[1] = &mem1->outputs[0];
      mem2[j]->inputs[0] = &add0;
      add0->inputs[0] = muladd0;
      #endif

      if(j == 0){
         ConnectUnits(versat,mem1,0,add0,1);
         //add0->inputs[1] = mem1;
      } else {
         ConnectUnits(versat,in_1_alulite,0,add0,1);
         //add0->inputs[1] = in_1_alulite;
      }
      in_1_alulite = add0;

      if(j != nSTAGE - 1)
         delay += 2;
   }

   //config mem2A to store ALULite output
   {
      volatile MemConfig* c = (volatile MemConfig*) mem2[nSTAGE - 1]->config;

      c->A.iter = 1;
      c->A.incr = 1;
      c->A.delay = 8 + delay + MEMP_LAT + ALULITE_LAT + MULADD_LAT;
      c->A.per = 1;
      c->A.duty = 1;
      c->A.in_wr = 1;
   }

   //printf("Result of accelerator\n");
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         for (int k = 0; k < nSTAGE; k++){
            volatile MemConfig* c = (volatile MemConfig*) mem0[k]->config;
            c->A.start = i * 5 + j;
         }
         volatile MemConfig* c = (volatile MemConfig*) mem2[nSTAGE - 1]->config;
         c->A.start = i * 3 + j;

         AcceleratorRun(versat,accel,mem2[nSTAGE - 1],MemTerminateFunction);

         printf("%d\t",mem2[nSTAGE - 1]->memMapped[i * 3 + j]);
         
         #if 0
         uart_finish();
         return 0;
         #endif
      }
      printf("\n");
   }
   printf("\n");

   OutputMemoryMap(versat);
   OutputVersatSource(versat,"versat_defs.vh","versat_instance.v");

   uart_finish();

   return 0;
}
#endif
