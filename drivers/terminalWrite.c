// A simple scroll supported, newline supported header to write to terminal
#include "terminalWrite.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* Hardware text mode color constants. */

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
uint8_t bg_colour = VGA_COLOR_BLACK;

void terminal_initialize(void) 
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, bg_colour);
    terminal_buffer = (uint16_t*) 0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_set_bg(uint8_t colour) {
    bg_colour = colour;
}

void terminal_setcolor(uint8_t color) 
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_writestring(const char* data);

void terminal_putchar(char c) 
{
    terminal_color = vga_entry_color(terminal_color, bg_colour);
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
	    terminal_row = 0;
    } 
}

void terminal_write(const char* data, size_t size) 
{
    for (size_t i = 0; i < size; i++) {    
        if (data[i] == '\n') {
            terminal_row++;
            terminal_column = 0;
        } else {
            if (data[i] == '\x7F')
                terminal_putchar((char)219);
            else
                terminal_putchar(data[i]);
        }
    }
}

bool allow_scroll = true;

void terminal_writestring(const char* data) 
{
    terminal_write(data, strlen(data)); 
    if (terminal_row >= (VGA_HEIGHT / 3 * 2) && allow_scroll) {
        terminal_initialize();
        terminal_write(data, strlen(data));
    }
}

