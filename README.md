# SpecOS
An x86 OS kernel from scratch.

> **NOTE**
> 
> This project is still in very early stages, and should NOT be used on real hardware. The code is quite messy and currently all in one big kernel C file.

SpecOS is a 32 bit operating system kernel for x86 processors, still in quite early stages, written in (questionable quality) C. It is (not very) powerful.

## Features:
* Basic keyboard driver with scanf implementation (backspaces are a little iffy)
* Basic test userspace with device shutdown, time/date driver, colours, and an echo command

## Future features
I'm currently working on handling some exception interrupts to make future development easier, as well as tidying up the code. After that, I hope to write a memory management system with paging (terrifying), then a FAT32 file system. Then I'd like to be able to parse and run ELF userland files, implement the C standard library, and maybe, just *maybe* I'll be able to get some networking stack at some point. It's a road to running Doom!

Note that a lot of this is unrealistic and probably won't happen - it's a lot easier said that done. But my hopes are high!
