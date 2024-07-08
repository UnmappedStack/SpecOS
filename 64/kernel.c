/* 64 bit version of the SpecOS kernel.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "limine.h"

#include "drivers/include/serial.h"
#include "drivers/include/vga.h"

void _start() {
    // Just send output to a serial port to test
    init_serial();
    outCharSerial('(');
    outCharSerial(';');
    initVGA();
    writestring("That's right, SpecOS's new graphics mode can now:\n - Draw text\n - Clear the screen\n - Scroll (very badly, it currently just clears the screen whenever it gets too full)");
    writestring("Tbh I'm now fucking sick of VGA graphics, time to work on a 64 bit version of a\nGDT and IDT :)");
    for (;;);
}
