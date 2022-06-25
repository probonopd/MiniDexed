#!/bin/bash

# this script install the dev environment and assume that the repositorw is alreadw cloned

# Choose your RPi
export RPI=4

#git clone https://github.com/probonopd/MiniDexed
#cd MiniDexed
mkdir -p kernels sdcard

# Get develop branch of circle
cd circle-stdlib/libs/circle
git checkout ae22928 # develop
cd -

# Recursively pull git submodules
git submodule update --init --recursive

# Install toolchain
if [ "${RPI}" -gt 2 ]
then
	wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
else
	wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz
fi
tar xvf gcc-arm-*-*.tar.xz 
export PATH=$(readlink -f ./gcc-*/bin/):$PATH

# Build dependencies and MiniDexed
./build.sh
cp ./src/kernel*.img ./kernels/

# Get Raspberry Pi boot files
cd ./circle-stdlib/libs/circle/boot
make
if [ "${RPI}" -gt 2 ]
then
	make armstub64
fi
cd -

# Make zip that contains Raspberry Pi 4 boot files. The contents can be copied to a FAT32 formatted partition on a microSD card
cd sdcard
../getsysex.sh
cd ..
cp -r ./circle-stdlib/libs/circle/boot/* sdcard
rm -rf sdcard/config*.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
cp ./src/config.txt ./src/minidexed.ini ./src/*img ./src/performance.ini sdcard/
echo "usbspeed=full" > sdcard/cmdline.txt
cd sdcard
cp ../kernels/* . || true
zip -r ../MaxiDexed_$GITHUB_RUN_NUMBER_$(date +%Y-%m-%d).zip *
cd -

# # Optionally, create a RPi image. This can be written to a microSD card using tools like Etcher or dd
# sudo apt install --yes  mount parted
# IMG="`date +%Y-%m-%d`_minidexed-RPi${RPI}.img"
# dd of="${IMG}" seek=50MiB bs=1 count=0
# sudo parted "${IMG}" mktable msdos
# sudo parted "${IMG}" mkpart primary fat32 2048s 100%
# DEV=`sudo losetup --find --partscan --show "${IMG}"`
# sudo mkfs.vfat -F 32 -n BOOT "${DEV}p1"
# mkdir boot
# sudo mount "${DEV}p1" boot
# sudo cp -R sdcard/* boot
# sudo umount boot
# sudo losetup -d "${DEV}"
# rm -r boot

# # Write to SD card
# sudo dd if="${IMG}" of=/dev/mmcblk0 bs=512k status=progress && sync