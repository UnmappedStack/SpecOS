/* Header for ../io.c as part of the SpecOS project.
 * I'm not putting this under a license cos it's super generic.
 */

#ifndef IO_H
#define IO_H

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t val);

void outw(uint16_t port, uint16_t val);

uint16_t inw(uint16_t port);

#endif 
