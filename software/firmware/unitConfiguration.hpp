#ifndef INCLUDED_UNIT_CONFIGURATION
#define INCLUDED_UNIT_CONFIGURATION

#include "versat.hpp"

void ConfigureSimpleVRead(FUInstance* inst, int numberItems,int* memory);
void ConfigureSimpleVWrite(FUInstance* inst, int numberItems,int* memory);
void ConfigureLeftSideMatrix(FUInstance* inst,int iterations);
void ConfigureRightSideMatrix(FUInstance* inst, int iterations);
void ConfigureMemoryLinear(FUInstance* inst, int amountOfData);
void ConfigureMemoryReceive(FUInstance* inst, int amountOfData,int interdataDelay);
void ConfigureLeftSideMatrixVRead(FUInstance* inst, int iterations);
void ConfigureRightSideMatrixVRead(FUInstance* inst, int iterations);
void ConfigureMatrixVWrite(FUInstance* inst,int amountOfData);

#endif // INCLUDED_UNIT_CONFIGURATION
