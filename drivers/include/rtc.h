/* Header for ../rtc.c, the real time clock driver as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#ifndef RTC_H
#define RTC_H

unsigned char getTimeUnit(int reg);

const char* wholeDate();

const char* wholeTime(); 

#endif 
