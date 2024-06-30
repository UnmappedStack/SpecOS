/*
Physical memory manager for SpecOS. Probably one of, if not the, most important files in the project.
I'm not gonna go into detail explaining what this does. Google it. 
However, for some context, this uses a pooling allocator, which is probably the simplest allocator that allows dynamic sizing
of page frames.
This is probably also the part of the project that relies on the least other files (just a couple for mem detection & debugging stuff.)
Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more info.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../drivers/terminalWrite.h"
#include "multiboot.h" // Memory detection
#include "pmm.h"
#include "../utils/string.h"
#include "../kernel.h"

struct pageFrame {
    bool free;
    uint32_t nextAvaliableFrame;
    uint32_t pfSize;
    uint32_t neededSize;
    uint8_t contents[0];
};

struct memSect {
    uint32_t start;
    uint32_t end;
    uint32_t size;
};

struct initialFrames {
    struct memSect preKernel;
    struct memSect postKernel;
};

/*
This function:
 - Finds start & end of largest avaliable section in physical memory
 - Finds start & end of kernel in physical memory
 - Forms an initialFrames struct, which basically holds where the bit before and after the kernel are.
These are the initial page frames.
*/
struct initialFrames findInitialFrames(multiboot_info_t* mbd, uint32_t magic) {
    // Find largest avaliable section in physical memory
    struct memSect largestWhole;
    largestWhole.size = 0;
    for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        // Make sure it's avaliable. If so, check it's size, and if it's more than the largest so far, replace it
        multiboot_memory_map_t* mmmt =
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        if (mmmt->len > largestWhole.size && mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            largestWhole.size = mmmt->len;
            largestWhole.start = mmmt->addr;
            largestWhole.end = mmmt->addr + mmmt->len;
        }
    }
    // Find the kernel
    extern uint32_t startkernel;
    extern uint32_t endkernel;
    // Split the original page frame into two sets of addresses, for before and after the kernel. 
    struct initialFrames toReturn;
    struct memSect beforeKernel;
    struct memSect afterKernel;
    beforeKernel.start = largestWhole.start + 1;
    beforeKernel.end = (uint32_t) &startkernel - 1;
    afterKernel.start = (uint32_t) &endkernel + 1;
    afterKernel.end = largestWhole.end - 1;
    toReturn.preKernel = beforeKernel;
    toReturn.postKernel = afterKernel;
    return toReturn;
}

// This function is... actually the name is pretty self-explanitory
// Returns address in the form of an integer of the first free thingymabob.
uint32_t initPMM(multiboot_info_t* mbd, uint32_t magic) {
    struct initialFrames firstFrames = findInitialFrames(mbd, magic);
    // Make two page frames and place them in physical memory at the correct locations.
    // First points to second, second one points to 0x00 (NULL, cos it's the last one)
    struct pageFrame p1;
    struct pageFrame p2;
    p1.free = p2.free = true;
    p1.nextAvaliableFrame = firstFrames.postKernel.start;
    p2.nextAvaliableFrame = 0x00;
    p1.pfSize = firstFrames.preKernel.end - firstFrames.preKernel.start;
    p2.pfSize = firstFrames.postKernel.end - firstFrames.postKernel.start;
    p1.neededSize = p2.neededSize = 0;
    // The contents isn't set yet.
    // Now place these at the appropriate place in memory
    struct pageFrame *p1Location = (struct pageFrame*) firstFrames.preKernel.start;
    struct pageFrame *p2Location = (struct pageFrame*) firstFrames.postKernel.start;
    *p1Location = p1;
    *p2Location = p2;
    return firstFrames.preKernel.start;
}

// Now for the fun stuff! kmalloc & kfree are the kernelspace versions of malloc & free
// calloc & realloc are userspace things. No need to implement them here.

// alas, first I need a function to split a page frame
void* splitPF(uint32_t location, uint32_t size) {
    // Create a new page frame & give it some values
    struct pageFrame newFrame;
    newFrame.pfSize = ((struct pageFrame*) location)->pfSize - ((struct pageFrame*) location)->neededSize;
    newFrame.neededSize = size;
    newFrame.free = false;
    newFrame.nextAvaliableFrame = ((struct pageFrame*) location)->nextAvaliableFrame;
    // Create a new version of the original frame
    struct pageFrame origFrameNew;
    origFrameNew.pfSize = ((struct pageFrame*) location)->neededSize;
    origFrameNew.neededSize = newFrame.pfSize;
    origFrameNew.nextAvaliableFrame = location + newFrame.pfSize;
    // Copy the contents of the original page frame
    for (int d = 0; d < origFrameNew.pfSize - sizeof(struct pageFrame); d++)
        origFrameNew.contents[d] = ((struct pageFrame*) location)->contents[d];
    // Put these new page frames into the correct spot in memory
    struct pageFrame *origFrameLocation = (struct pageFrame*) location;
    struct pageFrame *newFrameLocation = (struct pageFrame*) origFrameNew.nextAvaliableFrame;
    *origFrameLocation = origFrameNew;
    *newFrameLocation = newFrame;
    // Return a pointer to the new frame
    return ((void*) newFrameLocation) + sizeof(struct pageFrame);
}

void* malloc(int allocSize) {
    // Try find a page frame that's larger than/equal to it's required size + the new memory thingy's size
    // If it can't find any, report an out of memory error. 
    uint32_t toCheck = firstPageFrame;
    char buffer[9];
    while (1) {
        uint32_to_hex_string(((struct pageFrame*) toCheck)->pfSize, buffer);
        terminal_writestring("\n0x");
        terminal_writestring(buffer);
        if (((struct pageFrame*) toCheck)->pfSize >= (((struct pageFrame*) toCheck)->pfSize + allocSize))
            return splitPF(toCheck, allocSize);
        else {
            if (!((struct pageFrame*) toCheck)->nextAvaliableFrame)
                break;
            toCheck = ((struct pageFrame*) toCheck)->nextAvaliableFrame; 
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
 
