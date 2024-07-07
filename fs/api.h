// Header for api.c which is a file containing the kernelspace FAT32 file system API for SpecOS.
// Copyright (C) 2024 Jake Steinburger under the MIT license. See the github repo for more information.

#include <stdint.h>
#include <stdbool.h>

#ifndef API_H
#define API_H

struct cd {
    char path[100];
    uint32_t cluster;
};

void listCurrentDirectory(uint32_t currentDirectoryCluster);

struct cd changeDirectorySingle(char child[100], struct cd prevDir); 

char* cat(struct cd prevDir, char child[100], bool doEcho);

#endif
