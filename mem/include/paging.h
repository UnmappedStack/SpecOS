/* Header for ../paging.c, read it's description cos I can't bother typing it here.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include "../../limine.h"

#ifndef PAGING_H
#define PAGING_H

struct pmlEntry {
    uint8_t isPresent : 1;
    uint8_t isReadOrWrite : 1;
    uint8_t isUserOrSupervisor : 1;
    uint8_t pwt : 1;
    uint8_t cacheDisable : 1;
    uint8_t accessed : 1;
    uint8_t avaliable : 1;
    uint8_t rsvdOrPageSize : 1; // rsvd if normal pml entry, page size if it's a pdpt entry or page directory entry. doesn't really matter cos it's 0 either way.
    uint8_t avaliable2 : 4;
    uint64_t address : 40;
    // think there should be reserved bits here? think again! assume M = 51.
    uint16_t available3 : 11;
    uint8_t executeDisable : 1;
} __attribute__((packed));

struct pmlEntry* initPaging();

#endif
