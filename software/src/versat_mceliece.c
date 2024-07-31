#include "versat_crypto.h"

#include "versat_accel.h"
#include "unitConfiguration.h"

#include <string.h>

#include "printf.h"

#include "controlbits.h"
#include "benes.h"
#include "crypto_declassify.h"
#include "crypto_uint64.h"
#include "params.h"
#include "pk_gen.h"
#include "sk_gen.h"
#include "root.h"
#include "uint64_sort.h"
#include "util.h"
#include "crypto_hash.h"
#include "decrypt.h"
#include "randombytes.h"

#include "arena.h"

// Prevents "cast increases required alignment" warnings by gcc
// When mat is allocated in such a way that rows are guaranteed to be aligned
#define CAST_PTR(TYPE,PTR) ((TYPE) ((void*) (PTR)))

static McElieceConfig* vec;
static void* matAddr;

#define SBYTE (SYS_N / 8)
#define SINT (SBYTE / 4)

/**
 * \brief Copies row stored inside accelerator to memory
 * \param row pointer to buffer to store row data
 */
void ReadRow(uint32_t* row){
    for (int i = 0; i < SINT; i++){
        row[i] = VersatUnitRead(matAddr,i);
    }
}

/**
 * \brief Stores row inside accelerator memory
 * \param row pointer to buffer to load row data into accelerator
 */
void VersatLoadRow(uint32_t* row){
    VersatMemoryCopy(matAddr,CAST_PTR(int*,row),SINT * sizeof(int));
}

/**
 * This function applies XOR operation from the row receive as input to the row stored inside the accelerator
 * \brief Performs first loop of guassian matrix processing with one row
 * \param row of the matrix to process
 * \param mask to apply during the operation
 * \param first true if first loop
 */
void VersatMcElieceLoop1(uint8_t *row, uint8_t mask,bool first){
    static uint8_t savedMask = 0;
    uint32_t *row_int = CAST_PTR(uint32_t*,row);

    ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) row_int);
    if(first){
        // Disable writing to memory since in the first run the accelerator is filled with garbage data
        vec->mat.in0_wr = 0;
    } else {
        // The following runs enable memory write and configures the mask with the saved mask value.
        // The reason we have to use the savedMask is because the accelerator is one "run" ahead of the software.
        // Since the accelerator needs to do a run to load data. That means that when the accelerator contains
        // data for loop 0, this function is being called with the mask for loop 1.
        // It is easier to store the mask and used it, it simplifies the outer code.

        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);
        vec->mask.constant = mask_int;
        vec->mat.in0_wr = 1;
    }

    // Ends the accelerator if still running
    EndAccelerator();

    // And starts it again with the configuration that we just finished writing
    StartAccelerator();

    savedMask = mask;

    // This function ends with the accelerator still running
}

/**
 * This function applies XOR operation from the row stored inside the accelerator to the row received as input. Can only be called if k != row
 * \brief Performs second loop of guassian matrix processing with one row
 * \param mat the entire matrix as an array of arrays
 * \param timesCalled how many times the function was called. Used to simplify the manner in which the accelerator is configured
 * \param k index of the row going to be processed next
 * \param row index of the row being processed
 * \param mask to apply during the operation
 */
void VersatMcElieceLoop2(unsigned char** mat,int timesCalled,int k,int row,uint8_t mask){
    static uint8_t savedMask = 0;

    // The accelerator contains VRead and VWrite units.
    // To simplify the configuration, is useful to divide the data based on their state
    // data to read (next loop), data to process (current loop), data to write (result from previous loop)
    // It is helpful to represent this in code by having a variable identify which row belongs to each state.

    int toRead =    k;
    int toCompute = ((toRead - 1    == row) ? toRead - 2    : toRead - 1);
    int toWrite =   ((toCompute - 1 == row) ? toCompute - 2 : toCompute - 1);

    // Basically each state is processed individually.

    // If toRead is inside the range
    if(toRead < PK_NROWS){
        // configure the VRead unit to read data
        int *toRead_int = CAST_PTR(int*,mat[toRead]);

        vec->mat.in0_wr = 0;

        ConfigureSimpleVReadShallow(&vec->row, SINT,toRead_int);        
    } else {
        // Otherwise disable it so we can save some cycles.
        vec->row.enableRead = 0;
    }

    // Same logic for toCompute
    if(timesCalled >= 1 && toCompute >= 0 && toCompute < PK_NROWS){
        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);

        vec->mask.constant = mask_int;
    } else {
        // Make sure that toWrite is disabled so that we do not write garbage data to memory in the first loop
        ConfigureSimpleVWrite(&vec->writer, SINT, (int*) NULL);
        vec->writer.enableWrite = 0;
    }
    
    // And for toWrite
    if(timesCalled >= 2 && toWrite >= 0){
        int* toWrite_int = CAST_PTR(int*,mat[toWrite]);
        ConfigureSimpleVWrite(&vec->writer, SINT,toWrite_int);
    } else {
        // Need to disable write otherwise we write garbage data to memory
        vec->writer.enableWrite = 0;
    }

    // Ends the accelerator if still running
    EndAccelerator();

    // And starts it again with the configuration that we just finished writing
    StartAccelerator();

    savedMask = mask;

    // This function ends with the accelerator still running
}

static crypto_uint64 uint64_is_equal_declassify(uint64_t t, uint64_t u) {
    crypto_uint64 mask = crypto_uint64_equal_mask(t, u);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

static crypto_uint64 uint64_is_zero_declassify(uint64_t t) {
    crypto_uint64 mask = crypto_uint64_zero_mask(t);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}


/**
 * This function was taken from PQClean. Only a small portion of this function was altered to implement acceleration and to make it work in an embedded system. 
 * \brief Versat implementation of the pk_gen function.
 */
int Versat_pk_gen(unsigned char *pk, unsigned char *sk, const uint32_t *perm, int16_t *pi) {
    int i, j, k;
    int row, c;

    unsigned char mask;
    unsigned char b;

    int mark = MarkArena(globalArena);

    // Init needed values for versat later on.  
    CryptoAlgosConfig* topConfig = (CryptoAlgosConfig*) accelConfig;
    vec = (McElieceConfig*) &topConfig->eliece;
    matAddr = (void*) TOP_eliece_mat_addr;

    // Both the VRead and the memories process the same amount of data everytime
    // Might as well configure this part upfront, since it never changes.
    ConfigureSimpleVReadBare(&vec->row);

    vec->mat.iterA = 1;
    vec->mat.incrA = 1;
    vec->mat.iterB = 1;
    vec->mat.incrB = 1;
    vec->mat.perA = SINT + 1;
    vec->mat.dutyA = SINT + 1;
    vec->mat.perB = SINT + 1;
    vec->mat.dutyB = SINT + 1;

    uint64_t buf[ 1 << GFBITS ];

    unsigned char** mat = PushArray(globalArena,PK_NROWS,unsigned char*);
    for(int i = 0; i < PK_NROWS; i++){
        mat[i] = PushArray(globalArena,SYS_N / 8,unsigned char); // This guarantees that each row is properly aligned to a 32 bit boundary.
    }

    gf* g = PushArray(globalArena,SYS_T + 1,gf);
    gf* L = PushArray(globalArena,SYS_N,gf); // support
    gf* inv = PushArray(globalArena,SYS_N,gf);

    g[ SYS_T ] = 1;

    for (i = 0; i < SYS_T; i++) {
        g[i] = load_gf(sk);
        sk += 2;
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        buf[i] = perm[i];
        buf[i] <<= 31;
        buf[i] |= i;
    }

    uint64_sort(buf, 1 << GFBITS);

    for (i = 1; i < (1 << GFBITS); i++) {
        if (uint64_is_equal_declassify(buf[i - 1] >> 31, buf[i] >> 31)) {
            PopArena(globalArena,mark);
            return -1;
        }
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        pi[i] = buf[i] & GFMASK;
    }
    for (i = 0; i < SYS_N;         i++) {
        L[i] = bitrev(pi[i]);
    }

    // filling the matrix

    root(inv, g, L);

    for (i = 0; i < SYS_N; i++) {
        inv[i] = gf_inv(inv[i]);
    }

    for (i = 0; i < PK_NROWS; i++) {
        for (j = 0; j < SYS_N / 8; j++) {
            mat[i][j] = 0;
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

                mat[ i * GFBITS + k ][ j / 8 ] = b;
            }
        }

        for (j = 0; j < SYS_N; j++) {
            inv[j] = gf_mul(inv[j], L[j]);
        }
    }

    // This is the portion of the code that is accelerator with Versat.
    // This part basically performs gaussian elimination with a big bit matrix.
    // Elimination is performed using the XOR operation
    for (i = 0; i < (PK_NROWS + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            row = i * 8 + j;

            if (row >= PK_NROWS) {
                break;
            }

            uint32_t *out_int = CAST_PTR(uint32_t*,mat[row]);
            EndAccelerator(); // Make sure accelerator is not running

            // Store row to be processed inside accelerator memory
            VersatLoadRow(out_int);
            bool first = true;
            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[ row ][ i ] ^ mat[ k ][ i ];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                VersatMcElieceLoop1(mat[k],mask,first); // Process all the following rows to change the value of the accelerator internal memory (which contains a copy of mat[row])

                // We could fetch this value from the accelerator Versat, but it's easier to calculate it since it is only one.
                mat[row][i] ^= mat[k][i] & mask;
                first = false;
            }

            // Last run, use valid data to compute last operation and flush valid data
            VersatMcElieceLoop1(mat[PK_NROWS - 1],0,false);

            EndAccelerator();

            if ( uint64_is_zero_declassify((mat[ row ][ i ] >> j) & 1) ) { // return if not systematic
               PopArena(globalArena,mark);
               return -1;
            }

            ReadRow(out_int); // Read value from memory. mat[row] is now good

            int index = 0;
            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[k][i] >> j;
                    mask &= 1;
                    mask = -mask;

                    VersatMcElieceLoop2(mat,index,k,row,mask); // Change the other rows based on the value of the accelerator internal memory (which contains a copy of mat[row])
                    index += 1;
                }
            }

            // Need to flush two times. One to flush the valid data stored inside the accelerator and the second
            // so that the VWrite unit writes the data processed in the last run to memory.
            VersatMcElieceLoop2(mat,index++,PK_NROWS,row,0);
            VersatMcElieceLoop2(mat,index++,PK_NROWS + 1,row,0);
            vec->writer.enableWrite = 0;
        }
    }

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, mat[i] + PK_NROWS / 8, PK_ROW_BYTES);
    }

    PopArena(globalArena,mark);
    return 0;
}

void VersatMcEliece
(
    unsigned char *pk,
    unsigned char *sk
) {
    int i;
    unsigned char seed[ 33 ] = {64};
    unsigned char *rp, *skp;

    int sizeofR = SYS_N / 8 + (1 << GFBITS)*sizeof(uint32_t) + SYS_T * 2 + 32;
    int sizeofF = SYS_T * sizeof(gf);
    int sizeofPerm = (1 << GFBITS) * sizeof(uint32_t);

    int mark = MarkArena(globalArena);

    unsigned char* r = PushAndZeroArray(globalArena,sizeofR,unsigned char);

    gf* f = PushAndZeroArray(globalArena,SYS_T,gf);
    gf* irr = PushAndZeroArray(globalArena,SYS_T,gf);
    uint32_t* perm = PushAndZeroArray(globalArena,(1 << GFBITS),uint32_t);
    int16_t* pi = PushAndZeroArray(globalArena,(1 << GFBITS),int16_t);

    randombytes(seed + 1, 32);

    while (1) {
        rp = &r[ sizeofR - 32 ];
        skp = sk;

        shake(r, sizeofR, seed, 33);
        memcpy(skp, seed + 1, 32);
        skp += 32 + 8;
        memcpy(seed + 1, &r[ sizeofR - 32 ], 32);

        rp -= sizeofF;

        for (i = 0; i < SYS_T; i++) {
            f[i] = load_gf(rp + i * 2);
        }

        if (genpoly_gen(irr, f)) {
            continue;
        }

        for (i = 0; i < SYS_T; i++) {
            store_gf(skp + i * 2, irr[i]);
        }

        skp += IRR_BYTES;

        rp -= sizeofPerm;

        for (i = 0; i < (1 << GFBITS); i++) {
            perm[i] = load4(rp + i * 4);
        }

        // Everything else is mostly the same, with some code changes required to run this in an embedded system
        // The only change is this function here that implements a loop accelerated by Versat
        if (Versat_pk_gen(pk, skp - IRR_BYTES, perm, pi)) {
            continue;
        }

        controlbitsfrompermutation(skp, pi, GFBITS, 1 << GFBITS);
        skp += COND_BYTES;

        rp -= SYS_N / 8;
        memcpy(skp, rp, SYS_N / 8);

        store8(sk + 32, 0xFFFFFFFF);

        break;
    }

    PopArena(globalArena,mark);
}
