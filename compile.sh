#!/bin/bash

echo "Making sure Brew installations are accessable..."
[ -d /home/linuxbrew/.linuxbrew ] && eval $(/home/linuxbrew/.linuxbrew/bin/brew shellenv)

echo "Compiling kernel..."
i686-elf-gcc -ffreestanding -c kernel.c -o kernel.o
i686-elf-gcc -ffreestanding -c drivers/terminalWrite.c -o terminalWrite.o
i686-elf-gcc -ffreestanding -c drivers/disk.c -o disk.o
i686-elf-gcc -ffreestanding -c bouncy.c -o bouncy.o
nasm bootutils/padsectors.asm -f bin -o sectorpad.bin

echo "Compiling kernel entry..."
nasm kernel_entry.asm -f elf -o kernel_entry.o

echo "Linking..."
i686-elf-ld -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o disk.o terminalWrite.o bouncy.o --oformat binary

echo "Compiling bootloader..."
nasm bootloader.asm -f bin -o bootloader.bin

echo "Concatenating bootloader and kernel..."
cat bootloader.bin kernel.bin > SpecOS_nopad.bin
cat SpecOS_nopad.bin sectorpad.bin > SpecOS.bin

echo "Complete (status unknown)"
