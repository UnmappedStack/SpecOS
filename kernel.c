/*
SpecOS main kernel source, as part of the SpecOS complete kernel.
This is under the MIT license. See the GitHub repo for more info.
NOTE: This is an incomplete project (which may never be fully complete). Usage is at your own descretion.
This is just a hobby project, and not necessarily a good one at that.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fs/parseBootRecord.h"
#include "drivers/terminalWrite.h"
#include "drivers/disk.h"
#include "bouncy.h"
#include "drivers/rtc.h"
#include "utils/inx.h"
#include "sys/gdt.h"
#include "sys/idt.h"
#include "utils/string.h"
#include "drivers/keyboard.h"
#include "fs/readClusterChain.h"
#include "fs/decodeDirectory.h"

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

void test_userspace() {
    terminal_initialize();
    // Some cool ASCII art that fIGlet totally didn't generate
    terminal_writestring(" ____                   ___  ____\n");
    terminal_writestring("/ ___| _ __   ___  ___ / _ \\/ ___|\n");
    terminal_writestring("\\___ \\| '_ \\ / _ \\/ __| | | \\___ \\\n");
    terminal_writestring(" ___) | |_) |  __/ (__| |_| |___) |\n");
    terminal_writestring("|____/| .__/ \\___|\\___|\\___/|____/\n");
    terminal_writestring("      |_|\n");
    char inp[100];
    terminal_writestring("Kernel compilation date: ");
    terminal_writestring(__DATE__);
    terminal_writestring(" at ");
    terminal_writestring(__TIME__);
    terminal_writestring("\nSpecOS shell 2024. Type help for options.\n");
    while(1) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring(">> ");
        terminal_setcolor(VGA_COLOR_WHITE);
        scanf(inp);
        if (compareDifferentLengths(inp, "echo") != 0) {
            terminal_writestring("\nArgument: ");
            scanf(inp);
            terminal_writestring("\n");
            terminal_setcolor(VGA_COLOR_LIGHT_GREY);
            terminal_writestring(inp);
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "timedate") != 0) {
            terminal_writestring("\nTime: ");
            terminal_writestring(wholeTime());
            terminal_writestring("\nDate: ");
            terminal_writestring(wholeDate());
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "colours") || compareDifferentLengths(inp, "colors")) {
            terminal_writestring("\n");
            for (enum vga_color colour = VGA_COLOR_BLACK; colour <= VGA_COLOR_WHITE; colour++) {
                terminal_setcolor(colour);
                terminal_writestring("\x7F");
            }
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "poweroff")) {
            terminal_writestring("\nAre you sure you would like to power off your device? (y/N)");
            scanf(inp);
            if (compareDifferentLengths(inp, "y") != 0) {
                terminal_writestring("\nBeware that this only works on emulators such as Qemu, Bochs, VirtualBox, and Cloud \nHypervisor.\nIf you are running on real hardware, you may need to disconnect your device from power or press the physical power button.\nTrying to power off...\n");
                    outw(0xB004, 0x2000);
                    outw(0x604, 0x2000);
                    outw(0x4004, 0x3400);
                    outw(0x600, 0x34);
            }
        } else if (compareDifferentLengths(inp, "clear")) {
            terminal_initialize();
        } else if (compareDifferentLengths(inp, "help")) {
            terminal_initialize();
            terminal_writestring("COMMANDS:\n - help      Shows this help menu\n - poweroff  Turns off device\n - colours   Shows device colours (colors also works)\n - timedate  Shows the current time and date\n - clear     Clears shell\n - echo      Prints to screen.\n\nSpecOS is under the MIT license. See the GitHub page for more info.\n");
        } else if (compareDifferentLengths(inp, "readsect")) { 
            // NOTE: This is a debug command. It's not in the help list because it's not meant to be used until the feature is complete.
            terminal_writestring("\nTrying to read from sector 2048...\n");
            char* result = readdisk(2048);
            terminal_writestring("Successful read! Contents:\n");
            // Print each character in the sector
            for (int i = 0; i < 512; i++) {
                terminal_writestring(charToStr(result[i]));
            }
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "writesect")) {
            // NOTE: Like the previous function, this is a debug command.
            terminal_writestring("\nTrying to write to sector 30...\n");
            char buffer[512] = "SUCCESS!";
            writedisk(30, buffer);
            terminal_writestring("Successful write! Try reading sector 30 to test.\n");
        } else if (compareDifferentLengths(inp, "bouncy")) {
            bouncy();
        } else if (compareDifferentLengths(inp, "fstype")) {
            terminal_writestring("\n");
            terminal_writestring(readBoot().fileSysType);
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "sectorspercluster")) {
            terminal_writestring("\n");
            uint16_t bytesPerSect = readBoot().sectPerClust;
            char buffer[6];
            uint16_to_string(bytesPerSect, buffer);
            terminal_writestring(buffer);
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "rootsect")) {
            terminal_writestring("\n");
            uint32_t rootSect = getFirstSectorOfCluster(2);
            char* rootContents = readdisk(rootSect);
            struct directoryEntry buffer[10];
            parseDirectory(rootContents, buffer);
            for (int i = 0; i < 8; i++) {
                terminal_writestring(charToStr(buffer[1].filename[i]));
            }
            char* flagBuffer;
            size_t_to_str(buffer[1].minutesCreated, flagBuffer);
            terminal_writestring("\nMinute created: ");
            terminal_writestring(flagBuffer);
            terminal_writestring("\n");
        } else {
            terminal_setcolor(VGA_COLOR_RED);
            terminal_writestring("\nCommand not found.\n");
        }
    }
}


void init_kernel() {
    terminal_initialize();
    hide_vga_cursor();
    terminal_setcolor(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("Successful boot!\n");
    terminal_writestring("\n\n");
    terminal_writestring("Initialising GDT...\n");
    init_gdt();
    terminal_writestring("Initialising IDT...\n");
    idt_init();
    terminal_writestring("Initialising IRQs...\n");
    init_IRQ();
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
    test_userspace();
    terminal_initialize();
    terminal_setcolor(VGA_COLOR_RED);
    terminal_writestring("KERNEL PANIC: All userspace applications halted! Nothing to run. Freezing device.\n");
    __asm__("cli; hlt");
}

void kernel_main() {
    init_kernel();
}
