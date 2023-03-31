#include "fullMCELIECETests.hpp"

/* read integer value
 * return number of bytes read */
int get_int(uint8_t* ptr, unsigned int *i_val){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_int: invalid pointer\n");
        return -1;
    }
    /* read 1 byte at a time
     * write to int */
    *i_val = ptr[3];
    *i_val <<= 8;
    *i_val += ptr[2];
    *i_val <<= 8;
    *i_val += ptr[1];
    *i_val <<= 8;
    *i_val += ptr[0];
    return sizeof(int);
}

/* get pointer to seed and increment pointer */
/* get pointer to seed
 * return size of seed */
int get_seed(uint8_t *ptr, uint8_t **seed_ptr){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_seed: invalid pointer\n");
        return -1;
    }
    /* update seed pointer */
    *seed_ptr = ptr;

    return (sizeof(uint8_t)*(SEED_BYTES));
}

/* copy memory from pointer to another */
void mem_copy(uint8_t *src_buf, uint8_t *dst_buf, int size){
    if(src_buf == NULL || dst_buf == NULL){
        printf("mem_copy: invalid pointer\n");
        return;
    }
    int i = 0;
    while(i<size){
        dst_buf[i] = src_buf[i];
        i++;
    }
    return;
}

/* copy mesage to pointer, update pointer to after message
 * returns number of chars written
 */
int save_msg(uint8_t *ptr, uint8_t* msg, int size){
    if(ptr == NULL || msg == NULL || size < 0 ){
        printf("save_msg: invalid inputs\n");
        return -1;
    }

    // copy message to pointer
    mem_copy(msg, ptr, size); 
    return sizeof(uint8_t)*size;
}

void Full_McEliece_Test(Versat* versat) {
    printf("Init Full McEliece Test\n");
    uint8_t *seed = NULL;
    MemPool_Create(MEMORY_POOL_SIZE, MAX_NUM_SEEDS*SEED_BYTES);
    uint8_t *public_key = (uint8_t*) MemPool_Alloc( \
            PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES*sizeof(uint8_t));
    uint8_t *secret_key = (uint8_t*) MemPool_Alloc( \
            PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES*sizeof(uint8_t));
    unsigned int num_seeds = 0;

// Pointer to DDR_MEM
#ifdef PC
    char ddr_mem[3000000] = {0};
#else
#ifndef RUN_EXTMEM
    char *ddr_mem = (char*) (EXTRA_BASE);
#else
    char *ddr_mem = (char*) (1 << (FIRM_ADDR_W));
#endif
#endif
    // Init ethernet
    eth_init(ETHERNET_BASE);

    // instantiate Versat
    VersatInit(versat);

    // Receive Input Data
    int din_ptr = 0, din_size = 0;
    uint8_t *din_fp = (uint8_t*) ddr_mem;
    int dout_ptr = 0, dout_size = 0;
    uint8_t *dout_fp;

#ifdef SIM
    // Receive input data from uart 
    char input_file_name[] = "soc-in.bin";
    din_size = uart_recvfile(input_file_name, (char *) din_fp);    
#else
    // Receive input data from ethernet
    din_size = eth_rcv_variable_file((char *) din_fp);
#endif
    printf("Received file with %d bytes\n", din_size);

    // Calculate output size and allocate output memory
    din_ptr += get_int(din_fp + din_ptr, &num_seeds);
    dout_size = num_seeds*(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES+ \
            PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES);
    // Output file starts after input file
    // dout_fp = din_fp + din_size;
    dout_fp = (uint8_t*) MemPool_Alloc(dout_size);

    // Message test loop
    int i=0;
    for(i=0; i< (int) num_seeds; i++){
        // Parse seeds
        din_ptr += get_seed(&(din_fp[din_ptr]), &seed);
        printf("\ttest vector #%d/%d...", i+1, num_seeds);

        // TODO
        // Set Seed
        nist_kat_init(seed, NULL, 256);
        
        // McEliece Key Pair
        PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_keypair(public_key, secret_key);

        printf("done!\n");

        // Save public key to memory
        dout_ptr += save_msg(&(dout_fp[dout_ptr]), public_key, \
                PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES);
        // Save secret key to memory
        dout_ptr += save_msg(&(dout_fp[dout_ptr]), secret_key, \
                PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES);
    }

#ifdef SIM
    // Send message digests via uart
    char output_file_name[] = "soc-out.bin";
    uart_sendfile(output_file_name, dout_size, (char *) dout_fp);
#else
    // Send message digests via ethernet
    eth_send_variable_file((char *) dout_fp, dout_size);
#endif
    
    MemPool_Alloc(dout_size);
    MemPool_Alloc(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES*sizeof(uint8_t));
    MemPool_Alloc(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES*sizeof(uint8_t));
    MemPool_Report("program end");
    MemPool_Destroy();
    return;
}
