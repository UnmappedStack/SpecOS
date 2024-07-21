/* Header for ../keyboard.c, a PS/2 keyboard driver as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

void scanf(char* inp);
void isr_keyboard(void*);

#endif
