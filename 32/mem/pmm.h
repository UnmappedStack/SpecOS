#include "multiboot.h"
#include <stdint.h>
#include <stddef.h>

#ifndef PMM_H
#define PMM_H

struct kmallocNode {
    bool free;
    uint32_t nextAvaliableFrame;
    uint32_t pfSize;
    uint32_t neededSize;
    _Alignas(1024) uint8_t contents[0];
};


uint32_t initPMM(multiboot_info_t* mbd, uint32_t magic);

#endif
