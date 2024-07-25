/* Kernel-wide system information for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <stdbool.h>
#include <stdint.h>

#include "../limine.h"

#ifndef KERNEL_H
#define KERNEL_H

// some stuff it needs before the main struct
struct largestSection {
    int maxBegin;
    int maxLength;
    int bitmapReserved; // in bytes
};

struct GDTEntry {
    uint16_t limit1;
    uint16_t base1;
    uint8_t base2;
    uint8_t accessByte;
    uint8_t limit2 : 4;
    uint8_t flags : 4;
    uint8_t base3;
} __attribute__((packed));

struct GDTPtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct IDTEntry {
    uint16_t offset1;
    uint16_t segmentSelector;
    uint8_t ist : 3;
    uint8_t reserved0 : 5;
    uint8_t gateType : 4;
    uint8_t empty : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct GDTEntry;
struct GDTPtr;
struct IDTEntry;
struct idtr;

// and the main global kernel information
typedef struct {
    int chX; // these are the location of the next
    int chY; //   character to be placed in the terminal
    int bgColour; // framebuffer colour
    int colourOut;
    int screenWidth;
    int screenHeight;
    bool doPush; // debug: this should only be false on kernel panic
    char* last10[10]; // debug: last 10 stdio outputs
    struct largestSection largestSect; // info about location of the pmm's bitmap
    uint64_t hhdm; // limine higher half direct mapping
    uint64_t memmapEntryCount; // memory map for physical memory manager & memmap command
    struct limine_memmap_entry **memmapEntries;
    struct GDTEntry GDT[5]; // global descriptor table
    struct GDTPtr GDTR; // the pointer thingy to the GDT
    struct IDTEntry idt[256]; // the interrupt descriptor table
    struct idtr IDTPtr; 
} Kernel;

extern Kernel kernel;

#endif
