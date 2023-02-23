#include "fullAESTests.hpp"

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

/* get pointer to plaintext/key pair and increment pointer */
/* get pointer to plaintext
 * get pointer to key
 * return size of plaintext + key */
int get_ptext_key_pair(uint8_t *ptr, uint8_t **ptext_ptr, uint8_t **key_ptr){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_ptext_key_pair: invalid pointer\n");
        return -1;
    }
    /* update plaintext pointer */
    *ptext_ptr = ptr;
    /* update key pointer */
    *key_ptr = ptr + AES_BLK_SIZE;

    return (sizeof(uint8_t)*(AES_BLK_SIZE+AES_KEY_SIZE));
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

void Full_AES_Test(Versat* versat) {
    printf("Init Full AES Test\n");
    uint8_t *plaintext = NULL;
    uint8_t *key = NULL;
    uint8_t ciphertext[AES_BLK_SIZE] = {0};
    unsigned int num_msgs = 0;

// Pointer to DDR_MEM
#ifdef PC
    char ddr_mem[100000] = {0};
#else
#ifndef RUN_EXTMEM
    char *ddr_mem = (char*) (EXTRA_BASE);
#else
    char *ddr_mem = (char*) ((1<<(FIRM_ADDR_W)));
#endif
#endif
    // Init ethernet
    eth_init(ETHERNET_BASE);

    // instantiate Versat
    FUDeclaration* type = GetTypeByName(versat,MakeSizedString("ReadWriteAES"));
    Accelerator* accel = CreateAccelerator(versat);
    FUInstance* inst = CreateFUInstance(accel,type,MakeSizedString("Test"));

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
    din_ptr += get_int(din_fp + din_ptr, &num_msgs);
    dout_size = num_msgs*(AES_BLK_SIZE);
    // Output file starts after input file
    dout_fp = din_fp + din_size;

    // Initialize VersatAES
    Versat_init_AES(accel);

    // Message test loop
    int i=0;
    uint8_t *msg = NULL;
    for(i=0; i< (int) num_msgs; i++){
        // Parse plaintext and key pairs
        din_ptr += get_ptext_key_pair(&(din_fp[din_ptr]), &plaintext, &key);
        printf("\ttest vector #%d/%d...", i+1, num_msgs);

        VersatAES(versat, accel, ciphertext, plaintext, key);
        printf("done!\n");

        // Save to memory
        dout_ptr += save_msg(&(dout_fp[dout_ptr]), ciphertext, AES_BLK_SIZE);
    }

#ifdef SIM
    // Send message digests via uart
    char output_file_name[] = "soc-out.bin";
    uart_sendfile(output_file_name, dout_size, (char *) dout_fp);
#else
    // Send message digests via ethernet
    eth_send_variable_file((char *) dout_fp, dout_size);
#endif
    
    return;
}
