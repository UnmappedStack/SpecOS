/* 64 bit paging for the SpecOS kernel project.
 * INCOMPLETE FILE: There's some code here that's just outright not done. Don't expect it to work.
 * It isn't even called from the kernel. I'm still working on it, so ignore this file.
 * Note that this is NOT the VMM itself. The VMM is stored in heap.c.
 * WARNING: Heavily commented file cos I had no idea what I'm doing.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "include/mapKernel.h"
#include "include/pmm.h" // for dynamically allocating phys mem
#include "include/mapKernel.h"
#include "../utils/include/printf.h" // my bestie, printf debugging!
#include "../drivers/include/vga.h"
#include "include/paging.h"
#include "../include/kernel.h"


#define PAGE_ALIGN_DOWN(addr) ((addr / 4096) * 4096) // works cos of integer division
#define PAGE_ALIGN_UP(x) ((((x) + 4095) / 4096) * 4096)

/* uhhh yeah so I have no idea what I'm doing here, so I'm gonna write down what I DO understand here to help me.
 * So it's kinda like the 32 bit paging that I have done in Spec32, but with more layers, different address spaces, and a different number of entries.
 * The PMLs are just page directories stacked within each other. It'll have pml 1 through 4 (lower inside of higher levels),
 * and page entries (aka. phys->virt mappings) go within pml 1.
 * Hopefully should be pretty simple, so I'm gonna just give it a shot!
 */

// it needs to be able to actually map the pages
// this will require getting the pml indexes to use based on the virtual address to map
// idk how else to explain it really lol, sorry
void mapPage(uint64_t pml4[], uint64_t virtAddr, uint64_t physAddr, uint64_t flags) { 
    // get the indexes of each page directory level (aka pml)
    uint16_t pml1Index = (virtAddr >> 12) % 512;
    uint16_t pml2Index = (virtAddr >> (12 + 9)) % 512;
    uint16_t pml3Index = (virtAddr >> (12 + 18)) % 512;
    uint16_t pml4Index = (virtAddr >> (12 + 27)) % 512;
    // initialise all the levels
    uint64_t (*pml3Array)[512] = NULL;
    uint64_t (*pml2Array)[512] = NULL;
    uint64_t (*pml1Array)[512] = NULL;
    uint64_t mask = ~(((1ULL << (52 - 12 + 1)) - 1) << 12);
    uint64_t flagsClearAddr = mask & flags;
    // make sure the entries are there, and if they aren't, then make them be there. 
    if (pml4[pml4Index] & 1) {
        // it hasn't been set yet!
        // make an array of page entries to go in there at a dynamically allocated memory address
        uint64_t *newEntryPtr = (uint64_t*) kmalloc(); 
        // put a pointer to this pml3 array into this pml4 entry (and set up flags & stuff)
        pml4[pml4Index] = flagsClearAddr | ((uint64_t)newEntryPtr << 12);
        // make it into a virtual address and put stuff there
        pml3Array = (uint64_t (*)[512])(((uint64_t)newEntryPtr) + kernel.hhdm);
    } else {
        pml3Array = (uint64_t (*)[512])((((pml4[pml4Index] >> 12) & 40) << 12) + kernel.hhdm);
    }
    // now kinda just do the same thing for each layer (I'm not gonna reexplain for each time, it's the same as above)
    if ((*pml3Array)[pml3Index] & 1) {
        uint64_t *newEntryPtr = (uint64_t*) kmalloc();
        (*pml3Array)[pml3Index] = flagsClearAddr | ((uint64_t)newEntryPtr << 12);
        pml2Array = (uint64_t (*)[512])(((uint64_t)newEntryPtr) + kernel.hhdm);
    } else {
        pml2Array = (uint64_t (*)[512])(((((*pml3Array)[pml3Index] >> 12) & 40) << 12) + kernel.hhdm);
    }
    if ((*pml2Array)[pml2Index] & 1) {
        uint64_t *newEntryPtr = (uint64_t*) kmalloc();
        (*pml2Array)[pml2Index] = flagsClearAddr | ((uint64_t)newEntryPtr << 12);
        pml1Array = (uint64_t (*)[512])(((uint64_t)newEntryPtr) + kernel.hhdm);
    } else {
        pml1Array = (uint64_t (*)[512])(((((*pml2Array)[pml2Index] >> 12) & 40) << 12) + kernel.hhdm);
    }
    // now just put the stuff in and map it
    (*pml1Array)[pml1Index] = physAddr | flags;
}

// And now a version of mapPage to map consecutive pages.
void mapConsecutivePages(uint64_t pml4[], uint64_t startingVirtAddr, uint64_t startingPhysAddr, uint64_t flags, uint64_t numPages) {
    for (int i = 0; i < numPages; i++) {
        mapPage(pml4, startingVirtAddr + (i * 4096), startingPhysAddr + (i * 4096), flags);
    }
}

uint64_t* initPaging() {
    /* page entries will be defined later when filling pml1
     * and so will the pdpt when loading it into cr3
     * fill up pml1 with mappings
     * this initially sets up only kernelspace paging, and currently maps only the kernel memory
     * kernel (including the headers) starts at 0xffffffff80000000 in vmem (-kernel.hhdm for physical memory)
     * aCcorDiNG TO My caLcuLatIoNS, the kernel's size is about 108 kilobytes, which is 110592 bytes. 
     * That's 27 page frames. Just to be safe however, I'll give it one extra page frame in case the kernel grows.
     * So, it must map the kernel from 0xffffffff80000000 for 28 page frames in vmem.
     */
    uint64_t startingPageFrame = 0xffffffff80000000 / 4096;
    uint64_t endPageFrame = startingPageFrame + 28;
    printf("\nMapping pages...\n");
    mapKernel();
    // return some stuff so the entry point function of the kernel can reload cr3
    return kernel.pml4 + kernel.hhdm;
    // no need to enable paging, limine already enables it :D
}












