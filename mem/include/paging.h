/* Header for ../paging.c, read it's description cos I can't bother typing it here.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include "../../limine.h"

#ifndef PAGING_H
#define PAGING_H

uint64_t* initPaging();

void mapPage(uint64_t pml4[], uint64_t virtAddr, uint64_t flags);  

#endif
