#include "versat_crypto.h"

#include "versat_accel.h"

#include <string.h>

#include "unitConfiguration.h"

#include "printf.h"

// Constants used by SHA.
static uint32_t initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint32_t kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint32_t kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint32_t kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint32_t kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint32_t* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

// GLOBALS
static bool runInitialized = false; // The first accelerator run loads the data, only the next runs actual process valid data. Need to keep track of when the "actual" run starts

static void store_bigendian_32(uint8_t *x, uint32_t u) {
   x[3] = (uint8_t) u;
   u >>= 8;
   x[2] = (uint8_t) u;
   u >>= 8;
   x[1] = (uint8_t) u;
   u >>= 8;
   x[0] = (uint8_t) u;
}

// Initialize SHA, the only difference between runs is the pointer for the input
void InitVersatSHA(){
   CryptoAlgosConfig* config = (CryptoAlgosConfig*) accelConfig;
   SHAConfig* sha = &config->sha;

   *sha = (SHAConfig){0};

   // Configure VRead unit to output 16 values (of 4 bytes. 16 * 4 bytes = 64 bytes per run)
   ConfigureSimpleVRead(&sha->MemRead,16,NULL);

   // Configure the Constants memories to output 16 values
   ACCEL_Constants_mem_iterA = 1;
   ACCEL_Constants_mem_incrA = 1;
   ACCEL_Constants_mem_perA = 16;
   ACCEL_Constants_mem_dutyA = 16;
   ACCEL_Constants_mem_startA = 0;
   ACCEL_Constants_mem_shiftA = 0;

   // Loads Constants units with the constants defined by SHA
   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_sha_cMem0_mem_addr,ii,kConstants[0][ii]);
   }
   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_sha_cMem1_mem_addr,ii,kConstants[1][ii]);
   }
   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_sha_cMem2_mem_addr,ii,kConstants[2][ii]);
   }
   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_sha_cMem3_mem_addr,ii,kConstants[3][ii]);
   }

   // Need to swap endianess for this architecture
   ACCEL_TOP_sha_Swap_enabled = 1;
}

static size_t versat_crypto_hashblocks_sha256(const uint8_t *in, size_t inlen) {
   while (inlen >= 64) {
      ACCEL_TOP_sha_MemRead_ext_addr = (iptr) in; // Need to change input source every run
   
      // Loads data + performs work
      RunAccelerator(1);

      if(!runInitialized){
         runInitialized = true;

         // Only load state after doing the first run, since the first run is the one that loads valid data and only the following runs do the actual work.
         // This means that the result of the first run is garbage and we only want to set the initial valid state when we gonna process actual valid data.
         VersatUnitWrite(TOP_sha_State_s_0_reg_addr,0,initialStateValues[0]);
         VersatUnitWrite(TOP_sha_State_s_1_reg_addr,0,initialStateValues[1]);
         VersatUnitWrite(TOP_sha_State_s_2_reg_addr,0,initialStateValues[2]);
         VersatUnitWrite(TOP_sha_State_s_3_reg_addr,0,initialStateValues[3]);
         VersatUnitWrite(TOP_sha_State_s_4_reg_addr,0,initialStateValues[4]);
         VersatUnitWrite(TOP_sha_State_s_5_reg_addr,0,initialStateValues[5]);
         VersatUnitWrite(TOP_sha_State_s_6_reg_addr,0,initialStateValues[6]);
         VersatUnitWrite(TOP_sha_State_s_7_reg_addr,0,initialStateValues[7]);
      }

      in += 64;
      inlen -= 64;
   }

   // Note that at the end of this function the accelerator still needs one last run, since the accelerator contains valid data inside.
   // We do not do the run now because it is possible that this function gets called again before ending the algorithm

   return inlen;
}

void VersatSHA(uint8_t *out, const uint8_t *in, size_t inlen) {
   uint8_t padded[128];
   uint64_t bytes = inlen;

   // This is the function that handles the majority of the input
   versat_crypto_hashblocks_sha256(in, inlen);

   // The remaining code handles the padding of the last block
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
      
    }  padded[56] = (uint8_t) (bytes >> 53);
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

   // At this point the accelerator still contains valid data inside.
   // One last run to flush all the valid data and obtain the final state.
   RunAccelerator(1);

   // Read the values from the state registers. It is the output of the SHA algorithm
   store_bigendian_32(&out[0*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_0_reg_addr,0));
   store_bigendian_32(&out[1*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_1_reg_addr,0));
   store_bigendian_32(&out[2*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_2_reg_addr,0));
   store_bigendian_32(&out[3*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_3_reg_addr,0));
   store_bigendian_32(&out[4*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_4_reg_addr,0));
   store_bigendian_32(&out[5*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_5_reg_addr,0));
   store_bigendian_32(&out[6*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_6_reg_addr,0));
   store_bigendian_32(&out[7*4],(uint32_t) VersatUnitRead(TOP_sha_State_s_7_reg_addr,0));

   runInitialized = false; // At the end of each run, reset the runInitialized flag, since we have finished this "SHA run"
}