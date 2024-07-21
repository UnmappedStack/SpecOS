#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gdt.h"

// Segment descriptor structure
struct gdt_entry {
    uint16_t limit_low;     // The lower 16 bits of the limit
    uint16_t base_low;      // The lower 16 bits of the base
    uint8_t base_middle;    // The next 8 bits of the base
    uint8_t access;         // Access flags
    uint8_t granularity;
    uint8_t base_high;      // The last 8 bits of the base
} __attribute__((packed));

// Pointer to the GDT structure
struct gdt_ptr {
    uint16_t limit;         // The upper 16 bits of all selector limits
    uint32_t base;          // The address of the first gdt_entry_t struct
} __attribute__((packed));

// GDT array
struct gdt_entry gdt[3];

// GDT pointer
struct gdt_ptr gp;

// Function to set up a GDT entry
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    // Set the base address
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    // Set the limit
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    // Set flags
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

// Function to load the GDT
static inline void gdt_load() {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    // Load the GDT
    asm volatile("lgdt (%0)" : : "r"(&gp));
    asm volatile("mov $0x10, %%ax; \
                  mov %%ax, %%ds; \
                  mov %%ax, %%es; \
                  mov %%ax, %%fs; \
                  mov %%ax, %%gs; \
                  ljmp $0x08, $next; \
                  next:": : : "eax");
}

void init_gdt() {
    // Null segment
    gdt_set_gate(0, 0, 0, 0, 0);

    // Code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // 0x9A = present, ring 0, code segment
                                                // 0xCF = 4 KB granularity, 32-bit mode, limit in 4 KB blocks

    // Data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // 0x92 = present, ring 0, data segment
                                                // 0xCF = 4 KB granularity, 32-bit mode, limit in 4 KB blocks

    // Load the GDT
    gdt_load();
}
