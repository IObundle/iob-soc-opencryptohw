#include <new>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include "unitVCD.h"
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
   vcd->dump(); \
   unit->clk = 1; \
   unit->eval(); \
   vcd->dump();

#define RESET(unit) \
   unit->rst = 1; \
   UPDATE(unit); \
   unit->rst = 0;

#define START_RUN(unit) \
   UPDATE(unit); \
   unit->run = 1; \
   UPDATE(unit); \
   unit->run = 0;

#define PREAMBLE(type) \
   type* self = &data->unit; \
   VCDData* vcd = &data->vcd;

static int Mcounter = 0;
static int Fcounter = 0;

struct UnitFData{
   VxunitF unit;
   VCDData vcd;
};

static int32_t* UnitFInitializeFunction(FUInstance* inst){
   char buffer[256];

   UnitFData* data = new (inst->extraData) UnitFData();

   PREAMBLE(VxunitF);

   self->trace(&vcd->vcd,99);

   sprintf(buffer,"./trace_out/unitF%d.vcd",Fcounter++);
   vcd->open(buffer);

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
   PREAMBLE(VxunitF);

   UnitFConfig* config = (UnitFConfig*) inst->config;

   // Update config
   self->configDelay = inst->delays[0];

   START_RUN(self);

   return NULL;
}

static int32_t* UnitFUpdateFunction(FUInstance* inst){
   static int32_t results[8];

   UnitFData* data = (UnitFData*) inst->extraData;
   PREAMBLE(VxunitF);

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
   FUDeclaration decl = {};

   decl.name = "xunitF";
   decl.nInputs = 10;
   decl.nOutputs = 8;
   decl.nConfigs = ARRAY_SIZE(unitFConfigWires);
   decl.configWires = unitFConfigWires;
   decl.extraDataSize = sizeof(UnitFData);
   decl.initializeFunction = UnitFInitializeFunction;
   decl.startFunction = UnitFStartFunction;
   decl.updateFunction = UnitFUpdateFunction;
   decl.latency = 17;
   decl.type = VERSAT_TYPE_IMPLEMENTS_DELAY;

   FU_Type type = RegisterFU(versat,decl);

   return type;
}

struct UnitMData{
   VxunitM unit;
   VCDData vcd;
};

static int32_t* UnitMInitializeFunction(FUInstance* inst){
   char buffer[256];

   UnitMData* data = new (inst->extraData) UnitMData();
   PREAMBLE(VxunitM);

   ENABLE_TRACE(self,vcd);

   sprintf(buffer,"./trace_out/unitM%d.vcd",Mcounter++);
   vcd->open(buffer);

   INIT(self);

   self->in0 = 0;

   RESET(self);

   return NULL;
}

static int32_t* UnitMStartFunction(FUInstance* inst){
   UnitMData* data = (UnitMData*) inst->extraData;
   PREAMBLE(VxunitM);

   UnitMConfig* config = (UnitMConfig*) inst->config;

   // Update config
   self->configDelay = inst->delays[0];

   START_RUN(self);

   return NULL;
}

static int32_t* UnitMUpdateFunction(FUInstance* inst){
   static int32_t out;

   UnitMData* data = (UnitMData*) inst->extraData;
   PREAMBLE(VxunitM);

   self->in0 = GetInputValue(inst,0);

   UPDATE(self);

   out = self->out0;

   return &out;
}

EXPORT FU_Type RegisterUnitM(Versat* versat){
   FUDeclaration decl = {};

   decl.name = "xunitM";
   decl.nInputs = 1;
   decl.nOutputs = 1;
   decl.nConfigs = ARRAY_SIZE(unitMConfigWires);
   decl.configWires = unitMConfigWires;
   decl.extraDataSize = sizeof(UnitMData);
   decl.initializeFunction = UnitMInitializeFunction;
   decl.startFunction = UnitMStartFunction;
   decl.updateFunction = UnitMUpdateFunction;
   decl.latency = 17;
   decl.type = VERSAT_TYPE_IMPLEMENTS_DELAY;

   FU_Type type = RegisterFU(versat,decl);

   return type;
}
