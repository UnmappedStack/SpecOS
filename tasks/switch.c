/* Task switch for the SpecOS kernel project.
 * Incomplete file - Here be dragons!
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include "include/switch.h"
#include "../drivers/include/serial.h"
#include <stdint.h>
#include "../utils/include/io.h"

__attribute__((interrupt))
void taskSwitchISR(void*) {
    writeserial("\nTask switch [STUB!]\n");
    outb(0x20, 0x20); // Acknowledge interrupt from master PIC
}
