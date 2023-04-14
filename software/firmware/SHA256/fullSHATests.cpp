#include "fullSHATests.hpp"

/* read integer value
 * return number of bytes read */
int get_int(char* ptr, unsigned int *i_val){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_int: invalid pointer\n");
        return -1;
    }
    /* read 1 byte at a time
     * write to int */
    *i_val = (unsigned char) ptr[3];
    *i_val <<= 8;
    *i_val += (unsigned char) ptr[2];
    *i_val <<= 8;
    *i_val += (unsigned char) ptr[1];
    *i_val <<= 8;
    *i_val += (unsigned char) ptr[0];
    return sizeof(int);
}

/* get pointer to message and increment pointer */
/* get pointer to message
 * return size of message + padding */
int get_msg(char *ptr, uint8_t **msg_ptr, int size){
    /* check for valid ptr */
    if(ptr == NULL){
        printf("get_msg: invalid pointer\n");
        return -1;
    }
    /* update message pointer */
    *msg_ptr = (uint8_t*) ptr;
    /* calculate padding to next multiple of 4 */
    int padding = (4-(size%4))%4;

    return (sizeof(uint8_t)*size) + padding;
}

/* copy memory from pointer to another */
void mem_copy(uint8_t *src_buf, char *dst_buf, int size){
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
int save_msg(char *ptr, uint8_t* msg, int size){
    if(ptr == NULL || msg == NULL || size < 0 ){
        printf("save_msg: invalid inputs\n");
        return -1;
    }

    // copy message to pointer
    mem_copy(msg, ptr, size); 
    return sizeof(uint8_t)*size;
}
void Full_SHA_Test(Versat* versat) {
    printf("Init Full SHA Test\n");
    uint8_t digest[256] = {0};
    unsigned int num_msgs = 0;
    unsigned int msg_len = 0;

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
    FUDeclaration* type = GetTypeByName(versat,STRING("SHA"));
    Accelerator* accel = CreateAccelerator(versat);
    FUInstance* inst = CreateFUInstance(accel,type,STRING("Test"));
    SetSHAAccelerator(accel,inst);
    InitVersatSHA(versat,true);

    // Receive Input Data
    int din_ptr = 0, din_size = 0;
    char *din_fp = (char*) (ddr_mem + VERSAT_SHA_W_PTR_NBYTES);
    int dout_ptr = 0, dout_size = 0;
    char *dout_fp;

#ifdef SIM
    // Receive input data from uart 
    char input_file_name[] = "soc-in.bin";
    din_size = uart_recvfile(input_file_name, din_fp);    
#else
    // Receive input data from ethernet
    din_size = eth_rcv_variable_file(din_fp);
#endif
    printf("Received file with %d bytes\n", din_size);

    // Calculate output size and allocate output memory
    din_ptr += get_int(din_fp + din_ptr, &num_msgs);
    dout_size = num_msgs*HASH_SIZE;
    // Output file starts after input file
    dout_fp = din_fp + din_size;

    // Message test loop
    int i=0;
    uint8_t *msg = NULL;
    for(i=0; i< (int) num_msgs; i++){
        // Parse messages and length
        din_ptr += get_int(din_fp + din_ptr, &msg_len);
        din_ptr += get_msg(&(din_fp[din_ptr]), &msg, ((msg_len) ? msg_len : 1) );
        printf("\ttest vector #%d/%d...", i+1, num_msgs);

        VersatSHA(digest, msg, msg_len);
        printf("done!\n");

        // Save to memory
        dout_ptr += save_msg(&(dout_fp[dout_ptr]), digest, HASH_SIZE);
    }

// #ifdef SIM
    // Send message digests via uart
    char output_file_name[] = "soc-out.bin";
    uart_sendfile(output_file_name, dout_size, dout_fp);
// #else
//     // Send message digests via ethernet
//     eth_send_variable_file(dout_fp, dout_size);
// #endif
    
    return;
}
