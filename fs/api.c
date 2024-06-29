// SpecOS FAT32 API which can be called from the main kernel or the userspace (through interrupts)
// Copyright (C) 2024 Jake Steinburger under the MIT license. See the github repo for more information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../drivers/terminalWrite.h"
#include "api.h"
#include "decodeDirectory.h"
#include "readClusterChain.h"
#include "../drivers/disk.h"
#include "../utils/string.h"

bool checkAllowed(char toCheck) {
    char* allowed = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890!@#$%^&*()[]\\{}|;':\"/?.>,<`~";
    for (int i = 0; i < strlen(allowed); i++) {
        if (allowed[i] == toCheck)
            return true;
    }
    return false;
};

// Some padding things to make file names work properly
void padInputToFilename(char inp[11], char result[12]) {
    terminal_writestring("\nOriginal: ");
    for (int i = 0; i < 11; i++) {
        terminal_writestring(charToStr(inp[i]));
    }
    terminal_writestring("\n");
    // Get each of the actual filename characters and join them.
    int actualFilenameLength = 8;
    for (int i = 0; i < 8; i++) {
        if (!checkAllowed(inp[i])) {
            actualFilenameLength = i;
            break;
        }
        result[i] = inp[i];
    } 
    for (int i = 0; i < 12; i++) {
        terminal_writestring(charToStr(result[i]));
    }
    terminal_writestring(" <- plz just say BOOT");
    // Add a dot at the point after and the extension name, IF it has an extension (it won't if there's a folder)
    if (inp[10] != ' ' && inp[10] != '\0') {
        result[actualFilenameLength] = '.';
        for (int i = 1; i < 4; i++) {
            result[actualFilenameLength + i] = inp[7 + i];
        }
    }
}

// Function to list all files and folders in current directory (just their names)
void listCurrentDirectory(uint32_t currentDirectoryCluster) {
    // Get the absolute sector of the cluster given and read it
    uint32_t sect = getFirstSectorOfCluster(currentDirectoryCluster);
    char* rawContents = readdisk(sect);
    // Decode it into a structure that can be properly interpreted
    struct directoryEntry DEbuffer[10];
    parseDirectory(rawContents, DEbuffer);
    // And finally, print it all out!
    terminal_writestring("\n");
    for (int i = 0; i < 10; i++) {
        if (!DEbuffer[i].isSet)
            break;
        if (DEbuffer[i].isDirectory)
            terminal_setcolor(VGA_COLOR_CYAN);    
        for (int c = 0; c < 11; c++) {
            terminal_writestring(charToStr(DEbuffer[i].filename[c]));
        }
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

bool filenameCompare(char input[100], char fname[11]) {
    char buffer[12];
    padInputToFilename(fname, buffer); 
    terminal_writestring("\n");
    for (int i = 0; i < 12; i++) {
        if (buffer[i] == '\0')
            return true;
        if (input[i] != buffer[i])
            return false;
    }
    return true;
}

struct cd changeDirectorySingle(char child[100], struct cd prevDir) {
    allow_scroll = false;
    // Get the absolute sector of the cluster given and read it
    uint32_t sect = getFirstSectorOfCluster(prevDir.cluster);
    char* rawContents = readdisk(sect);
    // Decode it into a structure that can be properly interpreted
    struct directoryEntry DEbuffer[10];
    parseDirectory(rawContents, DEbuffer);
    // For each entry, check if the filename matches
    for (int i = 0; i < 10; i++) {
        terminal_writestring("Filename ");
        char* ibuff;
        size_t_to_str(i, ibuff);
        terminal_writestring(ibuff);
        terminal_writestring(": \n");
        if (filenameCompare(child, DEbuffer[i].filename)) {
            terminal_writestring("\nDirectory changing into: \"");
            char* buff;
            size_t_to_str(i, buff);
            terminal_writestring(buff);
            terminal_writestring("\"\n");
            // Make sure it's a folder
            if (DEbuffer[i].isDirectory) {
                struct cd newDir;
                newDir.cluster = DEbuffer[i].firstCluster;
                return newDir;
            } else {
                terminal_writestring("\nError: Not a directory.\n");
                return prevDir;
            }
        }
    }
    terminal_writestring("\nError: No such file or directory.\n");
    return prevDir;
}


























