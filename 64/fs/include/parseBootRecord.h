// Header file for parseBootRecord.c
// Copyright (C) 2024 Jake Steinburger under the MIT license.
// See the github repo for more info

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../../utils/include/string.h"

#ifndef PARSEBOOTRECORD_H
#define PARSEBOOTRECORD_H

struct bootRecord {
    uint8_t jmpBoot[3];
    uint8_t useless0;
    char OEMName[8];
    uint16_t bytesPerSect;
    uint8_t sectPerClust;
    uint16_t reservedSectCount;
    uint8_t numFats;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t media;
    uint16_t sectPerFat16;
    uint16_t sectPerTrack;
    uint16_t numHeads;
    uint32_t hiddenSects;
    uint32_t totalSectors32;
    uint32_t sectPerFat32;
    uint16_t extFlags;
    uint16_t fileSystemVersion;
    uint32_t rootCluster;
    uint16_t FSInfo;
    uint16_t bkBootSect;
    uint8_t reserved[12];
    uint8_t driveNumber;
    uint8_t reserved1;
    uint8_t extBootSig;
    uint32_t volID;
    char volLabel[11];
    uint8_t useless1;
    char fileSysType[8];
    uint8_t useless2;
    uint16_t bootCode[420];
    uint8_t useless3;
    uint16_t bootSig;
};

struct bootRecord readBoot();

#endif
