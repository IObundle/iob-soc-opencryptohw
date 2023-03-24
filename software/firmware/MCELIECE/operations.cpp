extern "C"{
#include "api.h"

#include "aes256ctr.h"
#include "controlbits.h"
#include "crypto_hash.h"
#include "decrypt.h"
#include "encrypt.h"
#include "params.h"
#include "pk_gen.h"
#include "randombytes.h"
#include "sk_gen.h"
#include "util.h"

#include <stdint.h>
#include <string.h>
#ifndef PC
#include "printf.h"
#else
#include <stdio.h>
#endif
#include "memory_pool.h"
}

int PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_enc(
    uint8_t *c,
    uint8_t *key,
    const uint8_t *pk
) {
    uint8_t two_e[ 1 + SYS_N / 8 ] = {2};
    uint8_t *e = two_e + 1;
    uint8_t one_ec[ 1 + SYS_N / 8 + (SYND_BYTES + 32) ] = {1};

    PQCLEAN_MCELIECE348864_CLEAN_encrypt(c, e, pk);

    crypto_hash_32b(c + SYND_BYTES, two_e, sizeof(two_e));

    memcpy(one_ec + 1, e, SYS_N / 8);
    memcpy(one_ec + 1 + SYS_N / 8, c, SYND_BYTES + 32);

    crypto_hash_32b(key, one_ec, sizeof(one_ec));

    return 0;
}

int PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_dec(
    uint8_t *key,
    const uint8_t *c,
    const uint8_t *sk
) {
    int i;

    uint8_t ret_confirm = 0;
    uint8_t ret_decrypt = 0;

    uint16_t m;

    uint8_t conf[32];
    uint8_t two_e[ 1 + SYS_N / 8 ] = {2};
    uint8_t *e = two_e + 1;
    uint8_t preimage[ 1 + SYS_N / 8 + (SYND_BYTES + 32) ];
    uint8_t *x = preimage;

    //

    ret_decrypt = (uint8_t)PQCLEAN_MCELIECE348864_CLEAN_decrypt(e, sk + SYS_N / 8, c);

    crypto_hash_32b(conf, two_e, sizeof(two_e));

    for (i = 0; i < 32; i++) {
        ret_confirm |= conf[i] ^ c[SYND_BYTES + i];
    }

    m = ret_decrypt | ret_confirm;
    m -= 1;
    m >>= 8;

    *x++ = (~m &     0) | (m &    1);
    for (i = 0; i < SYS_N / 8;         i++) {
        *x++ = (~m & sk[i]) | (m & e[i]);
    }
    for (i = 0; i < SYND_BYTES + 32; i++) {
        *x++ = c[i];
    }

    crypto_hash_32b(key, preimage, sizeof(preimage));

    return 0;
}

#ifdef SIM
int PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_keypair
(
    uint8_t *pk,
    uint8_t *sk
) {
    uint8_t seed[ 32 ];
    uint8_t nonce[ 16 ] = {0};

    // set seed
    randombytes(seed, sizeof(seed));

    // generate placeholder sk: SK_BYTES from seed
    PQCLEAN_MCELIECE348864_CLEAN_aes256ctr(sk, SK_BYTES, nonce, seed);

    // generate placeholder pk from placeholder sk
    PQCLEAN_MCELIECE348864_CLEAN_pk_gen(pk, NULL, sk + SYS_N / 8);

    return 0;
}
#else
int PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_keypair
(
    uint8_t *pk,
    uint8_t *sk
) {
    int i;
    uint8_t *seed = (uint8_t*) MemPool_Alloc(32*sizeof(uint8_t));
    uint8_t *r = (uint8_t*) MemPool_Alloc(( SYS_T * 2 + (1 << GFBITS)*sizeof(uint32_t) + SYS_N / 8 + 32 )*sizeof(uint8_t));
    size_t sizeof_r =  (SYS_T * 2 + (1 << GFBITS)*sizeof(uint32_t) + SYS_N / 8 + 32)*sizeof(uint8_t);
    uint8_t *nonce = (uint8_t*) MemPool_Alloc(16*sizeof(uint8_t));
    for (int i = 0; i < 16; i++) {
        nonce[i] = 0;
    }
    uint8_t *rp;

    gf *f = (gf*) MemPool_Alloc(SYS_T*sizeof(gf));
    gf *irr = (gf*) MemPool_Alloc(SYS_T*sizeof(gf));
    uint32_t *perm = (uint32_t*) MemPool_Alloc((1 << GFBITS)*sizeof(uint32_t));

    printf("pre randombytes\n");
    randombytes(seed, 32*sizeof(uint8_t));

    while (1) {
        rp = r;
        printf("pre aes\n");
        PQCLEAN_MCELIECE348864_CLEAN_aes256ctr(r, sizeof_r, nonce, seed);
        memcpy(seed, &r[ sizeof_r - 32 ], 32);

        for (i = 0; i < SYS_T; i++) {
            f[i] = PQCLEAN_MCELIECE348864_CLEAN_load2(rp + i * 2);
        }
        rp += SYS_T*sizeof(gf);
        printf("pre genpoly_gen\n");
        if (PQCLEAN_MCELIECE348864_CLEAN_genpoly_gen(irr, f)) {
            continue;
        }

        for (i = 0; i < (1 << GFBITS); i++) {
            printf("\tperm[%d]\n", i);
            perm[i] = PQCLEAN_MCELIECE348864_CLEAN_load4(rp + i * 4);
        }
        rp += (1 << GFBITS)*sizeof(uint32_t);
        printf("pre perm_check\n");
        if (PQCLEAN_MCELIECE348864_CLEAN_perm_check(perm)) {
            continue;
        }

        for (i = 0; i < SYS_T;   i++) {
            PQCLEAN_MCELIECE348864_CLEAN_store2(sk + SYS_N / 8 + i * 2, irr[i]);
        }
        printf("pre pk_gen\n");
        if (PQCLEAN_MCELIECE348864_CLEAN_pk_gen(pk, perm, sk + SYS_N / 8)) {
            continue;
        }

        memcpy(sk, rp, SYS_N / 8);
        printf("pre controlbits\n");
        PQCLEAN_MCELIECE348864_CLEAN_controlbits(sk + SYS_N / 8 + IRR_BYTES, perm);

        break;
    }

    MemPool_Free((1 << GFBITS)*sizeof(uint32_t)); // perm
    MemPool_Free(SYS_T*sizeof(gf)); // irr
    MemPool_Free(SYS_T*sizeof(gf)); // f
    MemPool_Free(16*sizeof(uint8_t)); // nonce
    MemPool_Free(( SYS_T * 2 + (1 << GFBITS)*sizeof(uint32_t) + SYS_N / 8 + 32 )*sizeof(uint8_t)); // r
    MemPool_Free(32*sizeof(uint8_t)); // seed

    return 0;
}
#endif // SIM
