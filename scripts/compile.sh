#!/bin/bash

echo "Making sure Brew installations are accessable..."
[ -d /home/linuxbrew/.linuxbrew ] && eval $(/home/linuxbrew/.linuxbrew/bin/brew shellenv)

echo "Compiling kernel..."
i686-elf-gcc -ffreestanding -c ../kernel.c -o ../bin/kernel.o
i686-elf-gcc -ffreestanding -c ../drivers/terminalWrite.c -o ../bin/terminalWrite.o
i686-elf-gcc -ffreestanding -c ../drivers/disk.c -o ../bin/disk.o
i686-elf-gcc -ffreestanding -c ../bouncy.c -o ../bin/bouncy.o
i686-elf-gcc -ffreestanding -c ../drivers/rtc.c -o ../bin/rtc.o
i686-elf-gcc -ffreestanding -c ../utils/inx.c -o ../bin/inx.o
i686-elf-gcc -ffreestanding -c ../sys/gdt.c -o ../bin/gdt.o
i686-elf-gcc -ffreestanding -c ../sys/idt.c -o ../bin/idt.o
i686-elf-gcc -ffreestanding -c ../utils/string.c -o ../bin/string.o
i686-elf-gcc -ffreestanding -c ../drivers/keyboard.c -o ../bin/keyboard.o

echo "Linking..."
i686-elf-ld -r -o ../bin/kernelout.o -Ttext 0x1000 ../bin/string.o ../bin/inx.o ../bin/gdt.o ../bin/idt.o ../bin/keyboard.o ../bin/kernel.o ../bin/disk.o ../bin/terminalWrite.o ../bin/bouncy.o ../bin/rtc.o

echo "Compiling multiboot header..."
nasm -felf32 ../multiboot.asm -o ../bin/multiboot.o

echo "Linking... (again, this time kernel with the multiboot header)"
i686-elf-gcc -T ../linker.ld -o kernel.bin -ffreestanding -O2 -nostdlib ../bin/multiboot.o ../bin/kernelout.o -lgcc

echo "Creating virtual FAT image with a loopback device..."
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
bash "$DIR/loopback.sh"
