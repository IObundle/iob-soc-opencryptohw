#include <cstdarg>
#include <cstdio>

#include "versat.hpp"
#include "utils.hpp"
#include "unitConfiguration.hpp"
#include "verilogWrapper.inc"

extern "C"{
int printf_(const char* format, ...);
}

#ifndef PC
#define printf printf_
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

int* TestSequentialInstance(Versat* versat,Accelerator* accel,FUInstance* inst,int numberValues,int numberOutputs,...){
   static int out[99];
   FUInstance* outputs[99];

   va_list args;
   va_start(args,numberOutputs);

   FUInstance* input = CreateFUInstance(accel,GetTypeByName(versat,MakeSizedString("Mem")),MakeSizedString("memIn"));

   ConnectUnits(input,0,inst,0);
   {
      volatile MemConfig* c = (volatile MemConfig*) input->config;

      c->iterA = 1;
      c->incrA = 1;
      c->perA = numberValues;
      c->dutyA = numberValues;
   }

   int registersAdded = 0;
   for(int i = 0; i < numberValues; i++){
      int val = va_arg(args,int);

      VersatUnitWrite(input,i,val);
   }

   registersAdded = 0;
   for(int i = 0; i < numberOutputs; i++){
      char buffer[128];
      int size = snprintf(buffer,128,"regOut%d",registersAdded++);
      outputs[i] = CreateFUInstance(accel,GetTypeByName(versat,MakeSizedString("Reg")),MakeSizedString(buffer,size));

      ConnectUnits(inst,i,outputs[i],0);
   }

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   AcceleratorRun(accel);

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

#define TEST_FAILED TestInfo(0)
#define TEST_PASSED TestInfo(1)

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

   bool result = (strcmp(expected,buffer) == 0);
   if(result){
      return TEST_PASSED;
   } else {
      printf("\n");
      printf("[%2d]Test failed: %s\n",testNumber,functionName);
      printf("    Expected: %s\n",expected);
      printf("    Result:   %s\n",buffer);
      printf("\n");

      return TEST_FAILED;
   }
}

#define TEST(TEST_NAME) static TestInfo TEST_NAME(Versat* versat,int testNumber)

TEST(TestMStage){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("M_Stage"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* top = CreateFUInstance(accel,type,MakeSizedString("Test"));

   FUInstance* inst = GetInstanceByName(accel,"Test","sigma");

   int constants[] = {7,18,3,17,19,10};
   for(size_t i = 0; i < ARRAY_SIZE(constants); i++){
      inst->config[i] = constants[i];
   }

   int* out = TestInstance(versat,accel,top,4,1,0x5a86b737,0xa9f9be83,0x08251f6d,0xeaea8ee9);

   return EXPECT("0xb89ab4ca","0x%x",out[0]);
}

TEST(TestM){
   FUDeclaration* type = GetTypeByName(versat,MakeSizedString("M"));
   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

   // Set constants to every entity
   FUInstance* stage = GetInstanceByName(accel,"Test","m0","sigma");

   int constants[] = {7,18,3,17,19,10};
   for(size_t i = 0; i < ARRAY_SIZE(constants); i++){
      stage->config[i] = constants[i];
   }

   int* out = TestSequentialInstance(versat,accel,inst,16,1,0x5a86b737,0xeaea8ee9,0x76a0a24d,0xa63e7ed7,0xeefad18a,0x101c1211,0xe2b3650c,0x5187c2a8,0xa6505472,0x08251f6d,0x4237e661,0xc7bf4c77,0xf3353903,0x94c37fa1,0xa9f9be83,0x6ac28509);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"%08x ",out[i]);
   }

   return EXPECT("b89ab4ca fc0ba687 6f70775f fd7fcf73 ddc5d5d7 b54ee23e 481631f5 9c325ada 1e01af58 11016b62 465da978 961e5ee7 9860640b 3f309ec4 439e4f9d 14ca5690 ","%s",buffer);
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

   return TEST_FAILED;
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
         ptr += sprintf(ptr,"%6d", VersatUnitRead(res,i * 3 + j));
      }
   }

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("  -520  -251   -49   -33   -42   303  -221  -100  -149","%s",buffer);
}

TEST(VersatMatrixMultiplication){
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

TEST(VersatMatrixMultiplicationVRead){
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


// When 1, need to pass 0 to enable test (changes enabler from 1 to 0)
#define REVERSE_ENABLED 0

#ifndef HARDWARE_TEST
   #define HARDWARE_TEST -1
   #define ENABLE_TEST(ENABLED) ((((ENABLED) && !REVERSE_ENABLED) || (REVERSE_ENABLED && !(ENABLED))) && (currentTest++ >= 0)) // The currentTest part is just to update the variable
#else
   #define ENABLE_TEST(ENABLED) (currentTest++ == hardwareTest)
#endif

#define TEST_NUMBER (currentTest - 1)

void AutomaticTests(Versat* versat){
   TestInfo info = TestInfo(0,0);
   int hardwareTest = HARDWARE_TEST;
   int currentTest = 0;

   if(ENABLE_TEST( 1 )){
      info += TestMStage(versat,TEST_NUMBER);
   }
   if(ENABLE_TEST( 0 )){
      info += TestM(versat,TEST_NUMBER);
   }
   if(ENABLE_TEST( 1 )){
      info += TestFStage(versat,TEST_NUMBER);
   }
   if(ENABLE_TEST( 1 )){
      info += VReadToVWrite(versat,TEST_NUMBER);
   }
   if(ENABLE_TEST( 1 )){
      info += StringHasher(versat,TEST_NUMBER);
   }
   if(ENABLE_TEST( 1 )){
      info += Convolution(versat,TEST_NUMBER);
   }
   if(ENABLE_TEST( 1 )){
      info += VersatMatrixMultiplication(versat,TEST_NUMBER);
   }
   if(ENABLE_TEST( 1 )){
      info += VersatMatrixMultiplicationVRead(versat,TEST_NUMBER);
   }

   printf("\nAutomatic tests done (passed/total): %d / %d\n",info.testsPassed,info.numberTests);
}







