#include <cstdio>

#include "versat.hpp"
#include "utils.hpp"

extern "C"{
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob-timer.h"
//#include "iob-ila.h"

#include "crypto/sha2.h"
#include "crypto/aes.h"

int printf_(const char* format, ...);
}

// Automatically times a block in number of counts
struct TimeIt{
   int line;
   char fileId;

   TimeIt(int line,char fileId){this->line = line;this->fileId = fileId;timer_reset();};
   ~TimeIt(){unsigned long long end = timer_get_count();printf("%c:%d %llu\n",fileId,line,end);}
};
#define TIME_IT(ID) TimeIt timer_##__LINE__(__LINE__,ID)

#ifdef PC
#define uart_finish(...) ((void)0)
#define uart_init(...) ((void)0)
#else
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
   //ila_init(ILA_BASE);

   printf("Init base modules\n");

   Versat* versat = InitVersat(VERSAT_BASE,1);

#if 0
   SetDebug(versat,VersatDebugFlags::OUTPUT_GRAPH_DOT,true);
#endif

   ParseCommandLineOptions(versat,argc,argv);

   ParseVersatSpecification(versat,"testVersatSpecification.txt");

   AutomaticTests(versat);

   uart_finish();

   return 0;
}

/*

Immediate plan:

Current plan:

Change the calculated inputs and outputs on the FUInstances to be full vector likes for the size
   If there isn't a connection, simply store a nullptr
      [Objective] Implement a way so that the memory units that have unconnected inputs can have a zero input
Take a look at hierarchical names (and how would I implemented them for a flatten operation)
At the very least, the specification parser must have good error reporting, on what is expected from the language
Simplify the process of creating FU units.
   Code a complete realloc routine that can be used at any point to make a valid accelerator
      Possible use it to implement the Removal of units
   Move any allocation needed to the Locking interface
      The units do not need output allocations before running the accelerator
      Maybe make it so that outputs allocations are allocated globally throught the accelerator
         Simplify the process of copying the stored outputs (a simple memcpy)

*/

/*

Implementation to do:

Software:

   Change hierarchical name from a char[] to a char* (Otherwise will bleed embedded memory dry)
      Software for now will simply malloc and forget, but eventually I should probably implement some form of string interning
      NOTE: After adding perfect hashing, name information is no longer required. Might not need to change afterall, for now hold on.

   Add true hierarchical naming for Flatten units
      - Simply get the full hierarchical representation and store it as the name of the unit, with parent set to null
      - Might need to change hierarchical name from array to char*
      - Need to take care about where strings end up. Do not want to fill embedded with useless data if possible

   Support the output of a module not starting at zero. (Maybe weird thing to do for now)
      More complex, but take a pass at anything that depends on order of instancing/declarating (a lot of assumptions are being made right now)
   [Type] Change ouput memory map and functions alike to use type system and simple output structs directly (Simpler and less error prone)
   [CalculateVersatData] naked memory allocation, add it to the accelerator locking mechanism

Embedded:

   Implement a perfect hashing scheme to accelerate simulation. GetInstanceByName

Hardware:

   Maybe think about changing the databus interface (go from native to a axi like interface)
      And possible add a length signal to indicate length to the dma unit

   Come up with another name to differentiate delays from the delay units

   Implement shadow registers
   Delay units with same inputs and delay values should be shared.
      In fact, a delay tree should be created. No need to have a delay of X and a delay of X+1, when we can have X and a Register.
         For now, delays are "fixed". Even thought programmable, the whole module unit only works for a specific delay value and therefore

   Introduce the concept of combinatorial units only
      These units can be "shared"

Delay:

   Add a special "Constant" edge, that doesn't have or add any delay regardless of anything (Time agnostic)
   To improve latency calculations, should have a indication of which outputs delay depend on which input ports delay (Think how dual port ram has two independent ports)
      Each output should keep track of exactly which input port it depends upon, and use that information in delay calculation
   Since delay is simply "how many cycles until seeing valid data", I barely understand what is the use of DELAY_TYPE_SOURCE_DELAY and DELAY_TYPE_SINK_DELAY. Is it really just because of Reg?
      I think I only need a single value to indicate the difference between a Reg and a Muladd (one produces that, and therefore should act as a source, while the other is a compute unit)
   Delay calculation not working if there is a separatation of flow
      Think adding a new unit to the start that doesn't connect to anywhere.
      The new unit will have pretty much the full latency as the delay, when it should be only the latency of the previous unit
         The algorithm doesn't take into account different flows, it works by assuming that everything ends up in a final unit which might not be true

Template Engine:

   Really need to simplify error if identifier not found.
   Add local variables, instead of everything global
   Take another pass at whitespace handling (Some whitespace is being consumed in blocks of text, care, might join together two things that are supposed to be seperated and do error)
   Need to return values from calls

Flatten:

   Give every declaration a "level" . simple = 0, composite = max(level of subunits) + 1, special = 0.
   Flatten is currently broken (might have been fixed, check it later?), only creating shallow instances. Useful for outputting graph, but not possible to simulate
      The fix is to create a function that copies and changes shallow instances and, in the end, it fixes memory allocation and instance pointers and initialization

Merge:

   Do merge of units in same "level". Iterate down while repeating

*/

/*

Known bugs:

The Pool class shouldn't store all that info, and it makes it a error to copy the class
   All the info needed is stored in the linked list on the pages, the Pool should pretty much work like a shell and therefore be copied around without a problem

*/


/*

Merge objectives:

Easy to use interface
Easily extandable - merging 3 should be as easy and as eficient as merging 2.

Current standard procedure:

   Merge creates a new declaration.
      FUDeclaration* merged = MergeTypes("MERGED",typeAccel1,typeAccel2);

   Can be used just like any other declaration

      FUDeclaration* type = GetTypeByName(versat,MakeSizedString("MERGED"));

   Same interface to instantiate a merge as any other unit

      Accelerator* accel = CreateAccelerator(versat);
      FUInstance* top = CreateFUInstance(accel,type,MakeSizedString("Test"));

   Problem, what to do if two units share the same name in the merge but are different types?

      FUInstance* inst = GetInstanceByName(accel,"Test","sigma");

   UseType(accelerator,typeAccel1); // or UseType(accelerator,0) or UseType(accelerator,1);

   Three solutions:

      [Probably the best] Add a explicit state to the accelerator that tells the current accelerator what type is being used.
         + GetInstance returns type based on accelerator state.
         + Embedded can just store N arrays of data, potently less to save space.
         - Potentially some confusion and errors, but a lot more simple than the 3rd way.

      Create a Union like abstraction, where a type acts as different types (the amount of everything memwise is simple the maximum for each unit in the union)
         + User doesn't have to worry about a thing.
         - How to handle a merge if the unit gets merged with another unit? Cannot have a union like structure because both types would need to fully exist in one implementation, which is then not possible

      Find a way of storing the parent declarations in each unit and make so that GetInstance needs to provide that data.
         Something like FUInstance* inst = GetInstanceByName(accel,"ACCEL1:","Test","sigma"); // There exists a ACCEL1:Test.sigma and a ACCEL2:Test.sigma (different units but same name) (The ACCEL1: qualifier can appear anywhere, as long as it is before a unit that needs disambiguation
         + Easier to implement in pc emulation
         - Potentially error prone for the user.
         - No idea how to implement it for the embedded case

   CalculateDelay(accel->versat,accel);

      Need to calculate delay based on current accelerator?
      Or is this something that is done in the MergeTypes function and then stored?

   SetDelayRecursive(accel);

      Probably is for the best to calculate delays in the MergeTypes function and then simple retrieve them here.
      Currently the information is stored in inst->baseDelay, which means that we cannot merge and keep delay information

   AcceleratorRun(accel);

      If everything is done correctly, this part shouldn't require any change

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

      Same for this part















*/


