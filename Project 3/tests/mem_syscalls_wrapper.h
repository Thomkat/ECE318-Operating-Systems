#include <stddef.h>

long int slob_get_total_alloc_mem_wrapper(void);
long int slob_get_total_free_mem_wrapper(void);
long int slob_alloc_mem_wrapper(size_t size);
long int slob_free_mem_wrapper(const void * p);