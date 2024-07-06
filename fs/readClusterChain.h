#include <stdint.h>

#ifndef READCLUSTERCHAIN_H
#define READCLUSTERCHAIN_H

uint32_t getFirstSectorOfCluster(int clust);

void readFile(int firstClust, char* buffer);
 
#endif
