// A simple scroll supported, newline supported header to write to terminal
#include "terminalWrite.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../utils/inx.h"
#include "../utils/string.h"
#include "serial.h"

/* Hardware text mode color constants. */

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
    return (uint16_t) uc | (uint16_t) color << 8;
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

void scrollTerminal();

void terminal_putchar(char c) 
{
    outCharSerial(c);
    terminal_color = vga_entry_color(terminal_color, bg_colour);
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
         while (terminal_row >= VGA_HEIGHT - 2 && allow_scroll)
            scrollTerminal();

    } 
}

void terminal_write(const char* data, size_t size) 
{
    for (size_t i = 0; i < size; i++) {    
        if (data[i] == '\n') {
            serial_writestring("\n");
            terminal_row++;
            terminal_column = 0;
            while (terminal_row >= VGA_HEIGHT - 2 && allow_scroll)
                scrollTerminal();
        } else {
            if (data[i] == '\x7F')
                terminal_putchar((char)219);
            else
                terminal_putchar(data[i]);
        }
    } 
}

bool allow_scroll = true;

void scrollTerminal() {
    size_t index;
    for (int l = 1; l < terminal_row; l++){
        for (int c = 0; c < VGA_WIDTH; c++) {
            index = l * VGA_WIDTH + c;
            terminal_putentryat(terminal_buffer[index], terminal_buffer[index] >> 8, c, l - 1);
        }
    }
    // Clear last line used
    for (int c = 0; c < VGA_WIDTH; c++) {
        terminal_putentryat(' ', VGA_COLOR_BLACK, c, terminal_row - 1);
    }
    terminal_row--;
}

void terminal_writestring(const char* data) 
{ 
    terminal_write(data, strlen(data)); 
}

void hide_vga_cursor() {
    outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void show_vga_cursor() { // This assumes placing it at 0,0 and can be moved seperately
    outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | 1);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | 1);
}

void update_cursor(int x, int y) {
	uint16_t pos = y * VGA_WIDTH + x;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}


