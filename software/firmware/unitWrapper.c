#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include "unitWrapper.h"

#define INSTANTIATE_ARRAY
#include "shaUnitData.h"
#undef INSTANTIATE_ARRAY

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

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
                                    0, // Extra memory
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);

   return type;
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
                                    0, // Extra memory
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);

   return type;
}