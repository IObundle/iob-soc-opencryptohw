#include <cstdio>

#include "versat.hpp"
#include "utils.hpp"

extern "C"{
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob-timer.h"

#ifndef PC
#include "iob-cache.h"
#endif

int printf_(const char* format, ...);
}

// #include "fullAESTests.hpp"

// Automatically times a block in number of counts
struct TimeIt{
   int line;
   char fileId;

   TimeIt(int line,char fileId){this->line = line;this->fileId = fileId;timer_reset();};
   ~TimeIt(){unsigned long long end = timer_get_count();printf("%c:%d %llu\n",fileId,line,end);}
};
#define TIME_IT(ID) TimeIt timer_##__LINE__(__LINE__,ID)

#ifndef PC
#define printf printf_
#endif

#define MEMSET(base, location, value) (*((volatile int*) (base + (sizeof(int)) * location)) = value)
#define MEMGET(base, location)        (*((volatile int*) (base + (sizeof(int)) * location)))

void AutomaticTests(Versat* versat);

int main(int argc,const char* argv[])
{
   //init uart
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);

#ifndef PC
   cache_init(0, DCACHE_ADDR_W);
#endif

   printf("McEliece Keypair\n");

   Versat* versat = InitVersat(VERSAT_BASE,1);

   SetDebug(versat,VersatDebugFlags::OUTPUT_ACCELERATORS_CODE,1);
   SetDebug(versat,VersatDebugFlags::OUTPUT_VERSAT_CODE,1);
   SetDebug(versat,VersatDebugFlags::USE_FIXED_BUFFERS,0);
   SetDebug(versat,VersatDebugFlags::OUTPUT_GRAPH_DOT,0);
   SetDebug(versat,VersatDebugFlags::OUTPUT_VCD,0);

   ParseCommandLineOptions(versat,argc,argv);

   ParseVersatSpecification(versat,"testVersatSpecification.txt");

   AutomaticTests(versat);

// #ifndef GENERATE_ONLY
//    Full_AES_Test(versat);
// #endif

   uart_finish();

   return 0;
}
