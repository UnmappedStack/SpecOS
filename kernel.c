/*
SpecOS main kernel source, as part of the SpecOS complete kernel.
This is under the MIT license. See the GitHub repo for more info.
NOTE: This is an incomplete project (which may never be fully complete). Usage is at your own descretion.
This is just a hobby project, and not necessarily a good one at that.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/terminalWrite.h"
#include "sys/gdt.h"
#include "sys/idt.h"
#include "shell.h"
#include "drivers/disk.h"
#include "utils/string.h"
#include "mem/detect.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "drivers/serial.h"

void dummy_test_entrypoint() {
}

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler. Please make sure that you use the correct compile_all.sh file in the GitHub repo to compile."
#endif

/* This will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "Must be compiled with an ix86-elf compiler."
#endif

uint32_t firstPageFrame;

void init_kernel(multiboot_info_t* mbd, unsigned int magic) {
    __asm__ ("cli");
    terminal_initialize();
    hide_vga_cursor();
    terminal_setcolor(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("Successful boot!\n");
    serial_writestring("Hello world from serial!\nNew lines are supported by default (:");
    terminal_writestring("\n\n");
    terminal_writestring("Initialising GDT...\n");
    init_gdt();
    terminal_writestring("Initialising IDT...\n");
    idt_init();
    terminal_writestring("Initialising IRQs...\n");
    init_IRQ();
    terminal_writestring("Initialising physical memory manager...\n");
    firstPageFrame = initPMM(mbd, magic);
    terminal_writestring("Initialising virtual memory manager...\n");
    initPaging(0, 1024);
    terminal_writestring("Initialising drive...\n");
    if (!identifyCompatibility()) {
        terminal_set_bg(VGA_COLOR_BLACK);
        terminal_initialize();
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("\n\n\n\n\n");
        char* errStr1 = "BOOT ERROR: Can't initialise drive.";
        char* errStr2 = "Make sure you have an ATA PIO mode compatible disk.";
        terminal_column = VGA_WIDTH / 2 - strlen(errStr1) / 2;
        for (int i = 0; i < strlen(errStr1); i++) {
           terminal_putchar(errStr1[i]);
        }
        terminal_writestring("\n");
        terminal_column = VGA_WIDTH / 2 - strlen(errStr2) / 2;
        for (int i = 0; i < strlen(errStr2); i++) {
            terminal_putchar(errStr2[i]);
        }
        terminal_setcolor(VGA_COLOR_DARK_GREY);
        allow_scroll = false;
        terminal_row = VGA_HEIGHT - 2;
        terminal_column = 0;
        terminal_writestring("IF YOU ARE USING QEMU...\nMake sure you use the -hda option when running.");
        __asm__("cli; hlt");
    } 
    test_userspace(mbd, magic);
    terminal_initialize();
    terminal_setcolor(VGA_COLOR_RED);
    terminal_writestring("KERNEL PANIC: All userspace applications halted!\nNothing to run. Freezing device.\n");
    __asm__("cli; hlt");
}

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
    init_kernel(mbd, magic);
}
