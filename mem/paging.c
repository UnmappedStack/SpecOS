/* 64 bit paging for the SpecOS kernel project.
 * INCOMPLETE FILE: There's some code here that's just outright not done. Don't expect it to work.
 * It isn't even called from the kernel. I'm still working on it, so ignore this file.
 * Note that this is NOT the VMM itself. The VMM is stored in heap.c.
 * WARNING: Heavily commented file cos I had no idea what I'm doing.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include "include/pmm.h" // for dynamically allocating phys mem
#include "../utils/include/printf.h" // my bestie, printf debugging!
#include "include/paging.h"

/* uhhh yeah so I have no idea what I'm doing here, so I'm gonna write down what I DO understand here to help me.
 * So it's kinda like the 32 bit paging that I have done in Spec32, but with more layers, different address spaces, and a different number of entries.
 * The PMLs are just page directories stacked within each other. It'll have pml 1 through 4 (lower inside of higher levels),
 * and page entries (aka. phys->virt mappings) go within pml 1.
 * Hopefully should be pretty simple, so I'm gonna just give it a shot!
 */

// make it know what stuff is
// this is one definition, cos pml 1->4 are all basically the same
// it's just layers of page directories
struct pmlEntry {
    uint8_t isPresent : 1;
    uint8_t isReadOrWrite : 1;
    uint8_t isUserOrSupervisor : 1;
    uint8_t pwt : 1;
    uint8_t cacheDisable : 1;
    uint8_t accessed : 1;
    uint8_t avaliable : 1;
    uint8_t rsvdOrPageSize : 1; // rsvd if normal pml entry, page size if it's a pdpt entry or page directory entry. doesn't really matter cos it's 0 either way.
    uint8_t avaliable2 : 4;
    uint64_t address : 40;
    // think there should be reserved bits here? think again! assume M = 51.
    uint16_t available3 : 11;
    uint8_t executeDisable : 1;
} __attribute__((packed));

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
/*
THIS FUNCTION IS COMMENTED OUT BECAUSE IT IS FAR FROM COMPLETE AND WON'T EVEN COMPILE SO JUST IGNORE IT
void mapPage(struct pmlEntry pml4[], uint64_t physAddr, uint64_t virtAddr, bool isKernelSpace, uint64_t hhdm) {
    // get the indexes of each page directory level (aka pml)
    uint16_t pml1Index = (virtAddr >> 12) % 512;
    uint16_t pml2Index = (virtAddr >> (12 + 9)) % 512;
    uint16_t pml3Index = (virtAddr >> (12 + 18)) % 512;
    uint16_t pml4Index = (virtAddr >> (12 + 27)) % 512;
    // make sure the entries are there, and if they aren't, then make them be there.
    if (pml4[pml4Index] == 0) {
        // it hasn't been set yet!
        // make an array of page entries to go in there at a dynamically allocated memory address
        struct pmlEntry *newEntryPtr = (struct pmlEntry*) kmalloc();
        newEntryPtr = (struct pmlEntry*)((uint64_t)newEntryPtr + hhdm);
        struct pmlEntry (*pml3Array)[512] = (struct pmlEntry (*)[512]);
        // put a pointer to this pml3 array into this pml4 entry (and set up flags & stuff)
        pml4[pml4Index] = makePageLevelEntry(true, (uint64_t) pml3Array);
    }
    // now kinda just do the same thing for each layer (I'm not gonna reexplain for each time, it's the same as above)
    if (pml4[pml4index][pml3Index] == 0) {
        static struct pmlEntry pml2Array[512] __attribute__ ((aligned(4096)));
        pml3Array[pml3Index] = makePageLevelEntry(true, (uint64_t) pml2Array);
    }
    if (pml4[pml4Index][pml3Index][pml2Index] == 0) {
        static struct pmlEntry pml1Array[512] __attribute__ ((aligned(4096)));
        pml2Array[pml2Index] = makePageLevelEntry(true, (uint64_t) pml1Array);
    }
    // now just put the stuff in and map it
    pml1Array[pml1Index] = makePageLevelEntry(isKernelSpace, physAddr);
}
*/
void initPaging(struct limine_hhdm_request hhdmRequest) {
    // get hhdm
    struct limine_hhdm_response *hhdmResponse = hhdmRequest.response;
    uint64_t hhdm = hhdmResponse->offset;
    // page entries will be defined later when filling pml1
    // and so will the pdpt when loading it into cr3
    // fill up pml1 with mappings
    // this initially sets up only kernelspace paging.
    /*for (int mapping = 0; mapping < 512; mapping++) {
        pml1[mapping] = makePageLevelEntry(true, mapping * 4096); // for now it just identity maps.
    }*/
    // load a pointer to pml4 into cr3 and change the stack to point elsewhere
    __asm__ volatile (
        "movq %0, %%cr3;"
        "movq %1, %%rsp"
        : : "r" ((uint64_t) pml4 + hhdm),
            "r" ((uint64_t) 0xfffffffffffff000)
    );
    // no need to enable paging, limine already enables it :D
}












