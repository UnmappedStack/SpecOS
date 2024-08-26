/* Header file for ../vector.c, as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stddef.h>

#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    uintptr_t nextElement;
    uintptr_t data;
} VectorElement;

typedef struct {
    uint8_t elementSize;
    uint16_t length;
    uintptr_t firstElement;
} Vector;


Vector newVect(uint8_t elementSize);

void vectPush(Vector vect, uint64_t value);

#endif
