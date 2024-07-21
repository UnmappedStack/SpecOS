#!/bin/bash

echo "Making sure Brew installations are accessable..."
[ -d /home/linuxbrew/.linuxbrew ] && eval $(/home/linuxbrew/.linuxbrew/bin/brew shellenv)

echo "Compiling kernel..."
i686-elf-gcc -ffreestanding -Wall -c ../kernel.c -o ../bin/kernel.o
i686-elf-gcc -ffreestanding -Wall -c ../drivers/terminalWrite.c -o ../bin/terminalWrite.o
i686-elf-gcc -ffreestanding -Wall -c ../drivers/disk.c -o ../bin/disk.o
i686-elf-gcc -ffreestanding -Wall -c ../bouncy.c -o ../bin/bouncy.o
i686-elf-gcc -ffreestanding -Wall -c ../drivers/rtc.c -o ../bin/rtc.o
i686-elf-gcc -ffreestanding -Wall -c ../utils/inx.c -o ../bin/inx.o
i686-elf-gcc -ffreestanding -Wall -c ../sys/gdt.c -o ../bin/gdt.o
i686-elf-gcc -ffreestanding -Wall -c ../sys/idt.c -o ../bin/idt.o
i686-elf-gcc -ffreestanding -Wall -c ../utils/string.c -o ../bin/string.o
i686-elf-gcc -ffreestanding -Wall -c ../drivers/keyboard.c -o ../bin/keyboard.o
i686-elf-gcc -ffreestanding -Wall -c ../fs/parseBootRecord.c -o ../bin/parseBootRecord.o
i686-elf-gcc -ffreestanding -Wall -c ../fs/readClusterChain.c -o ../bin/readClusterChain.o
i686-elf-gcc -ffreestanding -Wall -c ../fs/decodeDirectory.c -o ../bin/decodeDirectory.o
i686-elf-gcc -ffreestanding -Wall -c ../utils/binop.c -o ../bin/binop.o
i686-elf-gcc -ffreestanding -Wall -c ../shell.c -o ../bin/shell.o
i686-elf-gcc -ffreestanding -Wall -c ../fs/api.c -o ../bin/fsapi.o
i686-elf-gcc -ffreestanding -Wall -c ../mem/detect.c -o ../bin/detect.o
i686-elf-gcc -ffreestanding -Wall -c ../mem/pmm.c -o ../bin/pmm.o
i686-elf-gcc -ffreestanding -Wall -c ../mem/vmm.c -o ../bin/vmm.o
i686-elf-gcc -ffreestanding -Wall -c ../mem/kmalloc.c -o ../bin/kmalloc.o
i686-elf-gcc -ffreestanding -Wall -c ../drivers/serial.c -o ../bin/serial.o
i686-elf-gcc -ffreestanding -Wall -c ../userspace/parseElf.c -o ../bin/parseElf.o

echo "Linking..."
i686-elf-ld -r -o ../bin/kernelout.o -Ttext 0x1000 ../bin/parseElf.o ../bin/serial.o ../bin/detect.o ../bin/pmm.o ../bin/kmalloc.o ../bin/vmm.o ../bin/binop.o ../bin/string.o ../bin/inx.o ../bin/gdt.o ../bin/idt.o ../bin/keyboard.o ../bin/kernel.o ../bin/disk.o ../bin/parseBootRecord.o ../bin/readClusterChain.o ../bin/decodeDirectory.o ../bin/terminalWrite.o ../bin/fsapi.o ../bin/bouncy.o ../bin/rtc.o ../bin/shell.o

echo "Compiling multiboot header..."
nasm -felf32 ../multiboot.asm -o ../bin/multiboot.o

echo "Linking... (again, this time kernel with the multiboot header)"
i686-elf-gcc -T ../linker.ld -o ../bin/kernel.bin -ffreestanding -O2 -nostdlib ../bin/multiboot.o ../bin/kernelout.o -lgcc

echo "Creating virtual FAT image with a loopback device..."
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
bash "$DIR/loopback.sh"
