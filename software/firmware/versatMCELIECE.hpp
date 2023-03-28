#ifndef INCLUDED_VERSAT_MCELIECE
#define INCLUDED_VERSAT_MCELIECE

#include <cstdint>
#include <cstddef>

struct Versat;
struct Accelerator;
struct FUInstance;
struct FUDeclaration;

void VersatInit(Versat* versat);
void VersatLineXOR(uint8_t* out, uint8_t *mat, uint8_t *row, int n_cols, uint8_t mask);
#endif // INCLUDED_VERSAT_MCELIECE
