#include <cstdio>

#include "versat.hpp"
#include "utils.hpp"

#include "verilogWrapper.inc"

extern "C"{
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob-timer.h"

#include "crypto/sha2.h"
#include "crypto/aes.h"

#include "../test_vectors.h"

int printf_(const char* format, ...);
}

#ifdef PC
#define uart_finish(...) ((void)0)
#define uart_init(...) ((void)0)
#else
#define printf printf_
#endif

#define MEMSET(base, location, value) (*((volatile int*) (base + (sizeof(int)) * location)) = value)
#define MEMGET(base, location)        (*((volatile int*) (base + (sizeof(int)) * location)))

// Automatically times a block in number of counts
struct TimeIt{
   int line;
   char fileId;

   TimeIt(int line,char fileId);
   ~TimeIt();
};

TimeIt::TimeIt(int line,char fileId){
   this->line = line;
   this->fileId = fileId;
   timer_reset();
}

TimeIt::~TimeIt(){
   unsigned long long end = timer_get_count();
   printf("%c:%d %llu\n",fileId,line,end);
}

#if 1
#define TIME_IT(ID) TimeIt timer_##__LINE__(__LINE__,ID)
#else
#define TIME_IT(ID) do{}while(0)
#endif

#define HASH_SIZE (256/8)

int ClearCache(){
#ifdef PC
   static int buffer[4096 * 4096];
   int currentValue = 1;
   int lastValue = 0;

   for(int i = 0; i < 4096 * 4096; i += 4096){
      lastValue = buffer[i];
      buffer[i] = currentValue;
   }
   currentValue += 1;

   return lastValue;
#else
   volatile int* ptr = (volatile int*) 0;
   int sum = 0;
   for(int i = 0; i < 4096 * 16; i += 4096){
      int val = ptr[i];
      sum += val;
   }
   return sum;
#endif
}

#include "unitConfiguration.hpp"

void AutomaticTests(Versat* versat);

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

static uint initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

#if 0
#ifdef PC
   static char mem[1024*1024]; // 1 Mb
   #define DDR_MEM mem
#else
#if (RUN_DDR_SW==0)
   #define DDR_MEM (EXTRA_BASE)
#else
   #define DDR_MEM ((1<<(FIRM_ADDR_W)))
#endif
#endif
#endif

char GetHexadecimalChar(int value){
   if(value < 10){
      return '0' + value;
   } else{
      return 'a' + (value - 10);
   }
}

unsigned char* GetHexadecimal(const unsigned char* text, int str_size){
   static unsigned char buffer[2048+1];
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

static int readMemory[64];

// GLOBALS
Accelerator* accel;
FUInstance* registers[8];
volatile VReadConfig* readConfig;
bool initVersat = false;

void InstantiateSHA(Versat* versat){
   FUInstance* inst = nullptr;
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SHA"));
   accel = CreateAccelerator(versat);
   inst = CreateFUInstance(accel,type,MakeSizedString("SHA"));

   OutputUnitInfo(inst);

   FUInstance* read = GetInstanceByName(accel,"SHA","MemRead");
   {
      readConfig = (volatile VReadConfig*) read->config;

      // Versat side
      readConfig->iterB = 1;
      readConfig->incrB = 1;
      readConfig->perB = 16;
      readConfig->dutyB = 16;

      // Memory side
      readConfig->incrA = 1;
      readConfig->iterA = 1;
      readConfig->perA = 16;
      readConfig->dutyA = 16;
      readConfig->size = 8;
      readConfig->int_addr = 0;
      readConfig->pingPong = 1;
      readConfig->ext_addr = (int) readMemory; // Some place so no segfault if left unconfigured
   }

   for(int i = 0; i < 8; i++){
      registers[i] = GetInstanceByName(accel,"SHA","State","s%d",i,"reg");
   }

   for(int i = 0; i < 4; i++){
      FUInstance* mem = GetInstanceByName(accel,"SHA","cMem%d",i,"mem");

      for(int ii = 0; ii < 16; ii++){
         VersatUnitWrite(mem,ii,kConstants[i][ii]);
      }
   }

   FUInstance* swap = GetInstanceByName(accel,"SHA","Swap");

   SwapEndianConfig* swapConfig = (SwapEndianConfig*) swap->config;
   swapConfig->enabled = 1;
}

void TestSHA(Versat* versat){
   InstantiateSHA(versat);

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   printf("Expected: 42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa\n");
   versat_sha256(digest,msg_64,64);
   printf("Result:   %s\n",GetHexadecimal(digest, HASH_SIZE));

   // Gera o versat.
   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");
}

int main(int argc,const char* argv[])
{
   //init uart
   uart_init(UART_BASE,FREQ/BAUD);

   printf("here\n");

   timer_init(TIMER_BASE);

   printf("Init base modules\n");

   Versat* versat = InitVersat(VERSAT_BASE,1);

   ParseCommandLineOptions(versat,argc,argv);

   ParseVersatSpecification(versat,"testVersatSpecification.txt");

   #if 0
   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   printf("Expected: 42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa\n");
   sha256(digest,msg_64,64);
   printf("Result:   %s\n",GetHexadecimal(digest, HASH_SIZE));
   #endif

   #if 1
   AutomaticTests(versat);
   #endif

   #if 0
   char key[16];
   memset(key,0,16);

   struct AES_ctx ctx;
   AES_init_ctx(&ctx,key);

   char msg[16] = {0xf3,0x44,0x81,0xec,0x3c,0xc6,0x27,0xba,0xcd,0x5d,0xc3,0xfb,0x08,0xf2,0x73,0xe6};

   AES_ECB_encrypt(&ctx,msg);

   printf("%s\n",GetHexadecimal(msg,16));
   printf("0336763e966d92595a567cc9ce537f5e\n");
   #endif

#if AUTOMATIC_TEST == 1
   int i = 0;

   InstantiateSHA(versat);

   printf("[L = %d]\n", HASH_SIZE);

   //Message test loop
   for(i=0; i< NUM_MSGS; i++){
      versat_sha256(digest,msg_array[i],msg_len[i]);
      printf("\nLen = %d\n", msg_len[i]*8);
      printf("Msg = %s\n", GetHexadecimal(msg_array[i], (msg_len[i]) ? msg_len[i] : 1));
      printf("MD = %s\n",GetHexadecimal(digest, HASH_SIZE));
   }
   printf("\n");
#endif

   Hook(versat,nullptr,nullptr);
   uart_finish();

   return 0;
}

static size_t versat_crypto_hashblocks_sha256(const uint8_t *in, size_t inlen) {
   readConfig->ext_addr = (int) in;

   while (inlen >= 64) {
      // Loads data + performs work
      AcceleratorRun(accel);

      if(!initVersat){
         for(int i = 0; i < 8; i++){
            VersatUnitWrite(registers[i],0,initialStateValues[i]);
         }
         initVersat = true;
      }

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
   readConfig->ext_addr = (int) readMemory; // Need to put a valid address otherwise address sanitizer will complain of reading previous stack data

   AcceleratorRun(accel);

   for (size_t i = 0; i < 8; ++i) {
      uint val = *registers[i]->state;

      store_bigendian_32(&out[i*4],val);
   }

   initVersat = false; // At the end of each run, reset
}

/*

Immediate plan:

   Add a "accelerator view", which is nothing more that pointers to nodes and edges in a real accelerator except do not need to keep track of all of them.
      Useful for the merge algorithms, where I can make an accelerator view for each individual accelerator post-merge.
   Change the calculate delay algorithm to take in an accelerator view.
   Maybe change the calculate delay algorithm to store the info inside an array in the declaration, instead of inside each individual FUInstance.
      Need to first change declarations, so that every accelerator is associated to a declaration.
   Need to figure out what to do in regards to same name entities (Which will later affect the GetEntityByName calls)
      Possible solution:


Current plan:

Change the calculated inputs and outputs on the FUInstances to be full vector likes for the size
   If there isn't a connection, simply store a nullptr
      [Objective] Implement a way so that the memory units that have unconnected inputs can have a zero input
Take a look at hierarchical names (and how would I implemented them for a flatten operation)
At the very least, the specification parser must have good error reporting, on what is expected from the language
Simplify the process of creating FU units.
   Code a complete realloc routine that can be used at any point to make a valid accelerator
      Possible use it to implement the Removal of units
   Move any allocation needed to the Locking interface
      The units do not need output allocations before running the accelerator
      Maybe make it so that outputs allocations are allocated globally throught the accelerator
         Simplify the process of copying the stored outputs (a simple memcpy)

Figure out how to deal with the accelerator in the RegisterSubUnit function
   The simplest is to use the accelerator copy function
      But how to deal with the memory allocations?
         Either let the accelerator work like a normal accelerator (allocates memory even though it will not use it)
         Or have a non allocating flag that prevents from allocating
         Or use a smaller more compact structure to store the structures
            Problematic since I cannot use the same algorithms that I could use in the graph.
      Also, it should be helpful to add generic configurations, which are stored and handled by Versat

Use the instance names as the verilog module names instead of the declaration name
   Make the generated vcd more readable

Next:

Find a way to paralelize the simulation build and running.

Keep track off:

The delay value for delay units is how much to extend latency, while delay for the other units is how many cycles before valid data arrives. (Made more concrete be setting the delay on a delay unit to be a configuration)
Remove instances doesn't update the config / state / memMapped / delay pointers

*/

/*

Implementation to do:

Overall:

   Start implementing logging.
   Start implementing error reporting. Some things should give error but the program still goes on, put asserts.
   Errors should be simple: Report the error and then quit. Must errors cannot be carry over. No fail and retry

Software:

   Change hierarchical name from a char[] to a char* (Otherwise will bleed embedded memory dry)
      Software for now will simply malloc and forget, but eventually I should probably implement some form of string interning
      NOTE: After adding perfect hashing, name information is no longer required. Might not need to change afterall, for now hold on.

   Add true hierarchical naming for Flatten units
      - Simply get the full hierarchical representation and store it as the name of the unit, with parent set to null
      - Might need to change hierarchical name from array to char*
      - Need to take care about where strings end up. Do not want to fill embedded with useless data if possible

   Support the output of a module not starting at zero. (Maybe weird thing to do for now)
      More complex, but take a pass at anything that depends on order of instancing/declarating (a lot of assumptions are being made right now)
   [Type] Change ouput memory map and functions alike to use type system and simple output structs directly (Simpler and less error prone)
   [CalculateVersatData] naked memory allocation, add it to the accelerator locking mechanism

Embedded:

   Implement a perfect hashing scheme to accelerate simulation. GetInstanceByName

Hardware:

   Maybe think about changing the databus interface (go from native to a axi like interface)
      And possible add a length signal to indicate length to the dma unit

   Come up with another name to differentiate delays from the delay units

   Implement shadow registers
   Delay units with same inputs and delay values should be shared.
      In fact, a delay tree should be created. No need to have a delay of X and a delay of X+1, when we can have X and a Register.
         For now, delays are "fixed". Even thought programmable, the whole module unit only works for a specific delay value and therefore

   Introduce the concept of combinatorial units only
      These units can be "shared"

Delay:

   Add a special "Constant" edge, that doesn't have or add any delay regardless of anything (Time agnostic)
   To improve lantency calculations, should have a indication of which outputs delay depend on which input ports delay (Think how dual port ram has two independent ports)
      Each output should keep track of exactly which input port it depends upon, and use that information in delay calculation
   Since delay is simply "how many cycles until seeing valid data", I barely understand what is the use of DELAY_TYPE_SOURCE_DELAY and DELAY_TYPE_SINK_DELAY. Is it really just because of Reg?
      I think I only need a single value to indicate the difference between a Reg and a Muladd (one produces that, and therefore should act as a source, while the other is a compute unit)
   Delay calculation not working if there is a separatation of flow
      Think adding a new unit to the start that doesn't connect to anywhere.
      The new unit will have pretty much the full latency as the delay, when it should be only the latency of the previous unit
         The algorithm doesn't take into account different flows, it works by assuming that everything ends up in a final unit which might not be true

Template Engine:

   Really need to simplify error if identifier not found.
   Add local variables, instead of everything global
   Take another pass at whitespace handling (Some whitespace is being consumed in blocks of text, care, might join together two things that are supposed to be seperated and do error)
   Need to return values from calls

Flatten:

   Give every declaration a "level" . simple = 0, composite = max(level of subunits) + 1, special = 0.
   Flatten is currently broken (might have been fixed, check it later?), only creating shallow instances. Useful for outputting graph, but not possible to simulate
      The fix is to create a function that copies and changes shallow instances and, in the end, it fixes memory allocation and instance pointers and initialization

Merge:

   Do merge of units in same "level". Iterate down while repeating

*/

/*

Known bugs:

PC-Emul is different from Sim-Run when the latency for the Delay unit is set to zero. PC-Emul is still giving correct results when Sim-Run is giving wrong ones.
There is some disconnect between pc-euml and sim, especially when wrong latency values are given to the base units.
   Need to create an example that tries to test edge cases, in regards to latency.
   Work from the generated hardware (check if it does what I think it does) and then change PC-Emul until it gives the exact values on the vcd as the hardware.
   The values given must be the exact same, if the accelerator is running and producing valid data, I want the PC-Emul to produce the same values, even if those values are useless.



The Pool class shouldn't store all that info, and it makes it a error to copy the class
   All the info needed is stored in the linked list on the pages, the Pool can pretty much work like a shell and therefore be copied around without a problem

*/





