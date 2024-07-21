/*
Header file for memory detection in SpecOS (detect.c)
Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more info. 
*/

#include <stdint.h>
#include <stddef.h>

#ifndef DETECT_H
#define DETECT_H

#include "multiboot.h"

void detectMemmap(multiboot_info_t* mbd, uint32_t magic);

#endif
