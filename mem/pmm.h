#include "multiboot.h"
#include <stdint.h>
#include <stddef.h>

#ifndef PMM_H
#define PMM_H

uint32_t initPMM(multiboot_info_t* mbd, uint32_t magic);

void* kmalloc(int allocSize); 

void kfree(void* location);

#endif
