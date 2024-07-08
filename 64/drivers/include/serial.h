/* Header for serial.c as part of SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the github repo for more info.
 */

#ifndef SERIAL_H
#define SERIAL_H

void outCharSerial(char ch);

int init_serial();

#endif
