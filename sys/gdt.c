/* 64 bit long mode GDT for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more info.
 * NOTE: This will only work when compiled with GCC due to the use of __attribute__((noinline))
 * You'll have to change some stuff to work with other compilers. Beware!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "include/gdt.h"
#include "../drivers/include/vga.h"
#include "../include/kernel.h"

// define some shit

// yeah I didn't know what to call this function lol so it's kinda weird
// But, why does it not have a base & limit parameter? well this is 64 bit long mode, sooo... it's ignored.
// It'll just be set to 0.
struct GDTEntry putEntryTogether(uint8_t accessByte, uint8_t flags, uint32_t limit) {
    struct GDTEntry toReturn;
    // set stuff that's ignored to 0
    toReturn.base1 = toReturn.base2 = toReturn.base3 = 0;
    // split the limit into two parts and set it's values
    // I know technically I don't have to because it's ignored but whatever.
    toReturn.limit1 = limit & 0xFFFF; // first 16 bits
    toReturn.limit2 = (limit >> 16) & 0xF; // next 4 bits
    // the rest gets set pretty trivially
    toReturn.accessByte = accessByte;
    toReturn.flags = flags;
    return toReturn;
}

void setGate(int gateID, uint8_t accessByte, uint8_t flags, uint32_t limit) {
    kernel.GDT[gateID] = putEntryTogether(accessByte, flags, limit);
}

// this expects that the global gdt var has already been set
__attribute__((noinline))
void loadGDT() {
    // Make a GDTPtr thingy-ma-bob
    writestring("\nSetting GDT pointer...");
    kernel.GDTR.size = (sizeof(struct GDTEntry) * 5) - 1;
    kernel.GDTR.offset = (uint64_t) &kernel.GDT;
    // and now for the tidiest type of code in all of ever: inline assembly! yuck.
    writestring("\nLoading new GDT...");
    asm volatile("lgdt (%0)" : : "r" (&kernel.GDTR));
    // random comment but it feels weird making a pointer to a pointer.
    // now reload it
    writestring("\nReloading GDT...");
    asm volatile("push $0x08; \
                  lea .reload_CS(%%rip), %%rax; \
                  push %%rax; \
                  retfq; \
                  .reload_CS: \
                  mov $0x10, %%ax; \
                  mov %%ax, %%ds; \
                  mov %%ax, %%es; \
                  mov %%ax, %%fs; \
                  mov %%ax, %%gs; \
                  mov %%ax, %%ss" : : : "eax", "rax");
    // anyway now let's just hope I don't get a gpf.
}

void initGDT() {
    writestring("\nSetting GDT gates...");
    setGate(0, 0, 0, 0); // first one's gotta be null
    setGate(1, 0x9A, 0xA, 0xFFFFF); // kernel mode code segment
    setGate(2, 0x92, 0xC, 0xFFFFF); // kernel mode data segment
    setGate(3, 0xFA, 0xA, 0xFFFFF); // user mode code segment
    setGate(4, 0xF2, 0xC, 0xFFFFF); // user mode data segment
    loadGDT();
}




