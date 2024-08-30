/* 64 bit IDT implementation for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../mem/include/pmm.h"
#include "include/panic.h"
#include "../tasks/include/switch.h"
#include "../drivers/include/keyboard.h"
#include "include/idt.h"
#include "../drivers/include/vga.h"
#include "../utils/include/io.h"
#include "../kernel/include/kernel.h"
#include "../utils/include/printf.h"

// and the thingies to make it do stuff

void initIRQ();

// takes: IDT vector number (eg. 0x01 for divide by 0 exception), a pointer to an ISR (aka the function it calls), & the flags
void idtSetDescriptor(uint8_t vect, void* isrThingy, uint8_t gateType, uint8_t dpl, struct IDTEntry *IDTAddr) {
    struct IDTEntry* thisEntry = &IDTAddr[vect];
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


extern void divideException();
extern void debugException();
extern void breakpointException();
extern void overflowException();
extern void boundRangeExceededException();
extern void invalidOpcodeException();
extern void deviceNotAvaliableException();
extern void doubleFaultException();
extern void coprocessorSegmentOverrunException();
extern void invalidTSSException();
extern void segmentNotPresentException();
extern void stackSegmentFaultException();
extern void generalProtectionFaultException();
extern void pageFaultException();
extern void floatingPointException();
extern void alignmentCheckException(); 
extern void machineCheckException();
extern void simdFloatingPointException();
extern void virtualisationException();

void unmaskIRQ(int IRQ) {
    if (IRQ < 8)
        outb(0x21, ~(1 << (IRQ % 8)));
    else
        outb(0xA1, ~(1 << (IRQ % 8)));
}

void maskIRQ(int IRQ) {
    if (IRQ < 8)
        outb(0x21, (1 << (IRQ % 8)));
    else
        outb(0xA1, (1 << (IRQ % 8)));
}

void initIRQ(struct IDTEntry *IDTAddr) {
    remapPIC();
    // map some stuff
    idtSetDescriptor(33, &isr_keyboard, 14, 0, IDTAddr);
    idtSetDescriptor(32, &taskSwitchISR, 14, 0, IDTAddr);
    unmaskIRQ(1); // keyboard
    //unmaskIRQ(0); // pit
    // all the exceptions
    idtSetDescriptor(0, &divideException, 15, 0, IDTAddr);
    idtSetDescriptor(1, &debugException, 15, 0, IDTAddr);
    idtSetDescriptor(3, &breakpointException, 15, 0, IDTAddr);
    idtSetDescriptor(4, &overflowException, 15, 0, IDTAddr);
    idtSetDescriptor(5, &boundRangeExceededException, 15, 0, IDTAddr);
    idtSetDescriptor(6, &invalidOpcodeException, 15, 0, IDTAddr);
    idtSetDescriptor(7, &deviceNotAvaliableException, 15, 0, IDTAddr);
    idtSetDescriptor(8, &doubleFaultException, 15, 0, IDTAddr);
    idtSetDescriptor(9, &coprocessorSegmentOverrunException, 15, 0, IDTAddr);
    idtSetDescriptor(10, &invalidTSSException, 15, 0, IDTAddr);
    idtSetDescriptor(11, &segmentNotPresentException, 15, 0, IDTAddr);
    idtSetDescriptor(12, &stackSegmentFaultException, 15, 0, IDTAddr);
    idtSetDescriptor(13, &generalProtectionFaultException, 15, 0, IDTAddr);
    idtSetDescriptor(14, &pageFaultException, 15, 0, IDTAddr);
    idtSetDescriptor(16, &floatingPointException, 15, 0, IDTAddr);
    idtSetDescriptor(17, &alignmentCheckException, 15, 0, IDTAddr);
    idtSetDescriptor(18, &machineCheckException, 15, 0, IDTAddr);
    idtSetDescriptor(19, &simdFloatingPointException, 15, 0, IDTAddr);
    idtSetDescriptor(20, &virtualisationException, 15, 0, IDTAddr);
}

void initIDT() {
    struct IDTEntry *IDTAddr = (struct IDTEntry*) (kmalloc() + kernel.hhdm);
    kernel.IDTPtr.offset = (uintptr_t)IDTAddr;
    kernel.IDTPtr.size = ((uint16_t)sizeof(struct IDTEntry) *  256) - 1;
    asm volatile("lidt %0" : : "m"(kernel.IDTPtr));
    initIRQ(IDTAddr);
}






