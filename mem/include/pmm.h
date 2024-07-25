/* Header for ../pmm.c, the allocator for the SpecOS kernel.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include "../../limine.h"

#ifndef PMM_H
#define PMM_H

void initPMM();

void* kmalloc();

void kfree(void* location);

#endif
