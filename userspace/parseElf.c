/* File for parsing ELF headers as part of the SpecOS kernel.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more info.
 * Note that this file does not contain any code for actually running apps, it simply parses their headers.
 * This just assumes that apps are 32 bit. If it's not... it'll just crash the machine. 
 * Definitely not broken at all in any way.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../utils/string.h"
#include "../utils/binop.h"
#include "parseElf.h"

/* There are 2 different functions here, and all parse different parts of the elf:
 *  - ELF header
 *  - Program header
 * They each have their own struct data type, containing information about the executable.
 * At some point I'll also add parsing of the section header table, but not quite yet. 
 * Yes, I know this is very messy and repetitive code. Oh well.
 * Uhh one more thing actually, btw the structs programHeaderEntry and elfHeader are defined in parseElf.h 
 */

// This function expects that `buffer` has already been set to a size of `numEntries`
void parseProgramHeader(struct programHeaderEntry buffer[], int numEntries, char* contents, uint32_t programHeaderOffset, int endianness) {
    struct programHeaderEntry toReturn[numEntries];
    uint32_t offset;
    for (int i = 0; i < numEntries; i++) {
        offset = programHeaderOffset + i;
        if (endianness == 1) { // little endian?
            toReturn[i].entryType = combine32bit(contents[offset + 3], contents[offset + 2], contents[offset + 1], contents[offset]);
            toReturn[i].segmentOffsetInFile = combine32bit(contents[offset + 7], contents[offset + 6], contents[offset + 5], contents[offset + 4]);
            toReturn[i].segmentOffsetInVMem = combine32bit(contents[offset + 11], contents[offset + 10], contents[offset + 9], contents[offset + 8]);
            toReturn[i].segmentSizeInFile = combine32bit(contents[offset + 19], contents[offset + 18], contents[offset + 17], contents[offset + 16]);
            toReturn[i].segmentSizeInVMem = combine32bit(contents[offset + 23], contents[offset + 22], contents[offset + 21], contents[offset + 20]);
            toReturn[i].alignment = combine32bit(contents[offset + 31], contents[offset + 30], contents[offset + 29], contents[offset + 28]);
        } else {
            toReturn[i].entryType = combine32bit(contents[offset + 0], contents[offset + 1], contents[offset + 2], contents[offset + 3]);
            toReturn[i].segmentOffsetInFile = combine32bit(contents[offset + 4], contents[offset + 5], contents[offset + 6], contents[offset + 7]);
            toReturn[i].segmentOffsetInVMem = combine32bit(contents[offset + 8], contents[offset + 9], contents[offset + 10], contents[offset + 11]);
            toReturn[i].segmentSizeInFile = combine32bit(contents[offset + 16], contents[offset + 17], contents[offset + 18], contents[offset + 19]);
            toReturn[i].segmentSizeInVMem = combine32bit(contents[offset + 20], contents[offset + 21], contents[offset + 22], contents[offset + 23]);
            toReturn[i].alignment = combine32bit(contents[offset + 28], contents[offset + 29], contents[offset + 30], contents[offset + 31]); 
        }
    }
}

struct elfHeader parseElf(char* contents) {
    struct elfHeader toReturn;
    int endianness = contents[5]; // 1 = little endian, 2 = big endian
    toReturn.endianness = endianness;
    toReturn.isValid = contents[0] == 0x7F && (contents[1] == 'E' && contents[2] == 'L' && contents[3] == 'F');
    if (endianness == 1) { // little endian?
        toReturn.programEntryOffset = combine32bit(contents[27], contents[26], contents[25], contents[24]);
        toReturn.programHeaderOffset = combine32bit(contents[31], contents[30], contents[29], contents[28]);
        toReturn.sectionHeaderOffset = combine32bit(contents[27], contents[26], contents[25], contents[24]);
        toReturn.elfHeaderSize = ((uint16_t) contents[41]) | contents[40];
        toReturn.programHeaderTableEntrySize = ((uint16_t) contents[43]) | contents[42];
        toReturn.programHeaderTableNumEntries = ((uint16_t) contents[45]) | contents[44];
        toReturn.sectionHeaderTableNumEntries = ((uint16_t) contents[47]) | contents[46];
        toReturn.sectionHeaderStringTableIndex = ((uint16_t) contents[49]) | contents[48];
    } else {
        toReturn.programEntryOffset = combine32bit(contents[24], contents[25], contents[26], contents[27]);
        toReturn.programHeaderOffset = combine32bit(contents[28], contents[29], contents[30], contents[31]);
        toReturn.sectionHeaderOffset = combine32bit(contents[24], contents[25], contents[26], contents[27]);
        toReturn.elfHeaderSize = ((uint16_t) contents[40]) | contents[41];
        toReturn.programHeaderTableEntrySize = ((uint16_t) contents[42]) | contents[43];
        toReturn.programHeaderTableNumEntries = ((uint16_t) contents[44]) | contents[45];
        toReturn.sectionHeaderTableNumEntries = ((uint16_t) contents[46]) | contents[47];
        toReturn.sectionHeaderStringTableIndex = ((uint16_t) contents[48]) | contents[49];
    }
    // Parse program header table
    struct programHeaderEntry buffer[toReturn.programHeaderTableNumEntries];
    parseProgramHeader(buffer, toReturn.programHeaderTableNumEntries, contents, toReturn.programHeaderOffset, toReturn.endianness); 
    for (int i = 0; i < toReturn.programHeaderTableNumEntries; i++)
        toReturn.programHeader[i] = buffer[i];
    return toReturn;
}













