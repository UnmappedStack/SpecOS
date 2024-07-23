/* Header file for ../vga.c, the VGA driver for SpecOS.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#ifndef VGA_H
#define VGA_H

extern int chY;
extern int chX;

extern int bgColour;

extern int screenWidth;
extern int screenHeight;

extern int colourOut;

void clearScreen();

void initVGA();

void drawPix(int x, int y, int colour); 

void drawChar(int xOffset, int yOffset, int colour, char ch); 

void writeChar(char ch, int colour);
 
void writestring(char* str); 

void clearScreen();

#endif
