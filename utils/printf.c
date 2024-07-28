/* Scanf implementation for the SpecOS kernel project.
 * Copyright (c) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include "../drivers/include/vga.h"
#include "include/string.h"
#include "include/printf.h"
#include <stdarg.h>

void printf(char* format, ...) {
    va_list args;
    va_start(args, format);
    int i = 0;
    while (i < strlen(format)) {
        if (format[i] == '%') {
            i++;
            char buffer[10];
            if (format[i] == 'd' || format[i] == 'i') {
                uint64_to_string(va_arg(args, uint64_t), buffer);
                buffer[9] = 0;
                writestring(buffer);
            } else if (format[i] == 'c') {
                writestring(charToStr(va_arg(args, char)));
            } else if (format[i] == 'x') {
                char bufferx[20];
                bufferx[19] = 0;
                uint64_to_hex_string(va_arg(args, uint64_t), bufferx);
                writestring(bufferx);
            } else if (format[i] == 's') {
                writestring(va_arg(args, char*));
            }
        } else {
            writestring(charToStr(format[i]));
        }
        i++;
    }
    va_end(args);
}
