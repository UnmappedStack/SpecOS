#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "rtc.h"

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}



enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};

unsigned char get_RTC_register(int reg) {
      outb (cmos_address, reg);
      return inb(cmos_data);
}

unsigned char getTimeUnit(int reg) {
    unsigned char unit = get_RTC_register(reg);
    unsigned char registerB = get_RTC_register(0x0B);
    if (!(registerB & 0x04)) {
        if (reg == 0x04) { // If it's the hour unit, it get's converted differently
            unit =  ((unit & 0x0F) + (((unit & 0x70) / 16) * 10) ) | (unit & 0x80);
        } else {
            unit = (unit & 0x0F) + ((unit / 16) * 10);
        }
    }
    return unit;
}

const char* wholeDate() {
    unsigned char day = getTimeUnit(0x07);
    unsigned char month = getTimeUnit(0x08);
    unsigned char year = getTimeUnit(0x09);
    static char toReturn[9];
    // Convert day to string
    toReturn[0] = '0' + day / 10;
    toReturn[1] = '0' + day % 10;
    toReturn[2] = '/';

    // Convert month to string
    toReturn[3] = '0' + month / 10;
    toReturn[4] = '0' + month % 10;
    toReturn[5] = '/';

    // Convert year to string
    toReturn[6] = '0' + year / 10;
    toReturn[7] = '0' + year % 10;
    toReturn[8] = '\0'; // Null-terminate the string
    return toReturn;
}

const char* wholeTime() {
    unsigned char hour = getTimeUnit(0x04);
    unsigned char minute = getTimeUnit(0x02);
    static char toReturn[6];
    toReturn[0] = '0' + hour / 10;
    toReturn[1] = '0' + hour % 10;
    toReturn[2] = ':';
    toReturn[3] = '0' + minute / 10;
    toReturn[4] = '0' + minute % 10;
    toReturn[5] = '\0';
    return toReturn;
}
