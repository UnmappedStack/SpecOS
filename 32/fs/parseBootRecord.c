// Part of the FAT32 driver for SpecOS, which parses the partition's boot record information.
// Copyright (C) 2024 Jake Steinburger under the MIT license.
// See the github repo for more info.

// WARNING! You are about to see some very hack-y, badly written code. Sorry to anybody trying to contribute to this mess!
// I'll fix it later, I promise (:

#include "parseBootRecord.h"
#include "../drivers/disk.h"
#include "../drivers/terminalWrite.h"
#include "../utils/binop.h"
#include "../utils/string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// A couple functions that I'll later put into a utils file but can't bother doing rn until i get this working
// Function to copy memory
void memcpy(uint8_t *dest, const char *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = (uint8_t)src[i];
    }
}

struct bootRecord readBoot() {
    // Read the first sector of the file system partition (assume it's 2048 because that's what it's set as in the compilation script. Safe? No. Works when not dualbooting with another OS? Hopefully.)
    char* rawBootSect_str = readdisk(2048);
    // Stuff to convert it into a uint8_t[512]
    uint8_t rawBootSect[512];
    memcpy(rawBootSect, rawBootSect_str, 512);
    // Create an object to return and kinda just fill each of them up slowly
    struct bootRecord toReturn;
    struct bootRecord temp;
    temp.jmpBoot[0] = rawBootSect[0];
    temp.jmpBoot[1] = rawBootSect[1];
    temp.jmpBoot[2] = rawBootSect[2];
    memcpy(toReturn.jmpBoot, temp.jmpBoot, sizeof(temp.jmpBoot));
    for (int i = 0; i < 8; i++) {
        temp.OEMName[i] = rawBootSect[3 + i];
    }
    memcpy(toReturn.OEMName, temp.OEMName, sizeof(temp.OEMName));
    toReturn.bytesPerSect = ((uint16_t)rawBootSect[12] << 8) | rawBootSect[11]; 
    toReturn.sectPerClust = rawBootSect[13];
    toReturn.reservedSectCount = ((uint16_t)rawBootSect[15]) | rawBootSect[14];
    toReturn.numFats = rawBootSect[16];
    toReturn.rootEntryCount = ((uint16_t)rawBootSect[18]) | rawBootSect[17];
    toReturn.totalSectors16 = ((uint16_t)rawBootSect[20]) | rawBootSect[19];
    toReturn.media = rawBootSect[21];
    toReturn.sectPerFat16 = ((uint16_t)rawBootSect[23]) | rawBootSect[22];
    toReturn.sectPerTrack = ((uint16_t)rawBootSect[25]) | rawBootSect[24];
    toReturn.numHeads = ((uint16_t)rawBootSect[27]) | rawBootSect[26];
    toReturn.hiddenSects = combine32bit(rawBootSect[31], rawBootSect[30], rawBootSect[29], rawBootSect[28]);
    toReturn.totalSectors32 = combine32bit(rawBootSect[35], rawBootSect[34], rawBootSect[33], rawBootSect[32]);
    toReturn.sectPerFat32 = combine32bit(rawBootSect[39], rawBootSect[38], rawBootSect[37], rawBootSect[36]);
    toReturn.extFlags = ((uint16_t)rawBootSect[41]) | rawBootSect[40];
    toReturn.fileSystemVersion = ((uint32_t)rawBootSect[43]) | rawBootSect[42];
    toReturn.rootCluster = combine32bit(rawBootSect[47], rawBootSect[46], rawBootSect[45], rawBootSect[44]);
    toReturn.FSInfo = ((uint16_t)rawBootSect[49]) | rawBootSect[48];
    toReturn.bkBootSect = ((uint16_t)rawBootSect[51]) | rawBootSect[50];
    for (int i = 0; i < 12; i++) {
        toReturn.reserved[i] = rawBootSect[52 + i];
    }
    toReturn.driveNumber = rawBootSect[64];
    toReturn.reserved1 = rawBootSect[65];
    toReturn.extBootSig = rawBootSect[66];
    toReturn.volID = combine32bit(rawBootSect[67], rawBootSect[68], rawBootSect[69], rawBootSect[70]);
    for (int i = 0; i < 11; i++) {
        toReturn.volLabel[i] = rawBootSect[71 + i];
    }
    for (int i = 0; i < 8; i++) {
        toReturn.fileSysType[i] = rawBootSect[82 + i];
    }
    for (int i = 0; i < 420; i += 2) {
        toReturn.bootCode[i] = ((uint16_t)rawBootSect[90 + i]) | rawBootSect[91 + i];
    }
    toReturn.bootSig = ((uint16_t)rawBootSect[510]) | rawBootSect[511];
    return toReturn;
}



