#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include "unitWrapper.h"

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

EXPORT FU_Type RegisterUnitF(Versat* versat){
   FUDeclaration decl = {};

   decl.name = "xunitF";
   decl.nInputs = 10;
   decl.nOutputs = 8;
   decl.latency = 17;
   decl.type = VERSAT_TYPE_IMPLEMENTS_DELAY;

   FU_Type type = RegisterFU(versat,decl);

   return type;
}

EXPORT FU_Type RegisterUnitM(Versat* versat){
   FUDeclaration decl = {};

   decl.name = "xunitM";
   decl.nInputs = 1;
   decl.nOutputs = 1;
   decl.latency = 17;
   decl.type = VERSAT_TYPE_IMPLEMENTS_DELAY;

   FU_Type type = RegisterFU(versat,decl);

   return type;
}
