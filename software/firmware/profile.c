#include "profile.h"

// Global profile variables
unsigned int prof_global_us[P_NVARS] = {0};
unsigned int prof_sha256_us[P_NVARS] = {0};
unsigned int prof_GetHexadecimal_us[P_NVARS] = {0};
unsigned int prof_printf_us[P_NVARS] = {0};

void profile_report(){
    printf("PROFILE: REPORT\n");
    printf("PROFILE: Global time: %dus @%dMHz\n", prof_global_us[P_CNT], FREQ/1000000);
    PROF_REPORT(sha256)
    PROF_REPORT(GetHexadecimal)
    PROF_REPORT(printf)
}
