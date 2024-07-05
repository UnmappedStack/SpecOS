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
#include "kmalloc.h"

uint32_t page_directory[1024] __attribute__((aligned(4096)));

uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void loadpd(uint32_t pd[1024]) {
    // Maybe not a great way to do this, but... inline asm time!
    __asm__ __volatile__ ("mov %0, %%cr3"
        : : "r" (pd));
}

void enablePaging() {
    terminal_writestring("\nTrying to enable paging...");
    __asm__ __volatile__ ("mov %%cr0, %%eax; or $0x80000000, %%eax; mov %%eax, %%cr0"
        : : : "eax");
    terminal_writestring("\nPaging enabled.");
}

/*
To set up kernelspace paging, run:
```
initPaging(0, 1024);
```
To set the whole thing, right from the start.
For a userspace application's paging to run, use:
```
initPaging(0x400000, NUM_PAGEFRAMES);
```
Note that NUM_PAGEFRAMES should be specified in the ELF header.
*/
void initPaging(int startFrom, int numFrames) {
    terminal_setcolor(VGA_COLOR_CYAN);
    terminal_writestring("\nInitiating page directory...");
    // Init pd 
    for(int i = 0; i < 1024; i++)
        page_directory[i] = 0x00000002; // Supervisor, write enabled, not present
    // Init pt
    // fill all 1024 entries in the table, mapping 4 megabytes
    // this currently just maps it all to the same point in physical memory. This is just for testing.
    terminal_writestring("\nInitiating page table...");
    for (unsigned int i = startFrom; i < numFrames + startFrom; i++) {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes
        first_page_table[i] = (((startFrom == 0) ? (i * 0x1000) : ((int) kmalloc(1024)))) | 3; // attributes: supervisor level, read/write, present.
    }
    // And the non-allocated ones
    for (unsigned int i = numFrames; i < 1024 + startFrom; i++)
        first_page_table[i] = 0x00; // no attributes, it hasn't been allocated yet.
    terminal_writestring("\nLoading page table into page directory...");
    page_directory[0] = ((unsigned int)first_page_table) | 3;
    terminal_writestring("\nLoading page directory into cr3...");
    loadpd(page_directory);
    terminal_writestring("\nEnabling paging...");
    enablePaging();
    terminal_writestring("\nPaging enabled! It's ring 3 time (:");
}
