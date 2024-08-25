/* Header for ../paging.c, read it's description cos I can't bother typing it here.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include "../../limine.h"

#ifndef PAGING_H
#define PAGING_H

#define KERNEL_PFLAG_PRESENT 0b1
#define KERNEL_PFLAG_WRITE   0b10
#define KERNEL_PFLAG_USER    0b100
#define KERNEL_PFLAG_PXD     0b10000000000000000000000000000000000000000000000000000000000000 // a bit long lmao

#define PAGE_SIZE 4096
#define KERNEL_STACK_PAGES 2LL
#define KERNEL_STACK_PTR 0xFFFFFFFFFFFFF000LL
#define KERNEL_STACK_ADDR KERNEL_STACK_PTR-(KERNEL_STACK_PAGES*PAGE_SIZE)

#define PAGE_ALIGN_DOWN(addr) ((addr / 4096) * 4096) // works cos of integer division
#define PAGE_ALIGN_UP(x) ((((x) + 4095) / 4096) * 4096)

uint64_t* initPaging();

void mapPages(uint64_t pml4[], uint64_t virtAddr, uint64_t physAddr, uint64_t flags, uint64_t numPages);

void allocPages(uint64_t pml4[], uint64_t virtAddr, uint64_t flags, uint64_t numPages);

#endif
