/* gen_test_unitF.c
 * program to generate input .bin and expected output .bin for xunitF.v tests
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INPUT_LEN (64)
#define OUTPUT_LEN (8 * 4 * 16)

// From sha2.c
static const uint8_t iv_256[32] = {
    0x6a, 0x09, 0xe6, 0x67, 0xbb, 0x67, 0xae, 0x85, 0x3c, 0x6e, 0xf3,
    0x72, 0xa5, 0x4f, 0xf5, 0x3a, 0x51, 0x0e, 0x52, 0x7f, 0x9b, 0x05,
    0x68, 0x8c, 0x1f, 0x83, 0xd9, 0xab, 0x5b, 0xe0, 0xcd, 0x19};

static const uint32_t k_vector[16] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174};

#define ROTR_32(x, c) (((x) >> (c)) | ((x) << (32 - (c))))

#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define Sigma0_32(x) (ROTR_32(x, 2) ^ ROTR_32(x, 13) ^ ROTR_32(x, 22))
#define Sigma1_32(x) (ROTR_32(x, 6) ^ ROTR_32(x, 11) ^ ROTR_32(x, 25))

#define F_32(w, k)                                                             \
    T1 = h + Sigma1_32(e) + Ch(e, f, g) + (k) + (w);                           \
    T2 = Sigma0_32(a) + Maj(a, b, c);                                          \
    h = g;                                                                     \
    g = f;                                                                     \
    f = e;                                                                     \
    e = d + T1;                                                                \
    d = c;                                                                     \
    c = b;                                                                     \
    b = a;                                                                     \
    a = T1 + T2;

#define DEBUG_PRINT_VARS                                                       \
    printf("w0 = %x\n", w0);                                                   \
    printf("w1 = %x\n", w1);                                                   \
    printf("w2 = %x\n", w2);                                                   \
    printf("w3 = %x\n", w3);                                                   \
    printf("w4 = %x\n", w4);                                                   \
    printf("w5 = %x\n", w5);                                                   \
    printf("w6 = %x\n", w6);                                                   \
    printf("w7 = %x\n", w7);                                                   \
    printf("w8 = %x\n", w8);                                                   \
    printf("w9 = %x\n", w9);                                                   \
    printf("w10 = %x\n", w10);                                                 \
    printf("w11 = %x\n", w11);                                                 \
    printf("w12 = %x\n", w12);                                                 \
    printf("w13 = %x\n", w13);                                                 \
    printf("w14 = %x\n", w14);                                                 \
    printf("w15 = %x\n", w15);                                                 \
    printf("a = %x\n", a);                                                     \
    printf("b = %x\n", b);                                                     \
    printf("c = %x\n", c);                                                     \
    printf("d = %x\n", g);                                                     \
    printf("e = %x\n", e);                                                     \
    printf("f = %x\n", f);                                                     \
    printf("g = %x\n", g);                                                     \
    printf("h = %x\n", h);

#define REGS_TO_OUTPUT(i)                                                      \
    store_int(output + ((i)*4 * 8) + 0, a);                                    \
    store_int(output + ((i)*4 * 8) + 4, b);                                    \
    store_int(output + ((i)*4 * 8) + 8, c);                                    \
    store_int(output + ((i)*4 * 8) + 12, d);                                   \
    store_int(output + ((i)*4 * 8) + 16, e);                                   \
    store_int(output + ((i)*4 * 8) + 20, f);                                   \
    store_int(output + ((i)*4 * 8) + 24, g);                                   \
    store_int(output + ((i)*4 * 8) + 28, h);

static uint32_t load_bigendian_32(const uint8_t *x) {
    return (uint32_t)(x[3]) | (((uint32_t)(x[2])) << 8) |
           (((uint32_t)(x[1])) << 16) | (((uint32_t)(x[0])) << 24);
}

static void store_int(uint8_t *x, uint32_t u) {
    x[0] = (uint8_t)(u >> 8 * 3);
    x[1] = (uint8_t)(u >> 8 * 2);
    x[2] = (uint8_t)(u >> 8 * 1);
    x[3] = (uint8_t)(u);
}

FILE *create_bin_file(char *fname) {
    if (fname == NULL) {
        printf("Empty filename string\n");
        exit(1);
        return NULL;
    }

    FILE *fp = fopen(fname, "wb");
    if (fp == NULL) {
        printf("Could not open %s\n", fname);
        exit(1);
        return NULL;
    }
    return fp;
}

void append_to_file(FILE *fp, const uint8_t *buf, int size) {
    if (fp == NULL || buf == NULL || size < 1)
        return;

    fwrite(buf, sizeof(uint8_t), size, fp);
    return;
}

void write_bin_file(uint8_t *buf, int size, char *fname) {
    if (buf == NULL || size < 1 || fname == NULL)
        return;

    FILE *fp = fopen(fname, "wb");
    if (fp == NULL) {
        printf("Could not open %s\n", fname);
        exit(1);
        return;
    }

    fwrite(buf, sizeof(uint8_t), size, fp);
    fclose(fp);
    return;
}

uint32_t invert_bytes(uint32_t in) {
    uint8_t val[4];
    val[0] = (in) & 0x0FF;
    val[1] = (in >> 8) & 0x0FF;
    val[2] = (in >> 16) & 0x0FF;
    val[3] = (in >> 24) & 0x0FF;
    return load_bigendian_32(val);
}

int main(int argc, const char *argv[]) {
    uint8_t input[INPUT_LEN] = {0};

    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t f;
    uint32_t g;
    uint32_t h;
    uint32_t T1;
    uint32_t T2;

    FILE *fin = create_bin_file("xunitF_in.bin");

    // write initial state to input .bin
    append_to_file(fin, iv_256, 32);

    // initialize input vector
    for (int i = 0; i < INPUT_LEN; i++) {
        input[i] = (uint8_t)i;
    }

    // write w+k pairs to input .bin
    for (int i = 0; i < 16; i++) {
        append_to_file(fin, input + (i * 4), 4);
        uint32_t inv_k_val = invert_bytes(k_vector[i]);
        append_to_file(fin, (uint8_t *)(&inv_k_val), 4);
    }

    // Get 16x 32bit words
    uint32_t w0 = load_bigendian_32(input + 0);
    uint32_t w1 = load_bigendian_32(input + 4);
    uint32_t w2 = load_bigendian_32(input + 8);
    uint32_t w3 = load_bigendian_32(input + 12);
    uint32_t w4 = load_bigendian_32(input + 16);
    uint32_t w5 = load_bigendian_32(input + 20);
    uint32_t w6 = load_bigendian_32(input + 24);
    uint32_t w7 = load_bigendian_32(input + 28);
    uint32_t w8 = load_bigendian_32(input + 32);
    uint32_t w9 = load_bigendian_32(input + 36);
    uint32_t w10 = load_bigendian_32(input + 40);
    uint32_t w11 = load_bigendian_32(input + 44);
    uint32_t w12 = load_bigendian_32(input + 48);
    uint32_t w13 = load_bigendian_32(input + 52);
    uint32_t w14 = load_bigendian_32(input + 56);
    uint32_t w15 = load_bigendian_32(input + 60);

    // load iv_256 into a-h state registers
    a = load_bigendian_32(iv_256 + 0);
    b = load_bigendian_32(iv_256 + 4);
    c = load_bigendian_32(iv_256 + 8);
    d = load_bigendian_32(iv_256 + 12);
    e = load_bigendian_32(iv_256 + 16);
    f = load_bigendian_32(iv_256 + 20);
    g = load_bigendian_32(iv_256 + 24);
    h = load_bigendian_32(iv_256 + 28);

    uint8_t output[OUTPUT_LEN] = {0};
    // DEBUG_PRINT_VARS

    // Equivalent to xunitF FU
    F_32(w0, k_vector[0])
    REGS_TO_OUTPUT(0)
    F_32(w1, k_vector[1])
    REGS_TO_OUTPUT(1)
    F_32(w2, k_vector[2])
    REGS_TO_OUTPUT(2)
    F_32(w3, k_vector[3])
    REGS_TO_OUTPUT(3)
    F_32(w4, k_vector[4])
    REGS_TO_OUTPUT(4)
    F_32(w5, k_vector[5])
    REGS_TO_OUTPUT(5)
    F_32(w6, k_vector[6])
    REGS_TO_OUTPUT(6)
    F_32(w7, k_vector[7])
    REGS_TO_OUTPUT(7)
    F_32(w8, k_vector[8])
    REGS_TO_OUTPUT(8)
    F_32(w9, k_vector[9])
    REGS_TO_OUTPUT(9)
    F_32(w10, k_vector[10])
    REGS_TO_OUTPUT(10)
    F_32(w11, k_vector[11])
    REGS_TO_OUTPUT(11)
    F_32(w12, k_vector[12])
    REGS_TO_OUTPUT(12)
    F_32(w13, k_vector[13])
    REGS_TO_OUTPUT(13)
    F_32(w14, k_vector[14])
    REGS_TO_OUTPUT(14)
    F_32(w15, k_vector[15])
    REGS_TO_OUTPUT(15)

    // DEBUG_PRINT_VARS

    write_bin_file(output, OUTPUT_LEN, "xunitF_out.bin");

    return 0;
}
