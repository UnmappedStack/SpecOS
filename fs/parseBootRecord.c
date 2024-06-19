// Part of the FAT32 driver for SpecOS, which parses the partition's boot record information.
// Copyright (C) 2024 Jake Steinburger under the MIT license.
// See the github repo for more info.

#include "parseBootRecord.h"
#include "../drivers/disk.h"
#include "../drivers/terminalWrite.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// A couple functions that I'll later put into a utils file but can't bother doing rn until i get this working
// Function to zero out an array
void memset(uint8_t *array, uint8_t value, size_t size) {
    for (size_t i = 0; i < size; i++) {
        array[i] = value;
    }
}

// Function to copy memory
void memcpy(uint8_t *dest, const char *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = (uint8_t)src[i];
    }
}

uint32_t combine32bit(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
    return ((uint32_t)byte1 << 24) |
        ((uint32_t)byte2 << 16) |
        ((uint32_t)byte3 << 8)  |
        (uint32_t)byte4;
}

struct bootRecord readBoot() {
    // Read the first sector of the file system partition (assume it's 2048 because that's what it's set as in the compilation script. Safe? No. Works when not dualbooting with another OS? Hopefully.)
    char* rawBootSect_str = readdisk(2048);
    // Stuff to convert it into a uint8_t[512]
    uint8_t rawBootSect[512];
    memset(rawBootSect, 0, 512);
    memcpy(rawBootSect, rawBootSect_str, strlen(rawBootSect_str));
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
    toReturn.bytesPerSect = ((uint16_t)rawBootSect[11] << 8) | rawBootSect[12];
    if ((((uint16_t)rawBootSect[11] << 8) | rawBootSect[12]) == (uint16_t)0) {
        terminal_writestring("\nSomething's not right. The boot sector says that each sector is... 0 bytes?\n");
    }
    /*
    toReturn.sectPerClust = rawBootSect[13];
    toReturn.reservedSectCount = ((uint16_t)rawBootSect[14]) | rawBootSect[15];
    toReturn.numFats = rawBootSect[16];
    toReturn.rootEntryCount = ((uint16_t)rawBootSect[17]) | rawBootSect[18];
    toReturn.totalSectors16 = ((uint16_t)rawBootSect[19]) | rawBootSect[20];
    toReturn.media = rawBootSect[21];
    toReturn.sectPerFat16 = ((uint16_t)rawBootSect[22]) | rawBootSect[23];
    toReturn.sectPerTrack = ((uint16_t)rawBootSect[24]) | rawBootSect[25];
    toReturn.numHeads = ((uint16_t)rawBootSect[26]) | rawBootSect[27];
    toReturn.hiddenSects = combine32bit(rawBootSect[28], rawBootSect[29], rawBootSect[30], rawBootSect[31]);
    toReturn.totalSectors32 = combine32bit(rawBootSect[32], rawBootSect[33], rawBootSect[34], rawBootSect[35]);
    toReturn.sectPerFat32 = combine32bit(rawBootSect[36], rawBootSect[37], rawBootSect[38], rawBootSect[39]);
    toReturn.extFlags = ((uint16_t)rawBootSect[40]) | rawBootSect[41];
    toReturn.fileSystemVersion = ((uint32_t)rawBootSect[42]) | rawBootSect[43];
    toReturn.rootCluster = combine32bit(rawBootSect[44], rawBootSect[45], rawBootSect[46], rawBootSect[47]);
    toReturn.FSInfo = ((uint16_t)rawBootSect[48]) | rawBootSect[49];
    toReturn.bkBootSect = ((uint16_t)rawBootSect[50]) | rawBootSect[51];
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
    */
    return toReturn;
}



