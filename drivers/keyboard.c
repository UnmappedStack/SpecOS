/* Keyboard driver for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */


#include "include/keyboard.h"
#include "../sys/include/idt.h"
#include "include/vga.h"
#include "../kernel/include/kernel.h"
#include "../utils/include/string.h"
#include "../utils/include/io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool shifted = false;
bool capslock = false;

bool inScanf = false;

char wholeInput[100] = "";

unsigned int inputLength = 0;

unsigned char convertScancode(unsigned char scancode) {
   char characterTable[] = {
    0,    0,    '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',
    '-',  '=',  0,    0x09, 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
    'o',  'p',  '[',  ']',  0,    0,    'a',  's',  'd',  'f',  'g',  'h',
    'j',  'k',  'l',  ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',  0,    ' ',  0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0x1B, 0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0x0E, 0x1C, 0,    0,    0,
    0,    0,    0,    0,    0,    '/',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0,
    0,    0,    0,    0,    0,    0,    0,    0x2C
    };
    char shiftedCharacterTable[] = {
    0,    0,    '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',
    '_',  '+',  0,    0x09, 'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
    'O',  'P',  '{',  '}',  0,    0,    'A',  'S',  'D',  'F',  'G',  'H',
    'J',  'K',  'L',  ':',  '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',  0,    ' ',  0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0x1B, 0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0x0E, 0x1C, 0,    0,    0,
    0,    0,    0,    0,    0,    '?',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0,
    0,    0,    0,    0,    0,    0,    0,    0x2C,
    };
    if (scancode == 0x3A) {
        capslock = !capslock;
        return '\0';
    }
    if (scancode == 0x2A || scancode == 0x36) {
        shifted = true;
        return '\0';
    }

    if (scancode == 0x0E && inputLength != 0) {
        if (wholeInput[0] == '\0') return '\0'; // don't let it keep deleting characters outside of that input field!
        kernel.chX -= 10;
        writeChar(' ', 0x00);
        kernel.chX -= 10;
        // For some reason backspacing the first character is a special case cos otherwise it doesn't work
        if (inputLength == 1) {
            inputLength = 0;
            // just reset the whole thing
            int i = 0;
            while (1) {
                wholeInput[i] = '\0';
                if (wholeInput[i + 1] == '\0')
                    break;
                i++;
            }
        }
        inputLength--;
        removeLastChar(wholeInput);
        return '\0';
    }

    if (scancode == 0x1C) {
        inScanf = false;
        return '\0';
    }

    inputLength++;
    if (capslock || shifted) {
        shifted = false;
        return shiftedCharacterTable[scancode];
    }
    unsigned char toReturn = characterTable[scancode];
    shifted = false;
    return toReturn;
}

void removeNullChars(char arr[100]) {
    int i, j;
    int len = 0; // Variable to keep track of the current length of the string
    // Calculate the length of the string (excluding null characters)
    for (len = 0; len < 100; len++) {
        if (arr[len] == '\0') {
            break;
        }
    }
    // Traverse the array
    for (i = 0; i < len; i++) {
        // If current character is '\0' and not the last character
        if (arr[i] == '\0') {
            // Shift all subsequent characters one position to the left
            for (j = i; j < len; j++) {
                arr[j] = arr[j + 1];
            }
            // Adjust length of the string after shifting characters left
            len--;
            // Since we shifted characters left, recheck current index
            i--;
        }
    }
    // Ensure the last character is '\0' to terminate the string
    arr[len] = '\0';
}

void scanf(char* inp) {
    kernel.doPush = false;
    shifted = false;
    inScanf = true;
    inputLength = 0;
    //maskIRQ(0);
    // Reset wholeInput
    int i = 0;
    while (1) {
        if (wholeInput[i] == '\0')
            break;
        wholeInput[i] = '\0';
        i++;
    }
    while (inScanf) {
        asm("hlt");
        // This next part will done only when the next interrupt is called.
        if (inb(0x60) == 0x1C)
            break;
    }
    //unmaskIRQ(0);
    writestring("\0"); // I'm not really sure why, but if I don't write something to the terminal after the device crashes. 
    strcpy(inp, wholeInput);
    removeNullChars(inp);
    kernel.doPush = true;
}

__attribute__((interrupt))
void isr_keyboard(void*) {
    unsigned char status = inb(0x64); // Read the status register of the keyboard controller
    if (status & 0x01) { // Check if the least significant bit (bit 0) is set, indicating data is available
        unsigned char scan_code = inb(0x60); // Read the scan code from the keyboard controller
        // Check if it's a key press event (high bit not set)
        if (!(scan_code & 0x80) && inScanf) {
            // Handle the key press event
            char combined = convertScancode(scan_code);
            if (combined != '\0') {
                addCharToString(wholeInput, combined);
                writeChar(combined, 0xFFFFFF);
            }
        }
    }
    outb(0x20, 0x20); // Acknowledge interrupt from master PIC
    asm("sti");
} 

