/* TSS setup for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include "include/tss.h"
#include "../include/kernel.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../mem/include/paging.h"

void initTSS() {
    kernel.tss.rsp0 = KERNEL_STACK_PTR;
}
