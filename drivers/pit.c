/* PIT driver for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include "include/pit.h"
#include "include/vga.h"
#include <stdint.h>
#include "../utils/include/io.h"

#define HERTZ_DIVIDER 1190 // this would be about 1 ms

void initPIT() {
    writestring("Initialising PIT clock...\n");
    outb(0x43, 0b110100); // set mode to rate generator, channel 0, lobyte/hibyte, binary mode
    outb(0x40, (HERTZ_DIVIDER) & 0xFF);
    outb(0x40, (HERTZ_DIVIDER >> 8) & 0xFF);
}
