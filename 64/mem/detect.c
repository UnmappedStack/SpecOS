/* Physical memory detection for use with Limine bootloader as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include "../limine.h" // gimme dat mem map plz limine
#include "../drivers/include/vga.h" // gimme dat printf debugging plz
#include "../utils/include/string.h" // plz convert number to hex string
#include "../misc/bootInfo.h" // limine info
#include "../utils/include/printf.h"

// get stuff from limine
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

/* Note that the physical memory manager has it's own memory map detection.
 * This is just used for debugging in the shell.
 */
void detectMemmap() { 
    // get the stuff from limine
    struct limine_memmap_response *memmapResponse = memmapRequest.response;
    uint64_t memmapEntriesCount = memmapResponse->entry_count;
    struct limine_memmap_entry **memmapEntries = memmapResponse->entries;
    // process and display it
    for (int i = 0; i < memmapEntriesCount; i++) {
        writestring("\nBase: 0x");
        char buffer0[9];
        uint64_to_hex_string(memmapEntries[i]->base, buffer0);
        writestring(buffer0);
        writestring(" | Length: 0x");
        char buffer1[9];
        uint64_to_hex_string(memmapEntries[i]->length, buffer1);
        writestring(buffer1);
        writestring(" | Type: ");
        if (memmapEntries[i]->type == LIMINE_MEMMAP_USABLE ||
            memmapEntries[i]->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE ||
            memmapEntries[i]->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE)
            writestring("Avaliable");
        else
            printf("Not avaliable (%i)", memmapEntries[i]->type);
    }
    printf("\nThere are %i entries.\n", memmapEntriesCount);
} 




