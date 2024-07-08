#!/bin/bash

# compile kernel
make

# set up with limine
# Download the latest Limine binary release for the 7.x branch.
git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1

# Build "limine" utility.
make -C limine

# Create a directory which will be our ISO root.
mkdir -p iso_root

# Copy the relevant files over.
mkdir -p iso_root/boot
cp -v bin/SpecOS iso_root/boot/
mkdir -p iso_root/boot/limine
cp -v limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin \
      limine/limine-uefi-cd.bin iso_root/boot/limine/

# Create the EFI boot tree and copy Limine's EFI executables over.
mkdir -p iso_root/EFI/BOOT
cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/

# Create the bootable ISO.
xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o image.iso

# Install Limine stage 1 and 2 for legacy BIOS boot.
./limine/limine bios-install image.iso

# delete old stuff
rm -rf iso_root limine obj
