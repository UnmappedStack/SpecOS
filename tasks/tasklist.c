/* Task list for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include "include/tasklist.h"
#include <stdint.h>
#include <stddef.h>
#include "../kernel/include/kernel.h"
#include "../mem/include/paging.h"
#include "../utils/include/printf.h"
#include "../mem/include/pmm.h"

void initTaskList() {
    kernel.tasklistAddr = (uintptr_t)(kmalloc() + kernel.hhdm);
    Task *tasklist = (Task*)kernel.tasklistAddr;
    tasklist[0].PID = 0;
    tasklist[0].pml4Addr = (uintptr_t)(kernel.pml4);
    tasklist[0].rsp = KERNEL_STACK_PTR;
    tasklist[0].flags = TASK_PRESENT | TASK_RUNNING;
    tasklist[0].entryPoint = 0;
    kernel.tasklistUpto = 1;
}

// returns PID of new process
uint16_t initTask() {
    uint32_t maxTasks = 4096 / sizeof(Task);
    if (kernel.tasklistUpto > maxTasks) {
        printf("\n\n[KERNEL PANIC]: Can't spawn new process - too many tasks have already been created.\nHalting device.");
        asm("cli; hlt");
    }
    Task *tasklist = (Task*)kernel.tasklistAddr;
    printf("Task list up to: %i\n", kernel.tasklistUpto);
    tasklist[kernel.tasklistUpto].PID = kernel.tasklistUpto;
    tasklist[kernel.tasklistUpto].pml4Addr = 0;    // <-|
    tasklist[kernel.tasklistUpto].rsp = 0;         // <-|- will be set by `exec`
    tasklist[kernel.tasklistUpto].entryPoint = 0;  // <-|
    tasklist[kernel.tasklistUpto].flags = TASK_PRESENT | TASK_FIRST_EXEC;
    /* look for the next non-present task and set tasklistUpto to it's PID
     * this can't just increment tasklistUpto, because of the way processes are terminated.
     */
    uint16_t prevUpto = kernel.tasklistUpto;
    for (uint16_t p = 0; p < maxTasks; p++)
        if (!(tasklist[p].flags & 1)) kernel.tasklistUpto = p;
    return prevUpto;
}

void endProcess(uint16_t PID) {
    Task *tasklist = (Task*)kernel.tasklistAddr;
    tasklist[PID].flags = 0; // since PRESENT flag isn't set, it will never be run again
    kernel.tasklistUpto = PID;
}



















