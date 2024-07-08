/* VGA driver for SpecOS 64 bit version.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../limine.h"
#include "../misc/font.h"
#include "include/serial.h"

#include "include/vga.h"

// A bunch of stuff I gotta do to set up the frame buffer

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

int screenWidth;
int screenHeight;

void initVGA() {
    // I swear, this had better be the right version!
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
        __asm__("cli; hlt");
    // Plz give me a valid framebuffer lemon- I mean, Limine.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1)
        __asm__("cli; hlt");
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    screenWidth = framebuffer->width;
    screenHeight = framebuffer->height;
}

void drawPix(int x, int y, int colour) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;
    // uhh hopefully this'll work but tbh I have no idea.
    fb_ptr[y * (framebuffer->pitch / 4) + x] = colour;
}

int chX = 5;
int chY = 5;

void drawChar(int xOffset, int yOffset, int colour, char ch) {
    int firstByteIdx = ch * 8;
    bool doDrawBuffer;
    int colourBuffer;
    for (int by = 0; by < 8; by++) { 
        for (int bi = 0; bi < 8; bi++) {
            doDrawBuffer = (fontdata_8x8[firstByteIdx + by] >> (7 - bi)) & 1;
            colourBuffer = doDrawBuffer * colour;
            drawPix(xOffset + bi, yOffset + by, colourBuffer);
        }
    }
    // and then I just gotta hope it works (:
}

void writeChar(char ch, int colour) {
    drawChar(chX, chY, colour, ch);
    chX += 10;
}

int strlen(char* str) {
    int i = 0;
    while (1) {
        if (str[i] == '\0')
            return i;
        i++;
    }
}

void newline() {
    chY += 10;
    chX = 5;
}

void clearScreen() {
    for (int i = 0; i < screenWidth; i++) {
        for (int y = 0; y < screenHeight; y++)
           drawPix(i, y, 0x0); 
    }
}

void writestring(char* str) {
    // Not rlly doing proper scrolling for now. If it's too big, clear the screen
    if (chY >= (screenHeight / 3) * 2)
        clearScreen();
    for (int i = 0; i < strlen(str); i++) {
        /* obvs this makes newline if it reaches a newline char, BUT it also
         * makes a new line if chX + 8 (aka. the end of the next char to be drawn) is more than screenWidth.
         */
        if (str[i] == '\n') {
            newline();
            continue;
        } else if (chX > ((screenWidth / 3) * 2)) {
            newline(); // this is a seperate block cos in this case, it shouldn't skip to the next thingy
        }
        writeChar(str[i], 0xFFFFFF);
    }
}












