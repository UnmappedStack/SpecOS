/* Header for ../tasklist.c
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <stdint.h>

#ifndef TASKLIST_H
#define TASKLIST_H

#define TASK_PRESENT    0b001
#define TASK_FIRST_EXEC 0b010
#define TASK_RUNNING    0b100

typedef struct {
    uint16_t PID;
    uintptr_t pml4Addr;
    uintptr_t rip;
    uint8_t flags;
} Task;

void initTaskList();

uint16_t initTask();

#endif
