/* Task switch for the SpecOS kernel project.
 * Incomplete file - Here be dragons!
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include "include/switch.h"
#include "../drivers/include/serial.h"
#include <stdint.h>
#include "../utils/include/io.h"
#include "include/tasklist.h"
#include "../drivers/include/vga.h"
#include "../utils/include/printf.h"
#include <stddef.h>
#include "../kernel/include/kernel.h"

uint16_t taskSelect() {
    uint32_t maxTasks = 4096 / sizeof(Task);
    Task *tasklist = (Task*)kernel.tasklistAddr;
    for (size_t i = kernel.schedulerTurn; i < maxTasks; i++) {
        if (tasklist[i].flags & TASK_PRESENT) {
            kernel.schedulerTurn++;
            return i++;
        }
    }
    /* no more seem to be present.
     *  - If tasklist is larger than 1, go back to index one.
     *  - otherwise, fail, as there are no more userspace applications.
     */
    if (kernel.tasklistUpto >= 1) {
        // loop back around
        kernel.schedulerTurn = 2;
        return 1;
    }
    writestring("\n\nAll userspace applications halted, nothing left to do.\nHalting device.");
    asm("cli; hlt");
    return 0; // meaningless return value to make the compiler happy
}

void taskSwitch() {
    writeserial("\nTask switch\n");
    uint16_t task = taskSelect();
    printf("\nTask selected: %i\n", task);
}

__attribute__((interrupt))
void taskSwitchISR(void*) {
    taskSwitch();
    outb(0x20, 0x20); // Acknowledge interrupt from master PIC
}
