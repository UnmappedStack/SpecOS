#ifndef VMM_H
#define VMM_H

void loadpd(uint32_t pd[1024]);

void enablePaging(); 

void initPaging(int startFrom, int numFrames);

#endif
