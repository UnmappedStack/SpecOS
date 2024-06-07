#include "drivers/terminalWrite.h"
#include "bouncy.h"

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
} 

void wait10m() {
    for (int i = 0; i < 10000; i++)
        outb(0x80, 0);
}

void bouncy() {
    terminal_initialize();
    int x = VGA_WIDTH / 2;
    int y = VGA_HEIGHT / 2;
    int xv = 1;
    int yv = 1;
    float speed = 100;
    enum vga_color color = VGA_COLOR_BLACK;
    while (1) {
        if (color == VGA_COLOR_WHITE)
            color = VGA_COLOR_BLACK;
        color++;
        terminal_putentryat('O', color, x, y);
        x += xv;
        y += yv;
        if (speed > 10)
            speed -= 0.5;
        if (y >= VGA_HEIGHT || y <= 0)
            yv = -yv;
        if (x >= VGA_WIDTH || x <= 0)
            xv = -xv;
        for (int j = 0; j < speed; j++)
            wait10m();
    }
}
