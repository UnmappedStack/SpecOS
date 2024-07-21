#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef STRING_H
#define STRING_H

size_t strlen(const char* str);
void addCharToString(char *str, char c);
void removeLastChar(char *str);
void reverse(char str[], int length);
void size_t_to_str(size_t num, char* buffer);
void strcpy(char* dest, const char* src);
int compareDifferentLengths(const char *longer, const char *shorter); 
char* charToStr(char character);
void uint16_to_string(uint16_t num, char *str);
void uint32_to_string(uint32_t num, char *str);
void memset(uint8_t *array, uint8_t value, size_t size);
void uint32_to_hex_string(uint32_t num, char *str);

#endif
