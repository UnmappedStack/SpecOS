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
#include "../utils/binop.h"

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

// Used for skipping long file names
void removeElementIfNeeded(struct directoryEntry array[], int *size, int index) {
    // Shift elements to the left starting from index
    for (int i = index; i < *size - 1; ++i) {
        array[i] = array[i + 1];
    }
    (*size)--;  // Decrement size of the array
}

// Function to process the array and remove elements as needed
void processArray(struct directoryEntry array[], int *size) {
    int i = 0;
    while (i < *size) {
        if (array[i].filename[0] == '\0') {
            // If first character is null, remove the element
            removeElementIfNeeded(array, size, i);
        } else {
            i++;  // Move to next element if no removal was done
        }
    }
}

void parseDirectory(char* rawBinary, struct directoryEntry directoryBuffer[10]) {
    // First, divide it into 10 (or less) thingies of 32 bytes
    char rawEntries[10][32];
    splitString(rawBinary, rawEntries);
    // For each of them, add to directoryBuffer a decoded thingy
    for (int i = 0; i < 10; i++) {
        if (rawEntries[i][11] == 0x0F) // If it's a long file name entry, skip it - not supported yet!
            continue;
        directoryBuffer[i].isSet = (rawEntries[i][0] != '\0');
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
        // Skip a bunch of meta data that I can't get working right now and just find the first cluster
        directoryBuffer[i].firstCluster = combine32bit(rawEntries[i][21], rawEntries[i][20], rawEntries[i][27], rawEntries[i][26]);
    } 
    int listsize = 10;
    processArray(directoryBuffer, &listsize);
} 











