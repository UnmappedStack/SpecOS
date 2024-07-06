// Dynamic physical memory management functions for SpecOS
// Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more info.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "pmm.h"
#include "kmalloc.h"
#include "../utils/string.h"
#include "../kernel.h"
#include "../drivers/terminalWrite.h"

void* splitPF(uint32_t location, uint32_t size) {
    // Create a new page frame & give it some values
    struct kmallocNode newFrame;
    newFrame.pfSize = ((struct kmallocNode*) location)->pfSize - ((struct kmallocNode*) location)->neededSize;
    newFrame.neededSize = size;
    newFrame.free = false;
    newFrame.nextAvaliableFrame = ((struct kmallocNode*) location)->nextAvaliableFrame;
    // Create a new version of the original frame
    struct kmallocNode origFrameNew;
    origFrameNew.pfSize = ((struct kmallocNode*) location)->neededSize;
    origFrameNew.neededSize = origFrameNew.pfSize;
    origFrameNew.nextAvaliableFrame = location + origFrameNew.pfSize;
    // Copy the contents of the original page frame
    if (!((struct kmallocNode*) location)->free) {
        uint32_t max;
        if (origFrameNew.neededSize == 0)
            max = 0;
        else
            max = origFrameNew.neededSize - sizeof(struct kmallocNode);
        for (int d = 0; d < max; d++)
            origFrameNew.contents[d] = ((struct kmallocNode*) location)->contents[d];
    }
    // Put these new page frames into the correct spot in memory
    struct kmallocNode *origFrameLocation = (struct kmallocNode*) location;
    struct kmallocNode *newFrameLocation = (struct kmallocNode*) origFrameNew.nextAvaliableFrame; 
    *origFrameLocation = origFrameNew;
    *newFrameLocation = newFrame;
    // Return a pointer to the new frame
    return ((void*) newFrameLocation) + sizeof(struct kmallocNode);
}

void* kmalloc(int allocSize) {
    allocSize += sizeof(struct kmallocNode);
    // Try find a page frame that's larger than/equal to it's required size + the new memory thingy's size
    // If it can't find any, report an out of memory error. 
    uint32_t toCheck = firstPageFrame;
    while (1) {
        if (((struct kmallocNode*) toCheck)->pfSize >= (((struct kmallocNode*) toCheck)->neededSize + allocSize))
            return splitPF(toCheck, allocSize);
        else {
            if (!((struct kmallocNode*) toCheck)->nextAvaliableFrame)
                break;
            toCheck = ((struct kmallocNode*) toCheck)->nextAvaliableFrame; 
        }
    }
    // If it got here, it couldn't find any avaliable memory.
    terminal_setcolor(VGA_COLOR_RED);
    terminal_writestring("\nNo avaliable memory (PMM). Halting device.");
    __asm__ ("cli; hlt");
    // To please the compiler, return a pointer to an arbitrary point. 
    // This doesn't matter, cos the device is already frozen as memory couldn't be found.
    return (void*) 0x00;
}

void free(void* location) {
    ((struct kmallocNode*) location)->free = true;
    ((struct kmallocNode*) location)->neededSize = sizeof(struct kmallocNode);
}
 
