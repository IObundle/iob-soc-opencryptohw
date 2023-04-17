#ifndef INCLUDED_VERSAT_AES
#define INCLUDED_VERSAT_AES

#include <cstdint>
#include <cstddef>

// AES Sizes (bytes)
#define AES_BLK_SIZE (16)
#define AES_KEY_SIZE (32)

struct Versat;
struct Accelerator;
struct FUInstance;
struct FUDeclaration;

extern const uint8_t sbox[256];
extern const uint8_t mul2[];
extern const uint8_t mul3[];

void FillSBox(FUInstance* inst);

void FillSubBytes(Accelerator* accel);

void FillKeySchedule(FUInstance* inst);

void FillKeySchedule256(Accelerator* accel, int keyInst);

void FillRow(Accelerator* accel, int roundNum, int colNum);

void FillRound(Accelerator* accel, int roundNum);

void FillAES(Accelerator* accel); 

void Versat_init_AES(Accelerator* accel);

void VersatAES(Versat* versat, Accelerator* accel, uint8_t *result, uint8_t *cypher, uint8_t *key);
#endif // INCLUDED_VERSAT_AES
