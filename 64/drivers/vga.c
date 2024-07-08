/* VGA driver for SpecOS 64 bit version.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../limine.h"

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

void initVGA() {
    // I swear, this had better be the right version!
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
        __asm__("cli; hlt");
    // Plz give me a valid framebuffer lemon- I mean, Limine.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1)
        __asm__("cli; hlt");
}

void drawPix(int x, int y, int colour) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;
    // uhh hopefully this'll work but tbh I have no idea.
    fb_ptr[(framebuffer->pitch * y) + x * 4] = colour;
}
