/* Header for hard disk driver (ATA PIO mode) as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */


#ifndef DISK_H
#define DISK_H

char* readdisk(int32_t sect);

void writedisk(int32_t sect, char* data);

bool identifyCompatibility();

bool identifyInitiate();

#endif

