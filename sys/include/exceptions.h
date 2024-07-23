#include "panic.h"
#include "../../drivers/include/vga.h"

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

__attribute__((interrupt))
void divideErrorISR(void*) {
    writestring("zero divide");
    kpanic("Divide Error");
}

__attribute__((interrupt))
void debugExceptionISR(void*) {
    kpanic("Debug Exception");
}

__attribute__((interrupt))
void nmiInterruptISR(void*) {
    kpanic("NMI Interrupt");
}

__attribute__((interrupt))
void breakpointISR(void*) {
    kpanic("Breakpoint");
}

__attribute__((interrupt))
void overflowISR(void*) {
    kpanic("Overflow");
}

__attribute__((interrupt))
void boundRangeExceededISR(void*) {
    kpanic("Bound Range Exceeded");
}

__attribute__((interrupt))
void invalidOpcodeISR(void*) {
    writestring("bad opcode");
    kpanic("Invalid Opcode");
}

__attribute__((interrupt))
void deviceNotAvailableISR(void*) {
    kpanic("Device Not Available");
}

__attribute__((interrupt))
void doubleFaultISR(void*) {
    kpanic("Double Fault");
}

__attribute__((interrupt))
void coprocessorSegmentOverrunISR(void*) {
    kpanic("Coprocessor Segment Overrun");
}

__attribute__((interrupt))
void invalidTSSISR(void*) {
    kpanic("Invalid TSS");
}

__attribute__((interrupt))
void segmentNotPresentISR(void*) {
    kpanic("Segment Not Present");
}

__attribute__((interrupt))
void stackSegmentFaultISR(void*) {
    kpanic("Stack Segment Fault");
}

__attribute__((interrupt))
void generalProtectionFaultISR(void*) {
    kpanic("General Protection Fault");
}

__attribute__((interrupt))
void pageFaultISR(void*) {
    kpanic("Page Fault");
}

__attribute__((interrupt))
void x87FPUErrorISR(void*) {
    kpanic("x87 FPU Error");
}

__attribute__((interrupt))
void alignmentCheckISR(void*) {
    kpanic("Alignment Check");
}

__attribute__((interrupt))
void machineCheckISR(void*) {
    kpanic("Machine Check");
}

__attribute__((interrupt))
void simdFloatingPointExceptionISR(void*) {
    kpanic("SIMD Floating Point Exception");
}

__attribute__((interrupt))
void virtualizationExceptionISR(void*) {
    kpanic("Virtualization Exception");
}

__attribute__((interrupt))
void securityExceptionISR(void*) {
    kpanic("Security Exception");
}



#endif
