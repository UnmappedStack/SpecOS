#include "multiboot.h"
#include <stdint.h>
#include <stddef.h>

#ifndef PMM_H
#define PMM_H

void initPMM(multiboot_info_t* mbd, uint32_t magic);

#endif
