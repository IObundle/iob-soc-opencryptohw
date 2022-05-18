/* gen_test_unitM.c
 * program to generate input .bin and expected output .bin for unitM.v tests
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INPUT_LEN (64)
#define OUTPUT_LEN (64)
#define HEX_SIZE (16)

// From sha2.c
#define SHR(x, c) ((x) >> (c))
#define ROTR_32(x, c) (((x) >> (c)) | ((x) << (32 - (c))))

#define sigma0_32(x) (ROTR_32(x, 7) ^ ROTR_32(x, 18) ^ SHR(x, 3))
#define sigma1_32(x) (ROTR_32(x, 17) ^ ROTR_32(x, 19) ^ SHR(x, 10))

#define M_32(w0, w14, w9, w1) w0 = sigma1_32(w14) + (w9) + sigma0_32(w1) + (w0);

#define EXPAND_32                                                              \
    M_32(w0, w14, w9, w1)                                                      \
    M_32(w1, w15, w10, w2)                                                     \
    M_32(w2, w0, w11, w3)                                                      \
    M_32(w3, w1, w12, w4)                                                      \
    M_32(w4, w2, w13, w5)                                                      \
    M_32(w5, w3, w14, w6)                                                      \
    M_32(w6, w4, w15, w7)                                                      \
    M_32(w7, w5, w0, w8)                                                       \
    M_32(w8, w6, w1, w9)                                                       \
    M_32(w9, w7, w2, w10)                                                      \
    M_32(w10, w8, w3, w11)                                                     \
    M_32(w11, w9, w4, w12)                                                     \
    M_32(w12, w10, w5, w13)                                                    \
    M_32(w13, w11, w6, w14)                                                    \
    M_32(w14, w12, w7, w15)                                                    \
    M_32(w15, w13, w8, w0)

#define DEBUG_PRING_W_VARS                                                     \
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
    printf("w15 = %x\n", w15);

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

int main(int argc, const char *argv[]) {
    uint8_t input[INPUT_LEN] = {0};

    // initialize input vector
    for (int i = 0; i < INPUT_LEN; i++) {
        input[i] = (uint8_t)i;
    }

    write_bin_file(input, INPUT_LEN, "xunitM_in.bin");

    // // Generate next 16x 32bit words
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

    // DEBUG_PRING_W_VARS

    // Equivalent to xunitM FU
    EXPAND_32

    // DEBUG_PRING_W_VARS

    // write to output buffer
    uint8_t output[OUTPUT_LEN] = {0};
    store_int(output + 0, w0);
    store_int(output + 4, w1);
    store_int(output + 8, w2);
    store_int(output + 12, w3);
    store_int(output + 16, w4);
    store_int(output + 20, w5);
    store_int(output + 24, w6);
    store_int(output + 28, w7);
    store_int(output + 32, w8);
    store_int(output + 36, w9);
    store_int(output + 40, w10);
    store_int(output + 44, w11);
    store_int(output + 48, w12);
    store_int(output + 52, w13);
    store_int(output + 56, w14);
    store_int(output + 60, w15);

    write_bin_file(output, OUTPUT_LEN, "xunitM_out.bin");

    return 0;
}
