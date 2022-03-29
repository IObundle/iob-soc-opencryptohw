#include "profile.h"

// Global profile variables
unsigned int prof_global_us[P_NVARS] = {0};
unsigned int prof_sha256_us[P_NVARS] = {0};
unsigned int prof_eth_us[P_NVARS] = {0};
unsigned int prof_mem_us[P_NVARS] = {0};
unsigned int prof_printf_us[P_NVARS] = {0};


unsigned int prof_sha_init_us[P_NVARS] = {0};
unsigned int prof_sha_finalize_us[P_NVARS] = {0};
unsigned int prof_sha_ctxrelease_us[P_NVARS] = {0};
unsigned int prof_crypto_hashblocks_us[P_NVARS] = {0};
unsigned int prof_ld_big_endian_us[P_NVARS] = {0};
unsigned int prof_st_big_endian_us[P_NVARS] = {0};
unsigned int prof_F_32_us[P_NVARS] = {0};
unsigned int prof_Expand32_us[P_NVARS] = {0};

void profile_report(){
    printf("PROFILE: REPORT\n");
    printf("PROFILE: Global time: %dus @%dMHz\n", prof_global_us[P_CNT], FREQ/1000000);
    PROF_REPORT(sha256)
    printf("\t"); PROF_REPORT(sha_init)
    printf("\t"); PROF_REPORT(sha_finalize)
    printf("\t\t"); PROF_REPORT(crypto_hashblocks)
    printf("\t\t\t"); PROF_REPORT(ld_big_endian)
    printf("\t\t\t"); PROF_REPORT(st_big_endian)
    printf("\t\t\t"); PROF_REPORT(F_32)
    printf("\t\t\t"); PROF_REPORT(Expand32)
    printf("\t\t"); PROF_REPORT(sha_ctxrelease)
    PROF_REPORT(mem)
    PROF_REPORT(eth)
    PROF_REPORT(printf)
}
