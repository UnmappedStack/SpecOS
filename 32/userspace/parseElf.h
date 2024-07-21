/* Header file for parseElf.c, used for parsing ELF files so that applications can be run.
 * This is part of the SpecOS kernel project (https://github.com/jakeSteinburger/SpecOS).
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef PARSEELF_H
#define PARSEELF_H

struct programHeaderEntry {
    uint32_t entryType;
    uint32_t segmentOffsetInFile;
    uint32_t segmentOffsetInVMem;
    uint32_t segmentSizeInFile;
    uint32_t segmentSizeInVMem;
    uint32_t alignment;
};

struct elfHeader {
    bool isValid; // has magic number at start?
    uint8_t endianness;
    uint32_t programEntryOffset;
    uint32_t programHeaderOffset;
    uint32_t sectionHeaderOffset;
    uint16_t elfHeaderSize;
    uint16_t programHeaderTableEntrySize;
    uint16_t programHeaderTableNumEntries;
    uint16_t sectionHeaderTableEntrySize;
    uint16_t sectionHeaderTableNumEntries;
    uint16_t sectionHeaderStringTableIndex;
    struct programHeaderEntry programHeader[];
};

struct elfHeader parseElf(char* contents); 

#endif
