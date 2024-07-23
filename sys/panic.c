/* Panic screen for the SpecOS kernel project.
 *
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for details.
 * This is a loose parody of the blue screen of death on Windows.
 */

#include <stdint.h>
#include "../utils/include/string.h"
#include "../drivers/include/vga.h"
#include "include/panic.h"
#include "../misc/sadface.h"
#include "../utils/include/printf.h"

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
    for (int x = 0; x < screenWidth; x++) {
        for (int y = 0; y < screenHeight; y++) {
           drawPix(x, y, 0x0000FF); 
        }
    }
    colourOut = 0xFFFFFF;
    bgColour = 0x0000FF;
    writestring("\n\n\n\n");
    // show a bitmap of the classic windows ):
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            if (bitmap[(y * 100) + x])
                drawPix(x, y, 0xFFFFFF);
            chX++;
        }
        chY++;
    }
    chY = 100;
    chX = 5;
    writestring("Your SpecPC ran into a problem and needs to be frozen. We're just doing nothing while you wonder what went wrong.");
    writestring("\n\n102% complete.");
    chY += 30;
    // and now that the laughs are done, display some debug info so the screen is actually useful
    for (int x = 0; x < screenWidth; x++)
        drawPix(x, chY, 0xFFFFFF);
    chY += 5;
    chX = 5;
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
    stackTrace(10);
    asm("cli; hlt");
}
