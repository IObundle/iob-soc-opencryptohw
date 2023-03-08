#include <stdio.h>
#include <stdlib.h>

#define MCELIECE_SEEDBYTES (48)
#define MCELIECE348864_PUBLICKEYBYTES (261120)
// #define MCELIECE348864_SECRETKEYBYTES (6452)
#define MCELIECE348864_SECRETKEYBYTES (6492)

void print_hex_str(int len, char* str, char *logstr){
    int i = 0;
    int byte_val = 0;
    char hex_table[16] = "0123456789ABCDEF";

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
    char hex_table[16] = "0123456789ABCDEF";

    printf("MD = ");
    for( i = 0; i < md_len; i++){
        byte_val = (int) ((unsigned char) md[i]);
        printf("%c%c", hex_table[byte_val >> 4], hex_table[byte_val & 0xF]);
    }
    printf("\n\n");

    return;
}

int read_d_in_file(char *fname){
    int i = 0;
    int num_seeds = -1;
    int len = MCELIECE_SEEDBYTES;
    char *seed = NULL;
    FILE *fp_din = NULL;

    fp_din = fopen(fname, "rb");
    if(fp_din == NULL) {
        printf("Failed to open %s file\n", fname);
        return -1;
    }

    // allocate memory for public keys
    seed = (char *) malloc(len*sizeof(char));
    if(seed == NULL){
        printf("Failed to allocate memory for seed\n");
        fclose(fp_din);
        return -1;
    }

    // Number of seeds
    fread(&num_seeds, sizeof(int), 1, fp_din);
    printf("Number of public keys: %d\n", num_seeds);

    // Read all seeds
    for(i = 0; i< num_seeds; i++){
        fread(seed, sizeof(char), len, fp_din);

        printf("count = %d\n", i);
        // Print seed in hex format
        print_hex_str(len, seed, "seed");

    }

    // free seed memory space
    free(seed);
    //close file
    fclose(fp_din);

    return 0;
}

int read_d_out_file(char *fname){
    char *pk = NULL;
    char *sk = NULL;
    FILE *fp_dout = NULL;

    fp_dout = fopen(fname, "rb");
    if(fp_dout == NULL) {
        printf("Failed to open %s file\n", fname);
        return -1;
    }

    // allocate memory for public key
    pk = (char *) malloc(MCELIECE348864_PUBLICKEYBYTES*sizeof(char));
    if(pk == NULL){
        printf("Failed to allocate memory for public key\n");
        fclose(fp_dout);
        return -1;
    }

    // allocate memory for secret key
    sk = (char *) malloc(MCELIECE348864_SECRETKEYBYTES*sizeof(char));
    if(sk == NULL){
        printf("Failed to allocate memory for secret key\n");
        free(pk);
        fclose(fp_dout);
        return -1;
    }

    // Read blk_len chuncks until end of file
    while(feof(fp_dout) == 0){
        // read public key
        fread(pk, sizeof(char), MCELIECE348864_PUBLICKEYBYTES, fp_dout);
        // read secret key
        fread(sk, sizeof(char), MCELIECE348864_SECRETKEYBYTES, fp_dout);
        // Print keys in hex format
        print_hex_str(MCELIECE348864_PUBLICKEYBYTES, pk, "pk");
        print_hex_str(MCELIECE348864_SECRETKEYBYTES, sk, "sk");
    }

    // free memory space
    free(pk);
    free(sk);
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
        if(read_d_in_file(argv[1]) != 0)
            return -1;
    }

    if(argc > 2){
        if(read_d_out_file(argv[2]) != 0)
            return -1;
    }

    return 0;
}
