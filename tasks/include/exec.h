/* Header file for ../exec.c, as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#ifndef EXEC_H
#define EXEC_H

void runModuleElf(int moduleNum);
int exec(uintptr_t elfAddr);

#endif
