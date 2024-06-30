#include "string.h"
#include "../drivers/terminalWrite.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}


void memset(uint8_t *array, uint8_t value, size_t size) {
    for (size_t i = 0; i < size; i++) {
        array[i] = value;
    }
}

void addCharToString(char *str, char c) {
    // Iterate to find the end of the string
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    // Append the character
    str[i] = c;
    // Null-terminate the string
    str[i + 1] = '\0';
}

void removeLastChar(char *str) {
    for (int i = strlen(str); i > 0; i--) {
        if (str[i] != '\0') {
            str[i] = '\0';
            break;
        }
    }
}

void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void uint16_to_string(uint16_t num, char *str) {
    // Define a buffer large enough to hold the maximum uint16_t value in decimal
    char buffer[6];  // Max length of uint16_t in decimal is 5 digits, plus null terminator
    // Index to fill the buffer
    int index = 0;
    // Special case for zero
    if (num == 0) {
        buffer[index++] = '0';
    } else {
        // Extract digits from least significant to most significant
        while (num > 0) {
            buffer[index++] = '0' + (num % 10);  // Convert digit to character
            num /= 10;  // Move to the next digit
        }
    }
    // Null-terminate the buffer
    buffer[index] = '\0';
    // Reverse the buffer
    reverse(buffer, index);
    // Copy the reversed buffer to the output string (str)
    for (int i = 0; i <= index; ++i) {
        str[i] = buffer[i];
    } 
}

void uint32_to_hex_string(uint32_t num, char *str) {
    // Define a buffer large enough to hold the maximum uint32_t value in hex
    char buffer[9];  // Max length of uint32_t in hex is 8 digits, plus null terminator
    // Index to fill the buffer
    int index = 0;
    
    // Special case for zero
    if (num == 0) {
        buffer[index++] = '0';
    } else {
        // Extract hex digits from least significant to most significant
        while (num > 0) {
            uint8_t digit = num & 0xF;  // Get the least significant 4 bits (a hex digit)
            if (digit < 10) {
                buffer[index++] = '0' + digit;  // Convert digit to character
            } else {
                buffer[index++] = 'A' + (digit - 10);  // Convert digit to character
            }
            num >>= 4;  // Shift right by 4 bits to get the next hex digit
        }
    }
    // Null-terminate the buffer
    buffer[index] = '\0';
    // Reverse the buffer
    reverse(buffer, index);
    // Copy the reversed buffer to the output string (str)
    for (int i = 0; i <= index; ++i) {
        str[i] = buffer[i];
    }
}

int get_num_length(uint32_t num) {
    int length = 0;
    do {
        length++;
        num /= 10;
    } while (num > 0);
    return length;
}

void uint32_to_string(uint32_t num, char* str) {
    // Get the length of the number
    int length = get_num_length(num);

    // Null-terminate the string
    str[length] = '\0';

    // Fill the buffer with digits in reverse order
    int index = length - 1;
    do {
        str[index--] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);
}
/*
// Function to convert uint32_t to string
void uint32_to_string(uint32_t num, char *str) {
    // Define a buffer large enough to hold the maximum uint32_t value in decimal
    char buffer[11];  // Max length of uint32_t in decimal is 10 digits, plus null terminator

    // Index to fill the buffer
    int index = 0;

    // Special case for zero
    if (num == 0) {
        buffer[index++] = '0';
    } else {
        // Extract digits from least significant to most significant
        while (num > 0) {
            buffer[index++] = '0' + (num % 10);  // Convert digit to character
            num /= 10;  // Move to the next digit
        }
    }

    // Null-terminate the buffer
    buffer[index] = '\0';

    // Reverse the buffer
    reverse(buffer, index);

    // Copy the reversed buffer to the output string (str)
    for (int i = 0; i <= index; ++i) {
        str[i] = buffer[i];
    }
}
*/
// Function to convert size_t to string
void size_t_to_str(size_t num, char* buffer) {
    int i = 0;
    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    while (num != 0) {
        int rem = num % 10;
        buffer[i++] = rem + '0';
        num = num / 10;
    }
    buffer[i] = '\0';
    reverse(buffer, i);
}



void strcpy(char* dest, const char* src) {
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0'; // Null-terminate the destination string
}

int compareDifferentLengths(const char *longer, const char *shorter) {
    size_t num = 0;
    while (num < strlen(shorter)) {
        if (longer[num] != shorter[num])
            return 0;
        num++;
    }
    return 1;
}

char* charToStr(char character) {
    static char wholeStr[2];
    wholeStr[0] = character;
    wholeStr[1] = '\0';
    return wholeStr;
}
