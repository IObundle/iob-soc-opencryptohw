#include <cstdarg>
#include <cstdio>
#include <cstdint>

#include "versat.hpp"
#include "utils.hpp"
#include "unitConfiguration.hpp"
#include "verilogWrapper.inc"
#include "versatSHA.hpp"

extern "C"{
#include "../test_vectors.h"

//#include "iob-ila.h"
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

int* TestInstance(Versat* versat,Accelerator* accel,FUInstance* inst,int numberInputs,int numberOutputs,...){
   static int out[99];
   FUInstance* inputs[99];
   FUInstance* outputs[99];

   va_list args;
   va_start(args,numberOutputs);

   int registersAdded = 0;
   for(int i = 0; i < numberInputs; i++){

      char buffer[128];
      int size = snprintf(buffer,128,"regIn%d",registersAdded++);

      inputs[i] = CreateFUInstance(accel,GetTypeByName(versat,MakeSizedString("Reg")),MakeSizedString(buffer,size));

      int val = va_arg(args,int);

      VersatUnitWrite(inputs[i],0,val);

      ConnectUnits(inputs[i],0,inst,i);
   }

   registersAdded = 0;
   for(int i = 0; i < numberOutputs; i++){
      char buffer[128];
      int size = snprintf(buffer,128,"regOut%d",registersAdded++);
      outputs[i] = CreateFUInstance(accel,GetTypeByName(versat,MakeSizedString("Reg")),MakeSizedString(buffer,size));

      ConnectUnits(inst,i,outputs[i],0);
   }

   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   for(int i = 0; i < numberOutputs; i++){
      out[i] = outputs[i]->state[0];
   }

   va_end(args);

   return out;
}

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

TEST(TestMStage){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("M_Stage"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* top = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FUInstance* inst = GetInstanceByName(accel,"Test","sigma");

   #if 1
   int constants[] = {7,18,3,17,19,10};
   for(size_t i = 0; i < ARRAY_SIZE(constants); i++){
      inst->config[i] = constants[i];
   }
   #endif

   int* out = TestInstance(versat,accel,top,4,1,0x5a86b737,0xa9f9be83,0x08251f6d,0xeaea8ee9);

   return EXPECT("0xb89ab4ca","0x%x",out[0]);
}

TEST(TestFStage){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("F_Stage"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FUInstance* t = GetInstanceByName(accel,"Test","t");
   int constants[] = {6,11,25,2,13,22};
   for(size_t i = 0; i < ARRAY_SIZE(constants); i++){
      t->config[i] = constants[i];
   }

   int* out = TestInstance(versat,accel,inst,10,8,0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19,0x428a2f98,0x5a86b737);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 8; i++){
      ptr += sprintf(ptr,"0x%08x ",out[i]);
   }

   return EXPECT("0x568f3f84 0x6a09e667 0xbb67ae85 0x3c6ef372 0xf34e99d9 0x510e527f 0x9b05688c 0x1f83d9ab ","%s",buffer);
}

TEST(VReadToVWrite){
   Accelerator* accel = CreateAccelerator(versat);
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("VReadToVWrite"));
   CreateFUInstance(accel,type,MakeSizedString("test"));

   int readBuffer[16];
   int writeBuffer[16];

   FUInstance* reader = GetInstanceByName(accel,"test","read");
   FUInstance* writer = GetInstanceByName(accel,"test","write");

   ConfigureSimpleVRead(reader,16,readBuffer);
   ConfigureSimpleVWrite(writer,16,writeBuffer);

   for(int i = 0; i < 16; i++){
      readBuffer[i] = i;
   }

   #if 1
   AcceleratorRun(accel); // Load vread
   AcceleratorRun(accel); // Load vwrite
   AcceleratorRun(accel); // Write data
   #endif

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"%d ",writeBuffer[i]);
   }

   return EXPECT("0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 ","%s",buffer);
}

TEST(StringHasher){
   int weights[] = {17,67,109,157,199};
   char testString[] = "123249819835894981389Waldo198239812849825899904924oefhcasjngwoeijfjvakjndcoiqwj";

   Accelerator* accel = CreateAccelerator(versat);
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("StringHasher"));
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("test"));

   FUInstance* muladd = GetInstanceByName(accel,"test","mul1","mul");

   volatile MuladdConfig* conf = (volatile MuladdConfig*) muladd->config;
   conf->opcode = 0;
   conf->iterations = 99;
   conf->period = 1;
   conf->shift = 0;

   for(int i = 0; i < 5; i++){
      inst->config[i] = weights[i];
   }

   FUInstance* bytesIn = GetInstanceByName(accel,"test","bytesIn");
   FUInstance* bytesOut = GetInstanceByName(accel,"test","bytesOut");

   for(int i = 0; i < 5; i++){
      VersatUnitWrite(bytesIn,i,(int) ("Waldo"[i]));
   }

   ConfigureMemoryLinear(bytesIn,5);
   ConfigureMemoryReceive(bytesOut,1,1);

   AcceleratorRun(accel);

   int hash = VersatUnitRead(bytesOut,0);

   for(size_t i = 0; i < sizeof(testString); i++){
      VersatUnitWrite(bytesIn,i,(int) testString[i]);
   }

   ConfigureMemoryLinear(bytesIn,sizeof(testString));
   ConfigureMemoryReceive(bytesOut,sizeof(testString)-5,1);

   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   for(size_t i = 0; i < sizeof(testString) - 5; i++){
      int val = VersatUnitRead(bytesOut,i);

      if(hash == val){
         return EXPECT("21","%d",i);
      }
   }

   TEST_FAILED("Hash wasn't equal to anything");
}

TEST(Convolution){
   #define nSTAGE 5
   int pixels[25 * nSTAGE], weights[9 * nSTAGE], bias = 0;

   SeedRandomNumber(0);
   for (int j = 0; j < nSTAGE; j++){
      for (int i = 0; i < 25; i++)
      {
         pixels[25 * j + i] = GetRandomNumber() % 50 - 25;
      }

      for (int i = 0; i < 9; i++)
      {
         weights[9 * j + i] = GetRandomNumber() % 10 - 5;
      }

      if(j == 0)
      {
         bias = GetRandomNumber() % 20 - 10;
      }
   }

   Accelerator* accel = CreateAccelerator(versat);
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("Convolution"));
   CreateFUInstance(accel,type,MakeSizedString("test"));

   //write data in versat mems
   volatile VReadConfig* pixelConfigs[5];
   for (int j = 0; j < nSTAGE; j++){
      FUInstance* pixel = GetInstanceByName(accel,"test","stage%d",j,"pixels");

      ConfigureSimpleVRead(pixel,25,&pixels[25*j]);
      {
         volatile VReadConfig* config = (volatile VReadConfig*) pixel->config;
         pixelConfigs[j] = config;

         // B - versat side
         config->iterB = 3;
         config->incrB = 1;
         config->perB = 3;
         config->dutyB = 3;
         config->shiftB = 5 - 3;
      }

      #if 0
      //write 5x5 feature map in mem0
      for (i = 0; i < 25; i++){
         //VersatUnitWrite(pixels,i, GetRandomNumber() % 50 - 25);
      }
      #endif

      FUInstance* weight = GetInstanceByName(accel,"test","stage%d",j,"weights");

      //write 3x3 kernel and bias in mem1
      for(int i = 0; i < 9; i++){
         VersatUnitWrite(weight,i, weights[9*j + i]);
      }

      if(j == 0){
         FUInstance* bia = GetInstanceByName(accel,"test","bias");
         bia->config[0] = bias;

         {
            ConfigureMemoryLinear(weight,9);
         }

         {
            FUInstance* muladd = GetInstanceByName(accel,"test","stage0","muladd");
            volatile MuladdConfig* config = (volatile MuladdConfig*) muladd->config;

            config->iterations = 1;
            config->period = 9;
         }
      }
   }

   FUInstance* res = GetInstanceByName(accel,"test","res");

   volatile MemConfig* resConfig = (volatile MemConfig*) res->config;
   resConfig->iterA = 1;
   resConfig->incrA = 1;
   resConfig->perA = 1;
   resConfig->dutyA = 1;
   resConfig->in0_wr = 1;

   AcceleratorRun(accel); // Load vreads with initial good data

   for(int i = 0; i < 3; i++)
   {
      for(int j = 0; j < 3; j++)
      {
         for(int x = 0; x < 5; x++){
               pixelConfigs[x]->startB = i * 5 + j;
         }

         resConfig->startA = i * 3 + j;

         AcceleratorRun(accel);
      }
   }

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
         ptr += sprintf(ptr,"%d ", VersatUnitRead(res,i * 3 + j));
      }
   }

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("-520 -251 -49 -33 -42 303 -221 -100 -149 ","%s",buffer);
}

TEST(MatrixMultiplication){
   Accelerator* accel = CreateAccelerator(versat);
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("MatrixMultiplication"));
   CreateFUInstance(accel,type,MakeSizedString("test"));

   FUInstance* memA = GetInstanceByName(accel,"test","matA");
   FUInstance* memB = GetInstanceByName(accel,"test","matB");
   FUInstance* muladd = GetInstanceByName(accel,"test","ma");

   FUInstance* res = GetInstanceByName(accel,"test","res");

   int dimensions = 4;
   int size = dimensions * dimensions;

   ConfigureLeftSideMatrix(memA,dimensions);
   ConfigureRightSideMatrix(memB,dimensions);

   for(int i = 0; i < size; i++){
      VersatUnitWrite(memA,i,i+1);
      VersatUnitWrite(memB,i,i+1);
   }

   volatile MuladdConfig* conf = (volatile MuladdConfig*) muladd->config;

   conf->opcode = 0;
   conf->iterations = size;
   conf->period = dimensions;
   conf->shift = 0;

   ConfigureMemoryReceive(res,size,dimensions);

   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < dimensions; i++){
      for(int j = 0; j < dimensions; j++){
         ptr += sprintf(ptr,"%d ",VersatUnitRead(res,i*dimensions + j));
      }
   }

   return EXPECT("90 100 110 120 202 228 254 280 314 356 398 440 426 484 542 600 ","%s",buffer);
}

TEST(MatrixMultiplicationVRead){
   #define DIM 4
   int matrixA[DIM*DIM];
   int matrixB[DIM*DIM];
   int matrixRes[DIM*DIM];
   volatile int* resPtr = (volatile int*) matrixRes;

   Accelerator* accel = CreateAccelerator(versat);
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("MatrixMultiplicationVread"));
   CreateFUInstance(accel,type,MakeSizedString("test"));

   FUInstance* memA = GetInstanceByName(accel,"test","matA");
   FUInstance* memB = GetInstanceByName(accel,"test","matB");
   FUInstance* muladd = GetInstanceByName(accel,"test","ma");

   FUInstance* res = GetInstanceByName(accel,"test","res");

   int dimensions = DIM;
   int size = dimensions * dimensions;

   ConfigureLeftSideMatrixVRead(memA,dimensions);
   ConfigureRightSideMatrixVRead(memB,dimensions);

   {
   volatile VReadConfig* config = (volatile VReadConfig*) memA->config;
   config->ext_addr = (int) matrixA;
   }

   {
   volatile VReadConfig* config = (volatile VReadConfig*) memB->config;
   config->ext_addr = (int) matrixB;
   }

   for(int i = 0; i < size; i++){
      matrixA[i] = i + 1;
      matrixB[i] = i + 1;
   }

   volatile MuladdConfig* conf = (volatile MuladdConfig*) muladd->config;

   conf->opcode = 0;
   conf->iterations = size;
   conf->period = dimensions;
   conf->shift = 0;

   ConfigureMatrixVWrite(res,size);
   {
   volatile VWriteConfig* config = (volatile VWriteConfig*) res->config;
   config->ext_addr = (int) matrixRes;
   }

   AcceleratorRun(accel);
   AcceleratorRun(accel);
   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < dimensions; i++){
      for(int j = 0; j < dimensions; j++){
         ptr += sprintf(ptr,"%d ",resPtr[i*dimensions + j]);
      }
   }

   return EXPECT("90 100 110 120 202 228 254 280 314 356 398 440 426 484 542 600 ","%s",buffer);
}

TEST(VersatAddRoundKey){
   #if 0
   int input[] = {0x04,0xe0,0x48,0x28,
                  0x66,0xcb,0xf8,0x06,
                  0x81,0x19,0xd3,0x26,
                  0xe5,0x9a,0x7a,0x4c};

   int cypher[] ={0xa0,0x88,0x23,0x2a,
                  0xfa,0x54,0xa3,0x6c,
                  0xfe,0x2c,0x39,0x76,
                  0x17,0xb1,0x39,0x05
                  };
   #endif

   FUDeclaration* addRoundKey = GetTypeByName(versat,MakeSizedString("AddRoundKey"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,addRoundKey,MakeSizedString("Test"));

   int* out = TestInstance(versat,accel,inst,32,16,
                              0x04,0xe0,0x48,0x28, // Cypher
                              0x66,0xcb,0xf8,0x06,
                              0x81,0x19,0xd3,0x26,
                              0xe5,0x9a,0x7a,0x4c,
                              0xa0,0x88,0x23,0x2a, // Key
                              0xfa,0x54,0xa3,0x6c,
                              0xfe,0x2c,0x39,0x76,
                              0x17,0xb1,0x39,0x05);

   const char* expected = "0xa4 0x68 0x6b 0x02 0x9c 0x9f 0x5b 0x6a 0x7f 0x35 0xea 0x50 0xf2 0x2b 0x43 0x49 ";

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT(expected,"%s",buffer);
}

TEST(LookupTable){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("LookupTable"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   int addrA = 0;
   int addrB = 4;

   VersatUnitWrite(inst,addrA,0xf0);
   VersatUnitWrite(inst,addrB,0xf4);

   int* out = TestInstance(versat,accel,inst,2,2,addrA,addrB);

   char buffer[1024];
   sprintf(buffer,"0x%02x 0x%02x",out[0],out[1]);

   return EXPECT("0xf0 0xf4","%s",buffer);
}

static const uint8_t sbox[256] = {
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static void FillSBox(FUInstance* inst){
   for(int i = 0; i < 256; i++){
      VersatUnitWrite(inst,i,sbox[i]);
   }
}

static void FillSubBytes(FUInstance* inst){
   for(int i = 0; i < 8; i++){
      FillSBox(GetInstanceByName(inst,"s%d",i));
   }
}

TEST(VersatSubBytes){
   #if 0
   int input[] = {0x19,0xa0,0x9a,0xe9,
                  0x3d,0xf4,0xc6,0xf8,
                  0xe3,0xe2,0x8d,0x48,
                  0xbe,0x2b,0x2a,0x08};

   int expected[] = {0xd4,0xe0,0xb8,0x1e,
                     0x27,0xbf,0xb4,0x41,
                     0x11,0x98,0x5d,0x52,
                     0xae,0xf1,0xe5,0x30};
   #endif

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SBox"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   #if 1
   FillSubBytes(inst);
   #endif

   int* out = TestInstance(versat,accel,inst,16,16,0x19,0xa0,0x9a,0xe9,0x3d,0xf4,0xc6,0xf8,0xe3,0xe2,0x8d,0x48,0xbe,0x2b,0x2a,0x08);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xd4 0xe0 0xb8 0x1e 0x27 0xbf 0xb4 0x41 0x11 0x98 0x5d 0x52 0xae 0xf1 0xe5 0x30 ","%s",buffer);
}

TEST(VersatShiftRows){
   #if 0
   int input[] = {0xd4,0xe0,0xb8,0x1e,
                  0x27,0xbf,0xb4,0x41,
                  0x11,0x98,0x5d,0x52,
                  0xae,0xf1,0xe5,0x30};
   #endif

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("ShiftRows"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   //bool tmp = SetDebug(versat,true);

   int* out = TestInstance(versat,accel,inst,16,16,
                  0xd4,0xe0,0xb8,0x1e,
                  0x27,0xbf,0xb4,0x41,
                  0x11,0x98,0x5d,0x52,
                  0xae,0xf1,0xe5,0x30);

   //OutputMemoryMap(versat,accel);

   //SetDebug(versat,tmp);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xd4 0xe0 0xb8 0x1e 0xbf 0xb4 0x41 0x27 0x5d 0x52 0x11 0x98 0x30 0xae 0xf1 0xe5 ","%s",buffer);
}

static const uint8_t mul2[] = {
   0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
   0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
   0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
   0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
   0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
   0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
   0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
   0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
   0x1b,0x19,0x1f,0x1d,0x13,0x11,0x17,0x15,0x0b,0x09,0x0f,0x0d,0x03,0x01,0x07,0x05,
   0x3b,0x39,0x3f,0x3d,0x33,0x31,0x37,0x35,0x2b,0x29,0x2f,0x2d,0x23,0x21,0x27,0x25,
   0x5b,0x59,0x5f,0x5d,0x53,0x51,0x57,0x55,0x4b,0x49,0x4f,0x4d,0x43,0x41,0x47,0x45,
   0x7b,0x79,0x7f,0x7d,0x73,0x71,0x77,0x75,0x6b,0x69,0x6f,0x6d,0x63,0x61,0x67,0x65,
   0x9b,0x99,0x9f,0x9d,0x93,0x91,0x97,0x95,0x8b,0x89,0x8f,0x8d,0x83,0x81,0x87,0x85,
   0xbb,0xb9,0xbf,0xbd,0xb3,0xb1,0xb7,0xb5,0xab,0xa9,0xaf,0xad,0xa3,0xa1,0xa7,0xa5,
   0xdb,0xd9,0xdf,0xdd,0xd3,0xd1,0xd7,0xd5,0xcb,0xc9,0xcf,0xcd,0xc3,0xc1,0xc7,0xc5,
   0xfb,0xf9,0xff,0xfd,0xf3,0xf1,0xf7,0xf5,0xeb,0xe9,0xef,0xed,0xe3,0xe1,0xe7,0xe5
};

static const uint8_t mul3[] = {
   0x00,0x03,0x06,0x05,0x0c,0x0f,0x0a,0x09,0x18,0x1b,0x1e,0x1d,0x14,0x17,0x12,0x11,
   0x30,0x33,0x36,0x35,0x3c,0x3f,0x3a,0x39,0x28,0x2b,0x2e,0x2d,0x24,0x27,0x22,0x21,
   0x60,0x63,0x66,0x65,0x6c,0x6f,0x6a,0x69,0x78,0x7b,0x7e,0x7d,0x74,0x77,0x72,0x71,
   0x50,0x53,0x56,0x55,0x5c,0x5f,0x5a,0x59,0x48,0x4b,0x4e,0x4d,0x44,0x47,0x42,0x41,
   0xc0,0xc3,0xc6,0xc5,0xcc,0xcf,0xca,0xc9,0xd8,0xdb,0xde,0xdd,0xd4,0xd7,0xd2,0xd1,
   0xf0,0xf3,0xf6,0xf5,0xfc,0xff,0xfa,0xf9,0xe8,0xeb,0xee,0xed,0xe4,0xe7,0xe2,0xe1,
   0xa0,0xa3,0xa6,0xa5,0xac,0xaf,0xaa,0xa9,0xb8,0xbb,0xbe,0xbd,0xb4,0xb7,0xb2,0xb1,
   0x90,0x93,0x96,0x95,0x9c,0x9f,0x9a,0x99,0x88,0x8b,0x8e,0x8d,0x84,0x87,0x82,0x81,
   0x9b,0x98,0x9d,0x9e,0x97,0x94,0x91,0x92,0x83,0x80,0x85,0x86,0x8f,0x8c,0x89,0x8a,
   0xab,0xa8,0xad,0xae,0xa7,0xa4,0xa1,0xa2,0xb3,0xb0,0xb5,0xb6,0xbf,0xbc,0xb9,0xba,
   0xfb,0xf8,0xfd,0xfe,0xf7,0xf4,0xf1,0xf2,0xe3,0xe0,0xe5,0xe6,0xef,0xec,0xe9,0xea,
   0xcb,0xc8,0xcd,0xce,0xc7,0xc4,0xc1,0xc2,0xd3,0xd0,0xd5,0xd6,0xdf,0xdc,0xd9,0xda,
   0x5b,0x58,0x5d,0x5e,0x57,0x54,0x51,0x52,0x43,0x40,0x45,0x46,0x4f,0x4c,0x49,0x4a,
   0x6b,0x68,0x6d,0x6e,0x67,0x64,0x61,0x62,0x73,0x70,0x75,0x76,0x7f,0x7c,0x79,0x7a,
   0x3b,0x38,0x3d,0x3e,0x37,0x34,0x31,0x32,0x23,0x20,0x25,0x26,0x2f,0x2c,0x29,0x2a,
   0x0b,0x08,0x0d,0x0e,0x07,0x04,0x01,0x02,0x13,0x10,0x15,0x16,0x1f,0x1c,0x19,0x1a
};

static void FillRow(FUInstance* row){
   FUInstance* mul2_0 = GetInstanceByName(row,"mul2_0");
   FUInstance* mul2_1 = GetInstanceByName(row,"mul2_1");
   FUInstance* mul3_0 = GetInstanceByName(row,"mul3_0");
   FUInstance* mul3_1 = GetInstanceByName(row,"mul3_1");

   for(int i = 0; i < 256; i++){
      VersatUnitWrite(mul2_0,i,mul2[i]);
      VersatUnitWrite(mul2_1,i,mul2[i]);
      VersatUnitWrite(mul3_0,i,mul3[i]);
      VersatUnitWrite(mul3_1,i,mul3[i]);
   }
}

TEST(VersatDoRows){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("DoRow"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FillRow(inst);

   int* out = TestInstance(versat,accel,inst,4,4,0xdb,0x13,0x53,0x45);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 4; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x8e 0x4d 0xa1 0xbc ","%s",buffer);
}

TEST(VersatMixColumns){
   #if 0
   int input[] = {0xd4,0xe0,0xb8,0x1e,
                  0xbf,0xb4,0x41,0x27,
                  0x5d,0x52,0x11,0x98,
                  0x30,0xae,0xf1,0xe5};

   int expected[] = {0x04,0xe0,0x48,0x28,
                     0x66,0xcb,0xf8,0x06,
                     0x81,0x19,0xd3,0x26,
                     0xe5,0x9a,0x7a,0x4c};
   #endif

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("MixColumns"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   for(int i = 0; i < 4; i++){
      FillRow(GetInstanceByName(accel,"Test","d%d",i));
   }

   int* out = TestInstance(versat,accel,inst,16,16,0xd4,0xe0,0xb8,0x1e,0xbf,0xb4,0x41,0x27,0x5d,0x52,0x11,0x98,0x30,0xae,0xf1,0xe5);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x04 0xe0 0x48 0x28 0x66 0xcb 0xf8 0x06 0x81 0x19 0xd3 0x26 0xe5 0x9a 0x7a 0x4c ","%s",buffer);
}

TEST(FirstLineKey){
   #if 0
   int input[] = {0x09,0xcf,0x4f,0x3c,0x2b,0x7e,0x15,0x16,0x01};
   #endif

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("FirstLineKey"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   for(int i = 0; i < 2; i++){
      FUInstance* table = GetInstanceByName(accel,"Test","b%d",i);

      for(int ii = 0; ii < 256; ii++){
         VersatUnitWrite(table,ii,sbox[ii]);
      }
   }

   int* out = TestInstance(versat,accel,inst,9,4,0x09,0xcf,0x4f,0x3c,0x2b,0x7e,0x15,0x16,0x01);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 4; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xa0 0xfa 0xfe 0x17 ","%s",buffer);
}

static void FillKeySchedule(FUInstance* inst){
   for(int i = 0; i < 2; i++){
      FUInstance* table = GetInstanceByName(inst,"s","b%d",i);

      FillSBox(table);
   }
}

TEST(KeySchedule){
   #if 0
   int input[] = {0x2b,0x28,0xab,0x09,
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c,
                  0x01}; // rcon
   #endif

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("KeySchedule"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FillKeySchedule(inst);

   int* out = TestInstance(versat,accel,inst,17,16,0x2b,0x28,0xab,0x09,0x7e,0xae,0xf7,0xcf,0x15,0xd2,0x15,0x4f,0x16,0xa6,0x88,0x3c,0x01);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xa0 0x88 0x23 0x2a 0xfa 0x54 0xa3 0x6c 0xfe 0x2c 0x39 0x76 0x17 0xb1 0x39 0x05 ","%s",buffer);
}

void FillRound(FUInstance* round){
   for(int i = 0; i < 8; i++){
      FillSBox(GetInstanceByName(round,"subBytes","s%d",i));
   }

   for(int i = 0; i < 4; i++){
      FillRow(GetInstanceByName(round,"mixColumns","d%d",i));
   }
}

TEST(AESRound){
   #if 0
   int input[] = {0x19,0xa0,0x9a,0xe9, // cypher
                  0x3d,0xf4,0xc6,0xf8,
                  0xe3,0xe2,0x8d,0x48,
                  0xbe,0x2b,0x2a,0x08,
                  0xa0,0x88,0x23,0x2a, // key
                  0xfa,0x54,0xa3,0x6c,
                  0xfe,0x2c,0x39,0x76,
                  0x17,0xb1,0x39,0x05};
   #endif

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("MainRound"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FillRound(inst);

   int* out = TestInstance(versat,accel,inst,32,16,0x19,0xa0,0x9a,0xe9,0x3d,0xf4,0xc6,0xf8,0xe3,0xe2,0x8d,0x48,0xbe,0x2b,0x2a,0x08,0xa0,0x88,0x23,0x2a,0xfa,0x54,0xa3,0x6c,0xfe,0x2c,0x39,0x76,0x17,0xb1,0x39,0x05);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xa4 0x68 0x6b 0x02 0x9c 0x9f 0x5b 0x6a 0x7f 0x35 0xea 0x50 0xf2 0x2b 0x43 0x49 ","%s",buffer);
}

static void FillAES(FUInstance* inst){
   int rcon[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};
   for(int i = 0; i < 10; i++){
      FUInstance* constRcon = GetInstanceByName(inst,"rcon%d",i);
      constRcon->config[0] = rcon[i];

      FillKeySchedule(GetInstanceByName(inst,"key%d",i));
   }
   FillSubBytes(GetInstanceByName(inst,"subBytes"));

   for(int i = 0; i < 9; i++){
      FillRound(GetInstanceByName(inst,"round%d",i));
   }
}

static void FillAESAccelerator(Accelerator* accel){
   int rcon[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};
   for(int i = 0; i < 10; i++){
      FUInstance* constRcon = GetInstanceByName(accel,"Test","aes","rcon%d",i);
      constRcon->config[0] = rcon[i];

      for(int j = 0; j < 2; j++){
         FUInstance* table = GetInstanceByName(accel,"Test","aes","key%d",i,"s","b%d",j);

         FillSBox(table);
      }
   }
   for(int i = 0; i < 8; i++){
      FillSBox(GetInstanceByName(accel,"Test","aes","subBytes","s%d",i));
   }

   for(int i = 0; i < 9; i++){
      for(int j = 0; j < 8; j++){
         FillSBox(GetInstanceByName(accel,"Test","aes","round%d",i,"subBytes","s%d",j));
      }

      for(int j = 0; j < 4; j++){
         FUInstance* mul2_0 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul2_0");
         FUInstance* mul2_1 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul2_1");
         FUInstance* mul3_0 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul3_0");
         FUInstance* mul3_1 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul3_1");

         for(int i = 0; i < 256; i++){
            VersatUnitWrite(mul2_0,i,mul2[i]);
            VersatUnitWrite(mul2_1,i,mul2[i]);
            VersatUnitWrite(mul3_0,i,mul3[i]);
            VersatUnitWrite(mul3_1,i,mul3[i]);
         }
      }
   }
}

TEST(AES){
   #if 0
   int input[] = {0x32,0x88,0x31,0xe0, // cypher
                  0x43,0x5a,0x31,0x37,
                  0xf6,0x30,0x98,0x07,
                  0xa8,0x8d,0xa2,0x34,
                  0x2b,0x28,0xab,0x09, // key
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c};
   #endif

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("AES"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FillAES(inst);

   int* out = TestInstance(versat,accel,inst,32,16,0x32,0x88,0x31,0xe0,0x43,0x5a,0x31,0x37,0xf6,0x30,0x98,0x07,0xa8,0x8d,0xa2,0x34,0x2b,0x28,0xab,0x09,0x7e,0xae,0xf7,0xcf,0x15,0xd2,0x15,0x4f,0x16,0xa6,0x88,0x3c);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);
}

#include <cstdlib>

TEST(ReadWriteAES){
   int cypher[] = {0x32,0x88,0x31,0xe0,
                  0x43,0x5a,0x31,0x37,
                  0xf6,0x30,0x98,0x07,
                  0xa8,0x8d,0xa2,0x34};
   int key[] =    {0x2b,0x28,0xab,0x09,
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c};
   int result[16];

   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("ReadWriteAES"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   ConfigureSimpleVRead(GetInstanceByName(accel,"Test","cypher"),16,cypher);
   ConfigureSimpleVRead(GetInstanceByName(accel,"Test","key"),16,key);
   ConfigureSimpleVWrite(GetInstanceByName(accel,"Test","results"),16,result);

   FillAES(GetInstanceByName(accel,"Test","aes"));

   AcceleratorRun(accel);
   AcceleratorRun(accel);
   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",result[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);
}

int SimpleAdderInstance(Accelerator* accel,int a,int b){
   FUInstance* a1 = GetInstanceByName(accel,"Test","a1");
   FUInstance* a2 = GetInstanceByName(accel,"Test","a2");
   FUInstance* out = GetInstanceByName(accel,"Test","res");

   a1->config[0] = a;
   a2->config[0] = b;

   AcceleratorRun(accel);

   int result = out->state[0];

   return result;
}

TEST(SimpleAdder){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SimpleAdder"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   int result = SimpleAdderInstance(accel,3,4);

   return EXPECT("7","%d",result);
}

int ComplexAdderInstance(Accelerator* accel,int a,int b){
   FUInstance* b1 = GetInstanceByName(accel,"Test","b1");
   FUInstance* b2 = GetInstanceByName(accel,"Test","b2");
   FUInstance* out = GetInstanceByName(accel,"Test","memOut1");

   VersatUnitWrite(b1,0,a);
   VersatUnitWrite(b2,0,b);

   ConfigureMemoryReceive(out,1,1);

   AcceleratorRun(accel);

   int result = VersatUnitRead(out,0);

   return result;
}

int ComplexMultiplierInstance(Accelerator* accel,int a,int b){
   FUInstance* c1 = GetInstanceByName(accel,"Test","c1");
   FUInstance* c2 = GetInstanceByName(accel,"Test","c2");
   FUInstance* out = GetInstanceByName(accel,"Test","memOut2");

   VersatUnitWrite(c1,0,a);
   VersatUnitWrite(c2,0,b);

   ConfigureMemoryReceive(out,1,1);

   AcceleratorRun(accel);

   int result = VersatUnitRead(out,0);

   return result;
}

int SemiComplexAdderInstance(Accelerator* accel,int a,int b){
   FUInstance* d1 = GetInstanceByName(accel,"Test","d1");
   FUInstance* d2 = GetInstanceByName(accel,"Test","d2");
   FUInstance* out = GetInstanceByName(accel,"Test","memOut3");

   d1->config[0] = a;
   VersatUnitWrite(d2,0,b);

   ConfigureMemoryReceive(out,1,1);

   AcceleratorRun(accel);

   int result = VersatUnitRead(out,0);

   return result;
}

TEST(ComplexMultiplier){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("ComplexMultiplier"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   int result = ComplexMultiplierInstance(accel,4,5);

   return EXPECT("20","%d",result);
}

TEST(SimpleShareConfig){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SimpleShareConfig"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FUInstance* a1 = GetInstanceByName(accel,"Test","a1");
   FUInstance* a2 = GetInstanceByName(accel,"Test","a2");
   FUInstance* b1 = GetInstanceByName(accel,"Test","b1");
   FUInstance* b2 = GetInstanceByName(accel,"Test","b2");
   FUInstance* out0 = GetInstanceByName(accel,"Test","out0");
   FUInstance* out1 = GetInstanceByName(accel,"Test","out1");
   FUInstance* out2 = GetInstanceByName(accel,"Test","out2");

   a1->config[0] = 2;
   AcceleratorRun(accel);
   int res0 = out0->state[0];

   a1->config[0] = 0;
   a2->config[0] = 3;
   AcceleratorRun(accel);
   int res1 = out0->state[0];

   b2->config[0] = 4;
   AcceleratorRun(accel);
   int res2 = out1->state[0];

   a1->config[0] = 0;
   a2->config[0] = 0;
   b1->config[0] = 0;
   b2->config[0] = 0;

   a2->config[0] = 3;
   b2->config[0] = 4;
   AcceleratorRun(accel);
   int res3 = out2->state[0];

   return EXPECT("4 6 8 7","%d %d %d %d",res0,res1,res2,res3);
}

TEST(ComplexShareConfig){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("ComplexShareConfig"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   // Test by changing config for shared 1
   FUInstance* a11 = GetInstanceByName(accel,"Test","shared1","a1");
   FUInstance* a12 = GetInstanceByName(accel,"Test","shared1","a2");
   FUInstance* b11 = GetInstanceByName(accel,"Test","shared1","b1");
   FUInstance* b12 = GetInstanceByName(accel,"Test","shared1","b2");

   // But reading the output of shared 2 (should be the same, since same configuration = same results)
   FUInstance* out20 = GetInstanceByName(accel,"Test","shared2","out0");
   FUInstance* out21 = GetInstanceByName(accel,"Test","shared2","out1");
   FUInstance* out22 = GetInstanceByName(accel,"Test","shared2","out2");

   a11->config[0] = 2;
   AcceleratorRun(accel);
   int res0 = out20->state[0];

   a11->config[0] = 0;
   a12->config[0] = 3;
   AcceleratorRun(accel);
   int res1 = out20->state[0];

   b12->config[0] = 4;
   AcceleratorRun(accel);
   int res2 = out21->state[0];

   a11->config[0] = 0;
   a12->config[0] = 0;
   b11->config[0] = 0;
   b12->config[0] = 0;

   a12->config[0] = 3;
   b12->config[0] = 4;
   AcceleratorRun(accel);
   int res3 = out22->state[0];

   return EXPECT("4 6 8 7","%d %d %d %d",res0,res1,res2,res3);
}

TEST(SimpleFlatten){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SimpleAdder"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   Accelerator* flatten = Flatten(versat,accel,1);

   int result = SimpleAdderInstance(flatten,4,5);

   return EXPECT("9","%d",result);
}

TEST(FlattenShareConfig){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("ComplexShareConfig"));
   Accelerator* accel_ = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel_,type,MakeSizedString("Test"));

   Accelerator* flatten = Flatten(versat,accel_,99);

   // Test by changing config for shared 1
   FUInstance* a11 = GetInstanceByName(flatten,"Test","shared1","a1");
   FUInstance* a12 = GetInstanceByName(flatten,"Test","shared1","a2");
   FUInstance* b11 = GetInstanceByName(flatten,"Test","shared1","b1");
   FUInstance* b12 = GetInstanceByName(flatten,"Test","shared1","b2");

   // But reading the output of shared 2 (should be the same, since same configuration = same results)
   FUInstance* out20 = GetInstanceByName(flatten,"Test","shared2","out0");
   FUInstance* out21 = GetInstanceByName(flatten,"Test","shared2","out1");
   FUInstance* out22 = GetInstanceByName(flatten,"Test","shared2","out2");

   a11->config[0] = 2;
   AcceleratorRun(flatten);
   int res0 = out20->state[0];

   a11->config[0] = 0;
   a12->config[0] = 3;
   AcceleratorRun(flatten);
   int res1 = out20->state[0];

   b12->config[0] = 4;
   AcceleratorRun(flatten);
   int res2 = out21->state[0];

   a11->config[0] = 0;
   a12->config[0] = 0;
   b11->config[0] = 0;
   b12->config[0] = 0;

   a12->config[0] = 3;
   b12->config[0] = 4;
   AcceleratorRun(flatten);
   int res3 = out22->state[0];

   return EXPECT("4 6 8 7","%d %d %d %d",res0,res1,res2,res3);
}

TEST(FlattenSHA){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SHA"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   Accelerator* flatten = Flatten(versat,accel,99);

   SetSHAAccelerator(flatten,nullptr);

   InitVersatSHA(versat,true);

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   VersatSHA(digest,msg_64,64);

   return EXPECT("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa","%s",GetHexadecimal(digest, HASH_SIZE));
}

TEST(ComplexFlatten){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("ReadWriteAES"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst =  CreateFUInstance(accel,type,MakeSizedString("Test"));

   FUInstance* delay = GetInstanceByName(accel,"Test","buffer0");

   Accelerator* flatten = Flatten(versat,accel,99);

   int cypher[] = {0x32,0x88,0x31,0xe0,
                  0x43,0x5a,0x31,0x37,
                  0xf6,0x30,0x98,0x07,
                  0xa8,0x8d,0xa2,0x34};
   int key[] =    {0x2b,0x28,0xab,0x09,
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c};
   int result[16];

   ConfigureSimpleVRead(GetInstanceByName(flatten,"Test","cypher"),16,cypher);
   ConfigureSimpleVRead(GetInstanceByName(flatten,"Test","key"),16,key);
   ConfigureSimpleVWrite(GetInstanceByName(flatten,"Test","results"),16,result);

   FillAESAccelerator(flatten);

   AcceleratorRun(flatten);
   AcceleratorRun(flatten);
   AcceleratorRun(flatten);

   //CheckMemory(flatten,flatten);
   //DisplayAcceleratorMemory(flatten);

   OutputVersatSource(versat,flatten,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",result[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);
}

TEST(SimpleMergeNoCommon){
   FUDeclaration* typeA = GetTypeByName(versat,MakeSizedString("SimpleAdder"));
   FUDeclaration* typeB = GetTypeByName(versat,MakeSizedString("ComplexMultiplier"));

   FUDeclaration* merged = MergeAccelerators(versat,typeA,typeB,MakeSizedString("NoCommonMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,MakeSizedString("Test"));

   int resA = 0;
   int resB = 0;

   resA = SimpleAdderInstance(accel,3,4);
   resB = ComplexMultiplierInstance(accel,2,3);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("7 6","%d %d",resA,resB);
}

TEST(SimpleMergeUnitCommonNoEdge){
   FUDeclaration* typeA = GetTypeByName(versat,MakeSizedString("SimpleAdder"));
   FUDeclaration* typeB = GetTypeByName(versat,MakeSizedString("ComplexAdder"));

   FUDeclaration* merged = MergeAccelerators(versat,typeA,typeB,MakeSizedString("UnitCommonNoEdgeMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,MakeSizedString("Test"));

   int resA = 0;
   int resB = 0;

   ClearConfigurations(accel);
   resA = SimpleAdderInstance(accel,4,5);

   ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,typeB);
   resB = ComplexAdderInstance(accel,2,3);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("9 5","%d %d",resA,resB);
}

TEST(SimpleMergeUnitAndEdgeCommon){
   FUDeclaration* typeA = GetTypeByName(versat,MakeSizedString("SimpleAdder"));
   FUDeclaration* typeB = GetTypeByName(versat,MakeSizedString("SemiComplexAdder"));

   FUDeclaration* merged = MergeAccelerators(versat,typeA,typeB,MakeSizedString("UnitAndEdgeCommonMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,MakeSizedString("Test"));

   int resA = 0;
   int resB = 0;

   //ClearConfigurations(accel);
   resA = SimpleAdderInstance(accel,4,5);

   //ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,typeB);
   resB = SemiComplexAdderInstance(accel,2,3);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("9 5","%d %d",resA,resB);
}

TEST(SimpleMergeInputOutputCommon){
   FUDeclaration* typeA = GetTypeByName(versat,MakeSizedString("ComplexAdder"));
   FUDeclaration* typeB = GetTypeByName(versat,MakeSizedString("ComplexMultiplier"));

   FUDeclaration* merged = MergeAccelerators(versat,typeA,typeB,MakeSizedString("InputOutputCommonMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,MakeSizedString("Test"));

   int resA = 0;
   int resB = 0;

   //ClearConfigurations(accel);
   resA = ComplexAdderInstance(accel,4,5);

   //ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,typeB);
   resB = ComplexMultiplierInstance(accel,2,3);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("9 6","%d %d",resA,resB);
}

TEST(ComplexMerge){
   FUDeclaration* typeA = GetTypeByName(versat,MakeSizedString("SHA"));
   FUDeclaration* typeB = GetTypeByName(versat,MakeSizedString("ReadWriteAES"));

   FUDeclaration* merged = MergeAccelerators(versat,typeA,typeB,MakeSizedString("SHA_AES"));

   TEST_PASSED;
}

TEST(SHA){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SHA"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   SetSHAAccelerator(accel,inst);

   InitVersatSHA(versat,true);

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   VersatSHA(digest,msg_64,64);

   return EXPECT("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa","%s",GetHexadecimal(digest, HASH_SIZE));
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
   TEST_INST( 1 ,TestMStage);
   TEST_INST( 1 ,TestFStage);
   TEST_INST( 1 ,SHA);
#endif
#if 0
   TEST_INST( 1 ,VReadToVWrite);
   TEST_INST( 1 ,StringHasher);
   TEST_INST( 1 ,Convolution);
   TEST_INST( 1 ,MatrixMultiplication);
   TEST_INST( 1 ,MatrixMultiplicationVRead);
   TEST_INST( 1 ,VersatAddRoundKey);
   TEST_INST( 1 ,LookupTable);
   TEST_INST( 1 ,VersatSubBytes);
   TEST_INST( 1 ,VersatShiftRows);
#endif
#if 0
   TEST_INST( 1 ,VersatDoRows);
   TEST_INST( 1 ,VersatMixColumns);
   TEST_INST( 1 ,FirstLineKey);
   TEST_INST( 1 ,KeySchedule);
   TEST_INST( 1 ,AESRound);
   TEST_INST( 1 ,AES);
   TEST_INST( 1 ,ReadWriteAES);
   TEST_INST( 1 ,SimpleAdder);
   TEST_INST( 1 ,ComplexMultiplier);
#endif
#if 0
   TEST_INST( 1 ,SimpleShareConfig);
   TEST_INST( 1 ,ComplexShareConfig);
#endif
#if 0
   TEST_INST( 1 ,SimpleFlatten);
   TEST_INST( 1 ,FlattenShareConfig);
   TEST_INST( 1 ,ComplexFlatten);
   TEST_INST( 1 ,FlattenSHA); // Problem on top level static buffers. Maybe do flattening of accelerators with buffers already fixed.
#endif
#if 0
   TEST_INST( 1 ,SimpleMergeNoCommon);
   TEST_INST( 1 ,SimpleMergeUnitCommonNoEdge);
   TEST_INST( 1 ,SimpleMergeUnitAndEdgeCommon);
   TEST_INST( 1 ,SimpleMergeInputOutputCommon);
   TEST_INST( 1 ,ComplexMerge);
#endif

   //Free(versat);

   printf("\nAutomatic tests done (passed/total): %d / %d\n",info.testsPassed,info.numberTests);
}

/*

- The simplest way to fix the flattenSHA testcase is to probably do the flattening of the fixed delay graphs.

- Test the AES,SHA merging on PC-Emul

- Start working towards sim-run of merged accelerators.
   Just store the info for any unit that is not an operation into a array with a hierarchical structure.
      Might need to store a pointer to a string with the name of the declaration in order to resolve GetInstance("name:type") calls


*/



















