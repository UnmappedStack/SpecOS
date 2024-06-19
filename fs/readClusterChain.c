// This is some stuff to find the sector of a cluster and read a chain of clusters, essentially reading files.
// This file is part of the SpecOS kernel's FAT32 file system driver.
// Copyright (C) 2024 Jake Steinburger under the MIT license. See the github repo for more information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "readClusterChain.h"
#include "parseBootRecord.h"

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
