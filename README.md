# SpecOS
![Commit activity](https://img.shields.io/github/commit-activity/m/jakeSteinburger/SpecOS/main)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ddef159bc34148e4a89d2a600e9b61e5)](https://app.codacy.com/gh/jakeSteinburger/SpecOS/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
![GitHub top language](https://img.shields.io/github/languages/top/jakeSteinburger/SpecOS?logo=c&label=)
[![GitHub license](https://img.shields.io/github/license/jakeSteinburger/SpecOS)](https://github.com/jakeSteinburger/SpecOS/blob/main/LICENSE)

An x86-64 OS kernel from scratch.

> **NOTE**
> 
> This project is still in very early stages, and should NOT be used on real hardware. The code is quite messy and is currently a series of messy C files. Seriously, don't learn anything from this project.

SpecOS is a 64 bit operating system kernel for x86-64 processors, still in quite early stages, written in (questionable quality) C. It is (not very) powerful.

This used to be 32 bit, but has been transferred to a 64 bit operating system.

It uses a monolithic kernel, because I like having everything in one place. This may take some inspiration from other operating systems, but it is **not UNIX based**. 

See some of the resources I used for learning, as well as some other OS projects that I think are pretty amazing in [resources.md](https://github.com/jakeSteinburger/SpecOS/blob/main/resources.md). There's also a few unsolicited OS development opinions of mine in there (:

You may notice that SpecOS now has GRUB files that it didn't have before. This is because a custom bootloader was quite easy before, only needing to load specific sectors, but now that I need to create a FAT image, it becomes much more difficult. For this reason, I have moved away from a custom bootloader and now am using the GRUB bootloader.

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
Note that you must be inside the scripts directory to run the compile script.

This will generate a an executable disk image that you can run with qemu.
### Running
### On Qemu
Simply cd into the directory of the built .img file, and run:
```
qemu-system-x86_64 disk.img
```
You'll obviously need Qemu installed.

### On real hardware
This is ***not recommended***, and it has ***not been tested***, however you're welcome to give it a shot. In the directory of the .img file, run:

```
sudo dd if=disk.img of=/dev/sdN bs=4M status=progress
```

With /dev/sdN being the name of your USB. Then in your device's unique BIOS, change the boot order so it will boot from USB before your current OS, and restart with your now-formatted USB plugged in.

## Roadmap
Some things that I gotta do, and some things that I have done, in SpecOS. Yeah, there's a lot I haven't done. Really gives an idea on just how much there is to do.
- [X] First steps
  - [X] VGA driver
  - [X] Interrupt handling
  - [X] PS/2 Keyboard support
- [X] RTC driver
- [X] ATA PIO mode hard disk driver (28 bit LBA)
- [ ] FAT filesystem
  - [X] Read (cd, ls, cat)
  - [ ] Write (mkdir, touch, rm, editfile, cp, mv)
- [X] Do better scrolling
- [ ] Memory management
  - [X] Physical memory management (pooling allocator)
  - [X] Virtual memory management (paging)
  - [ ] Implement a heap (dynamic userspace stuff)
- [X] ELF parsing
- [ ] Switch to 64 bit higher half
  - [X] Set up Limine (w/ loopback device)
  - [X] Write 64 bit version of GDT & IDT
  - [X] Draw text to graphical framebuffer provided by limine
  - [X] Port to 64 bit version (FAT, RTC, hard disk driver, keyboard)
  - [X] Detect memory with limine
  - [X] Re-write PMM to use bitmap instead of pooling
  - [ ] Re-write 64 bit version of paging
- [ ] Jump to userspace (apps are a big milestone for me!)
- [ ] Write libc (maybe port one? I don't really know yet.)
- [ ] Some sort of NIC driver
- [ ] Port lwIP for networking (I can't bother writing this myself)
- [ ] Some ports
  - [ ] Vim (or some other text editor)
  - [ ] Python (possibly very difficult)
- [ ] More drivers for compatibility
  - [ ] USB (likely *very* hard)
  - [ ] SSD
  - [ ] HDMI
  - [ ] DVI
  - [ ]  More NIC drivers
- [ ] GUI
  - [ ] Window server + plotting pixels
  - [ ] Text with a custom font
  - [ ] Mouse driver
  - [ ] Present another app's frame buffer within the main GUI
  - [ ] Window management: moving, closing, resizing
  - [ ] A few GUI apps for settings etc.
- [ ] ACPI driver (really should be a higher priority...)
- [ ] Write a basic web browser (possibly very hard, but I was thinking of doing this as a seperate project anyway, so why not combine?)

It's a road to running DOOM!

Note that a lot of this is unrealistic and probably won't happen - it's a lot easier said that done. But my hopes are high (:

## Special thanks
You didn't *really* think that *I* could do this alone of all people, did you? I'm not a genius! Thank you so, so much to these people who have made my journey in operating system development so much easier!

 - [Bananymous](https://github.com/Bananymous) is a super smart guy, who taught me a *ton*. He also wrote the amazing [Banan-OS](https://github.com/Bananymous/banan-os)!
 - [Dcraftbg](https://github.com/Dcraftbg) has taught me a whole lot about paging and a few other topics. He also just managed to enter userspace on his [MinOS](https://github.com/Dcraftbg/MinOS), nice one :D
