#include <sys/syscall.h>
#include <unistd.h>
#include <stddef.h>
#include "mem_syscalls_wrapper.h"

long int slob_get_total_alloc_mem_wrapper(void) {
    return(syscall(__NR_slob_get_total_alloc_mem));
}

long int slob_get_total_free_mem_wrapper(void) {
    return(syscall(__NR_slob_get_total_free_mem));
}

long int slob_alloc_mem_wrapper(size_t size) {
    return(syscall(__NR_slob_alloc_mem, size));
}

long int slob_free_mem_wrapper(const void * p) {
    return(syscall(__NR_slob_free_mem, p));
}