#!/bin/bash

echo "Making sure Brew installations are accessable..."
[ -d /home/linuxbrew/.linuxbrew ] && eval $(/home/linuxbrew/.linuxbrew/bin/brew shellenv)

echo "Compiling kernel..."
i686-elf-gcc -ffreestanding -c kernel.c -o bin/kernel.o
i686-elf-gcc -ffreestanding -c drivers/terminalWrite.c -o bin/terminalWrite.o
i686-elf-gcc -ffreestanding -c drivers/disk.c -o bin/disk.o
i686-elf-gcc -ffreestanding -c bouncy.c -o bin/bouncy.o
i686-elf-gcc -ffreestanding -c drivers/rtc.c -o bin/rtc.o
i686-elf-gcc -ffreestanding -c utils/inx.c -o bin/inx.o
i686-elf-gcc -ffreestanding -c sys/gdt.c -o bin/gdt.o
i686-elf-gcc -ffreestanding -c sys/idt.c -o bin/idt.o
i686-elf-gcc -ffreestanding -c utils/string.c -o bin/string.o
i686-elf-gcc -ffreestanding -c drivers/keyboard.c -o bin/keyboard.o
nasm bootutils/padsectors.asm -f bin -o bin/sectorpad.bin

echo "Compiling kernel entry..."
nasm kernel_entry.asm -f elf -o bin/kernel_entry.o

echo "Linking..."
i686-elf-ld -o bin/kernel.bin -Ttext 0x1000 bin/kernel_entry.o bin/string.o bin/inx.o bin/gdt.o bin/idt.o bin/keyboard.o bin/kernel.o bin/disk.o bin/terminalWrite.o bin/bouncy.o bin/rtc.o --oformat binary

echo "Compiling bootloader..."
nasm bootloader.asm -f bin -o bin/bootloader.bin

echo "Concatenating bootloader and kernel..."
cat bin/bootloader.bin bin/kernel.bin > bin/SpecOS_nopad.bin
cat bin/SpecOS_nopad.bin bin/sectorpad.bin > SpecOS.bin

echo "Complete (status unknown)"
