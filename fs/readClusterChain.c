// This is some stuff to find the sector of a cluster and read a chain of clusters, essentially reading files.
// This file is part of the SpecOS kernel's FAT32 file system driver.
// Copyright (C) 2024 Jake Steinburger under the MIT license. See the github repo for more information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "readClusterChain.h"
#include "parseBootRecord.h"
#include "../drivers/disk.h"
#include "../utils/string.h"
#include "../drivers/terminalWrite.h"
#include "../utils/binop.h"

unsigned int getNextCluster(int currentCluster) {
    uint16_t firstFatSector = readBoot().reservedSectCount;
    unsigned char FAT_table[512];
    unsigned int fat_offset = currentCluster * 4;
    unsigned int fat_sector = firstFatSector + (fat_offset / 512);
    unsigned int ent_offset = fat_offset % 512;
    char* fatSect = readdisk(fat_sector + 2048);
    for (int i = 0; i < 512; i++)
        FAT_table[i] = fatSect[i];
    unsigned int table_value = combine32bit(FAT_table[ent_offset + 3],FAT_table[ent_offset + 2], FAT_table[ent_offset + 1], FAT_table[ent_offset]);
    table_value &= 0xFFFFFFF;
    if (table_value >= 0x0FFFFFF8)
        return 0; // End of cluster chain
    return table_value; 
}

uint32_t getFirstSectorOfCluster(int clust) {
    // Parse the boot sector to get some info that'll be needed
    struct bootRecord bpb = readBoot();
    // Count of sectors occupied by the root directory
    uint32_t rootSectors = ((bpb.rootEntryCount * 32) + (bpb.bytesPerSect - 1)) / bpb.bytesPerSect;
    // Get the first data sector
    uint32_t firstDataSect = bpb.reservedSectCount + (bpb.numFats * bpb.sectPerFat32) + rootSectors;
    // And finally, the sector to return (relative to the start of the disk, by adding 2048) 
    return (((clust - 2) * bpb.sectPerClust) + firstDataSect) + 2048;
}

void readFile(int firstClust, char* buffer) {
    int i = 0;
    int currentClust = firstClust;
    while (1) {
        char* thisSectContents = readdisk(getFirstSectorOfCluster(currentClust));
        for (int j = 0; j < 512; j++) {
            buffer[i + j] = thisSectContents[j];
        }
        i += 512;
        currentClust = getNextCluster(currentClust);
        if (currentClust == 0)
            break;
    }
    // Add 3 special characters to the end to show it's done
    buffer[i] = 4;
    buffer[i + 1] = 0;
    buffer[i + 2] = 4;
} 
