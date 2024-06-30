/*
Physical memory detection for SpecOS, which currently relies on GRUB to recieve a memory map.
Original file based on osdev.org example with no license. Modifications are under the following license:
Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more info.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "multiboot.h"
#include "../utils/string.h"
#include "../drivers/terminalWrite.h"

void detectMemmap(multiboot_info_t* mbd, uint32_t magic) {
    /* Make sure the magic number matches for memory mapping*/
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_writestring("\ninvalid magic number!\n");
        return;
    }

    /* Check bit 6 to see if we have a valid memory map */
    if(!(mbd->flags >> 6 & 0x1)) {
        terminal_writestring("\ninvalid memory map given by GRUB bootloader\n");
        return;
    }
    // Loop through the memory map and display the values
    int i;
    for(i = 0; i < mbd->mmap_length; 
        i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        char startAddrBuffer[9];
        char lengthBuffer[9];
        uint32_to_hex_string(mmmt->addr, startAddrBuffer);
        uint32_to_hex_string(mmmt->len, lengthBuffer);
        terminal_writestring("\nStart Addr: 0x");
        terminal_writestring(startAddrBuffer);
        terminal_writestring(" | Length: 0x");
        terminal_writestring(lengthBuffer);
        if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            terminal_writestring(" | Avaliable");
        } else {
            terminal_writestring(" | Not Avaliable");
        }
    } 
    // Check where the kernel is in memory
    extern uint32_t startkernel;
    extern uint32_t endkernel;
    char kernelStartBuffer[9];
    char kernelEndBuffer[9];
    uint32_to_hex_string((uint32_t) &startkernel, kernelStartBuffer);
    uint32_to_hex_string((uint32_t) &endkernel, kernelEndBuffer);
    terminal_writestring("\nKernel is located from 0x");
    terminal_writestring(kernelStartBuffer);
    terminal_writestring(" to 0x");
    terminal_writestring(kernelEndBuffer);
    terminal_writestring(" (Size: 0x");
    char ksizeBuffer[9];
    uint32_to_hex_string(((uint32_t) &endkernel) - ((uint32_t) &startkernel), ksizeBuffer);
    terminal_writestring(ksizeBuffer);
    terminal_writestring(")\n");
}





