#ifndef HPP_FULLAESTESTS_HPP
#define HPP_FULLAESTESTS_HPP

extern "C"{
// #include "iob-cache.h"
#include "periphs.h"
#include "iob-uart.h"
#include "iob-eth.h"
#ifndef PC
#include "printf.h"
#else
#include <stdio.h>
#endif
#include "api.h"
#include "nistkatrng.h"
#include "memory_pool.h"
}
#include "versat.hpp"
#include "versatMCELIECE.hpp"

#define SEED_BYTES (48)
#define MAX_NUM_SEEDS (10)
#define MEMORY_POOL_SIZE (10000000)

int get_int(char* ptr, unsigned int *i_val);
int get_ptext_key_pair(uint8_t *ptr, uint8_t **ptext_ptr, uint8_t **key_ptr);
int save_msg(char *ptr, uint8_t* msg, int size);
void mem_copy(uint8_t *src_buf, char *dst_buf, int size);
void Full_McEliece_Test(Versat* versat);
#endif
