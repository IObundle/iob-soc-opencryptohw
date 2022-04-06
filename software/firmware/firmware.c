#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"
#include "string.h"

#include "versat.h"
#include "iob_timer.h"

#include "crypto/sha2.h"
#include "crypto/aes.h"

#include "../test_vectors.h"
#include "unitWrapper.h"
#include "unitVerilogWrappers.h"

#include "utils.h"

#define HASH_SIZE (256/8)

void versat_sha256(uint8_t *out, const uint8_t *in, size_t inlen);

static void store_bigendian_32(uint8_t *x, uint64_t u) {
    x[3] = (uint8_t) u;
    u >>= 8;
    x[2] = (uint8_t) u;
    u >>= 8;
    x[1] = (uint8_t) u;
    u >>= 8;
    x[0] = (uint8_t) u;
}

static uint32_t initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint32_t kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint32_t kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint32_t kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint32_t kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint32_t* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

#ifdef PC
    static char mem[1024*1024]; // 1 Mb
    #define DDR_MEM mem
#else
#if (RUN_DDR_SW==0)
    #define DDR_MEM (EXTRA_BASE)
#else
    #define DDR_MEM ((1<<(FIRM_ADDR_W)))
#endif
#endif

char GetHexadecimalChar(int value){
    if(value < 10){
        return '0' + value;
    } else{
        return 'a' + (value - 10);
    }
}

char* GetHexadecimal(const char* text, int str_size){
    static char buffer[2048+1];
    int i;

    for(i = 0; i< str_size; i++){
        if(i * 2 > 2048){
            printf("\n\n<GetHexadecimal> Maximum size reached\n\n");
            buffer[i*2] = '\0';
            return buffer;
        }

        int ch = (int) ((unsigned char) text[i]);

        buffer[i*2] = GetHexadecimalChar(ch / 16);
        buffer[i*2+1] = GetHexadecimalChar(ch % 16);
    }

    buffer[(i)*2] = '\0';

    return buffer;
}

static int unitFBits[] = {8};
static int readMemory_[64];
static int* readMemory;
static int writeMemory_[64];
static int* writeMemory;

// GLOBALS
Versat* versat;
Accelerator* accel;
FUInstance* wMem;
FUInstance* read;
FUInstance* stateReg[8];
bool initVersat = false;

// Versat specific units
FUDeclaration* ADD;
FUDeclaration* REG;
FUDeclaration* VREAD;
FUDeclaration* VWRITE;
FUDeclaration* MEM;
FUDeclaration* DEBUG;

FUDeclaration* M_32;

void ClearCache(){
    int count = 0;

    for(int i = 0; i < 1024; i += 16){
        count += writeMemory[i];
    }

    printf("Clear cache: %d [ignore]\n\n",count);
}

#if 0
FUInstance* UnitM(Accelerator* accel,FUInstance* in){
    FUInstance* regs[16];

    for(int i = 0; i < 16; i++){
        regs[i] = CreateFUInstance(accel,REG);
    }
    for(int i = 0; i < 15; i++){
        ConnectUnits(regs[i+1],0,regs[i],0);
    }

    FUInstance* val = M_32(accel,regs[0],regs[14],regs[9],regs[1]);

    FUInstance* mux = CreateFUInstance(accel,DELAY_MUX);

    ConnectUnits(in,0,mux,0);
    ConnectUnits(val,0,mux,1);

    ConnectUnits(mux,0,regs[15],0);

    return regs[0];
}
#endif

void ParseVersatSpecification(Versat* versat,FILE* file);

void RegisterOperators(Versat* versat){
    char* unary[] = {"NOT"};
    char* binary[] = {"XOR","ADD","AND","OR","RHR","SHR","RHL","SHL"};

    FUDeclaration decl = {};
    decl.nOutputs = 1;

    decl.nInputs = 1;
    for(int i = 0; i < ARRAY_SIZE(unary); i++){
        FixedStringCpy(decl.name.str,unary[i],MAX_NAME_SIZE);
        RegisterFU(versat,decl);
    }

    decl.nInputs = 2;
    for(int i = 0; i < ARRAY_SIZE(binary); i++){
        FixedStringCpy(decl.name.str,binary[i],MAX_NAME_SIZE);
        RegisterFU(versat,decl);
    }
}

int main(int argc,const char* argv[])
{
    //init uart
    uart_init(UART_BASE,FREQ/BAUD);
    timer_init(TIMER_BASE);

    // Force alignment on a 64 byte boundary
    readMemory = readMemory_;
    writeMemory = writeMemory_;
    while((((int)readMemory) & (16 * sizeof(int) - 1)) != 0)
        readMemory += 1;

    while((((int)writeMemory) & (16 * sizeof(int) - 1)) != 0)
        writeMemory += 1;

    MAKE_VERSAT(versat);

    InitVersat(versat,VERSAT_BASE,1);

    // Versat specific units
    ADD = RegisterAdd(versat);
    REG = RegisterReg(versat);
    VREAD = RegisterVRead(versat);
    VWRITE = RegisterVWrite(versat);
    MEM = RegisterMem(versat,10);
    DEBUG = RegisterDebug(versat);
    RegisterDelay(versat);
    RegisterCircuitInput(versat);
    RegisterCircuitOutput(versat);

    RegisterOperators(versat);

    // Sha specific units
    FUDeclaration* UNIT_F = RegisterUnitF(versat);
    FUDeclaration* UNIT_M = RegisterUnitM(versat);

    FILE* file = fopen("testVersatSpecification.txt","r");

    return 0;

    ParseVersatSpecification(versat,file);

    #if 1
    {
    //FUDeclaration* type = GetTypeByName(versat,"M");
    FUDeclaration* type = GetTypeByName(versat,"TestSpec");
    //FUDeclaration* type = GetTypeByName(versat,"TestDelay");
    Accelerator* accel = CreateAccelerator(versat);
    FUInstance* inst = CreateFUInstance(accel,type);

    for(int i = 0; i < inst->declaration->nInputs; i++){
        FUInstance* mem = CreateFUInstance(accel,REG);
        ConnectUnits(mem,0,inst,i);
    }
    for(int i = 0; i < inst->declaration->nOutputs; i++){
        FUInstance* mem = CreateFUInstance(accel,REG);
        ConnectUnits(inst,i,mem,0);
    }

    {
    FILE* dotFile = fopen("versat.dot","w");
    OutputGraphDotFile(accel,dotFile,0);
    fclose(dotFile);
    }

    Accelerator* flatten = Flatten(versat,accel,99);

    {
    FILE* dotFile = fopen("flatten.dot","w");
    OutputGraphDotFile(flatten,dotFile,0);
    fclose(dotFile);
    }

    CalculateDelay(versat,flatten);

    {
    FILE* dotFile = fopen("delayed.dot","w");
    OutputGraphDotFile(flatten,dotFile,0);
    fclose(dotFile);
    }

    volatile int* r1 = (volatile int*) GetInstanceByName(accel,"r1");

    return 0;
    }
    #endif

    #if 0
    read = CreateFUInstance(accel,VREAD);
    {
        volatile VReadConfig* c = (volatile VReadConfig*) read->config;

        // Versat side
        c->iterB = 1;
        c->incrB = 1;
        c->perB = 16;
        c->dutyB = 16;

        // Memory side
        c->incrA = 1;
        c->iterA = 1;
        c->perA = 16;
        c->dutyA = 16;
        c->size = 8;
        c->int_addr = 0;
        c->pingPong = 1;
        c->ext_addr = (int) readMemory; // Some place so no segfault if left unconfigured
    }

    FUInstance* unitF[4];
    for(int i = 0; i < 4; i++){
        unitF[i] = CreateFUInstance(accel,UNIT_F);

        if(i > 0){
            for(int ii = 0; ii < 8; ii++){
                ConnectUnits(unitF[i-1],ii,unitF[i],ii);
            }
        }
    }

    FUInstance* kMem[4]; // Could be done by using 1 memory, change later
    for(int i = 0; i < 4; i++){
        kMem[i] = CreateFUInstance(accel,MEM);
        volatile MemConfig* c = (volatile MemConfig*) kMem[i]->config;

        c->iterA = 1;
        c->incrA = 1;
        c->perA = 16;
        c->dutyA = 16;

        for (int ii = 0; ii < 16; ii++)
        {
            VersatUnitWrite(kMem[i],ii,kConstants[i][ii]);
        }
    }

    for(int i = 0; i < 8; i++){
        stateReg[i] = CreateFUInstance(accel,REG);
        VersatUnitWrite(stateReg[i],0,initialStateValues[i]);
        ConnectUnits(stateReg[i],0,unitF[0],i);
    }

    for(int i = 0; i < 8; i++){
        FUInstance* add = CreateFUInstance(accel,ADD);

        ConnectUnits(stateReg[i],0,add,0);
        ConnectUnits(unitF[3],i,add,1);
        ConnectUnits(add,0,stateReg[i],0);
    }

    ConnectUnits(read,0,unitF[0],8);

    FUInstance* unitM[3];
    for(int i = 0; i < 3; i++){
        unitM[i] = CreateFUInstance(accel,UNIT_M);

        ConnectUnits(unitM[i],0,unitF[i+1],8);
        if(i != 0){
            ConnectUnits(unitM[i-1],0,unitM[i],0);
        }
    }
    ConnectUnits(read,0,unitM[0],0);

    for(int i = 0; i < 4; i++){
        ConnectUnits(kMem[i],0,unitF[i],9);
    }
    #endif

    accel = Flatten(versat,accel,1);

    //CalculateDelay(versat,accel);

    {
    FILE* dotFile = fopen("versat.dot","w");
    OutputGraphDotFile(accel,dotFile,1);
    fclose(dotFile);
    }

    return 0;

    #if 0
    CalculateDelay(accel);

    {
    FILE* dotFile = fopen("versat.dot","w");
    OutputGraphDotFile(accel,dotFile);
    fclose(dotFile);
    }

    for(int i = 0; i < accel->nInstances; i++){
        FUInstance* inst = &accel->instances[i];

        printf("%s %d %d\n",inst->entityName,inst->delays[0],inst->delays[1]);
    }

    //OutputVersatSource(versat,"versat_defs.vh","versat_instance.v","versat_constants.c");
    #endif

    #if 0
    SaveConfiguration(accel,0);
    OutputGraphDotFile(accel,stdout);

    // Gera o versat.
    OutputVersatSource(versat,"versat_defs.vh","versat_instance.v","versat_constants.c");
    #endif

    char digest[256];

    for(int i = 0; i < 256; i++){
        digest[i] = 0;
    }

#ifdef AUTOMATIC_TEST
    int i = 0;

    printf("[L = %d]\n", HASH_SIZE);

    //Message test loop
    for(i=0; i< NUM_MSGS; i++){
        versat_sha256(digest,msg_array[i],msg_len[i]);
        printf("\nLen = %d\n", msg_len[i]*8);
        printf("Msg = %s\n", GetHexadecimal(msg_array[i], (msg_len[i]) ? msg_len[i] : 1));
        printf("MD = %s\n",GetHexadecimal(digest, HASH_SIZE));
    }
    printf("\n");
#else

    #if 1
    printf("Expected: 42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa\n");
    versat_sha256(digest,msg_64,64);
    printf("Result:   %s\n",GetHexadecimal(digest, HASH_SIZE));
    #endif

    #if 0
    char key[16];
    memset(key,0,16);

    struct AES_ctx ctx;
    AES_init_ctx(&ctx,key);

    char msg[16] = {0xf3,0x44,0x81,0xec,0x3c,0xc6,0x27,0xba,0xcd,0x5d,0xc3,0xfb,0x08,0xf2,0x73,0xe6};

    AES_ECB_encrypt(&ctx,msg);

    printf("%s\n",GetHexadecimal(msg,16));
    printf("0336763e966d92595a567cc9ce537f5e\n");
    #endif

#endif

    uart_finish();

    return 0;
}

static uint32_t load_bigendian_32(const uint8_t *x) {
     return (uint32_t)(x[3]) | (((uint32_t)(x[2])) << 8) |
                (((uint32_t)(x[1])) << 16) | (((uint32_t)(x[0])) << 24);
}

static size_t versat_crypto_hashblocks_sha256(const uint8_t *in, size_t inlen) {
    uint32_t w[16];

    {
        volatile VReadConfig* c = (volatile VReadConfig*) read->config;
        c->ext_addr = (int) w;
    }

    while (inlen >= 64) {
        w[0]  = load_bigendian_32(in + 0);
        w[1]  = load_bigendian_32(in + 4);
        w[2]  = load_bigendian_32(in + 8);
        w[3]  = load_bigendian_32(in + 12);
        w[4]  = load_bigendian_32(in + 16);
        w[5]  = load_bigendian_32(in + 20);
        w[6]  = load_bigendian_32(in + 24);
        w[7]  = load_bigendian_32(in + 28);
        w[8]  = load_bigendian_32(in + 32);
        w[9]  = load_bigendian_32(in + 36);
        w[10] = load_bigendian_32(in + 40);
        w[11] = load_bigendian_32(in + 44);
        w[12] = load_bigendian_32(in + 48);
        w[13] = load_bigendian_32(in + 52);
        w[14] = load_bigendian_32(in + 56);
        w[15] = load_bigendian_32(in + 60);

        // Loads data + performs work
        AcceleratorRun(accel);

        // Since vread currently reads before outputing, this piece of code is set before aceleratorRun
        // Eventually it will need to be moved to after
        #if 1
        if(!initVersat){
            for(int i = 0; i < 8; i++){
                VersatUnitWrite(stateReg[i],0,initialStateValues[i]);
            }
            initVersat = true;
        }
        #endif

        in += 64;
        inlen -= 64;
    }

    return inlen;
}

void versat_sha256(uint8_t *out, const uint8_t *in, size_t inlen) {
    uint8_t padded[128];
    uint64_t bytes = 0 + inlen;

    versat_crypto_hashblocks_sha256(in, inlen);
    in += inlen;
    inlen &= 63;
    in -= inlen;

    for (size_t i = 0; i < inlen; ++i) {
        padded[i] = in[i];
    }
    padded[inlen] = 0x80;

    if (inlen < 56) {
        for (size_t i = inlen + 1; i < 56; ++i) {
            padded[i] = 0;
        }
        padded[56] = (uint8_t) (bytes >> 53);
        padded[57] = (uint8_t) (bytes >> 45);
        padded[58] = (uint8_t) (bytes >> 37);
        padded[59] = (uint8_t) (bytes >> 29);
        padded[60] = (uint8_t) (bytes >> 21);
        padded[61] = (uint8_t) (bytes >> 13);
        padded[62] = (uint8_t) (bytes >> 5);
        padded[63] = (uint8_t) (bytes << 3);
        versat_crypto_hashblocks_sha256(padded, 64);
    } else {
        for (size_t i = inlen + 1; i < 120; ++i) {
            padded[i] = 0;
        }
        padded[120] = (uint8_t) (bytes >> 53);
        padded[121] = (uint8_t) (bytes >> 45);
        padded[122] = (uint8_t) (bytes >> 37);
        padded[123] = (uint8_t) (bytes >> 29);
        padded[124] = (uint8_t) (bytes >> 21);
        padded[125] = (uint8_t) (bytes >> 13);
        padded[126] = (uint8_t) (bytes >> 5);
        padded[127] = (uint8_t) (bytes << 3);
        versat_crypto_hashblocks_sha256(padded, 128);
    }

    // Does the last run with valid data
    AcceleratorRun(accel);

    for (size_t i = 0; i < 8; ++i) {
        uint32_t val = *stateReg[i]->state;

        store_bigendian_32(&out[i*4],val);
    }

    initVersat = false; // At the end of each run
}

/*

Things to do:

SHA implementation with macroinstances
AES implementation.

Need to do:

    Add the concept of macroinstances
    Add creation of macroinstances
    Add specification parsing

*/

/*

Things to do:

Implement the concept of configuration:
    Board code should not need to perform any type of computation
    Config and delay data are encoded in the code as a set of arrays. Generate .c file. (Cannot do IO on the board, versat.c declares variables as extern, add generated .c into compiled code)
    The configuration is stored in hardware, using the configuration register.
    The versat initializes by transfering configuration data to the configuration register.
    Store both config and delay values

Differentiate between done in IO units and done in source + sink units:
    The hardware circuit only needs to implement done for IO units.
    Sink and source always take exactly X cycles to be done with, meaning that the circuit would only need to implement a counter for these units

Accelerator has pointer for

*/
