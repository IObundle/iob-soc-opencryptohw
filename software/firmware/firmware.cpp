#include <cstdio>

extern "C"{
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob_timer.h"

#include "crypto/sha2.h"
#include "crypto/aes.h"

#include "../test_vectors.h"

int printf_(const char* format, ...);
}

#define printf printf_

#include "versat.hpp"
#include "utils.hpp"

#include "unitWrapper.hpp"
#include "unitVerilogWrappers.hpp"

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

unsigned char* GetHexadecimal(const unsigned char* text, int str_size){
    static unsigned char buffer[2048+1];
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
Accelerator* accel;
bool initVersat = false;

void ClearCache(){
    int count = 0;

    for(int i = 0; i < 1024; i += 16){
        count += writeMemory[i];
    }

    printf("Clear cache: %d [ignore]\n\n",count);
}

void ParseVersatSpecification(Versat* versat,FILE* file);
FUDeclaration* REG;

int32_t* TestInstance(Accelerator* accel,FUInstance* inst,int numberInputs,int numberOutputs,...){
    static int32_t out[99];
    FUInstance* inputs[99];
    FUInstance* outputs[99];

    va_list args;
    va_start(args,numberOutputs);

    #ifdef PC
    Assert(accel->instances.Size() == 1);
    #endif

    int registersAdded = 0;
    for(int i = 0; i < numberInputs; i++){

        char buffer[128];
        int size = snprintf(buffer,128,"regIn%d",registersAdded++);

        inputs[i] = CreateNamedFUInstance(accel,REG,MakeSizedString(buffer,size),nullptr);

        int32_t val = va_arg(args,int32_t);

        VersatUnitWrite(inputs[i],0,val);

        ConnectUnits(inputs[i],0,inst,i);
    }

    registersAdded = 0;
    for(int i = 0; i < numberOutputs; i++){
        char buffer[128];
        int size = snprintf(buffer,128,"regOut%d",registersAdded++);
        outputs[i] = CreateNamedFUInstance(accel,REG,MakeSizedString(buffer,size),nullptr);

        ConnectUnits(inst,i,outputs[i],0);
    }

    CalculateDelay(accel->versat,accel);

    #if 0
    Accelerator* flatten = Flatten(accel->versat,accel,1);

    {
    FILE* dotFile = fopen("flatten.dot","w");
    OutputGraphDotFile(flatten,dotFile,1);
    fclose(dotFile);
    }
    #endif

    OutputVersatSource(accel->versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

    AcceleratorRun(accel);
    accel->locked = false;
    for(int i = 0; i < numberInputs; i++){
        RemoveFUInstance(accel,inputs[i]);
    }

    for(int i = 0; i < numberOutputs; i++){
        out[i] = outputs[i]->state[0];

        RemoveFUInstance(accel,outputs[i]);
    }

    #ifdef PC
    Assert(accel->instances.Size() == 1);
    #endif

    va_end(args);

    return out;
}

void TestMStage(Versat* versat){
    FUDeclaration* type = GetTypeByName(versat,MakeSizedString("M_Stage"));
    Accelerator* accel = CreateAccelerator(versat);
    FUInstance* inst = CreateNamedFUInstance(accel,type,MakeSizedString("Test"),nullptr);

    int constants[] = {7,18,3,17,19,10};
    for(int ii = 0; ii < ARRAY_SIZE(constants); ii++){
        inst->config[ii] = constants[ii];
    }

    int32_t* out = TestInstance(accel,inst,4,1,0x5a86b737,0xa9f9be83,0x08251f6d,0xeaea8ee9);

    printf("Expected: 0xb89ab4ca\n");
    printf("Got:      0x%x\n",out[0]);
}

void TestM(Versat* versat){
    FUDeclaration* type = GetTypeByName(versat,MakeSizedString("M"));
    accel = CreateAccelerator(versat);
    FUInstance* inst = CreateNamedFUInstance(accel,type,MakeSizedString("Test"),nullptr);

    int constants[] = {7,18,3,17,19,10};

    // Set constants to every entity (memcpy is slower, does byte a byte)
    for(int i = 0; i < 16; i++){
        for(int ii = 0; ii < ARRAY_SIZE(constants); ii++){
            inst->config[i*6+ii] = constants[ii];
        }
    }

    printf("b89ab4ca fc0ba687 6f70775f fd7fcf73 ddc5d5d7 b54ee23e 481631f5 9c325ada 1e01af58 11016b62 465da978 961e5ee7 9860640b 3f309ec4 439e4f9d 14ca5690\n");

    int32_t* out = TestInstance(accel,inst,16,16,0x5a86b737,0xeaea8ee9,0x76a0a24d,0xa63e7ed7,0xeefad18a,0x101c1211,0xe2b3650c,0x5187c2a8,0xa6505472,0x08251f6d,0x4237e661,0xc7bf4c77,0xf3353903,0x94c37fa1,0xa9f9be83,0x6ac28509);

    for(int i = 0; i < inst->declaration->nOutputs; i++){
        printf("%08x ",out[i]);
    }
    printf("\n");
}

void TestDelay(Versat* versat){
    #if 0
    FUDeclaration* type = GetTypeByName(versat,MakeSizedString("TestDelay"));
    accel = CreateAccelerator(versat);
    FUInstance* inst = CreateFUInstance(accel,type);

    CalculateDelay(versat,accel);

    int32_t* out = TestInstance(accel,inst,0x2);

    for(int i = 0; i < inst->declaration->nOutputs; i++){
        printf("%08x ",out[i]);
    }
    #endif
}

void TestSHA(Versat* versat){
    FUDeclaration* type = GetTypeByName(versat,MakeSizedString("SHA"));
    accel = CreateAccelerator(versat);
    FUInstance* inst = CreateFUInstance(accel,type);

    FUInstance* read = GetInstanceByName(accel,"SHA","MemRead");
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

    for(int i = 0; i < 4; i++){
        FUInstance* inst = GetInstanceByName(accel,"SHA","cMem%d",i);

        volatile MemConfig* c = (volatile MemConfig*) inst->config;

        c->iterA = 1;
        c->incrA = 1;
        c->perA = 16;
        c->dutyA = 16;

        for (int ii = 0; ii < 16; ii++)
        {
            VersatUnitWrite(inst,ii,kConstants[i][ii]);
        }
    }

    for(int i = 0; i < 8; i++){
        FUInstance* inst = GetInstanceByName(accel,"SHA","State","s%d",i,"reg");

        VersatUnitWrite(inst,0,initialStateValues[i]);
    }

    CalculateDelay(versat,accel);

    // Gera o versat.
    //OutputVersatSource(versat,"versat_instance.v");
}

void RegisterTypes();

int main(int argc,const char* argv[])
{
    unsigned char digest[256];

    //init uart
    uart_init(UART_BASE,FREQ/BAUD);
    timer_init(TIMER_BASE);

    RegisterTypes();

    #if 0
    uart_finish();
    return 0;
    #endif

    #ifdef PC
    printf("%s\n",argv[0]);
    #endif

    for(int i = 0; i < 256; i++){
        digest[i] = 0;
    }

    // Force alignment on a 64 byte boundary
    Versat versatInst = {};
    Versat* versat = &versatInst;
    InitVersat(versat,VERSAT_BASE,1);

    REG = GetTypeByName(versat,MakeSizedString("xreg"));

    #ifdef PC
    // Sha specific units
    FUDeclaration* UNIT_F = RegisterUnitF(versat);
    FUDeclaration* UNIT_M = RegisterUnitM(versat);

    FILE* file = fopen("testVersatSpecification.txt","r");

    ParseVersatSpecification(versat,file);
    #endif

    #if 0
    TestMStage(versat);

    uart_finish();
    return 0;
    #endif

    #if 1
    TestM(versat);

    uart_finish();
    return 0;
    #endif

    #if 0
    TestDelay(versat);

    uart_finish();
    return 0;
    #endif

    #if 0
    printf("Expected: 42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa\n");
    sha256(digest,msg_64,64);
    printf("Result:   %s\n",GetHexadecimal(digest, HASH_SIZE));
    #endif

    #if 0
    TestSHA(versat);

    uart_finish();
    return 0;
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

    #if 0
    printf("Expected: 42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa\n");
    versat_sha256(digest,msg_64,64);
    printf("Result:   %s\n",GetHexadecimal(digest, HASH_SIZE));
    #endif

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
        FUInstance* read = GetInstanceByName(accel,"SHA","MemRead");

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

        #if 1
        if(!initVersat){
            for(int i = 0; i < 8; i++){
                FUInstance* inst = GetInstanceByName(accel,"SHA","State","s%d",i,"reg");

                VersatUnitWrite(inst,0,initialStateValues[i]);
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
        FUInstance* inst = GetInstanceByName(accel,"SHA","State","s%d",i,"reg");

        uint32_t val = *inst->state;

        store_bigendian_32(&out[i*4],val);
    }

    initVersat = false; // At the end of each run, reset
}

/*

Currently:

1) Figure out where the trap condition is happening

Things to do:

SHA implementation with macroinstances
AES implementation.
Fix makefile dependencies, make it proper (defines (like -DPC) only for firmware, not for other source files)

*/

/*

Implementation to do:

Overall:

    Start implementing error reporting. Some things should give error but the program still goes on, put asserts.
    Errors should be simple: Report the error and then quit. Must errors cannot be carry over. No fail and retry

Software:

    Change hierarchical name from a char[] to a char* (Otherwise will bleed embedded memory dry)
        Software for now will simply malloc and forget, but eventually I should probably implement string interning
        NOTE: After adding perfect hashing, name information is no longer required. Might not need to change afterall, for now hold on.

    Add true hierarchical naming for Flatten units
        - Simply get the full hierarchical representation and store it as the name of the unit, with parent set to null
        - Might need to change hierarchical name from array to char*
        - Need to take care about where strings end up. Do not want to fill embedded with useless data if possible

    Take a look at compute units that need delay information (think multiply accumulate, not delay but acts same way) [should simple by set inputDelay as delay]
    Support the output of a module not starting at zero. (Maybe weird thing to do for now)
        More complex, but take a pass at anything that depends on order of instancing/declarating (a lot of assumptions are being made right now)
    Go back and check the output memory map for individual accelerators, there is some bugs for the memory allocation right now
    Lock accelerator can resize memory allocated to fit exactly with amount of memory needed

Embedded:

    Write source code containing info for the embedded side using template engine
    Implement a FSK hashing scheme to accelerate simulation. GetInstanceByName

Hardware:

    Remove done/run/clk if not needed
    Take another look at circuit input and output, instanciating units only to pass data through seems wrong way to go about it.

Template Engine:

    Really need to simply error if identifier not found.
    Eliminate pointers entirely. You can never use dot access in a pointer anyway. If the dot is used on a pointer, then simply collapse to base type, no matter how many pointers there are
    Add local variables, instead of everything global
    Take another pass at whitespace handling (Some whitespace is being consumed in blocks of text, care, might join together two things that are supposed to be seperated and do error)

Struct Parser:

    Rewrite the dumb stuff from the struct parser

Verilog Parser:

    Parse units interfaces to generate unitWrappers automatically and register units automatically (do not know how to handle latency, though)

Flatten:

    Give every declaration a "level" variable. simple = 0, composite = max(instancesLevel) + 1, special = 0.

Merge:

    Do merge of units in same "level". Iterate down while repeating

Improvements:

    Empty functions in the embed code occupy space and do nothing but cluter source code file. Replace functions with macros that expand to nothing in the embed side,

*/












