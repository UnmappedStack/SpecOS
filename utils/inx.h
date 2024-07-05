#include <stddef.h>
#include <stdint.h>

#ifndef INX_H
#define INX_H

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t val);

void outw(uint16_t port, uint16_t val);

uint16_t inw(uint16_t port);

#endif 
