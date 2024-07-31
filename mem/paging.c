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
#include "include/pmm.h" // for dynamically allocating phys mem
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

// make it know what stuff is
// this is one definition, cos pml 1->4 are all basically the same
// it's just layers of page directories
// set up the structure
struct pmlEntry pml4[512] __attribute__((aligned(4096)));

// and make it do stuff (great comments, ik)

// this only has a couple of arguments, cos it just needs that bit of info to work out the rest.
// a lot of it is fancy pants stuff it doesn't need, so it'll just leave those as 0.
// this also works for a page directory entry and a pdpt technically.
struct pmlEntry makePageLevelEntry(bool isKernelSpace, uint64_t address) {
    struct pmlEntry toReturn;
    // put the address in
    // assuming M = 51, get bits 12 to M.
    toReturn.address = (address >> 12);
    // set it to present. If it's in kernelspace, set supervisor.
    toReturn.isPresent = 1;
    if (isKernelSpace)
        toReturn.isUserOrSupervisor = 1;
    // and return :D
    return toReturn;
}

// it also needs to be able to actually map the pages
// this will require getting the pml indexes to use based on the virtual address to map
// idk how else to explain it really lol, sorry
void mapPage(struct pmlEntry pml4[], uint64_t physAddr, uint64_t virtAddr, bool isKernelSpace) { 
    // get the indexes of each page directory level (aka pml)
    uint16_t pml1Index = (virtAddr >> 12) % 512;
    uint16_t pml2Index = (virtAddr >> (12 + 9)) % 512;
    uint16_t pml3Index = (virtAddr >> (12 + 18)) % 512;
    uint16_t pml4Index = (virtAddr >> (12 + 27)) % 512;
    // initialise all the levels
    struct pmlEntry (*pml3Array)[512] = NULL;
    struct pmlEntry (*pml2Array)[512] = NULL;
    struct pmlEntry (*pml1Array)[512] = NULL;
    // make sure the entries are there, and if they aren't, then make them be there. 
    if (pml4[pml4Index].isPresent == 0) {
        // it hasn't been set yet!
        // make an array of page entries to go in there at a dynamically allocated memory address
        struct pmlEntry *newEntryPtr = (struct pmlEntry*) kmalloc(); 
        // put a pointer to this pml3 array into this pml4 entry (and set up flags & stuff)
        pml4[pml4Index] = makePageLevelEntry(isKernelSpace, (uint64_t) newEntryPtr);
        // make it into a virtual address and put stuff there
        pml3Array = (struct pmlEntry (*)[512])(((uint64_t)newEntryPtr) + kernel.hhdm);
    } else {
        pml3Array = (struct pmlEntry (*)[512])((pml4[pml4Index].address << 12) + kernel.hhdm);
    }
    // now kinda just do the same thing for each layer (I'm not gonna reexplain for each time, it's the same as above)
    if ((*pml3Array)[pml3Index].isPresent == 0) {
        struct pmlEntry *newEntryPtr = (struct pmlEntry*) kmalloc();
        (*pml3Array)[pml3Index] = makePageLevelEntry(isKernelSpace, (uint64_t) newEntryPtr);
        pml2Array = (struct pmlEntry (*)[512])(((uint64_t)newEntryPtr) + kernel.hhdm);
    } else {
        pml2Array = (struct pmlEntry (*)[512])(((*pml3Array)[pml3Index].address << 12) + kernel.hhdm);
    }
    if ((*pml2Array)[pml2Index].isPresent == 0) {
        struct pmlEntry *newEntryPtr = (struct pmlEntry*) kmalloc();
        (*pml2Array)[pml2Index] = makePageLevelEntry(isKernelSpace, (uint64_t) newEntryPtr);
        pml1Array = (struct pmlEntry (*)[512])(((uint64_t)newEntryPtr) + kernel.hhdm);
    } else {
        pml1Array = (struct pmlEntry (*)[512])(((*pml2Array)[pml2Index].address << 12) + kernel.hhdm);
    }
    // now just put the stuff in and map it
    (*pml1Array)[pml1Index] = makePageLevelEntry(isKernelSpace, physAddr);
}

// And now a version of mapPage to map consecutive pages.
void mapConsecutivePages(struct pmlEntry pml4[], uint64_t startingPhysAddr, uint64_t startingVirtAddr,
                         bool isKernelSpace, int numPages) {
    for (int i = 0; i < numPages; i++) {
        mapPage(pml4, startingPhysAddr + (i * 4096), startingVirtAddr + (i * 4096), isKernelSpace);
    }
}

struct pmlEntry* initPaging() {
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
    printf("\nMapping pages...");
    for (uint64_t i = startingPageFrame; i < endPageFrame; i++) { 
        mapPage(pml4, (uint64_t) kmalloc(), i, true);
    }
    // return some stuff so the entry point function of the kernel can reload cr3
    return pml4 + kernel.hhdm;
    // no need to enable paging, limine already enables it :D
}












