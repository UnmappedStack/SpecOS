# SpecOS
An x86 OS kernel from scratch.

> **NOTE**
> 
> This project is still in very early stages, and should NOT be used on real hardware. The code is quite messy and currently all split between just two C files, a single header file, and a series of Assembly files for the bootloader.

SpecOS is a 32 bit operating system kernel for x86 processors, still in quite early stages, written in (questionable quality) C. It is (not very) powerful.

## Building and running
This works best on Linux (Mac should also be fine). If you're using Windows, it's best to use WSL.
### To compile
Make sure that you have GCC, NASM and Qemu installed, as well as the i686-elf toolkit. This is easy to install with Brew (I even used Brew on Linux). Then simply clone the repo, cd into it, make `compile.sh` runnable and run the script:
```
git clone https://github.com/jakeSteinburger/SpecOS.git
cd SpecOS
chmod +x compile.sh
./compile.sh
```
This will generate a an executable disk image that you can run with qemu.
### Running
### On Qemu
Simply cd into the directory of the built .bin file, and run:
```
qemu-system-i386 -fda SpecOS.bin
```
You'll obviously need Qemu installed.

### On real hardware
This is ***not recommended***, and it has ***not been tested***, however you're welcome to give it a shot. In the directory of the .bin file, run:

```
sudo dd if=SpecOS.bin of=/dev/sdN bs=4M status=progress
```

With /dev/sdN being the name of your USB. Then in your device's unique BIOS, change the boot order so it will boot from USB before your current OS, and restart with your now-formatted USB plugged in.

## Features:
* Basic keyboard driver with scanf implementation
* Basic test userspace with device shutdown, time/date driver, colours, and an echo command

## Future features
I'm currently working on handling some exception interrupts to make future development easier, as well as tidying up the code. After that, I hope to write a memory management system with paging (terrifying), then a FAT32 file system. Then I'd like to be able to parse and run ELF userland files, implement the C standard library, and maybe, just *maybe* I'll be able to get some networking stack at some point. It's a road to running Doom!

Note that a lot of this is unrealistic and probably won't happen - it's a lot easier said that done. But my hopes are high!
