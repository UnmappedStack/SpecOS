/* A little kernelspace heap for SpecOS.
 * This isn't the PMM. Instead of allocating fixed size page frames, it allocates dynamically sized memory.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include "include/kheap.h"
#include "../drivers/include/vga.h"
#include "../kernel/include/kernel.h"
#include "../utils/include/string.h"
#include "include/pmm.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    bool isFree;
    int size;
    int requiredSize;
    uintptr_t nextPool;
    uint8_t data[0];
} Pool;

void initKHeap() {
    writestring("Initiating kernelspace heap...\n");
    kernel.kheapAddr = (uintptr_t)kmalloc() + kernel.hhdm;
    Pool initPool;
    initPool.isFree = true;
    initPool.size = 4096;
    initPool.requiredSize = sizeof(Pool);
    initPool.nextPool = -1; // if it's -1, it's the last one
    *((Pool*)kernel.kheapAddr) = initPool;
}

void* splitPool(Pool *addr, int size) {
    Pool newPool;
    newPool.requiredSize     = size;
    newPool.size             = addr->size - addr->requiredSize;
    newPool.nextPool         = addr->nextPool;
    newPool.isFree           = false;
    addr->size               = addr->requiredSize;
    addr->nextPool           = (uintptr_t)(addr->size + ((uint64_t)addr));
    *((Pool*)addr->nextPool) = newPool;
    return (void*)(addr->nextPool);
}

void* malloc(int size) {
    size += sizeof(Pool);
    Pool* toCheck = (Pool*)kernel.kheapAddr;
    while (true) {
        if (toCheck->isFree) {
            toCheck->isFree = false;
            toCheck->requiredSize = size;
            return (void*)toCheck + sizeof(Pool);
        } else if (toCheck->size > toCheck->requiredSize + size) {
            return splitPool(toCheck, size) + sizeof(Pool);
        } else if (toCheck->nextPool == NULL) {
            Pool newPool;
            newPool.requiredSize = size;
            newPool.size = 4096;
            newPool.isFree = false;
            newPool.nextPool = -1;
            uint64_t newPoolAddr = ((uint64_t)kmalloc) + kernel.hhdm;
            *((Pool*)newPoolAddr) = newPool;
            toCheck->nextPool = (uintptr_t)newPoolAddr;
            return (void*)newPoolAddr + kernel.hhdm;
        }
        toCheck = (Pool*) toCheck->nextPool;
    }
}

void free(void* location) {
    Pool* poolAddr         = (Pool*)((uint64_t)location - sizeof(Pool));
    poolAddr->isFree       = true;
    poolAddr->requiredSize = sizeof(Pool);
    memset(poolAddr->data, 0, poolAddr->size - sizeof(Pool));
    poolAddr->size         = sizeof(Pool);
}
