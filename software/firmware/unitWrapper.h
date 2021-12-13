#ifndef INCLUDED_UNIT_WRAPPER
#define INCLUDED_UNIT_WRAPPER

#include "versat.h"

#ifdef __cplusplus
#define EXPORT extern "C"
#else
#define EXPORT
#endif

EXPORT FU_Type RegisterUnitF(Versat* versat);
EXPORT FU_Type RegisterUnitM(Versat* versat);

#endif //INCLUDED_UNIT_WRAPPER
