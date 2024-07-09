/* 64 bit long mode GDT for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more info.
 * NOTE: This will only work when compiled with GCC due to the use of __attribute__((noinline))
 * You'll have to change some stuff to work with other compilers. Beware!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "include/gdt.h"

// define some shit

struct GDTEntry {
    uint16_t limit1;
    uint16_t base1;
    uint16_t base2;
    uint8_t accessByte;
    uint8_t limit2 : 4;
    uint8_t flags : 4;
    uint8_t base3;
} __attribute__((packed));

struct GDTEntry GDT[3];

struct GDTPtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

// yeah I didn't know what to call this function lol so it's kinda weird
// But, why does it not have a base & limit parameter? well this is 64 bit long mode, sooo... it's ignored.
// It'll just be set to 0.
struct GDTEntry putEntryTogether(uint8_t accessByte, uint8_t flags) {
    struct GDTEntry toReturn;
    // set stuff that's ignored to 0
    toReturn.limit1 = toReturn.limit2 = toReturn.base1 = toReturn.base2 = toReturn.base3 = 0;
    // the rest gets set pretty trivially
    toReturn.accessByte = accessByte;
    toReturn.flags = flags;
    return toReturn;
}

void setGate(int gateID, uint8_t accessByte, uint8_t flags) {
    GDT[gateID] = putEntryTogether(accessByte, flags);
}

// this expects that the global gdt var has already been set
__attribute__((noinline))
void loadGDT() {
    // Make a GDTPtr thingy-ma-bob
    struct GDTPtr ptr;
    ptr.size = (sizeof(struct GDTEntry) * 3) - 1;
    ptr.offset = (uint64_t) &GDT;
    // and now for the tidiest type of code in all of ever: inline assembly! yuck.
    asm volatile("lgdt (%0)" : : "r" (&ptr));
    // random comment but it feels weird making a pointer to a pointer.
    // now reload it
    asm volatile("push $0x08; \
                  lea .reload_CS(%%rip), %%rax; \
                  push %%rax; \
                  retfq; \
                  .reload_CS:; \
                  mov $0x10, %%ax; \
                  mov %%ax, %%ds; \
                  mov %%ax, %%es; \
                  mov %%ax, %%fs; \
                  mov %%ax, %%gs; \
                  mov %%ax, %%ss; \
                  ret" : : : "eax", "rax");
    // anyway now let's just hope I don't get a gpf.
}

void initGDT() {
    setGate(0, 0, 0); // first one's gotta be null
    setGate(1, 0x9A, 0xC); // kernel mode code segment
    setGate(2, 0x92, 0xC); // kernel mode data segment
    // TODO: Add usermode segments (only when I actually try jump to userspace)
    loadGDT();
}




