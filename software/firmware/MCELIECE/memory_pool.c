#include "memory_pool.h"

static MemoryPool pool;

void MemPool_Create(int pool_size, int offset) {
  if (pool_size > 0) {
#ifdef PC
    pool.pool_ptr = (uint8_t *)malloc(pool_size * sizeof(uint8_t));
#else
    pool.pool_ptr = (uint8_t *) ((1 << (FIRM_ADDR_W))+offset);
#endif
    if (pool.pool_ptr != NULL) {
      pool.pool_size = pool_size;
      pool.free_ptr = pool.pool_ptr;
      pool.max_usage = 0;
    }
  }
  return;
}

void MemPool_Destroy(void) {
  if (pool.pool_ptr != NULL) {
#ifdef PC
    free(pool.pool_ptr);
#endif
    pool.pool_ptr = NULL;
    pool.free_ptr = NULL;
    pool.pool_size = 0;
    pool.max_usage = 0;
  }
  return;
}

void *MemPool_Alloc(int alloc_size) {
  uint8_t *alloc_ptr = NULL;
  if (alloc_size < (pool.pool_size - (pool.free_ptr - pool.pool_ptr))) {
    alloc_ptr = pool.free_ptr;
    pool.free_ptr += alloc_size;
    if (pool.max_usage < (pool.free_ptr - pool.pool_ptr)) {
      pool.max_usage = (pool.free_ptr - pool.pool_ptr);
    }
  }
  return (void*) alloc_ptr;
}

void *MemPool_Calloc(int alloc_size) {
    // allocate memory
    uint8_t* alloc_ptr = MemPool_Alloc(alloc_size);
    // set memory to zero
    for (int i = 0; i < alloc_size; i++) {
        alloc_ptr[i] = 0;
    }
    return (void*) alloc_ptr;
}

void MemPool_Free(int free_size) {
  if (free_size < (pool.free_ptr - pool.pool_ptr)) {
    pool.free_ptr -= free_size;
  } else {
    pool.free_ptr = pool.pool_ptr;
  }
  return;
}

void MemPool_Report(char *rpt_str) { 
    printf("%s:\n", rpt_str);
    printf("\tPool Size: %d\n", pool.pool_size);
    printf("\tCurrent Usage: %ld\n", pool.free_ptr - pool.pool_ptr);
    printf("\tMax Usage: %d\n", pool.max_usage);
    return;
}
