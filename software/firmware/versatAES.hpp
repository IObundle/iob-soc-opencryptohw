#ifndef INCLUDED_VERSAT_AES
#define INCLUDED_VERSAT_AES

#include <cstdint>
#include <cstddef>

// AES Sizes (bytes)
#define AES_BLK_SIZE (16)
#define AES_KEY_SIZE (16)

struct Versat;
struct Accelerator;
struct FUInstance;
struct FUDeclaration;

extern const uint8_t sbox[256];
extern const uint8_t mul2[];
extern const uint8_t mul3[];

void FillSBox(FUInstance* inst);

void FillSubBytes(FUInstance* inst);

void FillKeySchedule(FUInstance* inst);

void FillRow(FUInstance* row);

void FillRound(FUInstance* round);

void FillAES(FUInstance* inst); 

void VersatAES(Versat* versat, uint8_t *result, uint8_t *cypher, uint8_t *key) ;
#endif // INCLUDED_VERSAT_AES
