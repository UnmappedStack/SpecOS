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
#include "../utils/include/string.h"
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

void debugPml4() {
    for (int i = 0; i < 512; i++)
        printf("Index %i of pml4: %i\n", i, kernel.pml4[i]);
}

#define PHYS_ADDR_MASK 0xFFFF000000000000

// it needs to be able to actually map the pages
// this will require getting the pml indexes to use based on the virtual address to map
// idk how else to explain it really lol, sorry
void mapPages(uint64_t pml4[], uint64_t virtAddr, uint64_t physAddr, uint64_t flags, uint64_t numPages) {
    writestring(" [DEBUG] Trying to map a new set of pages...\n");
    virtAddr &= ~0xFFFF000000000000;
    // get the indexes of each page directory level (aka pml)
    uint64_t pml1Index = (virtAddr >> 12) & 511;
    uint64_t pml2Index = (virtAddr >> (12 + 9)) & 511;
    uint64_t pml3Index = (virtAddr >> (12 + 18)) & 511;
    uint64_t pml4Index = (virtAddr >> (12 + 27)) & 511;
    for (; pml4Index < 512; pml4Index++) {
        uint64_t *pml3Addr = NULL;
        if (pml4[pml4Index] == 0) {
            pml4[pml4Index] = (uint64_t)kmalloc();
            printf("Address pml4 value will be at: %x\n", pml4[pml4Index]);
            pml3Addr = (uint64_t*)(pml4[pml4Index] + kernel.hhdm);
            memset((uint8_t*)pml3Addr, 0, 8 * 512);
            pml4[pml4Index] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
        } else {
            pml3Addr = (uint64_t*)PAGE_ALIGN_DOWN((pml4[pml4Index] & PHYS_ADDR_MASK) + kernel.hhdm);
        }
        
        for (; pml3Index < 512; pml3Index++) {
            uint64_t *pml2Addr = NULL;
            if (pml3Addr[pml3Index] == 0) {
                pml3Addr[pml3Index] = (uint64_t)kmalloc();
                pml2Addr = (uint64_t*)(pml3Addr[pml3Index] + kernel.hhdm);
                memset((uint8_t*)pml2Addr, 0, 8 * 512);
                pml3Addr[pml3Index] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
            } else {
                pml2Addr = (uint64_t*)PAGE_ALIGN_DOWN((pml3Addr[pml3Index] & PHYS_ADDR_MASK) + kernel.hhdm);
            }

            for (; pml2Index < 512; pml2Index++) {
                uint64_t *pml1Addr = NULL;
                if (pml2Addr[pml2Index] == 0) {
                    pml2Addr[pml2Index] = (uint64_t)kmalloc();
                    pml1Addr = (uint64_t*)(pml2Addr[pml2Index] + kernel.hhdm);
                    memset((uint8_t*)pml1Addr, 0, 8 * 512);
                    pml2Addr[pml2Index] |= flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
                } else {
                    pml1Addr = (uint64_t*)PAGE_ALIGN_DOWN((pml2Addr[pml2Index] & PHYS_ADDR_MASK) + kernel.hhdm);
                }
                
                for (; pml1Index < 512; pml1Index++) {
                    pml1Addr[pml1Index] = physAddr | flags;
                    numPages--;
                    physAddr += 4096;
                    if (numPages == 0) {
                        writestring(" [DEBUG] Finished mapping pages.\n");
                        return;
                    }
                }
                pml1Index = 0;
            }
            pml2Index = 0;
        }
        pml3Index = 0;
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
    printf("Pages mapped successfully\n");
    //printf("pml4 contents: \n");
    //debugPml4();
    // return some stuff so the entry point function of the kernel can reload cr3
    return kernel.pml4 + kernel.hhdm;
    // no need to enable paging, limine already enables it :D
}












