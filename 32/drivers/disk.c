// This is an implementation of disk read/write function I'm currently working on for ATA PIO mode as part of SpecOS.
// (C) Copyright 2024 Jake Steinburger (under MIT license, see GitHub repo for more info)
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "disk.h"
#include "terminalWrite.h"
#include "../utils/inx.h"
#include "../utils/string.h"

// IDENTIFY is split into two functions, initiate (run when actually reading/writing disk) and compatibility (run on device startup)
// Compatibility is used to verify compatibility of the drive and make sure it's an existing ATA PIO drive.
// Initiate gets the drive ready for read/write operations. It also runs compatibility from within as it contains some parts of the setup function.
bool identifyInitiate() {
    terminal_writestring("Initiating drive...");
    outb(0x3F6, 0); // Reset the drive
    // Double check that it's compatible still.
    identifyCompatibility();
    // Keep polling until one of the status ports are bit 3 or until bit 0 sets.
    while (((inb(0x1F7) & 8) >> 3) == 0 && ((inb(0x1F7) & 1) >> 0) == 0);
    // Now it's either returned an error or success.
    if (((inb(0x1F7) & 1) >> 0) != 0) {
        terminal_writestring("Error flag set.\n");
        return false; // Error
    } else {
        terminal_writestring("Ready to read/write disk.\n");
        return true; // It's ready to read from the data port
    }
}

bool identifyCompatibility() {
    outb(0x1F6, 0xA0); // Select the master drive and send the selection to the primary bus
    // set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    // Send the identify command to the command IO port
    outb(0x1F7, 0xEC);
    // Read the status port
    // It should be read 14 times first, and only the next one should be paid attention to (according to what I read on osdev wiki)
    for (int j = 0; j < 14; j++)
        inb(0x1F7);
    if (inb(0x1F7) == 0) { // The drive does not exist
        terminal_writestring("The drive does not exist.\n");
        return false;
    } else {
        // Poll the status port until bit 7 clears 
        while (((inb(0x1F7) & 0x80) >> 7) != 0);
        // Make sure LBAmid and LBAhi ports are non-zero
        if (inb(0x1F4) != 0 && inb(0x1F5) != 0) {
            terminal_writestring("Not ATA.\n");
            return false; // The drive is not ATA
        } else {
            // Read values from the ATA data register (not really to do anything with it, just so that the actual disk data can be read)
            for (int i = 0; i < 256; i++)
                inw(0x1F0);
            return true;
        }
    }
}

int get_bit(unsigned char num, int x) {
    // Shift 1 x positions to the right and perform bitwise AND with num
    return (num >> x) & 1;
}

void showErrorTypes() {
    // This will be run assuming the error register is set, and will read the disk's error register to find what the issue is
    char *errorTypes[8] = {
        "AMNF - Address mark not found.",
        "TKZNF - Track zero not found.",
        "ABRT - Aborted command.",
        "MCR - Media change request.",
        "IDNF - ID not found.",
        "MC - Media changed.",
        "UNC - Uncorrectable data error.",
        "BBK - Bad Block detected."
    };
    int error_register = inb(0x1F1); 
    for (int i = 0; i < 8; i++) {
        if (get_bit(error_register, i)) {
            terminal_writestring("ERROR: ");
            terminal_writestring(errorTypes[i]);
            terminal_writestring("\n");
        }
    }
} 

// Used to reset ports after read complete
void wait_400() {
    outb(0x80, 0); // This should wait more than enough time
}

bool wait_ready(bool wait_drm) {
    uint8_t status;
    for (int i = 0; i < 15; i++)
        status = inb(0x1F7);
    while (wait_drm && !(status & 0x08)) {
        if (status & (0x01 | 0x02))
            return false;
        status = inb(0x1F7);
    }
    return true;
}

char* accessDisk(int32_t sect, bool isWrite, uint8_t data[512]) { 
    // Address the master drive and the first 4 bites of the LBA
    outb(0x1F6, 0xE0 | (0 << 4) | ((sect >> 24) & 0x0F));
    wait_400();
    // Send NULL byte to data register (OSDev wiki says this is useless but I'll add it anyway just to be safe)
    outb(0x1F1, 0x00);
    // Set the sector count (assumed 1)
    outb(0x1F2, (unsigned char) 1);
    // Send the low 8 bits of the LBA
    outb(0x1F3, (unsigned char) sect);
    // Send the mid 8 bits
    outb(0x1F4, (unsigned char) (sect >> 8));
    // Send the high 8 bits
    outb(0x1F5, (unsigned char) (sect >> 16));
    // Send the READ SECTORS or WRITE SECTORS command, depending on direction given
    if (isWrite)
        outb(0x1F7, 0x30); 
    else
        outb(0x1F7, 0x20);
    // Poll (4 times, ignoring the first 3)
    for (int i = 0; i < 4; i++) {
        int8_t byte;
        while(1) {
            byte = inb(0x1F7);
            if (((byte >> 3) & 1) != 0 && ((byte >> 7) & 1) == 0)
                break;      
        }
    }
    static char buffer8[512];
    if (isWrite) { 
        terminal_writestring("Writing to disk: ");
        terminal_writestring(data);
        terminal_writestring("\n"); 
        // Make sure there aren't any errors
        showErrorTypes();
        // Put the data into the data port one 16 bit char at a time (256 values max)
        uint16_t buffer;
        for (int i = 0; i < 512; i += 2) { 
            buffer = ((uint16_t)data[i + 1] << 8) | data[i];
            outw(0x1F0, buffer); 
        }
        wait_400();
        if (!wait_ready(true)) {
            showErrorTypes();
            return NULL;
        }
        // Send CACHE FLUSH command
        outb(0x1F7, 0xE7);
        if (!wait_ready(false)) {
            showErrorTypes();
            return NULL;
        } 
    } else {
        // Transfer 256 16-bit values into the buffer from 0x1F0 and convert them into 512 8-bit values
        uint16_t buffer16;
        int j = 0;
        for (int i = 0; i < 256; i++) { 
            buffer16 = inw(0x1F0);
            buffer8[j + 1] = (char) ((buffer16 >> 8) & 0xFF);
            buffer8[j] = (char) (buffer16 & 0xFF);
            j += 2;
        }
    }
    wait_400();
    showErrorTypes();
    return buffer8;
}

uint8_t emptyData[512];

char* readdisk(int32_t sect) {
    return accessDisk(sect, false, emptyData);
}

void writedisk(int32_t sect, char* data) {
    accessDisk(sect, true, data);
}













