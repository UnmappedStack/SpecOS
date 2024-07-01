/*
Virtual memory manager (using paging) for SpecOS.
Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.

Oh fuck, I finally got to writing the scary bit. The bit everybody says is where they gave up on OSDev.
That's right, the infamous virtual memory manager! I've written some drivers. I've written a physical memory manager.
I have nothing left to do to procrastinate. I've gotta do it. And maybe, just maybe, it'll even work.

Note that this is a non-pse, non-pae pager thingy-ma-bob. Obviously it uses paging cos it's the 21st century, I'm not using segmentation.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../drivers/terminalWrite.h"
#include "pmm.h"
#include "vmm.h"

void loadpd(uint32_t pd[1024]) {
    // Maybe not a great way to do this, but... inline asm time!
    __asm__ __volatile__ ("mov 8(%0), %%eax; mov %%eax, %%cr3"
        : : "r" (pd) : "eax");
}

void enablePaging() {
    __asm__ __volatile__ ("mov %%cr0, %%eax; or $0x80000000, %%eax; mov %%eax, %%cr0"
        : : : "eax");
}

void initPaging() {
    // Init pd
    uint32_t page_directory[1024] __attribute__((aligned(4096))); 
    for(int i = 0; i < 1024; i++)
        page_directory[i] = 0x00000002; // Supervisor, write enabled, not present
    // Init pt
    uint32_t first_page_table[1024] __attribute__((aligned(4096)));
    // fill all 1024 entries in the table, mapping 4 megabytes
    // this currently just maps it all to the same point in physical memory. This is just for testing.
    for(unsigned int i = 0; i < 1024; i++) {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes
        first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }
    page_directory[0] = ((unsigned int)first_page_table) | 3;
    loadpd(page_directory);
    enablePaging();
}
