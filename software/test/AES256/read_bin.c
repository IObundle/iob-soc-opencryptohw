#include <stdio.h>
#include <stdlib.h>

#define AESBLKSIZE (128/8)
#define KEYSIZE (128/8)

void print_hex_str(int len, char* str, char *logstr){
    int i = 0;
    int byte_val = 0;
    char hex_table[16] = "0123456789abcdef";

    printf("%s = ", logstr);
    for( i = 0; i < len; i++){
        byte_val = (int) ((unsigned char) str[i]);
        printf("%c%c", hex_table[byte_val >> 4], hex_table[byte_val & 0xF]);
    }
    printf("\n");

    return;
}

void print_hex_md(char* md, int md_len){
    int i = 0;
    int byte_val = 0;
    char hex_table[16] = "0123456789abcdef";

    printf("MD = ");
    for( i = 0; i < md_len; i++){
        byte_val = (int) ((unsigned char) md[i]);
        printf("%c%c", hex_table[byte_val >> 4], hex_table[byte_val & 0xF]);
    }
    printf("\n\n");

    return;
}

int read_d_in_file(char *fname, int keysize){
    int i = 0;
    int num_txts = -1;
    int len = AESBLKSIZE, msg_len = 0;
    char *ptext = NULL;
    char *key = NULL;
    FILE *fp_din = NULL;

    fp_din = fopen(fname, "rb");
    if(fp_din == NULL) {
        printf("Failed to open %s file\n", fname);
        return -1;
    }

    // allocate memory for plaintexts
    ptext = (char *) malloc(len*sizeof(char));
    if(ptext == NULL){
        printf("Failed to allocate memory for plaintext\n");
        fclose(fp_din);
        return -1;
    }
    // allocate memory for keys
    key = (char *) malloc(keysize*sizeof(char));
    if(key == NULL){
        printf("Failed to allocate memory for key\n");
        free(ptext);
        fclose(fp_din);
        return -1;
    }

    // Number of plaintexts
    fread(&num_txts, sizeof(int), 1, fp_din);
    printf("Number of plaintexts/key pairs: %d\n", num_txts);

    // Read all plaintexts/key pairs
    for(i = 0; i< num_txts; i++){
        fread(ptext, sizeof(char), len, fp_din);

        fread(key, sizeof(char), keysize, fp_din);
        
        printf("COUNT = %d\n", i);
        // Print length and message in hex format
        print_hex_str(keysize, key, "KEY");
        // Print plaintext in hex format
        print_hex_str(len, ptext, "PLAINTEXT");
        printf("CIPHERTEXT = \n\n");

    }

    // free plaintext memory space
    free(ptext);
    // free key memory space
    free(key);
    //close file
    fclose(fp_din);

    return 0;
}

int read_d_out_file(char *fname){
    int i = 0;
    int blk_len = AESBLKSIZE; // AES blocks always 128 bits
    char *ciphertext = NULL;
    FILE *fp_dout = NULL;

    fp_dout = fopen(fname, "rb");
    if(fp_dout == NULL) {
        printf("Failed to open %s file\n", fname);
        return -1;
    }

    // allocate memory for ciphertext
    ciphertext = (char *) malloc(blk_len*sizeof(char));
    if(ciphertext == NULL){
        printf("Failed to allocate memory for ciphertext\n");
        fclose(fp_dout);
        return -1;
    }

    // Read blk_len chuncks until end of file
    while(feof(fp_dout) == 0){

        fread(ciphertext, sizeof(char), blk_len, fp_dout);

        // Print length and message in hex format
        print_hex_str(blk_len, ciphertext, "CIPHERTEXT");

    }

    // free message digest memory space
    free(ciphertext);
    //close file
    fclose(fp_dout);

    return 0;
}
int main(int argc, char *argv[]){
    printf("Read Bin Program\n");

    int i = 0;

    if(argc < 2){
        printf("Usage: ./read_bin [d_in.bin] [d_out.bin]\n");
        return -1;
    }

    for(i = 1; i < argc; i++){
        printf("arg[%d]: %s\n", i, argv[i]);
    }

    if(argc > 1){
        if(read_d_in_file(argv[1], KEYSIZE) != 0)
            return -1;
    }

    if(argc > 2){
        if(read_d_out_file(argv[2]) != 0)
            return -1;
    }

    return 0;
}
