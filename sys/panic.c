/* Panic screen for the SpecOS kernel project.
 *
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for details.
 * This is a loose parody of the blue screen of death on Windows.
 */

#include <stdbool.h>
#include <stdint.h>
#include "../include/kernel.h"
#include "../utils/include/string.h"
#include "../drivers/include/vga.h"
#include "include/panic.h"
#include "../misc/sadface.h"
#include "../utils/include/printf.h"
#include "../drivers/include/vga.h"
#include "../limine.h"

// turn a stack address into the name of a function (fancy, ik)
__attribute__((used, section(".requests")))
static volatile struct limine_kernel_file_request kernelElfRequest = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0
};

struct elfSectionHeader {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addressAlign;
    uint64_t entrySize;
} __attribute__ ((packed));

struct symtabEntry {
    uint32_t name;
    uint8_t info;
    uint8_t other;
    uint16_t shndx;
    uint64_t value;
    uint64_t size;
};// __attribute__((packed));

void assert(bool condition) {
    if (condition) {
        writestring("\nASSERT: True\n");
    } else {
        writestring("\nFailed assert.\n");
        asm("cli; hlt");
    }
}

void getFunctionName(uint64_t address) {
    struct limine_kernel_file_response kernelElfResponse = *kernelElfRequest.response;
    struct limine_file kernelFile = *kernelElfResponse.kernel_file;
    char* kernelFileStart = (char*) kernelFile.address;
    uint64_t kernelFileLength = kernelFile.size;
    if (kernelFileStart[0] == 0x7F && kernelFileStart[1] == 'E' && kernelFileStart[2] == 'L' && kernelFileStart[3] == 'F' &&
        kernelFileStart[4] == 2) {
        // it's a valid 64 bit elf file!
        // parse the section header table
        struct elfSectionHeader *sectionHeaderOffset = (struct elfSectionHeader*)(*(uint64_t*)((kernelFileStart + 40)) + ((uint64_t)kernelFileStart));
        uint16_t sectionHeaderNumEntries = *(uint16_t*)(kernelFileStart + 60);
        // parse the section header string table
        uint16_t sectionHeaderStringTableIndex = *(uint16_t*)(kernelFileStart + 62);
        char* sectionHeaderStringTableOffset = (char*)(sectionHeaderOffset[sectionHeaderStringTableIndex].offset + ((uint64_t)kernelFileStart));
        // now look through each section header entry, comparing the name, and find the offset of .strtab and .symtab
        struct symtabEntry *symtabOffset = 0;
        char* strtabOffset = 0;
        uint64_t symtabIndex = 0;
        for (int i = 0; i < sectionHeaderNumEntries; i++) {
            // check if it's .symtab or .strtab by comparing it's string in the section header string table
            if (strcmp(&sectionHeaderStringTableOffset[sectionHeaderOffset[i].name], ".strtab")) {
                strtabOffset = (char*)(sectionHeaderOffset[i].offset + (uint64_t)kernelFileStart);
            }
            if (strcmp(&sectionHeaderStringTableOffset[sectionHeaderOffset[i].name], ".symtab")) {
                symtabOffset = (struct symtabEntry*)(sectionHeaderOffset[i].offset + (uint64_t)kernelFileStart);
                symtabIndex = i;
            }
        }
        // now go through .symtab, looking for an entry who's address is equal to `address`
        for (int i = 0; i < sectionHeaderOffset[symtabIndex].size / sizeof(struct symtabEntry); i++) {
            if (address >= symtabOffset[i].value && address < symtabOffset[i].value + symtabOffset[i].size) {
                // found! parse the string table to find the actual thingy
                printf("   %s\n", (&strtabOffset[symtabOffset[i].name]));
                return;
            }
        } 
        // if it got to this point, it's invalid.
        printf("  <INVALID>\n");
    } else {
        printf("  <INVALID>");
    }
} 

struct stackFrame {
    struct stackFrame* rbp;
    uint64_t rip;
};

void stackTrace(int m) {
    writestring("\n\n==== Stack Trace: ====\n\n"); // 22
    struct stackFrame *stack;
    asm("mov %%rbp, %0" : "=r"(stack));
    for (unsigned int f = 0; stack && f < m; f++) {
        printf(" 0x%x", stack->rip);
        if (stack->rip)
            getFunctionName(stack->rip);
        else
            writestring("\n");
        stack = stack->rbp;
    }
}

void kpanic(char* exception) {
    kernel.doPush = false;
    for (int x = 0; x < kernel.screenWidth; x++) {
        for (int y = 0; y < kernel.screenHeight; y++) {
           drawPix(x, y, 0x0000FF); 
        }
    }
    kernel.colourOut = 0xFFFFFF;
    kernel.bgColour = 0x0000FF;
    writestring("\n\n\n\n");
    // show a bitmap of the classic windows ):
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            if (bitmap[(y * 100) + x])
                drawPix(x, y, 0xFFFFFF);
            kernel.chX++;
        }
        kernel.chY++;
    }
    kernel.chY = 100;
    kernel.chX = 5;
    writestring("Your SpecPC ran into a problem and needs to be frozen. We're just doing nothing while you wonder what went wrong.");
    writestring("\n\n102% complete.");
    kernel.chY += 30;
    // and now that the laughs are done, display some debug info so the screen is actually useful
    for (int x = 0; x < kernel.screenWidth; x++)
        drawPix(x, kernel.chY, 0xFFFFFF);
    kernel.chY += 5;
    kernel.chX = 5;
    writestring("============= DEBUG ============="); // 24
    printf("\n\n Fault: %s", exception);
    char bufferGDTR[19];
    char bufferIDTR[19];
    char bufferCR3[19];
    char bufferCR2[19];
    uint64_t gdtrVal;
    uint64_t idtrVal;
    uint64_t cr3Val;
    uint64_t cr2Val;
    asm("sgdt %0" : "=m"(gdtrVal));
    asm("sidt %0" : "=m"(idtrVal));
    asm("mov %%cr3, %0" : "=r"(cr3Val));
    asm("mov %%cr2, %0" : "=r"(cr2Val));
    uint64_to_hex_string(gdtrVal, bufferGDTR);
    bufferGDTR[18] = '\0';
    uint64_to_hex_string(idtrVal, bufferIDTR);
    bufferIDTR[18] = '\0';
    uint64_to_hex_string(cr3Val, bufferCR3);
    bufferCR3[18] = '\0';
    uint64_to_hex_string(cr2Val, bufferCR2);
    writestring("\n\n=== Register Dump: ===\n\n GDTR: 0x"); // 22
    writestring(bufferGDTR);
    writestring("\n IDTR: 0x");
    writestring(bufferIDTR);
    writestring("\n CR3: 0x");
    writestring(bufferCR3);
    writestring("\n CR2: 0x");
    writestring(bufferCR2);
    stackTrace(10);/*
    writestring("\n\n== Last 10 stdio outputs: ==\n\n");
    for (int i = 0; i < 10; i++)
        writestring(kernel.last10[i]);
    */
     
    asm("cli; hlt");
}
