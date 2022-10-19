#include "versatSHA.hpp"

#include "versat.hpp"
#include "unitConfiguration.hpp"
#include "verilogWrapper.inc"

static uint initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

// GLOBALS
static Accelerator* accel;
static FUInstance* shaInstance;
static FUInstance* registers[8];
static volatile VReadConfig* readConfig;
static bool initVersat = false;

void SetSHAAccelerator(Accelerator* accel_,FUInstance* shaInstance_){
   accel = accel_;
   shaInstance = shaInstance_;
}

static void InstantiateSHA(Versat* versat){
   if(shaInstance){
      FUInstance* read = GetInstanceByName(shaInstance,"MemRead");
      readConfig = (volatile VReadConfig*) read->config;

      ConfigureSimpleVRead(read,16,nullptr); // read address is configured before accelerator run

      for(int i = 0; i < 8; i++){
         registers[i] = GetInstanceByName(shaInstance,"State","s%d",i,"reg");
      }

      for(int i = 0; i < 4; i++){
         FUInstance* mem = GetInstanceByName(shaInstance,"cMem%d",i,"mem");

         for(int ii = 0; ii < 16; ii++){
            VersatUnitWrite(mem,ii,kConstants[i][ii]);
         }
      }
   } else { // Assume that accel is a flatten instance of SHA
      FUInstance* read = GetInstanceByName(accel,"Test","MemRead");
      readConfig = (volatile VReadConfig*) read->config;

      ConfigureSimpleVRead(read,16,nullptr); // read address is configured before accelerator run

      for(int i = 0; i < 8; i++){
         registers[i] = GetInstanceByName(accel,"Test","State","s%d",i,"reg");
      }

      for(int i = 0; i < 4; i++){
         FUInstance* mem = GetInstanceByName(accel,"Test","cMem%d",i,"mem");

         for(int ii = 0; ii < 16; ii++){
            VersatUnitWrite(mem,ii,kConstants[i][ii]);
         }
      }
   }
}

static void store_bigendian_32(uint8_t *x, uint64_t u) {
   x[3] = (uint8_t) u;
   u >>= 8;
   x[2] = (uint8_t) u;
   u >>= 8;
   x[1] = (uint8_t) u;
   u >>= 8;
   x[0] = (uint8_t) u;
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

void VersatSHA(uint8_t *out, const uint8_t *in, size_t inlen) {
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

   AcceleratorRun(accel);

   for (size_t i = 0; i < 8; ++i) {
      uint val = *registers[i]->state;

      store_bigendian_32(&out[i*4],val);
   }

   initVersat = false; // At the end of each run, reset
}

void InitVersatSHA(Versat* versat,bool outputVersatSource){
   static bool initialized = false;

   if(initialized){
      return;
   }

   Assert(accel && "Must set the accelerator before calling InitVersatSHA");

   InstantiateSHA(versat);

   // Gera o versat.
   if(outputVersatSource){
      OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");
   }
}


