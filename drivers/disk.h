#ifndef DISK_H
#define DISK_H

char* readdisk(int32_t sect);

void writedisk(int32_t sect, char* data);

bool identifyCompatibility();

bool identifyInitiate();

#endif
