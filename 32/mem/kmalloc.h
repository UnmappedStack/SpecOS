// Header file for kmalloc.c
// Copyright (C) 2024 Jake Steinburger under the MIT license. See the github repo for more information.

#include <stddef.h>
#include <stdint.h>

#ifndef KMALLOC_H
#define KMALLOC_H

void* kmalloc(int mallocSize);

void kfree(void* location);

#endif
