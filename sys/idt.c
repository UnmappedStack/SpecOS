/* 64 bit IDT implementation for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "include/panic.h"
#include "../drivers/include/keyboard.h"
#include "include/idt.h"
#include "../drivers/include/vga.h"
#include "../utils/include/io.h"
#include "include/exceptions.h"
#include "../include/kernel.h"

// and the thingies to make it do stuff

void initIRQ();

// takes: IDT vector number (eg. 0x01 for divide by 0 exception), a pointer to an ISR (aka the function it calls), & the flags
void idtSetDescriptor(uint8_t vect, void* isrThingy, uint8_t gateType, uint8_t dpl) {
    struct IDTEntry* thisEntry = &kernel.idt[vect];
    // set the thingies
    // isr offset
    thisEntry->offset1 = (uint64_t)isrThingy & 0xFFFF; // first 16 bits
    thisEntry->offset2 = ((uint64_t)isrThingy >> 16) & 0xFFFF; // next 16 bits (I think? not sure if this is wrong)
    thisEntry->offset3 = ((uint64_t)isrThingy >> 32) & 0xFFFFFFFF; // next 32 bits (again, not sure if i did that right.)
    // gdt segment
    thisEntry->segmentSelector = 0x08; // addresses kernel mode code segment in gdt
    // some "flags". idk why the wiki calls these flags tbh.
    thisEntry->ist = 0; // idk what this should be but apparently it can just be 0
    thisEntry->gateType = gateType; // Trap or interrupt gate?
    thisEntry->dpl = dpl;
    thisEntry->present = 1;
}

// define more stuff for PIC (hardware, yay!)

void remapPIC() {
    // ICW1: Start initialization of PIC
    outb(0x20, 0x11); // Master PIC
    outb(0xA0, 0x11); // Slave PIC

    // ICW2: Set interrupt vector offsets
    outb(0x21, 0x20); // Master PIC vector offset
    outb(0xA1, 0x28); // Slave PIC vector offset

    // ICW3: Tell Master PIC there is a slave PIC at IRQ2 (0000 0100)
    outb(0x21, 0x04);
    // Tell Slave PIC its cascade identity (0000 0010)
    outb(0xA1, 0x02);

    // ICW4: Set PIC to x86 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask interrupts on both PICs
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

// stuff to set it all up
void initIRQ() {
    remapPIC();
    // map some stuff
    idtSetDescriptor(33, &isr_keyboard, 14, 0); 
    outb(0x21, ~(1 << 1)); // unmask keyboard IRQ 
    // all the exceptions
    idtSetDescriptor(0, &divideErrorISR, 15, 0);
    idtSetDescriptor(1, &debugExceptionISR, 15, 0);
    idtSetDescriptor(2, &nmiInterruptISR, 15, 0);
    idtSetDescriptor(3, &breakpointISR, 15, 0);
    idtSetDescriptor(4, &overflowISR, 15, 0);
    idtSetDescriptor(5, &boundRangeExceededISR, 15, 0);
    idtSetDescriptor(6, &invalidOpcodeISR, 15, 0);
    idtSetDescriptor(7, &deviceNotAvailableISR, 15, 0);
    idtSetDescriptor(8, &doubleFaultISR, 15, 0);
    idtSetDescriptor(9, &coprocessorSegmentOverrunISR, 15, 0);
    idtSetDescriptor(10, &invalidTSSISR, 15, 0);
    idtSetDescriptor(11, &segmentNotPresentISR, 15, 0);
    idtSetDescriptor(12, &stackSegmentFaultISR, 15, 0);
    idtSetDescriptor(13, &generalProtectionFaultISR, 15, 0);
    idtSetDescriptor(14, &pageFaultISR, 15, 0);
    idtSetDescriptor(16, &x87FPUErrorISR, 15, 0);
    idtSetDescriptor(17, &alignmentCheckISR, 15, 0);
    idtSetDescriptor(18, &machineCheckISR, 15, 0);
    idtSetDescriptor(19, &simdFloatingPointExceptionISR, 15, 0);
    idtSetDescriptor(20, &virtualizationExceptionISR, 15, 0);
    idtSetDescriptor(30, &securityExceptionISR, 15, 0);
    asm("sti");
}

void initIDT() {
    writestring("\nSetting IDT descriptors..."); 
    writestring("\nCreating IDTR (that IDT pointer thingy)...");
    kernel.IDTPtr.offset = (uintptr_t)&kernel.idt[0];
    kernel.IDTPtr.size = ((uint16_t)sizeof(struct IDTEntry) *  256) - 1;
    writestring("\nLoading IDTR into the register thingy...");
    asm volatile("lidt %0" : : "m"(kernel.IDTPtr));
    writestring("\nSetting up IRQ hardware thingy...");
    initIRQ();
}






