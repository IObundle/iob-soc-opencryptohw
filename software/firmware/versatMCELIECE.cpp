
#include "versatMCELIECE.hpp"

#include "versat.hpp"
#include "unitConfiguration.hpp"
#include "verilogWrapper.inc"

extern "C" {
// #ifndef PC
// #include "iob-cache.h"
// #endif
int printf_(const char* format, ...);
}

#ifndef PC
#define printf printf_
#endif


// GLOBALS
static Accelerator* accel = NULL;
static FUInstance* McElieceInstance = NULL;
static FUDeclaration* type = NULL;
// static int run_cnt = 0;
// static int first_run = 1;

void VersatInit(Versat* versat) {
    if (accel == NULL) {
        accel = CreateAccelerator(versat);
    }
    if (type == NULL) {
        type = GetTypeByName(versat,MakeSizedString("VectorLikeOperation"));
    }
    if (McElieceInstance == NULL) {
        McElieceInstance = CreateFUInstance(accel,type,MakeSizedString("Test"));
    }
    return;
}
void VersatLineXOR(uint8_t *mat, uint8_t *row, int n_cols, uint8_t mask) {
   uint32_t mask_int = (mask) | (mask << 8) | (mask << 8*2) | (mask << 8*3);
   uint32_t *mat_int = (uint32_t*) mat;
   uint32_t *row_int = (uint32_t*) row;
   int n_cols_int = (n_cols >> 2);

   ConfigureSimpleVRead(GetInstanceByName(accel,"Test","row"), n_cols_int, (int*) row_int);

   FUInstance* matInst = GetInstanceByName(accel,"Test","mat");
   ConfigureMemoryLinear(matInst, n_cols_int);
   for (int c = 0; c < n_cols_int; c++){
       VersatUnitWrite(matInst,c,mat_int[c]);
   }

   FUInstance* maskInst = GetInstanceByName(accel,"Test","mask");
   maskInst->config[0] = mask_int;

   FUInstance* outputInst = GetInstanceByName(accel,"Test","output");
   // ConfigureMemoryReceive(outputInst, n_cols_int, 1);
   ConfigureMemoryLinearOut(outputInst, n_cols_int);
   
   AcceleratorRun(accel); // Fills vread with valid data
   AcceleratorRun(accel);

   // printf("\t%d\n", run_cnt);
   // run_cnt++;
   
   // if (first_run == 1){
   //     printf("mask: 0x%02x\n", mask);
   //     printf("mask_int: 0x%08x\n", mask_int);
   //     printf("mat:\n");
   //     for(int i = 0; i < n_cols_int; i++){
   //        printf("0x%08x ", mat_int[i]);
   //     }
   //     printf("\nrow:\n");
   //     for(int i = 0; i < n_cols_int; i++){
   //        printf("0x%08x ", row_int[i]);
   //     }
   // }
   //
   for (int c = 0; c < n_cols_int; c++){
        mat_int[c] = VersatUnitRead(outputInst,c);
   }
   //
   // if (first_run == 1){
   //     printf("\nResult:\n");
   //     for(int i = 0; i < n_cols_int; i++){
   //        printf("0x%08x ", mat_int[i]);
   //     }
   //     first_run = 0;
   // }
   
   return;
}
