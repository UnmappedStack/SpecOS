/*
Some binary operation functions written for SpecOS.
Copyright (C) 2024 Jake Steinburger under the MIT license.
See the github repo for more information.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "binop.h"

uint32_t combine32bit(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
    return ((uint32_t)byte1 << 24) |
        ((uint32_t)byte2 << 16) |
        ((uint32_t)byte3 << 8)  |
        (uint32_t)byte4;
}
