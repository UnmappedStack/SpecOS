# SpecOS
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ddef159bc34148e4a89d2a600e9b61e5)](https://app.codacy.com/gh/jakeSteinburger/SpecOS/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
![GitHub top language](https://img.shields.io/github/languages/top/jakeSteinburger/SpecOS?logo=c&label=)
[![GitHub license](https://img.shields.io/github/license/jakeSteinburger/SpecOS)](https://github.com/jakeSteinburger/SpecOS/blob/main/LICENSE)

An x86 OS kernel from scratch.

> **NOTE**
> 
> This project is still in very early stages, and should NOT be used on real hardware. The code is quite messy and is currently a series of messy C files. Seriously, don't learn anything from this project.

SpecOS is a 32 bit operating system kernel for x86 processors, still in quite early stages, written in (questionable quality) C. It is (not very) powerful.

It uses a monolithic kernel, because I like having everything in one place. This may take some inspiration from other operating systems, but it is **not UNIX based**. 

See some of the resources I used for learning, as well as some other OS projects that I think are pretty amazing in [resources.md](https://github.com/jakeSteinburger/SpecOS/blob/main/resources.md). There's also a few unsolicited OS development opinions of mine in there (:

You may notice that SpecOS now has GRUB files that it didn't have before. This is because a custom bootloader was quite easy before, only needing to load specific sectors, but now that I need to create a FAT image, it becomes much more difficult. For this reason, I have moved away from a custom bootloader and now am using the GRUB bootloader.

## Building and running
This works best on Linux (Mac should also be fine). If you're using Windows, it's best to use WSL.
### To compile
Make sure that you have GCC, NASM and Qemu installed, as well as the i686-elf toolkit. This is easy to install with Brew (I even used Brew on Linux). Then simply clone the repo, cd into it, make `compile.sh` runnable and run the script:
```
git clone https://github.com/jakeSteinburger/SpecOS.git
cd SpecOS/scripts
chmod +x compile.sh
bash compile.sh
```
Note that you must be inside the scripts directory to run the compile script.

This will generate a an executable disk image that you can run with qemu.
### Running
### On Qemu
Simply cd into the directory of the built .bin file, and run:
```
qemu-system-i386 -hda disk.img
```
You'll obviously need Qemu installed.

### On real hardware
This is ***not recommended***, and it has ***not been tested***, however you're welcome to give it a shot. In the directory of the .bin file, run:

```
sudo dd if=SpecOS/scripts/disk.img of=/dev/sdN bs=4M status=progress
```

With /dev/sdN being the name of your USB. Then in your device's unique BIOS, change the boot order so it will boot from USB before your current OS, and restart with your now-formatted USB plugged in.

## Features:
* Basic keyboard driver with scanf implementation
* Basic test userspace with device shutdown, time/date driver, colours, and an echo command
* 28 bit ATA PIO mode driver which can read/write specific sectors (not in help menu yet, as it's a debug feature at the moment.)

## Future features
I plan to write a FAT32 file system driver, then I hope to write a memory management system with paging (terrifying). Then I'd like to be able to parse and run ELF userland files, implement the C standard library, and maybe, just *maybe* I'll be able to get some networking stack at some point. It's a road to running Doom!

Note that a lot of this is unrealistic and probably won't happen - it's a lot easier said that done. But my hopes are high!
