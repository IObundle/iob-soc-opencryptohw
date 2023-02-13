#ifndef INCLUDED_VERSAT_SHA
#define INCLUDED_VERSAT_SHA

#include <cstdint>
#include <cstddef>

struct Versat;
struct Accelerator;
struct FUInstance;

static const int HASH_SIZE = (256/8);

void SetSHAAccelerator(Accelerator* accel,FUInstance* shaInstance); // Sets the accelerator instance

void InitVersatSHA(Versat* versat,bool outputVersatSource);
void VersatSHA(uint8_t *out, const uint8_t *in, size_t inlen);

#endif // INCLUDED_VERSAT_SHA
