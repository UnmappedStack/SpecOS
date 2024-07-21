// This is a test shell for SpecOS. While the function is called `test_userspace`, this is run in ring 0 kernel space.
// Note that this part of SpecOS will probably be gone as soon as I get apps and memory management working.
// For that reason, I don't care at all what you do with this part of the code. Steal it if you want, it's written like crap
// That doesn't apply to the rest of the project btw, I would like to be credited if you use the rest in your own OS (to the extent of the MIT license, of course)
// However, it is still under the MIT license like the rest of the project. See the github repo for more info.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fs/parseBootRecord.h"
#include "drivers/terminalWrite.h"
#include "drivers/disk.h"
#include "bouncy.h"
#include "drivers/rtc.h"
#include "utils/inx.h"
#include "utils/string.h"
#include "drivers/keyboard.h"
#include "fs/readClusterChain.h"
#include "fs/decodeDirectory.h"
#include "fs/api.h"
#include "shell.h"
#include "mem/detect.h"
#include "mem/pmm.h"
#include "mem/kmalloc.h"
#include "userspace/parseElf.h"

void test_userspace(multiboot_info_t* mbd, unsigned int magic) {
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
    // Set the current directory to the root (and create the current directory object)
    struct cd currentDirectory;
    currentDirectory.path[0] = '/';
    currentDirectory.cluster = 2;
    while(1) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring(">> ");
        terminal_setcolor(VGA_COLOR_WHITE);
        scanf(inp);
        if (compareDifferentLengths(inp, "echo")) {
            terminal_writestring("\nArgument: ");
            scanf(inp);
            terminal_writestring("\n");
            terminal_setcolor(VGA_COLOR_LIGHT_GREY);
            terminal_writestring(inp);
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "memmap")) {
            detectMemmap(mbd, magic); 
        } else if (compareDifferentLengths(inp, "timedate")) {
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
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "clear")) {
            terminal_initialize();
        } else if (compareDifferentLengths(inp, "help")) { 
            terminal_writestring("\nCOMMANDS:\n - help      Shows this help menu\n - poweroff  Turns off device\n - colours   Shows device colours (colors also works)\n - timedate  Shows the current time and date\n - clear     Clears shell\n - echo      Prints to screen.\n - ls        List files\n - cd        Change directory\n - cat       Read file\nSpecOS is under the MIT license. See the GitHub page for more info.\n");
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
        } else if (compareDifferentLengths(inp, "ls")) {
            listCurrentDirectory(currentDirectory.cluster); 
        } else if (compareDifferentLengths(inp, "cd")) {
            terminal_writestring("\nArgument: ");
            scanf(inp);
            terminal_writestring("\n");
            currentDirectory = changeDirectorySingle(inp, currentDirectory); 
        } else if (compareDifferentLengths(inp, "cat")) {
            terminal_writestring("\nArgument: ");
            scanf(inp);
            cat(currentDirectory, inp, true);
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "kmalloc")) {
            uint16_t *testThingy = (uint16_t*) kmalloc(sizeof(uint16_t));
            char buffer[9];
            uint32_to_hex_string((uint32_t) testThingy, buffer);
            terminal_writestring("\nLocation dynamically provided by kernel PMM: 0x");
            terminal_writestring(buffer);
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "elf")) {
            terminal_writestring("\nArgument: ");
            scanf(inp);
            char* fileContents = cat(currentDirectory, inp, false);
            struct elfHeader header = parseElf(fileContents);
            char* buffer;
            size_t_to_str(header.isValid, buffer);
            terminal_writestring("Is valid?: ");
            terminal_writestring(buffer);
            terminal_writestring("\n");
        } else {
            terminal_setcolor(VGA_COLOR_RED);
            terminal_writestring("\nCommand not found.\n");
        }
    }
}






