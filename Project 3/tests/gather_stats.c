#include "mem_syscalls_wrapper.h"
#include <stdio.h>
#include <stdlib.h>

#define ITERATIONS 50 /*How many times it will run to get the average*/
#define ALLOCATIONS 15000 /*How many allocations to make each time*/
#define SMALL /*Define what size allocations to make (SMALL, MEDIUM, LARGE)*/

int main(int argc, char **argv) {
    long int totalFreeSpaceAfterAlloc = 0;
    long int totalAllocatedSpaceAfterAlloc = 0;
    long int totalFreeSpaceAfterFree = 0;
    long int totalAllocatedSpaceAfterFree = 0;
    void **arr = NULL;
    arr = (void **)malloc(ALLOCATIONS * sizeof(void *));
    int i,j;

    for(i=0; i<ITERATIONS; i++) {
        /*Make allocations*/
        for(j=0; j<ALLOCATIONS; j++) {
            #ifdef SMALL
            /*Allocate memory < 256 bytes*/
            arr[j] = (void *)slob_alloc_mem_wrapper(150);
            #endif
            #ifdef MEDIUM
            /*Allocate memory >= 256 bytes and < 1024 bytes*/
            arr[j] = (void *)slob_alloc_mem_wrapper(800);
            #endif
            #ifdef LARGE
            /*Allocate memory >= 1024 bytes*/
            arr[j] = (void *)slob_alloc_mem_wrapper(1200);
            #endif
        }

        /*Get memory stats after the allocations*/
        totalFreeSpaceAfterAlloc += slob_get_total_free_mem_wrapper();
        totalAllocatedSpaceAfterAlloc += slob_get_total_alloc_mem_wrapper();

        /*Free the allocations made above*/
        for(j=0; j<ALLOCATIONS; j++) {
            slob_free_mem_wrapper(arr[j]);
        }

        /*Get stats after freeing memory*/
        totalFreeSpaceAfterFree += slob_get_total_free_mem_wrapper();
        totalAllocatedSpaceAfterFree += slob_get_total_alloc_mem_wrapper();
    }
    free(arr);

    /*Calculate and print stats*/
    totalFreeSpaceAfterAlloc = totalFreeSpaceAfterAlloc/ITERATIONS;
    totalAllocatedSpaceAfterAlloc = totalAllocatedSpaceAfterAlloc/ITERATIONS;
    totalFreeSpaceAfterFree = totalFreeSpaceAfterFree/ITERATIONS;
    totalAllocatedSpaceAfterFree = totalAllocatedSpaceAfterFree/ITERATIONS;

    printf("~~~~~~~~~~~~~\nAVERAGE AFTER ALLOCATION:\nALLOCATED: %ld\nFREE: %ld\n~~~~~~~~~~~~~\nAVERAGE AFTER FREE:\nALLOCATED: %ld\nFREE: %ld\n~~~~~~~~~~~~~\n\n\n", 
    totalAllocatedSpaceAfterAlloc, totalFreeSpaceAfterAlloc, 
    totalAllocatedSpaceAfterFree, totalFreeSpaceAfterFree);

    return 0;
}