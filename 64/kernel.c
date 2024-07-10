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

void _start() {
    // Just send output to a serial port to test
    init_serial();
    outCharSerial('(');
    outCharSerial(';');
    initVGA();
    writestring("Trying to initialise GDT...\n");
    initGDT();
    writestring("\nGDT successfully initialised! (as far as can be told. All I know is that there isn't a gpf.)");
    writestring("\nTrying to initialise IDT & everything related...");
    initIDT();
    writestring("\nOkaayyyy, there wasn't a fault. Test to see if calling an interrupt will do stuff...");
    asm("int $0x80");
    for (;;);
}
