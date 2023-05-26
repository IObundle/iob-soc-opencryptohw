#ifndef INCLUDED_VERSAT_AES
#define INCLUDED_VERSAT_AES

#include <cstdint>
#include <cstddef>

#include "versatExtra.hpp"

// AES Sizes (bytes)
#define AES_BLK_SIZE (16)
#define AES_KEY_SIZE (32)

struct Versat;
struct Accelerator;
struct FUInstance;
struct FUDeclaration;

extern const uint32_t sbox[256];
extern const uint32_t mul2[];
extern const uint32_t mul3[];

void FillSBox(FUInstance* inst);

void FillSubBytes(Accelerator* accel, FUInstance* inst);

void FillKeySchedule256(Accelerator* accel, FUInstance* inst);

void FillRow(Accelerator* accel, FUInstance* inst);

void FillRound(Accelerator* accel, FUInstance* inst);

void FillRoundPairAndKey(Accelerator* topLevel,FUInstance* roundAndKey);

void FillAES(Accelerator* accel); 

void Versat_init_AES(Accelerator* accel);

void VersatAES(SimpleAccelerator* simple, uint8_t *result, uint8_t *cypher, uint8_t *key);
#endif // INCLUDED_VERSAT_AES
