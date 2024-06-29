#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef DECODEDIRECTORY_H
#define DECODEDIRECTORY_H

struct directoryEntry {
    bool isSet;
    char filename[8];
    char extension[3];
    bool readOnlyFlag;
    bool hiddenFlag;
    bool systemFlag;
    bool volumeIDFlag;
    bool isDirectory;
    bool archiveFlag;
    uint8_t reservedNT;
    uint8_t creationTimeHundredthsOfASecond; // Yeah, i know. it's badly named. deal with it (:
    uint8_t hourCreated;
    uint8_t minutesCreated;
    uint8_t secondsCreated;
    uint8_t yearCreated;
    uint8_t monthCreated;
    uint8_t dayCreated;
    uint8_t yearLastAccessed;
    uint8_t monthLastAccessed;
    uint8_t dayLastAccessed;
    uint32_t firstCluster;
    uint8_t hourLastModified;
    uint8_t minutesLastModified;
    uint8_t secondsLastModified;
    uint8_t yearLastModified;
    uint8_t monthLastModified;
    uint8_t dayLastModified;
    uint8_t fileSizeBytes;
};

void parseDirectory(char* rawBinary, struct directoryEntry directoryBuffer[10]); 

#endif
