#include "unitConfiguration.hpp"

#include "verilogWrapper.inc"

void IntSet(int* buffer,int value,int byteSize){
   int nInts = byteSize / 4;

   for(int i = 0; i < nInts; i++){
      buffer[i] = value;
   }
}

void ConfigureSimpleVRead(FUInstance* inst, int numberItems,int* memory){
   IntSet(inst->config,0,sizeof(VReadConfig));
   volatile VReadConfig* c = (volatile VReadConfig*) inst->config;

   Assert(numberItems > 0);

   // Memory side
   c->incrA = 1;
   c->iterA = 1;
   c->perA = numberItems;
   c->dutyA = numberItems;
   c->size = 8;
   c->int_addr = 0;
   c->pingPong = 0;

   // B - versat side
   c->iterB = numberItems;
   c->incrB = 1;
   c->perB = 1;
   c->dutyB = 1;
   c->ext_addr = (int) memory;
   c->length = numberItems - 1; // AXI requires length of len - 1
}

void ConfigureSimpleVWrite(FUInstance* inst, int numberItems,int* memory){
   IntSet(inst->config,0,sizeof(VWriteConfig));
   volatile VWriteConfig* c = (volatile VWriteConfig*) inst->config;

   Assert(numberItems > 0);

   // Write side
   c->incrA = 1;
   c->iterA = 1;
   c->perA = numberItems;
   c->dutyA = numberItems;
   c->size = 8;
   c->int_addr = 0;
   c->pingPong = 0;
   c->length = numberItems - 1;
   c->ext_addr = (int) memory;

   // Memory side
   c->iterB = numberItems;
   c->perB = 1;
   c->dutyB = 1;
   c->incrB = 1;
}

void ConfigureLeftSideMatrix(FUInstance* inst,int iterations){
   IntSet(inst->config,0,sizeof(MemConfig));
   volatile MemConfig* config = (volatile MemConfig*) inst->config;

   config->iterA = iterations;
   config->perA = iterations;
   config->dutyA = iterations;
   config->startA = 0;
   config->shiftA = -iterations;
   config->incrA = 1;
   config->reverseA = 0;
   config->iter2A = 1;
   config->per2A = iterations;
   config->shift2A = 0;
   config->incr2A = iterations;
}

void ConfigureRightSideMatrix(FUInstance* inst, int iterations){
   IntSet(inst->config,0,sizeof(MemConfig));
   volatile MemConfig* config = (volatile MemConfig*) inst->config;

   config->iterA = iterations;
   config->perA = iterations;
   config->dutyA = iterations;
   config->startA = 0;
   config->shiftA = -(iterations * iterations - 1);
   config->incrA = iterations;
   config->reverseA = 0;
   config->iter2A = 1;
   config->per2A = iterations;
   config->shift2A = 0;
   config->incr2A = 0;
}

void ConfigureMemoryLinear(FUInstance* inst, int amountOfData){
   IntSet(inst->config,0,sizeof(MemConfig));
   volatile MemConfig* config = (volatile MemConfig*) inst->config;

   config->iterA = 1;
   config->perA = amountOfData;
   config->dutyA = amountOfData;
   config->incrA = 1;
}

void ConfigureMemoryReceive(FUInstance* inst, int amountOfData,int interdataDelay){
   IntSet(inst->config,0,sizeof(MemConfig));
   volatile MemConfig* config = (volatile MemConfig*) inst->config;

   config->iterA = amountOfData;
   config->perA = interdataDelay;
   config->dutyA = 1;
   config->startA = 0;
   config->shiftA = 0;
   config->incrA = 1;
   config->in0_wr = 1;
   config->reverseA = 0;
   config->iter2A = 0;
   config->per2A = 0;
   config->shift2A = 0;
   config->incr2A = 0;
}

void ConfigureLeftSideMatrixVRead(FUInstance* inst, int iterations){
   IntSet(inst->config,0,sizeof(VReadConfig));
   volatile VReadConfig* config = (volatile VReadConfig*) inst->config;

   int numberItems = iterations * iterations;

   Assert(numberItems > 0);

   config->incrA = 1;
   config->iterA = 1;
   config->perA = numberItems;
   config->dutyA = numberItems;
   config->size = 8;
   config->int_addr = 0;
   config->pingPong = 0;
   config->length = numberItems - 1;

   config->iterB = iterations;
   config->perB = iterations;
   config->dutyB = iterations;
   config->startB = 0;
   config->shiftB = -iterations;
   config->incrB = 1;
   config->reverseB = 0;
   config->iter2B = 1;
   config->per2B = iterations;
   config->shift2B = 0;
   config->incr2B = iterations;
}

void ConfigureRightSideMatrixVRead(FUInstance* inst, int iterations){
   IntSet(inst->config,0,sizeof(VReadConfig));
   volatile VReadConfig* config = (volatile VReadConfig*) inst->config;

   int numberItems = iterations * iterations;

   Assert(numberItems > 0);

   config->incrA = 1;
   config->iterA = 1;
   config->perA = numberItems;
   config->dutyA = numberItems;
   config->size = 8;
   config->int_addr = 0;
   config->pingPong = 0;
   config->length = numberItems - 1;

   config->iterB = iterations;
   config->perB = iterations;
   config->dutyB = iterations;
   config->startB = 0;
   config->shiftB = -(iterations * iterations - 1);
   config->incrB = iterations;
   config->reverseB = 0;
   config->iter2B = 1;
   config->per2B = iterations;
   config->shift2B = 0;
   config->incr2B = 0;
}

void ConfigureMatrixVWrite(FUInstance* inst,int amountOfData){
   IntSet(inst->config,0,sizeof(VWriteConfig));
   volatile VWriteConfig* config = (volatile VWriteConfig*) inst->config;

   Assert(amountOfData > 0);

   config->incrA = 1;
   config->iterA = 1;
   config->perA = amountOfData;
   config->dutyA = amountOfData;
   config->size = 8;
   config->int_addr = 0;
   config->pingPong = 0;
   config->length = amountOfData - 1;

   config->iterB = amountOfData;
   config->perB = 4;
   config->dutyB = 1;
   config->incrB = 1;
}



