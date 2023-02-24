#include <stdio.h>
#include <stdlib.h>


void print_hex_msg(int len, char* msg, int msg_len){
    int i = 0;
    int byte_val = 0;
    char hex_table[16] = "0123456789abcdef";

    printf("Len = %d\n", len*8);
    printf("Msg = ");
    for( i = 0; i < msg_len; i++){
        byte_val = (int) ((unsigned char) msg[i]);
        printf("%c%c", hex_table[byte_val >> 4], hex_table[byte_val & 0xF]);
    }
    printf("\nMD = \n\n");

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

int read_d_in_file(char *fname){
    int i = 0;
    int num_msgs = -1;
    int len = 0, msg_len = 0;
    char *msg = NULL;
    FILE *fp_din = NULL;

    fp_din = fopen(fname, "rb");
    if(fp_din == NULL) {
        printf("Failed to open %s file\n", fname);
        return -1;
    }

    // Number of messages
    fread(&num_msgs, sizeof(int), 1, fp_din);
    printf("Number of messages: %d\n", num_msgs);

    // Read all messages
    for(i = 0; i< num_msgs; i++){
        // message length
        fread(&len, sizeof(int), 1, fp_din);
        // actual message length
        if(len == 0)
            msg_len = 1; // special case for 0
        else
            msg_len = len;

        // allocate memory for message
        msg = (char *) malloc(msg_len*sizeof(char));
        if(msg == NULL){
            printf("Failed to allocate memory for message #%d\n", i);
            fclose(fp_din);
            return -1;
        }
        fread(msg, sizeof(char), msg_len, fp_din);

        // Print length and message in hex format
        print_hex_msg(len, msg, msg_len);

        // free message memory space
        free(msg);
    }

    //close file
    fclose(fp_din);

    return 0;
}

int read_d_out_file(char *fname){
    int i = 0;
    int md_len = 32; // message digests always 32 bytes
    char *md = NULL;
    FILE *fp_dout = NULL;

    fp_dout = fopen(fname, "rb");
    if(fp_dout == NULL) {
        printf("Failed to open %s file\n", fname);
        return -1;
    }

    // allocate memory for message
    md = (char *) malloc(md_len*sizeof(char));
    if(md == NULL){
        printf("Failed to allocate memory for message #%d\n", i);
        fclose(fp_dout);
        return -1;
    }

    // Read md_len chuncks until end of file
    while(feof(fp_dout) == 0){

        fread(md, sizeof(char), md_len, fp_dout);

        // Print length and message in hex format
        print_hex_md(md, md_len);

    }

    // free message digest memory space
    free(md);
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
