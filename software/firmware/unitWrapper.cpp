#include <new>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include <verilated_vcd_c.h>

#include "unitWrapper.h"

#include "VxunitM.h"
#include "VxunitF.h"

#define INSTANTIATE_ARRAY
#include "shaUnitData.h"
#undef INSTANTIATE_ARRAY

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

#define INIT(unit) \
   unit->run = 0; \
   unit->clk = 0; \
   unit->rst = 0;

#define UPDATE(unit) \
   unit->clk = 0; \
   unit->eval(); \
   tfp->dump(5 * data->timesDumped++); \
   unit->clk = 1; \
   unit->eval(); \
   tfp->dump(5 * data->timesDumped++);

#define RESET(unit) \
   unit->rst = 1; \
   UPDATE(unit); \
   unit->rst = 0;

#define START_RUN(unit) \
   unit->run = 1; \
   UPDATE(unit); \
   unit->run = 0;

static bool initTracing = false;
static int Mcounter = 0;
static int Fcounter = 0;

static VerilatedVcdC* vcdFiles[128];
static int openedVcdFiles = 0;

static void closeOpenedVcdFiles(){
   //printf("Closing vcd files\n");
   for(int i = 0; i < openedVcdFiles; i++){
      vcdFiles[i]->close();
   }
}

struct UnitFData{
   VxunitF unit;
   VerilatedVcdC vcd;
   int timesDumped;
};

static int32_t* UnitFInitializeFunction(FUInstance* inst){
   static char buffer[256];

   if(!initTracing){
      Verilated::traceEverOn(true);
      initTracing = true;
      atexit(closeOpenedVcdFiles);
   }

   UnitFData* data = (UnitFData*) inst->extraData;

   VxunitF* self = new (&data->unit) VxunitF();
   VerilatedVcdC* tfp = new (&data->vcd) VerilatedVcdC;
   vcdFiles[openedVcdFiles++] = tfp;

   data->timesDumped = 0;

   self->trace(tfp, 99);  // Trace 99 levels of hierarchy

   sprintf(buffer,"./trace_out/unitF%d.vcd",Fcounter++);

   tfp->open(buffer);

   INIT(self);
   
   self->in0 = 0;
   self->in1 = 0;
   self->in2 = 0;
   self->in3 = 0;
   self->in4 = 0;
   self->in5 = 0;
   self->in6 = 0;
   self->in7 = 0;
   self->in8 = 0;
   self->in9 = 0;

   RESET(self);

   return NULL;
}

static int32_t* UnitFStartFunction(FUInstance* inst){
   UnitFData* data = (UnitFData*) inst->extraData;
   VxunitF* self = &data->unit;
   VerilatedVcdC* tfp = &data->vcd;
   UnitFConfig* config = (UnitFConfig*) inst->config;

   // Update config
   self->configDelay = config->configDelay;

   START_RUN(self);

   return NULL;
}

static int32_t* UnitFUpdateFunction(FUInstance* inst){
   static int32_t results[8];

   UnitFData* data = (UnitFData*) inst->extraData;
   VxunitF* self = &data->unit;
   VerilatedVcdC* tfp = &data->vcd;

   self->in0 = GetInputValue(inst,0);
   self->in1 = GetInputValue(inst,1);
   self->in2 = GetInputValue(inst,2);
   self->in3 = GetInputValue(inst,3);
   self->in4 = GetInputValue(inst,4);
   self->in5 = GetInputValue(inst,5);
   self->in6 = GetInputValue(inst,6);
   self->in7 = GetInputValue(inst,7);
   self->in8 = GetInputValue(inst,8);
   self->in9 = GetInputValue(inst,9);

   UPDATE(self);

   results[0] = self->out0;
   results[1] = self->out1;
   results[2] = self->out2;
   results[3] = self->out3;
   results[4] = self->out4;
   results[5] = self->out5;
   results[6] = self->out6;
   results[7] = self->out7;

   return results;
}

EXPORT FU_Type RegisterUnitF(Versat* versat){
   FU_Type type = RegisterFU(versat,"xunitF",
                                    10, // n inputs
                                    8, // n outputs
                                    ARRAY_SIZE(unitFConfigWires), // Config
                                    unitFConfigWires,
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    false, // IO
                                    sizeof(UnitFData), // Extra memory
                                    UnitFInitializeFunction,
                                    UnitFStartFunction,
                                    UnitFUpdateFunction,
                                    NULL);

   return type;
}

struct UnitMData{
   VxunitM unit;
   VerilatedVcdC vcd;
   int timesDumped;
};

static int32_t* UnitMInitializeFunction(FUInstance* inst){
   static char buffer[256];

   if(!initTracing){
      Verilated::traceEverOn(true);
      initTracing = true;
      atexit(closeOpenedVcdFiles);
   }

   UnitMData* data = (UnitMData*) inst->extraData;

   VxunitM* self = new (&data->unit) VxunitM();
   VerilatedVcdC* tfp = new (&data->vcd) VerilatedVcdC;
   vcdFiles[openedVcdFiles++] = tfp;

   data->timesDumped = 0;

   self->trace(tfp, 99);  // Trace 99 levels of hierarchy

   sprintf(buffer,"./trace_out/unitM%d.vcd",Mcounter++);

   tfp->open(buffer);

   INIT(self);
   
   self->in0 = 0;

   RESET(self);

   return NULL;
}

static int32_t* UnitMStartFunction(FUInstance* inst){
   UnitMData* data = (UnitMData*) inst->extraData;
   VxunitM* self = &data->unit;
   VerilatedVcdC* tfp = &data->vcd;
   UnitMConfig* config = (UnitMConfig*) inst->config;

   // Update config
   self->configDelay = config->configDelay;

   START_RUN(self);

   return NULL;
}

static int32_t* UnitMUpdateFunction(FUInstance* inst){
   static int32_t out;

   UnitMData* data = (UnitMData*) inst->extraData;
   VxunitM* self = &data->unit;
   VerilatedVcdC* tfp = &data->vcd;

   self->in0 = GetInputValue(inst,0);

   UPDATE(self);

   out = self->out0;

   return &out;
}

EXPORT FU_Type RegisterUnitM(Versat* versat){
   FU_Type type = RegisterFU(versat,"xunitM",
                                    1, // n inputs
                                    1, // n outputs
                                    ARRAY_SIZE(unitMConfigWires), // Config
                                    unitMConfigWires,
                                    0, // State
                                    NULL,
                                    0, // MemoryMapped
                                    false, // IO
                                    sizeof(UnitMData), // Extra memory
                                    UnitMInitializeFunction,
                                    UnitMStartFunction,
                                    UnitMUpdateFunction,
                                    NULL);

   return type;
}
