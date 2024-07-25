// This is a test shell for SpecOS. While the function is called `test_userspace`, this is run in ring 0 kernel space.
// Note that this part of SpecOS will probably be gone as soon as I get apps and memory management working.
// For that reason, I don't care at all what you do with this part of the code. Steal it if you want, it's written like crap
// That doesn't apply to the rest of the project btw, I would like to be credited if you use the rest in your own OS (to the extent of the MIT license, of course)
// However, it is still under the MIT license like the rest of the project. See the github repo for more info.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/include/vga.h"
#include "drivers/include/rtc.h"
#include "utils/include/io.h"
#include "utils/include/string.h"
#include "drivers/include/keyboard.h"
#include "fs/include/api.h"
#include "include/shell.h"
#include "mem/include/detect.h"
#include "utils/include/printf.h"
#include "include/kernel.h"
#include "mem/include/pmm.h"
#include "limine.h"

void test_userspace() {
    clearScreen();
    kernel.colourOut = 0x878a87;
    // Some cool ASCII art that fIGlet totally didn't generate
    writestring(" ____                   ___  ____\n");
    writestring("/ ___| _ __   ___  ___ / _ \\/ ___|\n");
    writestring("\\___ \\| '_ \\ / _ \\/ __| | | \\___ \\\n");
    writestring(" ___) | |_) |  __/ (__| |_| |___) |\n");
    writestring("|____/| .__/ \\___|\\___|\\___/|____/\n");
    writestring("      |_|\n");
    char inp[100];
    printf("Kernel compilation date: %s at %s", __DATE__, __TIME__);
    writestring("\nSpecOS shell 2024. Type help for options.\n");
    kernel.colourOut = 0xFFFFFF;
    // Set the current directory to the root (and create the current directory object)
    struct cd currentDirectory;
    currentDirectory.path[0] = '/';
    currentDirectory.cluster = 2;
    while(1) {
        kernel.colourOut = 0x19e026;
        writestring(">> ");
        kernel.colourOut = 0xFFFFFF;
        scanf(inp);
        if (compareDifferentLengths(inp, "echo")) {
            writestring("\nArgument: ");
            scanf(inp);
            writestring("\n");
            kernel.colourOut = 0x878a87;
            writestring(inp);
            writestring("\n");
        } else if (compareDifferentLengths(inp, "timedate")) {
            writestring("\nTime: ");
            writestring(wholeTime());
            writestring("\nDate: ");
            writestring(wholeDate());
            writestring("\n");
        } else if (compareDifferentLengths(inp, "kmalloc")) {
            uint8_t* ptr;
            ptr = (uint8_t*) kmalloc();
            char buf[9];
            uint64_to_hex_string((uint64_t) ptr, buf);
            printf("\n4096 byte block dynamically allocated by the kernel: offset 0x%s\n", buf);
        } else if (compareDifferentLengths(inp, "poweroff")) {
            writestring("\nAre you sure you would like to power off your device? (y/N)");
            scanf(inp);
            if (compareDifferentLengths(inp, "y") != 0) {
                writestring("\nBeware that this only works on emulators such as Qemu, Bochs, VirtualBox, and Cloud \nHypervisor.\nIf you are running on real hardware, you may need to disconnect your device from power or press the physical power button.\nTrying to power off...\n");
                    outw(0xB004, 0x2000);
                    outw(0x604, 0x2000);
                    outw(0x4004, 0x3400);
                    outw(0x600, 0x34);
            }
            writestring("\n");
        } else if(compareDifferentLengths(inp, "reboot")) {
            // call an undefined interrupt to crash and restart the machine
            uint64_t zero = 0;
            asm("lidt %0" : : "m"(zero)); // get rid of the IDT
            asm("int $0x90");
        } else if(compareDifferentLengths(inp, "panic")) {
            asm("int $0x01"); // debug isr
        }  else if (compareDifferentLengths(inp, "clear")) {
            clearScreen();
        } else if (compareDifferentLengths(inp, "help")) { 
            writestring("\nCOMMANDS:\n - help      Shows this help menu\n - poweroff  Turns off device\n - colours   Shows device colours (colors also works)\n - timedate  Shows the current time and date\n - clear     Clears shell\n - echo      Prints to screen.\n - ls        List files\n - cd        Change directory\n - cat       Read file\n - reboot    Reboot device\n - panic     Force a blue screen of death\nSpecOS is under the MIT license. See the GitHub page for more info.\n");
        } else if (compareDifferentLengths(inp, "memmap")) {
            detectMemmap();
        } else if (compareDifferentLengths(inp, "ls")) {
            listCurrentDirectory(currentDirectory.cluster); 
        } else if (compareDifferentLengths(inp, "cd")) {
            writestring("\nArgument: ");
            scanf(inp);
            writestring("\n");
            currentDirectory = changeDirectorySingle(inp, currentDirectory); 
        } else if (compareDifferentLengths(inp, "cat")) {
            writestring("\nArgument: ");
            scanf(inp);
            writestring("\n");
            cat(currentDirectory, inp, true);
            writestring("\n");
        } else {
            kernel.colourOut = 0xff0022;
            writestring("\nCommand not found.\n");
        }
    }
}






