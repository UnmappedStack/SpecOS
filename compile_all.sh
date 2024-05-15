#!/bin/bash

echo "Assembling boot.s..."
i686-elf-as boot.s -o boot.o

echo "Compiling C kernel..."
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "Linking the kernel..."
i686-elf-gcc -T linker.ld -o specos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

if grub-file --is-x86-multiboot specos.bin; then
  echo Multiboot confirmed
else
  echo ERROR: The file is not multiboot.
fi

echo "Setting up environment..."
rm -r isodir
mkdir -p isodir/boot/grub
cp specos.bin isodir/boot/specos.bin
cp grub.cfg isodir/boot/grub/grub.cfg

echo "Creating a bootable image..."
grub-mkrescue -o specos.iso isodir

echo "Complete."
