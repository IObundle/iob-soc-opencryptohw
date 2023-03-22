#ifndef H_MEMORY_POOL_H
#define H_MEMORY_POOL_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct MemoryPool_ {
  uint8_t *pool_ptr;
  uint8_t *free_ptr;
  int pool_size;
  int max_usage;
};
typedef struct MemoryPool_ MemoryPool;

void MemPool_Create(int pool_size);
void MemPool_Destroy(void);
void *MemPool_Alloc(int alloc_size);
void MemPool_Free(int free_size);
void MemPool_Report(char *rpt_str);

#endif // H_MEMORY_POOL_H
