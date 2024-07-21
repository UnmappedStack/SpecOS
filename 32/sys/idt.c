#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "idt.h"
#include "gdt.h"
#include "../utils/inx.h"
#include "../kernel.h"
#include "../drivers/terminalWrite.h"

#define IDT_MAX_DESCRIPTORS 32 // or whatever value you intend to use
bool vectors[IDT_MAX_DESCRIPTORS] = {false};
typedef struct {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t     reserved;     // Set to zero
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

typedef struct {
    uint16_t    limit;
    uint32_t    base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr;

void (*isr_stub_table[34])() = {
    NULL
};

void idt_set_descriptor(uint8_t vector, void (*isr)(), uint8_t flags) {
    isr_stub_table[vector] = isr;
    idt[vector].isr_low = (uint32_t)isr & 0xFFFF;
    idt[vector].kernel_cs = 0x08;
    idt[vector].reserved = 0;
    idt[vector].attributes = flags | 0x60;
    idt[vector].isr_high = ((uint32_t)isr >> 16) & 0xFFFF;
}

void isr_divide_by_zero() {
    terminal_setcolor(VGA_COLOR_RED);
    terminal_writestring("EXCEPTION: Divide by zero\n");
    __asm__("cli; hlt");
}

void isr_invalid_opcode() {
    terminal_setcolor(VGA_COLOR_RED);
    terminal_writestring("EXCEPTION: Invalid opcode\n");
    __asm__("cli; hlt");
}

void isr_page_fault() {
    terminal_setcolor(VGA_COLOR_RED);
    terminal_writestring("EXCEPTION: Page fault\n");
    __asm__("cli; hlt");
}

void isr_stub_divide_by_zero() {
    // Push error code and call the actual ISR
    asm volatile("push $0");
    asm volatile("jmp isr_divide_by_zero");
}

void isr_stub_invalid_opcode() {
    // Push error code and call the actual ISR
    asm volatile("push $0");
    asm volatile("jmp isr_invalid_opcode");
}

void isr_stub_page_fault() {
    // Push error code and call the actual ISR
    asm volatile("push $0");
    asm volatile("jmp isr_page_fault");
}

void test_helloworld_syscall() {
    terminal_writestring("\nHello, world!\n");
}

void idt_init() {
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint32_t)&idt;
    idt_set_descriptor(0, isr_divide_by_zero, 0x8E);
    idt_set_descriptor(6, isr_invalid_opcode, 0x8E);
    idt_set_descriptor(14, isr_page_fault, 0x8E);
    idt_set_descriptor(0x80, test_helloworld_syscall, 0x8E);
    __asm__("lidt %0" : : "m"(idtr));
}

void remap_PIC() {
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

void init_IRQ() {
    remap_PIC();
    idt_set_descriptor(33, isr_keyboard, 0x8E); // Map IRQ 1 (keyboard) to vector 33
    outb(0x21, ~(1 << 1)); // Unmask IRQ 1 (keyboard)
    __asm__("sti");
}
