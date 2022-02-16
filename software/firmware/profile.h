#include "stdio.h"
#include "iob_timer.h"

#define P_NVARS (3)
#define P_CNT (0)
#define P_STR (1)
#define P_END (2)

extern unsigned int prof_global_us[P_NVARS];
extern unsigned int prof_sha256_us[P_NVARS];
extern unsigned int prof_GetHexadecimal_us[P_NVARS];
extern unsigned int prof_printf_us[P_NVARS];

#define PROF_START(var) {prof_##var##_us[P_STR] = timer_time_us();}
#define PROF_STOP(var) {prof_##var##_us[P_END] = timer_time_us(); \
    prof_##var##_us[P_CNT] += prof_##var##_us[P_END] - prof_##var##_us[P_STR];}

#define PROF_REPORT(var) { \
    printf("PROFILE: %s time: %dus (%d%%)\n", #var, prof_##var##_us[P_CNT], \
            (prof_##var##_us[P_CNT]*100)/prof_global_us[P_CNT]); }

void profile_report();
