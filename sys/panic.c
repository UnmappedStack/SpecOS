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
};

void getFunctionName() {
    struct limine_kernel_file_response kernelElfResponse = *kernelElfRequest.response;
    struct limine_file kernelFile = *kernelElfResponse.kernel_file;
    char* kernelFileStart = (char*) kernelFile.address;
    uint64_t kernelFileLength = kernelFile.size;
    char buffer[19];
    char buffer2[19];
    uint64_to_hex_string((uint64_t) kernelFileStart, buffer2);
    uint64_to_hex_string(kernelFileLength, buffer);
    buffer[18] = 0;
    printf("\nKernel address: 0x%s\nKernel size: 0x%s\n", buffer2, buffer);
    if (kernelFileStart[0] == 0x7F && kernelFileStart[1] == 'E' && kernelFileStart[2] == 'L' && kernelFileStart[3] == 'F' &&
        kernelFileStart[4] == 2) {
        // it's a valid 64 bit elf file!
        // parse the section header table
        char* sectionHeaderOffset = (char*)((uint64_t)(*(char*)&kernelFileStart[40]) & 0xFFFFFFFFFFFFFF);
        uint16_t sectionHeaderNumEntries = (kernelFileStart[60] << 8) | kernelFileStart[61];
         
    } else {
        printf(" <INVALID>");
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
    char buffer[19];
    for (unsigned int f = 0; stack && f < m; f++) {
        memset(buffer, 0, 18);
        uint64_to_hex_string(stack->rip, buffer);
        buffer[18] = '\0';
        printf(" 0x%s\n", buffer);
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
    getFunctionName();*/
    asm("cli; hlt");
}
