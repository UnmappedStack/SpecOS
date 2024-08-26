/* Vector implementation for SpecOS - dynamically sized arrays.
 * This is a bit tricky since there's no calloc or realloc in kernelspace, only raw malloc and free.
 * It's not the most efficient, so don't use it when not necessary, and make sure to always delete one when not needed anymore.
 * However, it'll be *very* useful at times, so it is here for a good reason.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "include/printf.h"
#include "include/vector.h"
#include "../mem/include/kheap.h"
#include "../sys/include/panic.h"

#define BYTES_MAX_SIZE(bytes) ((1ULL << ((bytes) * 8)) - 1)

Vector newVect(uint8_t elementSize) {
    Vector toReturn;
    toReturn.firstElement = 0;
    toReturn.length       = 0;
    toReturn.elementSize  = elementSize;
    return toReturn;
}

void vectPush(Vector vect, uint64_t value) {
    if (value > BYTES_MAX_SIZE(vect.elementSize)) {
        printf("\n\nFailed to append element to vector: element is larger in size than specified vector size."
               "\nExpected vector element size: %i byte(s)"
               "\nElement that was attempted to push: %b"
               "\nMax expected size: %i\n\nHalting device.", 
               vect.elementSize, value, BYTES_MAX_SIZE(vect.elementSize));
        asm("cli; hlt");
    }
    uintptr_t dataAddr = (uintptr_t) malloc(vect.elementSize);
    *((uint64_t*) dataAddr) = value;
    VectorElement newElement;
    newElement.nextElement = 0;
    newElement.data        = dataAddr;
    uintptr_t elementAddr = (uintptr_t) malloc(sizeof(VectorElement));
    printf("Address provided by malloc: 0x%x\n", elementAddr);
    *((VectorElement*)elementAddr) = newElement;
    if (vect.firstElement == 0) {
        vect.firstElement = elementAddr;
    } else {
        // find last element
        VectorElement *toCheck = (VectorElement*) vect.firstElement;
        while (true) {
            if (toCheck->nextElement == 0) {
                toCheck->nextElement = elementAddr;
                break;
            } else {
                toCheck = (VectorElement*) toCheck->nextElement;
            }
        }
    }
}
