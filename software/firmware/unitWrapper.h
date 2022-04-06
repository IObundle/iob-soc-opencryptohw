#ifndef INCLUDED_UNIT_WRAPPER
#define INCLUDED_UNIT_WRAPPER

#include "versat.h"

#ifdef __cplusplus
#define EXPORT extern "C"
#else
#define EXPORT
#endif

EXPORT FUDeclaration* RegisterUnitF(Versat* versat);
EXPORT FUDeclaration* RegisterUnitM(Versat* versat);

#endif //INCLUDED_UNIT_WRAPPER
