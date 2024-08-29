/* This file is responsible for some pretty important shit.
 * It creates a new task in the task list, parses elf files, and basically just initiates a program so it's ready to run.
 * Uhhh also known as an `exec` function.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../utils/include/string.h"
#include "include/exec.h"
#include "../mem/include/pmm.h"
#include "../mem/include/paging.h"
#include "include/tasklist.h"
#include "../utils/include/printf.h"
#include "../kernel/include/kernel.h"

struct elfFileHeader {
    char id[16];
    uint16_t type;
    uint16_t machineType;
    uint32_t version;
    uint64_t entry;
    uint64_t programHeaderOffset;
    uint64_t sectionHeaderOffset;
    uint32_t flags;
    uint16_t headerSize;
    uint16_t programHeaderEntrySize;
    uint16_t programHeaderEntryCount;
    uint16_t sectionHeaderEntrySize;
    uint16_t sectionHeaderEntryCount;
    uint16_t sectionNameStringTableIndex;
} __attribute__ ((packed));

struct elfProgramHeader {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t virtualAddress;
    uint64_t rsvd;
    uint64_t sizeInFile;
    uint64_t sizeInMemory;
    uint64_t align;
} __attribute__ ((packed));

bool checkValidExecutable(struct elfFileHeader fileHeader) {
    // check elf magic number
    char elfID[5];
    memcpy(elfID, fileHeader.id, 4);
    elfID[4] = 0;
    if (!strcmp("\x7f" "ELF", elfID)) {
        printf("\nFailed to run executable: Invalid ELF file.\n");
        return false;
    }
    // check endianess (little endian only for now)
    if (fileHeader.id[5] != 1) {
        printf("\nIncorrect endianness: Elf must be little endian, but appears to be in big endian format.\n");
        return false;
    }
    // check it's 64 bit (no backwards compat yet, cos it has a slightly different format)
    if (fileHeader.id[4] != 2) {
        printf("\nFailed to run executable: Elf must be 64 bit, but appears to be a 32 bit program.\n");
        return false;
    }
    printf("\nValid executable.\n");
    return true;
}

// returns index
size_t firstLoadableSegment(struct elfProgramHeader *programHeaderEntries, size_t numEntries) {
    size_t    smallestIndex;
    uintptr_t smallestAddr;
    bool      isFirstLoadable;
    for (size_t i = 0; i < numEntries; i++) {
        if (programHeaderEntries[i].type == 1) { // loadable
            if (programHeaderEntries[i].virtualAddress < smallestAddr || isFirstLoadable) {
                smallestIndex = i;
                smallestAddr = programHeaderEntries[i].virtualAddress;
                isFirstLoadable = false;
            }
        }
    }
    return smallestIndex;
}

size_t lastLoadableSegment(struct elfProgramHeader *programHeaderEntries, size_t numEntries) {
    size_t    largestIndex = 0;
    uintptr_t largestAddr = 0;
    for (size_t i = 0; i < numEntries; i++) {
        if (programHeaderEntries[i].type == 1) { // loadable
            if (programHeaderEntries[i].virtualAddress > largestAddr) {
                largestIndex = i;
                largestAddr = programHeaderEntries[i].virtualAddress;
            }
        }
    }
    return largestIndex;
}

// returns status code
int exec(uintptr_t elfAddr) {
    struct elfFileHeader fileHeader = *((struct elfFileHeader*)(elfAddr));
    if (!checkValidExecutable(fileHeader)) return 1;
    struct elfProgramHeader *programHeaderEntries = (struct elfProgramHeader*)(fileHeader.programHeaderOffset + elfAddr);
    size_t numProgramHeaderEntries = fileHeader.programHeaderEntryCount;
    for (size_t i = 0; i < numProgramHeaderEntries; i++) {
        if (programHeaderEntries[i].type == 1) {// loadable
            printf("Section %i | Data offset: 0x%x | Load to vaddr: 0x%x | Size (file): 0x%x | Size (memory): 0x%x | Type: %i\n",
                i, programHeaderEntries[i].offset, programHeaderEntries[i].virtualAddress, programHeaderEntries[i].sizeInFile, programHeaderEntries[i].sizeInMemory, programHeaderEntries[i].type);
        }
    }
    struct elfProgramHeader firstSegment = programHeaderEntries[firstLoadableSegment(programHeaderEntries, numProgramHeaderEntries)];
    struct elfProgramHeader lastSegment  = programHeaderEntries[lastLoadableSegment(programHeaderEntries, numProgramHeaderEntries)];
    uint64_t pagesToMap = PAGE_ALIGN_UP((lastSegment.virtualAddress + lastSegment.sizeInMemory) - firstSegment.virtualAddress) / 4096;
    uintptr_t physFirstPage = (uintptr_t)kmalloc();
    for (int i = 0; i < pagesToMap - 1; i++)
        kmalloc();
    // actually copy the data over
    for (size_t s = 0; s < numProgramHeaderEntries; s++) {
        uintptr_t copyFrom = programHeaderEntries[s].offset + elfAddr;
        uintptr_t copyTo   = (physFirstPage + kernel.hhdm) + (programHeaderEntries[s].virtualAddress - firstSegment.virtualAddress);
        memcpy((char*)copyTo, (char*)copyFrom, programHeaderEntries[s].sizeInFile);
    }
    // create new pml4 & map into vmem
    uint64_t *newPageTree = initPaging(false);
    mapPages((uint64_t*)(((uint64_t)(newPageTree)) + kernel.hhdm), firstSegment.virtualAddress, (uint64_t)physFirstPage, KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER | KERNEL_PFLAG_PRESENT, pagesToMap);
    // set up a new stack
    int numUserspaceStackPages = 2;
    uintptr_t userspaceStackBase = kernel.hhdm - 4096;
    uintptr_t userspaceStackAddr = userspaceStackBase - numUserspaceStackPages;
    allocPages((uint64_t*)(((uint64_t)(newPageTree)) + kernel.hhdm), userspaceStackAddr, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER, numUserspaceStackPages);
    // create a new task list entry
    size_t taskIndex = initTask();
    Task *tasklist = (Task*)kernel.tasklistAddr;
    tasklist[taskIndex].pml4Addr   = (uintptr_t)newPageTree;
    tasklist[taskIndex].entryPoint = (uintptr_t)fileHeader.entry;
    tasklist[taskIndex].rsp        =            userspaceStackAddr;
    return 0;
}

void runModuleElf(int moduleNum) {
    struct limine_file **modules = kernel.moduleFiles.modules;
    struct limine_file module1 = (*modules)[moduleNum];
    printf("\nRunning elf file: %s\n", module1.path);
    int status = exec((uintptr_t)(module1.address));
    printf("Process returned with status %i.\n", status);
}















