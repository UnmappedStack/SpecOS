/* Header file for ../kheap.c
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more info.
 */

#ifndef KHEAP_H
#define KHEAP_H

void initKHeap();

void* malloc(int size);

void free(void* location);

#endif
