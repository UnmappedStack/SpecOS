/* Maps kernel into virtual memory, as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "include/mapKernel.h"
#include "../include/kernel.h"
#include "../utils/include/printf.h"
#include "include/paging.h"
#include "../limine.h"

// get all the labels defined in the linker file
extern uint64_t p_kernel_start[];
extern uint64_t p_nxe_enabled_start[];
extern uint64_t p_nxe_enabled_end[];
extern uint64_t p_writeallowed_start[];
extern uint64_t p_writeallowed_end[];

uint64_t kernel_start = (uint64_t)p_kernel_start;
uint64_t nxe_enabled_start = (uint64_t)p_nxe_enabled_start;
uint64_t nxe_enabled_end = (uint64_t)p_nxe_enabled_end;
uint64_t writeallowed_start = (uint64_t)p_writeallowed_start;
uint64_t writeallowed_end = (uint64_t)p_writeallowed_end;


// void mapPages(uint64_t pml4[], uint64_t virtAddr, uint64_t physAddr, uint64_t flags, uint64_t numPages)

void mapKernel() {
    printf("kernel_start:       %x\n"
           "nxe_enabled_start:  %x\n"
           "nxe_enabled_end:    %x\n"
           "writeallowed_start: %x\n"
           "writeallowed_end:   %x\n",
           kernel_start, nxe_enabled_start, nxe_enabled_end, writeallowed_start, writeallowed_end);
    uint64_t lengthBuffer;
    uint64_t physBuffer;
    /* map from kernel_start to nxe_enabled_start with nothing but `present` (.text section) */
    lengthBuffer = PAGE_ALIGN_UP(nxe_enabled_start) - PAGE_ALIGN_DOWN(kernel_start);
    printf("kernel_start - 0xffff80000000 = %x\n", kernel_start - 0xffffffff80000000);
    physBuffer = kernel.kernelAddress.physical_base + (kernel_start - 0xffffffff80000000);
    mapPages(kernel.pml4, PAGE_ALIGN_DOWN(kernel_start), physBuffer, KERNEL_PFLAG_PRESENT, lengthBuffer / 4096);
    /* map from nxe_enabled_start to writeallowed_start with `present` and `pxd` */
    physBuffer = kernel.kernelAddress.physical_base + (nxe_enabled_start - 0xffffffff80000000);
    lengthBuffer = PAGE_ALIGN_UP(writeallowed_start) - PAGE_ALIGN_DOWN(nxe_enabled_start);
    mapPages(kernel.pml4, PAGE_ALIGN_DOWN(nxe_enabled_start), physBuffer, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_PXD, lengthBuffer / 4096);
    /* map from writeallowed_start to writeallowed_end with `present`, `pxd`, and `write` flags */
    lengthBuffer = PAGE_ALIGN_UP(writeallowed_end) - PAGE_ALIGN_DOWN(writeallowed_start);
    physBuffer = kernel.kernelAddress.physical_base + (writeallowed_start - 0xffffffff80000000);
    mapPages(kernel.pml4, PAGE_ALIGN_DOWN(writeallowed_start), physBuffer, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_PXD | KERNEL_PFLAG_WRITE, lengthBuffer);
}
