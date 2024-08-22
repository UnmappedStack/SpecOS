/* VGA driver for SpecOS 64 bit version.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../limine.h"
#include "../misc/font.h"
#include "../utils/include/string.h"
#include "include/serial.h"
#include "../fs/include/api.h" // for reading font files
#include "../misc/bootInfo.h"
#include "../include/kernel.h"
#include "include/vga.h"

// A bunch of stuff I gotta do to set up the frame buffer
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void pushBackLastString(char* newStr) {
    for (int i = 0; i < 9; i++)
        kernel.last10[i] = kernel.last10[i + 1];
    kernel.last10[9] = newStr;
}

void initVGA() {
    // I swear, this had better be the right version!
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
        __asm__("cli; hlt");
    // Plz give me a valid framebuffer lemon- I mean, Limine.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1)
        __asm__("cli; hlt");
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    kernel.screenWidth = framebuffer->width;
    kernel.screenHeight = framebuffer->height;
    kernel.bpp = framebuffer->bpp;
}

void drawPix(int x, int y, int colour) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;
    // uhh hopefully this'll work but tbh I have no idea.
    //int pixelWidth = framebuffer->bpp;
    //fb_ptr[y * (framebuffer->pitch / pixelWidth) + x] = colour;
    uint32_t* location = (uint32_t*)(((uint8_t*)fb_ptr) + y*framebuffer->pitch);
    location[x] = colour;
}

void drawChar(int xOffset, int yOffset, int colour, char ch) {
    int firstByteIdx = ch * 16;
    bool doDrawBuffer;
    int colourBuffer;
    for (int by = 0; by < 16; by++) { 
        for (int bi = 0; bi < 8; bi++) {
            doDrawBuffer = (fontGlyphBuffer[firstByteIdx + by] >> (7 - bi)) & 1;
            if (doDrawBuffer)
                colourBuffer = colour;
            else
                colourBuffer = kernel.bgColour;
            drawPix(xOffset + bi, yOffset + by, colourBuffer);
        }
    }
    // and then I just gotta hope it works (:
}

void writeChar(char ch, int colour) {
    drawChar(kernel.chX, kernel.chY, colour, ch);
    kernel.chX += 10;
    outCharSerial(ch);
}

void scrollLine();

void newline() {
    kernel.chY += 16;
    kernel.chX = 5;
    if (kernel.chY > kernel.screenHeight - 17) scrollLine();
}

void clearScreen() {
    for (int i = 0; i < kernel.screenWidth; i++) {
        for (int y = 0; y < kernel.screenHeight; y++)
           drawPix(i, y, 0x0); 
    }
    kernel.chX = 5;
    kernel.chY = 5;
}

void scrollPixel(int pixels) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;
    for (int y = 0; y <= kernel.screenHeight; y++) {
        for (int x = 0; x < kernel.screenWidth; x++) {
            if (y > (kernel.screenHeight - pixels)) {
                drawPix(x, y, kernel.bgColour);
                continue;
            }
            // get the colour at the pixel the line above
            uint32_t *location = (uint32_t*)(((uint8_t*)fb_ptr) + (y + pixels) * framebuffer->pitch);
            uint32_t abovePixelValue = location[x];
            // draw it at this spot
            drawPix(x, y, abovePixelValue);
        }
    }
}

void scrollLine() {
    scrollPixel(33);
    kernel.chY -= 33;
}

void writestring(char* str) {
    if (kernel.doPush)
        pushBackLastString(str);
    if (kernel.chY > kernel.screenHeight - 16) scrollLine();
    for (int i = 0; i < strlen(str); i++) {
        /* obvs this makes newline if it reaches a newline char, BUT it also
         * makes a new line if chX + 8 (aka. the end of the next char to be drawn) is more than screenWidth.
         */
        if (str[i] == '\n') {
            newline();
            outCharSerial('\n');
            continue;
        } else if (kernel.chX > (kernel.screenWidth - 9)) {
            newline(); // this is a seperate block cos in this case, it shouldn't skip to the next thingy
        }
        writeChar(str[i], kernel.colourOut);
    }
}












