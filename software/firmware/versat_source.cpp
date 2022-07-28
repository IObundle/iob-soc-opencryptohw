extern "C"{
#include <stdio.h>
#include "system.h"
#include "periphs.h"
}

#include "versat_sha.hpp"

int main(int argc, const char* argv[])
{
  //instantiate versat 
  Versat versatInst = {};
  Versat* versat = &versatInst;
  InitVersat(versat,VERSAT_BASE,1); 

  // Sha specific units
  // Need to RegisterFU, can ignore return value

  ParseVersatSpecification(versat,"testVersatSpecification.txt");

  Accelerator* accel_SHA = InstantiateSHA(versat);

  // Generate versat sources
  OutputVersatSource(versat,accel_SHA,"versat_instance.v","versat_defs.vh","versat_data.inc");

  return 0;
}
