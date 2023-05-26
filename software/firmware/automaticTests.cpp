#include <cstdarg>
#include <cstdio>
#include <cstdint>

#include "versat.hpp"
#include "versatExtra.hpp"
#include "utils.hpp"
#include "unitConfiguration.hpp"
#include "verilogWrapper.inc"
#include "basicWrapper.inc"
#include "versatSHA.hpp"
#include "versatAES.hpp"

extern "C"{
#include "../test_vectors.h"
//#include "crypto/sha2.h"

int printf_(const char* format, ...);
}

#ifndef PC
#define printf printf_
#endif

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"

// Disable accelerator running in PC if doing an hardware test
#if 0
#if defined(HARDWARE_TEST) && defined(PC)
#define AcceleratorRun(...) ((void)0)
#endif
#endif

struct TestInfo{
   int testsPassed;
   int numberTests;

   TestInfo(int passed, int numberTests = 1):testsPassed(passed),numberTests(numberTests){};

   TestInfo& operator+=(TestInfo t){
      testsPassed += t.testsPassed;
      numberTests += t.numberTests;

      return *this;
   }
};

#define TEST_FAILED(REASON) do{ printf("\n[%2d]Test failed: %s\n\t%s\n\n",testNumber,__PRETTY_FUNCTION__,REASON); return TestInfo(0);} while(0)
#define TEST_PASSED return TestInfo(1)

#define FORCE_FAIL 0

// Care with the testNumber variable. Every test must have one
#define EXPECT(...) Expect_(__PRETTY_FUNCTION__,testNumber,__VA_ARGS__)
static TestInfo Expect_(const char* functionName,int testNumber, const char* expected,const char* format, ...) __attribute__ ((format (printf, 4, 5)));

static TestInfo Expect_(const char* functionName,int testNumber, const char* expected,const char* format, ...){
   va_list args;
   va_start(args,format);

   char buffer[1024];
   int size = vsprintf(buffer,format,args);
   Assert(size < 1024);

   va_end(args);

   #if FORCE_FAIL
      expected = "";
   #endif

   bool result = (strcmp(expected,buffer) == 0);
   if(result){
      TEST_PASSED;
   } else {
      printf("\n");
      printf("[%2d]Test failed: %s\n",testNumber,functionName);
      printf("    Expected: %s\n",expected);
      printf("    Result:   %s\n",buffer);
      printf("              ");
      for(int i = 0; expected[i] != '\0'; i++){
         if(buffer[i] == '\0'){
            printf("^");
            break;
         }
         if(buffer[i] != expected[i]){
            printf("^");
         } else {
            printf(" ");
         }
      }

      printf("\n");

      return TestInfo(0);
   }
}

#define TEST(TEST_NAME) static TestInfo TEST_NAME(Versat* versat,int testNumber)

#include <cstdlib>

#define SYS_N (3488)
#define VEC_SZ (SYS_N / 8) // 436
// 16, 64 works
// 3488 / 8 = 436 does not work
// 128 seems to hang
// #define VEC_SZ (132)
TEST(VectorLikeOperation){
   uint32_t mat[VEC_SZ/4] = {0};
   uint32_t row[VEC_SZ/4] = {0};
   uint32_t expected[VEC_SZ/4] = {0};
   uint32_t result[VEC_SZ/4] = {0};
   uint32_t mask = 0xFFFFFFFF;
   int i = 0;
   int n_cols = VEC_SZ / 4;
   for(i=0; i<n_cols;i++){
       mat[i] = (uint32_t) i*4;
       row[i] = (uint32_t) 0xFF;
       for(int j=1; j<4; j++){
           mat[i] = (mat[i] << 8) | (i*4 + j);
           row[i] = (row[i] << 8) | 0xFF;
       }
       expected[i] = ~(mat[i]);
   }

   FUDeclaration* type = GetTypeByName(versat,STRING("VectorLikeOperation"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,STRING("Test"));

   // printf("config vread\n");
   ConfigureSimpleVRead(GetInstanceByName(accel,"Test","row"), VEC_SZ / 4,(int*) row);

   // printf("config mem mat\n");
   FUInstance* matInst = GetInstanceByName(accel,"Test","mat");
   ConfigureMemoryLinear(matInst, VEC_SZ / 4);
   for (int c = 0; c < VEC_SZ / 4; c++){
       VersatUnitWrite(matInst,c,mat[c]);
   }

   // printf("config mask\n");
   FUInstance* maskInst = GetInstanceByName(accel,"Test","mask");
   maskInst->config[0] = mask;

   // printf("config output\n");
   FUInstance* outputInst = GetInstanceByName(accel,"Test","output");
   // ConfigureMemoryReceive(outputInst, VEC_SZ / 4, 1);
   // ConfigureMemoryLinearOut(outputInst, VEC_SZ / 4);
   ConfigureMemoryLinearOut(outputInst, VEC_SZ / 4);
   
   // printf("accel 1\n");
   AcceleratorRun(accel); // Fills vread with valid data
   // printf("accel 2\n");
   AcceleratorRun(accel);

   // printf("read results\n");
   for (int c = 0; c < VEC_SZ/4; c++){
        result[c] = VersatUnitRead(outputInst,c);
   }
   
   // printf("output versat\n");
   OutputVersatSource(versat,accel,".");

   // printf("cmp results\n");
   if (memcmp(result, expected, VEC_SZ/4) == 0) {
       printf("Test Passed\n");
       TEST_PASSED;
   } else {
       printf("Input:\n");
       for(int i = 0; i < VEC_SZ/4; i++){
          printf("0x%08x ", mat[i]);
       }
       printf("\nResult:\n");
       for(int i = 0; i < VEC_SZ/4; i++){
          printf("0x%08x ", result[i]);
       }
       printf("\n\nExpected:\n");
       for(int i = 0; i < VEC_SZ/4; i++){
          printf("0x%08x ", expected[i]);
       }
       TEST_FAILED("Result differ from expected value");
   }

}

TEST(SHA){
   FUDeclaration* type = GetTypeByName(versat,STRING("SHA"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,STRING("Test"));

   SetSHAAccelerator(accel,inst);

   InitVersatSHA(versat,true);

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   VersatSHA(digest,msg_64,64);

   //printf("%s\n",buffer);

   return EXPECT("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa","%s",GetHexadecimal(digest, HASH_SIZE));
}

TEST(AESWithIterative){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"AES256WithIterative");

   FUInstance* t = GetInstanceByName(test.accel,"Test","mk0","roundPairAndKey");
   FUInstance* s = GetInstanceByName(test.accel,"Test","subBytes");
   FUInstance* k = GetInstanceByName(test.accel,"Test","key6");
   FUInstance* r = GetInstanceByName(test.accel,"Test","round0");

   FillSubBytes(test.accel,s);
   FillKeySchedule256(test.accel,k);
   FillRound(test.accel,r);

   FUInstance* merge = GetInstanceByName(test.accel,"Test","mk0","Merge0");
   merge->config[0] = 8;

   int rcon[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40};
   for(int i = 0; i < 7; i++){
      FUInstance* inst = GetInstanceByName(test.accel,"Test","rcon%d",i);
      inst->config[0] = rcon[i];
   }

   FillRoundPairAndKey(test.accel,t);

   int* out = RunSimpleAccelerator(&test, 0xcc,0xc6,0x2c,0x6b,
                                          0x0a,0x09,0xa6,0x71,
                                          0xd6,0x44,0x56,0x81,
                                          0x8d,0xb2,0x9a,0x4d,
                                          0xcc,0x22,0xda,0x78,
                                          0x7f,0x37,0x57,0x11,
                                          0xc7,0x63,0x02,0xbe,
                                          0xf0,0x97,0x9d,0x8e,
                                          0xdd,0xf8,0x42,0x82,
                                          0x9c,0x2b,0x99,0xef,
                                          0x3d,0xd0,0x4e,0x23,
                                          0xe5,0x4c,0xc2,0x4b);

   OutputVersatSource(versat,&test,".");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xdf 0x86 0x34 0xca 0x02 0xb1 0x3a 0x12 0x5b 0x78 0x6e 0x1d 0xce 0x90 0x65 0x8b ","%s",buffer);
}

// When 1, need to pass 0 to enable test (changes enabler from 1 to 0)
#define REVERSE_ENABLED 0

#define DISABLED (REVERSE_ENABLED)

#ifndef HARDWARE_TEST
   #define HARDWARE_TEST -1
   #define ENABLE_TEST(ENABLED) (!(ENABLED) != !(REVERSE_ENABLED))
#else
   #define ENABLE_TEST(ENABLED) (currentTest == hardwareTest)
#endif

#define TEST_INST(ENABLED,TEST_NAME) do { if(ENABLE_TEST( ENABLED )){ \
      TestInfo test = TEST_NAME(versat,currentTest); \
      if(test.testsPassed == test.numberTests) printf("%32s [%02d] - OK\n",#TEST_NAME,currentTest); \
      info += test; \
     \
   } \
   currentTest += 1; } while(0)

void AutomaticTests(Versat* versat){
   TestInfo info = TestInfo(0,0);
   int hardwareTest = HARDWARE_TEST;
   int currentTest = 0;

#if 1
   TEST_INST( 1 ,SHA);                  // HARDWARE_TEST = 0
   TEST_INST( 1, AESWithIterative);     // HARDWARE_TEST = 1
   TEST_INST( 1, VectorLikeOperation); // HARDWARE_TEST = 2
#endif

   //Free(versat);

   printf("\nAutomatic tests done (passed/total): %d / %d\n",info.testsPassed,info.numberTests);
}
