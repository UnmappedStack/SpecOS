/* Header for ../readClusterChain.c, a part of the FAT32 implementation within the SpecOS kernel project.
 * Copyright 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>

#ifndef READCLUSTERCHAIN_H
#define READCLUSTERCHAIN_H

uint32_t getFirstSectorOfCluster(int clust);

int readFile(int firstClust, char buffer[1000000]);
 
#endif
