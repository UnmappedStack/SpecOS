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

typedef struct {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss
} TaskSwitchRegisters;


uint16_t taskSelect() {
    uint32_t maxTasks = 4096 / sizeof(Task);
    Task *tasklist = (Task*)kernel.tasklistAddr;
    for (size_t i = kernel.schedulerTurn; i < maxTasks; i++) {
        if (tasklist[i].flags & TASK_PRESENT) {
            kernel.schedulerTurn++;
            return ++i;
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

extern void pushAllRegisters();

void taskSwitch() {
    pushAllRegisters();
}

void taskSwitchPart2() {
    uint16_t taskIndex = taskSelect();
    Task task = ((Task*) kernel.tasklistAddr)[taskIndex];
    if (task.flags & TASK_FIRST_EXEC)
        allocPages((uint64_t*)(task.pml4Addr + kernel.hhdm), USER_STACK_ADDR, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER, USER_STACK_PAGES);
    KERNEL_SWITCH_PAGE_TREE((uint64_t*)(task.pml4Addr));
    asm("movq %0, %%rsp" : : "r" (task.rsp));
    if (task.flags & TASK_FIRST_EXEC) {
        // set all registers to zero
        asm volatile(
            "movq $0, %%rbp\n"
            "push $0\npush $0\n" // push zero on the stack twice
            "push $0\n" // rax
            "push $0\n" // rbx
            "push $0\n" // rcx
            "push $0\n" // rdx
            "push $0\n" // rsi
            "push $0\n" // rdi
            "push $0\n" // rbp
            "push $0\n" // r8
            "push $0\n"
            "push $0\n"
            "push $0\n" // ...
            "push $0\n"
            "push $0\n"
            "push $0\n"
            "push $0\n" // r15
            "push %0\n" // rip
            "push %1\n" // cs (offset for user code segment in GDT)
            "push $0\n" // rflags
            "push %2\n" // rsp
            "push %3" // ss (offset for user data segment in GDT)
            : : "r" (task.entryPoint), "r" ((uint64_t)(0x20 | 3)), "r" (USER_STACK_PTR), "r" ((uint64_t)(0x28 | 3))
        );
    }
    // set all registers to their value on the stack & jump
    asm("iretq");
    for(;;);
}

__attribute__((interrupt))
void taskSwitchISR(void*) {
    taskSwitch();
    outb(0x20, 0x20); // Acknowledge interrupt from master PIC
}
