/* 64 bit version of the SpecOS kernel.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "limine.h"

#include "drivers/include/serial.h"
#include "drivers/include/vga.h"
#include "sys/include/gdt.h"
#include "sys/include/idt.h"
#include "drivers/include/keyboard.h"
#include "drivers/include/rtc.h"

void _start() {
    // Just send output to a serial port to test
    init_serial();
    initVGA();
    writestring("Trying to initialise GDT...\n");
    initGDT();
    writestring("\nGDT successfully initialised! (as far as can be told. All I know is that there isn't a gpf.)");
    writestring("\n\nTrying to initialise IDT & everything related...\n");
    initIDT();
    writestring("\n\nTime: ");
    writestring(wholeTime());
    writestring("\nDate: ");
    writestring(wholeDate());
    writestring("\n\nWhat's your name?: \n");
    char buffer[100];
    scanf(buffer);
    writestring("\nHello, ");
    writestring(buffer);
    writestring("!\n");
    for (;;);
}
