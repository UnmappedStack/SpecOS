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
    writestring("Graphics mode string writing works!"); 
    writestring("\nYay!");
    for (;;);
}
