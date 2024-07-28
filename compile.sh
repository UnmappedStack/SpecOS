#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "You don't have the necessary permissions to work with files in the /mnt directory, which is necessary for mounting the loopback device. Please run as root, you can try append sudo to your command."
  exit
fi

make # compile the kernel

# clone limine repo and build it

git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1

make -C limine

# do partition/loopback-y stuff (copied from old 32 bit version of SpecOS)

if test -f "disk.img"; then
    rm disk.img
fi

if test -f "/mnt/specos"; then
    rm -rf /mnt/specos
fi

mkdir /mnt/specos

# Make an empty img file with a bunch of zeros (512 byte sectors), about 70 Megabytes large (should be enough for now)
echo "Creating empty disk image file..."
dd if=/dev/zero of=disk.img bs=512 count=131072

# Add a partition that spans the whole disk
echo "Adding a partition..."
fdisk disk.img << EOF
n
p
 


a
w
EOF

# Make a couple loopback devices for installing GRUB and all that stuff
echo "Creating loopback device (GRUB)..."
sudo losetup /dev/loop0 disk.img # GRUB
echo "Creating loopback device (FAT32 FS)..."
sudo losetup /dev/loop1 disk.img -o 1048576 # File system

# Make a FAT32 file system
echo "Making the file system (FAT32)..."
sudo mkdosfs -F32 -f 2 /dev/loop1

# Mount it
echo "Mounting onto /mnt/specos..."
sudo mount /dev/loop1 /mnt/specos


# Install limine (idk if i did this one right)
echo "Installing Limine bootloader..."
./limine/limine bios-install disk.img

mkdir /mnt/specos/boot

# Add the kernel (this assumes that the kernel bin image and the grub config file are in the cd & ../bin)
echo "Copying kernel files..."
cp bin/SpecOS /mnt/specos/boot
cp limine.cfg /mnt/specos/boot

echo "Copying boot files..."
mkdir /mnt/specos/EFI
mkdir /mnt/specos/EFI/BOOT

cp limine/limine-bios.sys /mnt/specos/boot
cp limine/BOOTX64.EFI /mnt/specos/EFI/BOOT

mkdir /mnt/specos/stuff
echo "Hey there! This is a test file on a FAT32 file system on SpecOS. Long live the kernel!" >> /mnt/specos/stuff/testfile.txt
cp fscopy/font.bin /mnt/specos

# unmount it
echo "Unmounting..."
umount /mnt/specos

# Delete loopback device
losetup -d /dev/loop0
losetup -d /dev/loop1

echo "Complete! disk.img should now be in the current directory and bootable with Qemu."
# There should now be a disk image in the current directory (:

echo "Size of kernel: "
du -h bin/SpecOS

# if image could be built, run it in qemu
if test -f "bin/SpecOS"; then
    qemu-system-x86_64 disk.img -d int -serial stdio --no-reboot --no-shutdown -accel kvm
fi

# delete old stuff
rm -rf bin obj limine /mnt/specos

