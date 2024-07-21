#include "keyboard.h"
#include "../sys/idt.h"
#include "terminalWrite.h"
#include "../utils/string.h"
#include "../utils/inx.h"

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
        terminal_column--;
        terminal_putentryat(' ', VGA_COLOR_BLACK, terminal_column, terminal_row);
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
    show_vga_cursor();
    update_cursor(terminal_column, terminal_row + 1);
    shifted = false;
    inScanf = true;
    // Reset wholeInput
    int i = 0;
    while (1) {
        if (wholeInput[i] == '\0')
            break;
        wholeInput[i] = '\0';
        i++;
    }
    while (inScanf) {
        __asm__("hlt");
        update_cursor(terminal_column, terminal_row + 1);
        // This next part will done only when the next interrupt is called.
        if (inb(0x60) == 0x1C)
            break;
    }
    terminal_write("\0", 1); // I'm not really sure why, but if I don't write something to the terminal after the device crashes. 
    strcpy(inp, wholeInput);
    removeNullChars(inp);
    hide_vga_cursor();
}

void isr_keyboard() {
    unsigned char status = inb(0x64); // Read the status register of the keyboard controller
    if (status & 0x01) { // Check if the least significant bit (bit 0) is set, indicating data is available
        unsigned char scan_code = inb(0x60); // Read the scan code from the keyboard controller
        // Check if it's a key press event (high bit not set)
        if (!(scan_code & 0x80) && inScanf) {
            // Handle the key press event
            char combined = convertScancode(scan_code);
            if (combined != '\0') {
                addCharToString(wholeInput, combined);
                terminal_putchar(combined);
            }
       }
    }
    outb(0x20, 0x20); // Acknowledge interrupt from master PIC
    outb(0xa0, 0x20); // Acknowledge interrupt from slave PIC
    init_IRQ();
}


