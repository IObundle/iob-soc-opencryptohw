#ifndef HPP_FULLSHATESTS_HPP
#define HPP_FULLSHATESTS_HPP

#include <cstdio>
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
}
#include "versat.hpp"
#include "versatSHA.hpp"

#define HASH_SIZE (256/8)

#define VERSAT_SHA_W_PTR_SIZE (16)
#define VERSAT_SHA_W_PTR_NBYTES (4*VERSAT_SHA_W_PTR_SIZE)

int get_int(char* ptr, unsigned int *i_val);
int get_msg(char *ptr, uint8_t **msg_ptr, int size);
int save_msg(char *ptr, uint8_t* msg, int size);
void mem_copy(uint8_t *src_buf, char *dst_buf, int size);
void Full_SHA_Test(Versat* versat);
#endif
