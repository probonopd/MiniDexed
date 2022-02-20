# MiniDexed ![](https://github.com/probonopd/MiniDexed/actions/workflows/build.yml/badge.svg)

[Dexed](https://asb2m10.github.io/dexed/) is a FM synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer. MiniDexed is a port to run it on a bare metal Raspberry Pi (without a Linux kernel or operating system). __This is a work in progress. Contributions are highly welcome.__

## TODO

 Contributions are highly welcome.

- [x] Get [Synth_Dexed](https://codeberg.org/dcoredump/Synth_Dexed) to build with [circle-stdlib](https://github.com/smuehlst/circle-stdlib)
- [x] Upload SD card contents to [GitHub Releases](../../releases)
- [x] Get it to boot on a Raspberry Pi 4
- [x] Get it to react to USB MIDI
- [x] Get it to react to MIDI via Raspberry Pi 4 GPIO
- [x] Get it to produce some sound on the headphone jack
- [x] Get it to produce some sound produced by Dexed
- [x] Load a default (hardcoded) sysex
- [x] Get it to boot on a Raspberry Pi 3
- [ ] Add functionality for loading `.syx` files (e.g., from [Dexed_cart_1.0.zip](http://hsjp.eu/downloads/Dexed/Dexed_cart_1.0.zip))
- [ ] Get 8 Dexed instances to run simultaneously (like in a TX816) and mix their output together
- [ ] Add a way to configure each Dexed instance
- [ ] Allow for each Dexed instance to be stereo shifted
- [ ] Add reverb effect

I am wondering whether we can run multiple Dexed instances, in order to recreate basically an open source equivalent of the TX802 (8 DX7 instances without the keyboard in one box).

## Usage

* Download from [GitHub Releases](../../releases)
* Unzip
* Put the files into the root directory of a FAT32 formatted partition on microSD card
* Boot in Raspberry Pi 3 or 4

## Building locally

E.g., on Ubuntu 20.04:

```
RPI=4

git clone https://github.com/probonopd/MiniDexed
cd MiniDexed

# Recursively pull git submodules
git submodule update --init --recursive

# Install toolchain
wget -q https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
tar xf gcc-arm-*-*.tar.xz 
export PATH=$(readlink -f ./gcc-*/bin/):$PATH

# Build circle-stdlib library
cd circle-stdlib/
./configure -r ${RPI} --prefix "aarch64-none-elf-"
make -j$(nproc)
cd ..

# Build MiniDexed
cd src
make -j$(nproc)
ls *.img
cd ..

# Get Raspberry Pi boot files
cd ./circle-stdlib/libs/circle/boot
make
make armstub64
cd -

# Make zip that contains Raspberry Pi 4 boot files. The contents can be copied to a FAT32 formatted partition on a microSD card
mkdir -p sdcard
cp -r ./circle-stdlib/libs/circle/boot/* sdcard
mv sdcard/config64.txt sdcard/config.txt
rm -rf sdcard/config32.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
cp ./src/*img sdcard/
zip -r MiniDexed_Raspberry_Pi_${RPI}.zip sdcard/*

# Optionally, create a RPi image. This can be written to a microSD card using tools like Etcher or dd
sudo apt install --yes  mount parted
IMG="`date +%Y-%m-%d`_minidexed-RPi${RPI}.img"
dd of="${IMG}" seek=50MiB bs=1 count=0
parted "${IMG}" mktable msdos
parted "${IMG}" mkpart primary fat32 2048s 49MiB
DEV=`sudo losetup --find --partscan --show "${IMG}"`
sudo mkfs.vfat -F 32 -n BOOT "${DEV}p0"
mkdir boot
sudo mount "${DEV}p1" boot
sudo cp sdcard/* boot
sudo umount boot
sudo losetup -d "${DEV}"
rm -r boot
```

## Acknowledgements

* [asb2m10](https://github.com/asb2m10/dexed) for the [Dexed](https://github.com/asb2m10/dexed) sound engine
* [dcoredump](https://github.com/dcoredump) for https://codeberg.org/dcoredump/Synth_Dexed, a port of Dexed for embedded systems
* [rsta2](https://github.com/rsta2) for https://github.com/rsta2/circle, the library to run code on bare betal Raspberry Pi (without a Linux kernel or operating system) and for the initial MiniDexed code 
* [smuehlst](https://github.com/smuehlst) for https://github.com/smuehlst/circle-stdlib, a version with Standard C and C++ library support
