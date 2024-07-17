/* Allocator for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 * This uses a simple bitmap allocator for 1024 byte size page frames, but I may switch to a buddy allocator in the future.
 */

#include <stdint.h>
#include <stdbool.h>

#include "../drivers/include/vga.h"
#include "../utils/include/printf.h"
#include "../limine.h"
#include "../utils/include/string.h"
#include "include/pmm.h"
#include "include/detect.h"

struct pmemBitmap {
    uint8_t bitmap[0];
};

struct pmemData {
    _Alignas(1024) uint8_t data[0];
};

void initPMM(struct limine_memmap_request memmapRequest) {
    struct limine_memmap_response *memmapResponse = memmapRequest.response;
    uint64_t memmapEntriesCount = memmapResponse->entry_count;
    struct limine_memmap_entry **memmapEntries = memmapResponse->entries;
    int maxBegin = 0;
    int maxLength = 0;
    // Find the largest avaliable entry to use for allocation
    for (int i = 0; i < memmapEntriesCount; i++) {
        if (memmapEntries[i]->type == LIMINE_MEMMAP_USABLE &&
            memmapEntries[i]->length > maxLength) {
            // set it as this one
            maxBegin = memmapEntries[i]->base;
            maxLength = memmapEntries[i]->length;
        }   
    }
    // WARNING: Heavily commented section, because I was trying to get my brain to understand it
    // now that it's gotten the largest avaliable/reclaimable segment, allocate the first bit of space for the bitmap
    // and the rest for the actual data.
    // first of all, find the amount needed to allocate for the bitmap
    int bitmapReserved;
    int n = 1;
    while (1) { // keeps adding another page frame for the bitmap until it's enough
        if (n * 1024 * 8 > // bitmap size (in bits)
            (maxLength / 1024) - n) { // number of page frame to allocate
            // it's enough space: set this to be the correct number of page frames to reserve for the bitmap (in bytes)
            bitmapReserved = n * 1024;
            break;
        }
        n++;
    }
    // initialise that point in memory
    // set the bitmap to a proper array, set all to 0
    struct pmemBitmap memBuffBitmap;
    for (int i = 0; i < bitmapReserved; i++)
        memBuffBitmap.bitmap[i] = 0;
    // set all of the data to 0
    struct pmemData memBuffData;
    for (int i = 0; i < maxBegin - bitmapReserved; i++)
        memBuffData.data[i] = 0;
    // put it at the right point in memory
    *((struct pmemBitmap*) maxBegin) = memBuffBitmap;
    *((struct pmemData*) (maxBegin + bitmapReserved)) = memBuffData;
    // display stuff for debugging
    char b1[9];
    char b2[9];
    char b3[9];
    uint64_to_hex_string(maxBegin, b1);
    uint64_to_hex_string(maxLength, b2);
    uint64_to_hex_string(bitmapReserved, b3);
    printf("\nChosen segment starts at 0x%s, has a size of 0x%s, and reserves 0x%s bytes for the bitmap. Detected memory map:\n", b1, b2, b3);
}





