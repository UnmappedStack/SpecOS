// Some code to turn a raw binary directory cluster chain into a proper interpretable directory data structure.
// This currently has some limits, like it doesn't support long file names yet. I'll get to that.
// This is part of the SpecOS kernel project.
// Copyright (C) 2024 Jake Steinburger under the MIT license. See the github repo for more info.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "decodeDirectory.h"
#include "../drivers/terminalWrite.h"
#include "../utils/string.h"

// A function that really should be in the string.h utils header, but I can't bother doing it properly
void splitString(const char *input, char output[10][32]) {
    int t = 0;
    for (int entry = 0; entry < 10; entry++) {
        for (int ch = 0; ch < 32; ch++) {
            output[entry][ch] = input[t];
            t++;
        }
    } 
}

int getBit(unsigned char num, int x) {
    // Shift 1 x positions to the right and perform bitwise AND with num
    return (num >> x) & 1;
}

void parseDirectory(char* rawBinary, struct directoryEntry directoryBuffer[10]) {
    // First, divide it into 10 (or less) thingies of 32 bytes
    char rawEntries[10][32];
    splitString(rawBinary, rawEntries);
    // For each of them, add to directoryBuffer a decoded thingy 
    for (int i = 0; i < 10; i++) {
        // Offset 0: File name
        for (int j = 0; j < 8; j++) {
            directoryBuffer[i].filename[j] = rawEntries[i][j];
        }
        // Offset 8: Extension/type
        for (int j = 0; j < 3; j++) {
            directoryBuffer[i].extension[j] = rawEntries[i][j + 8];
        }
        // Offset 11: Flags (there's a few)
        uint8_t flags = rawEntries[i][11];
        /* Bits 7 and 8 are reserved.
           1 = READONLY, 2 = HIDDEN, 3 = SYSTEM, 4 = VOLUME_ID, 5 = DIRECTORY (pretty important), and 6 = ARCHIVE */
        bool *flagsArray[6] = {&directoryBuffer[i].readOnlyFlag,
                               &directoryBuffer[i].hiddenFlag,
                               &directoryBuffer[i].systemFlag,
                               &directoryBuffer[i].volumeIDFlag,
                               &directoryBuffer[i].isDirectory,
                               &directoryBuffer[i].archiveFlag};
        for (int j = 0; j < 6; ++j) {
            *flagsArray[j] = getBit(flags, j);
        }
        // Offset 13: Creation time in hundredths of a second.
        uint16_t timeCreatedWhole = ((uint16_t)rawEntries[i][15] << 8) | rawEntries[i][14];
        directoryBuffer[i].hourCreated = (getBit(timeCreatedWhole, 4) << 4) | (getBit(timeCreatedWhole, 3) << 3) | (getBit(timeCreatedWhole, 2) << 2) | (getBit(timeCreatedWhole, 1) << 1) | getBit(timeCreatedWhole, 0);
        directoryBuffer[i].minutesCreated = (timeCreatedWhole >> 5) & 0x3F; 
    }
} 











