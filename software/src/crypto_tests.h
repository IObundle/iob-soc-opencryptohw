#pragma once

#include <stdint.h>
#include <string.h>

/** \file
 * Declares the API for all the functions and structures related to running the testcases
 */

/**
 * A String that remembers its size. Safer than C-Style strings
 */
typedef struct{
  //! Pointer to data
  char* str;
  //! Size of string
  int size;  
} String;

#define STRING(str) (String){str,strlen(str)}

/**
 * The result of a testcase. Stores all the info, from tests that failed to the time taken (on average).
 * Also stores the result of the same run in software to compare to the versat implementation 
 */
typedef struct {
  //! How much time it took to initialize Versat. Does not take into account initialization time of the algorithm itself. 
  int initTime;
  //! Total amount of tests taken
  int tests;    
  //! How many tests passed. For a successful run, goodTests == tests
  int goodTests; 
  //! Accumulation of all the time taken by Versat implementation. Includes initialization time specific to the algorithm itself
  int versatTimeAccum; 
  //! Accumulation of all the time taken by software only implementation. Includes initialization time specific to the algorithm itself
  int softwareTimeAccum; 
  //! Wether the test had an early exit because there was some problem with the test content.
  int earlyExit; 
} TestState;

//! IOBb-SoC firmware must implement this function so that testcases can record time taken
int GetTime(); 

/** 
 * Testcases follow a structures where values are in the form "VAL = ...". This function can search for VAL and return a pointer to the first character after the string
 * \brief Simple function to parse content file and find specific values.
 * \param ptr points to current location inside str
 * \param str value that we are searching for
 * \return pointer to a location past str if str was found otherwise NULL.
 */

char* SearchAndAdvance(char* ptr,String str);

/**
 * Parse number from content. Do not need to handle errors since the testcase files are given error free.
 * \brief Parse number
 * \param ptr to number in text format.
 * \return returns parsed number
 */
int ParseNumber(char* ptr);

/**
 * Must be the first function called before any interaction with the accelerator
 * \brief Initializes the runtime of Versat.
 * \param versatAddress address of the accelerator
 */
void InitializeCryptoSide(int versatAddress);

/** 
 * This function does not fetch data from outside, it stores internally the contents of a simple test. This makes sure that we can tests algorithms even if we do not have file transfer capabilities. Returns 0 on success, any other number on failure.
 * \brief Runs Versat SHA tests for simulation KAT.
 * \return 0 if successful, any other number if error
 */
int VersatSHASimulationTests();

/** 
 * This function does not fetch data from outside, it stores internally the contents of a simple test. This makes sure that we can tests algorithms even if we do not have file transfer capabilities. Returns 0 on success, any other number on failure.
 * \brief Runs Versat AES tests for simulation KAT.
 * \return 0 if successful, any other number if error
 */
int VersatAESSimulationTests();

/** 
 * This function obtains KAT data from outside. Calls VersatCommonSHATests to run tests
 * \brief Runs Versat SHA tests for embedded.
 * \return 0 if successful, any other number if error
 */
int VersatSHATests();

/** 
 * This function obtains KAT data from outside. Calls VersatCommonAESTests to run tests
 * \brief Runs Versat AES tests for embedded.
 * \return 0 if successful, any other number if error
 */
int VersatAESTests();

/** 
 * Implements the McEliece tests. This function obtains KAT data from outside.
 * \brief Implements and runs the Versat McEliece tests for embedded.
 * \return 0 if successful, any other number if error
 */
int VersatMcElieceTests();

/** 
 * Parses content and runs testcases with the given values and compares to the expected result
 * \brief Fuction that implements the SHA tests.
 * \param content a String with the content of the KAT
 * \return the result of running all the tests
 */
TestState VersatCommonSHATests(String content);

/** 
 * Parses content and runs testcases with the given values and compares to the expected result
 * \brief Fuction that implements the AES tests.
 * \param content a String with the content of the KAT
 * \return the result of running all the tests
 */
TestState VersatCommonAESTests(String content);
