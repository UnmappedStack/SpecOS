/* 64 bit paging for the SpecOS kernel project.
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

// set up the structures
struct pmlEntry pml4[512] __attribute__((aligned(4096)));
struct pmlEntry pml3[512] __attribute__((aligned(4096)));
struct pmlEntry pml2[512] __attribute__((aligned(4096)));
struct pmlEntry pml1[512] __attribute__((aligned(4096)));

// and make it do stuff (great comments, ik)

// this only has a couple of arguments, cos it just needs that bit of info to work out the rest.
// a lot of it is fancy pants stuff it doesn't need, so it'll just leave those as 0.
// this also works for a page directory entry and a pdpt technically.
struct pmlEntry makePageLevelEntry(bool isKernelSpace, uint64_t address) {
    struct pmlEntry toReturn;
    // put the address in
    // assuming M = 51, get bits 12 to M.
    toReturn.address = (address >> 12) & 40; // the 40 is `M - 12`
    // set it to present. If it's in kernelspace, set supervisor.
    toReturn.isPresent = 1;
    if (isKernelSpace)
        toReturn.isUserOrSupervisor = 1;
    // and return :D
    return toReturn;
}

void initPaging(struct limine_hhdm_request hhdmRequest) {
    // get hhdm
    struct limine_hhdm_response *hhdmResponse = hhdmRequest.response;
    uint64_t hhdm = hhdmResponse->offset;
    // page entries will be defined later when filling pml1
    // and so will the pdpt when loading it into cr3
    // fill up pml1 with mappings
    // this initially sets up only kernelspace paging.
    for (int mapping = 0; mapping < 512; mapping++) {
        pml1[mapping] = makePageLevelEntry(true, mapping * 4096); // for now it just identity maps.
    }
    // now put the layers into each other
    pml2[0] = makePageLevelEntry(true, ((uint64_t) pml1) + hhdm);
    pml3[0] = makePageLevelEntry(true, ((uint64_t) pml2) + hhdm);
    pml4[0] = makePageLevelEntry(true, ((uint64_t) pml3) + hhdm);
    // load a pointer to pml4 into cr3 and change the stack to point elsewhere
    __asm__ volatile (
        "movq %0, %%cr3;"
        "movq %1, %%rsp"
        : : "r" ((uint64_t) pml4 + hhdm),
            "r" ((uint64_t) 0xfffffffffffff000)
    );
    // no need to enable paging, limine already enables it :D
}












