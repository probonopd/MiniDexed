# MiniDexed ![](https://github.com/probonopd/MiniDexed/actions/workflows/build.yml/badge.svg)

[Dexed](https://asb2m10.github.io/dexed/) is a FM synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer. MiniDexed is a port to run it on a bare metal Raspberry Pi (without a Linux kernel or operating system). __Currently it is not functional yet. Contributions are highly welcome.__

## TODO

 Contributions are highly welcome.

- [x] Get Dexed to build with [circle-stdlib](https://github.com/smuehlst/circle-stdlib)
- [x] Upload SD card contents to [GitHub Releases](../../releases)
- [x] Get it to boot on a Raspberry Pi 4
- [x] Get it to react to USB MIDI
- [x] Get it to react to MIDI via Raspberry Pi 4 GPIO
- [x] Get it to produce some sound on the headphone jack
- [ ] Get it to produce some sound produced by Dexed
- [ ] Get 8 Dexed instances to run simultaneously (like in a TX816) and mix their output together
- [ ] Add functionality for loading `.syx` files (e.g., from [Dexed_cart_1.0.zip](http://hsjp.eu/downloads/Dexed/Dexed_cart_1.0.zip))
- [ ] Add a way to configure each Dexed instance
- [ ] Allow for each Dexed instance to be stereo shifted
- [ ] Add reverb effect

I am wondering whether Dexed could be ported to Circle, in order to recreate basically an open source equivalent of the TX802 (8 DX7 instances without the keyboard in one box).

## Usage

* Download from [GitHub Releases](../../releases)
* Unzip
* Put the files into the root directory of a FAT32 formatted partition on microSD card
* Boot in Raspberry Pi 4

## Building locally

E.g., on Ubuntu 20.04:

```
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
./configure -r 4 --prefix "aarch64-none-elf-"
make -j$(nproc)
cd ..

# Build MiniDexed
export PATH=$(readlink -f ./gcc-*/bin/):$PATH
cd src
make -j$(nproc)
ls *.img
cd ..

# Get Raspberry Pi boot files
cd ./circle-stdlib/libs/circle/boot
make
make armstub64
cd -

# Make zip that contains Raspberry Pi 4 boot files
mkdir -p sdcard
cp -r ./circle-stdlib/libs/circle/boot/* sdcard
mv sdcard/config64.txt sdcard/config.txt
rm -rf sdcard/config32.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
cp ./src/*img sdcard/
zip -r MiniDexed_Raspberry_Pi_4.zip sdcard/*
```

## Acknowledgements

* [dcoredump](https://github.com/dcoredump) for https://codeberg.org/dcoredump/MicroDexed, a Dexed port to a microcontroller
* [rsta2](https://github.com/rsta2) for the initial code and for https://github.com/rsta2/circle, the library to run code on bare betal Raspberry Pi (without a Linux kernel or operating system)
* [smuehlst](https://github.com/smuehlst) for https://github.com/smuehlst/circle-stdlib, a version with Standard C and C++ library support
