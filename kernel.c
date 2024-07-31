/* 64 bit version of the SpecOS kernel.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "limine.h"

#include "drivers/include/serial.h"
#include "drivers/include/vga.h"
#include "sys/include/gdt.h"
#include "sys/include/idt.h"
#include "drivers/include/keyboard.h"
#include "drivers/include/rtc.h"
#include "drivers/include/disk.h"
#include "utils/include/string.h"
#include "include/shell.h"
#include "utils/include/printf.h"
#include "mem/include/pmm.h"
#include "limine.h"
#include "mem/include/paging.h"
#include "sys/include/panic.h"
#include "include/kernel.h"

Kernel kernel = {0};

// get stuff from limine so that other kernel modules can use it
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdmRequest = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

void initKernelData() {
    // init info for the terminal & stdio
    kernel.colourOut = 0xFFFFFF;
    kernel.doPush = true;
    kernel.chX = 5;
    kernel.chY = 5;
    // bootloader information
    kernel.hhdm = (hhdmRequest.response)->offset;
    struct limine_memmap_response memmapResponse = *memmapRequest.response;
    kernel.memmapEntryCount = memmapResponse.entry_count;
    kernel.memmapEntries = memmapResponse.entries;
}

void _start() {
    initKernelData();
    init_serial();
    initVGA();
    // Just send output to a serial port to test
    writestring("Trying to initialise GDT...\n");
    initGDT();
    writestring("\nGDT successfully initialised! (as far as can be told. All I know is that there isn't a gpf.)");
    writestring("\n\nTrying to initialise IDT & everything related...\n");
    initIDT();
    writestring("\nStarting physical memory manager...");
    initPMM();
    // this is commented out cos paging doesn't work yet and it's still in progress.
    /*writestring("\nInitiating paging...");
    struct pmlEntry* pml4Address = initPaging(); 
    printf("\nPages mapped, trying to reload cr3...");
    // load a pointer to pml4 into cr3 and change the stack to point elsewhere
    __asm__ volatile (
//        "movq %0, %%rsp;"
//        "movq %1, %%rbp;"
        "movq %1, %%cr3"
        : : "r" ((uint64_t) 0xfffffffffffff000),
            "r" ((uint64_t) pml4Address)
    );
    printf("\nPaging successfully enabled!");*/
    test_userspace();
    for (;;);
}
