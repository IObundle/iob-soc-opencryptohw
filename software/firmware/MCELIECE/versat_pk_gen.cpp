/*
  This file is for public-key generation
*/

extern "C"{
#include <string.h>

#include "benes.h"
#include "controlbits.h"
#include "gf.h"
#include "params.h"
#include "pk_gen.h"
#include "root.h"
#include "util.h"
#include "memory_pool.h"
#include "printf.h"
}

#include "versatMCELIECE.hpp"

static int test = 0;
static int n_systematic = 0;

/* input: secret key sk */
/* output: public key pk */
int PQCLEAN_MCELIECE348864_CLEAN_pk_gen(uint8_t *pk, uint32_t *perm, const uint8_t *sk) {
    int i, j, k;
    int row, c;

    uint64_t *buf = (uint64_t*) MemPool_Alloc((1 << GFBITS)*sizeof(uint64_t));

    uint8_t *mat = (uint8_t*) MemPool_Alloc(( GFBITS * SYS_T )*( SYS_N / 8 )*sizeof(uint8_t));
    uint8_t mask;
    uint8_t b;

    gf *g = (gf*) MemPool_Alloc((SYS_T + 1)*sizeof(gf)); // Goppa polynomial
    gf *L = (gf*) MemPool_Alloc((SYS_N)*sizeof(gf)); // support
    gf *inv = (gf*) MemPool_Alloc((SYS_N)*sizeof(gf));

    //

    g[ SYS_T ] = 1;

    printf("\tinit g\n");
    for (i = 0; i < SYS_T; i++) {
        g[i] = PQCLEAN_MCELIECE348864_CLEAN_load2(sk);
        g[i] &= GFMASK;
        sk += 2;
    }

    printf("\tinit buf\n");
    for (i = 0; i < (1 << GFBITS); i++) {
        buf[i] = perm[i];
        buf[i] <<= 31;
        buf[i] |= i;
    }

    printf("\tsort\n");
    PQCLEAN_MCELIECE348864_CLEAN_sort_63b(1 << GFBITS, buf);

    printf("\tbuf\n");
    for (i = 0; i < (1 << GFBITS); i++) {
        perm[i] = buf[i] & GFMASK;
    }
    printf("\tL\n");
    for (i = 0; i < SYS_N;         i++) {
        L[i] = PQCLEAN_MCELIECE348864_CLEAN_bitrev((gf)perm[i]);
    }

    // filling the matrix

    printf("\troot\n");
    PQCLEAN_MCELIECE348864_CLEAN_root(inv, g, L);

    printf("\tinv\n");
    for (i = 0; i < SYS_N; i++) {
        inv[i] = PQCLEAN_MCELIECE348864_CLEAN_gf_inv(inv[i]);
    }

    printf("\tinit mat 0\n");
    for (i = 0; i < PK_NROWS; i++) {
        for (j = 0; j < SYS_N / 8; j++) {
            // mat[i][j] = 0;
            mat[i*(SYS_N/8)+j] = 0;
        }
    }

    for (i = 0; i < SYS_T; i++) {
        for (j = 0; j < SYS_N; j += 8) {
            for (k = 0; k < GFBITS;  k++) {
                b  = (inv[j + 7] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 6] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 5] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 4] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 3] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 2] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 1] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 0] >> k) & 1;

                mat[( i * GFBITS + k)*(SYS_N/8) + (j / 8) ] = b;
            }
        }
        printf("\t3l i: %d\n", i);

        for (j = 0; j < SYS_N; j++) {
            inv[j] = PQCLEAN_MCELIECE348864_CLEAN_gf_mul(inv[j], L[j]);
        }

    }

    // gaussian elimination
    printf("\tgaussian elimination\n", i);

    for (i = 0; i < (GFBITS * SYS_T + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            row = i * 8 + j;

            if (row >= GFBITS * SYS_T) {
                break;
            }

            for (k = row + 1; k < GFBITS * SYS_T; k++) {
                mask = mat[ row*(SYS_N/8) + i ] ^ mat[ k*(SYS_N/8) + i ];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                if (mask != 0){
                    printf("\trow pre: %d\n", row);
                    VersatLineXOR(&(mat[row*(SYS_N/8)+0]), &(mat[k*(SYS_N/8)+0]), SYS_N / 8, mask);
                    // printf("\trow post: %d\n", row);
                }
                // for (c = 0; c < SYS_N / 8; c++) {
                //     mat[ row ][ c ] ^= mat[ k ][ c ] & mask;
                // }
            }

            if ( ((mat[ row*(SYS_N/8) + i ] >> j) & 1) == 0 ) { // return if not systematic
                n_systematic++;
                printf("\ttest: %d | n_systematic: %d\n", test, n_systematic);
                MemPool_Free((SYS_N)*sizeof(gf)); // inv
                MemPool_Free((SYS_N)*sizeof(gf)); // L
                MemPool_Free((SYS_T + 1)*sizeof(gf)); // g
                MemPool_Free(( GFBITS * SYS_T )*( SYS_N / 8 )*sizeof(uint8_t)); // mat
                MemPool_Free((1 << GFBITS)*sizeof(uint64_t)); // buf
                return -1;
            }

            for (k = 0; k < GFBITS * SYS_T; k++) {
                if (k != row) {
                    mask = mat[ k*(SYS_N/8) + i ] >> j;
                    mask &= 1;
                    mask = -mask;

                    for (c = 0; c < SYS_N / 8; c++) {
                        mat[ k*(SYS_N/8) + c ] ^= mat[ row*(SYS_N/8) + c ] & mask;
                    }
                }
            }
        }
    }

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, &(mat[i*(SYS_N/8)]) + PK_NROWS / 8, PK_ROW_BYTES);
    }

    test++; // success: move to next test
    n_systematic = 0; // success: reset counter

    MemPool_Free((SYS_N)*sizeof(gf)); // inv
    MemPool_Free((SYS_N)*sizeof(gf)); // L
    MemPool_Free((SYS_T + 1)*sizeof(gf)); // g
    MemPool_Free(( GFBITS * SYS_T )*( SYS_N / 8 )*sizeof(uint8_t)); // mat
    MemPool_Free((1 << GFBITS)*sizeof(uint64_t)); // buf
    return 0;
}

