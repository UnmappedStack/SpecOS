/* just get some stuff from limine
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more info.
 */

#ifndef BOOTINFO_H
#define BOOTINFO_H

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

#endif
