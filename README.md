# MiniDexed ![](https://github.com/probonopd/MiniDexed/actions/workflows/build.yml/badge.svg)

[Dexed](https://asb2m10.github.io/dexed/) is a FM synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer. MiniDexed is a port to run it on a bare metal Raspberry Pi (without a Linux kernel or operating system). __This is a work in progress. Contributions are highly welcome.__

## TODO

 Contributions are highly welcome.

- [x] Get [Synth_Dexed](https://codeberg.org/dcoredump/Synth_Dexed) to build with [circle-stdlib](https://github.com/smuehlst/circle-stdlib)
- [x] Upload SD card contents to [GitHub Releases](../../releases)
- [x] Get it to build for and boot on a Raspberry Pi 4
- [x] Get it to react to USB MIDI
- [x] Get it to react to MIDI via Raspberry Pi 4 GPIO
- [x] Get it to produce some sound on the headphone jack
- [x] Get it to produce some sound produced by Dexed
- [x] Load a default (hardcoded) sysex
- [x] Get it to build for and boot on a Raspberry Pi 3
- [x] Get it to build for and boot on a Raspberry Pi 2
- [x] Get it to build for and boot on a Raspberry Pi 1 and Raspberry Pi Zero
- [x] Support multiple voices through Program Change and Bank Change LSB/MSB MIDI messages
- [x] Add functionality for loading `.syx` files from SD card (e.g., using `getsysex.sh` or from [Dexed_cart_1.0.zip](http://hsjp.eu/downloads/Dexed/Dexed_cart_1.0.zip))
- [x] Show voice name on optional [HD44780 display](https://www.berrybase.de/sensoren-module/displays/alphanumerische-displays/alphanumerisches-lcd-16x2-gr-252-n/gelb)
- [ ] Get 8 Dexed instances to run simultaneously (like in a TX816) and mix their output together
- [ ] Add a way to configure each Dexed instance through Performance sysex messages
- [ ] Allow for each Dexed instance to be stereo shifted
- [ ] Add reverb effect

I am wondering whether we can run multiple Dexed instances, in order to recreate basically an open source equivalent of the TX802 (8 DX7 instances without the keyboard in one box).

## Usage

* In the case of Raspberry Pi 4, Update the firmware and bootloader to the latest version (not doing this may cause USB reliability issues)
* Download from [GitHub Releases](../../releases)
* Unzip
* Put the files into the root directory of a FAT32 formatted partition on SD/microSD card
* Put SD/microSD card into Raspberry Pi 1, 2, 3 or 4 (Zero and Zero 2 can probably be used but need HDMI or a supported i2c DAC for audio out)
* Attach headphones to the headphone jack using `SoundDevice=pwm` in `minidexed.ini` (default)
* Alternatively, attach a  PCM5102A or PCM5122 based DAC and select i2c sound output using `SoundDevice=i2s | sndhdmi` in `minidexed.ini`
* Alternatively, attach a HDMI display with sound and select HDMI sound output using `SoundDevice=hdmi` in `minidexed.ini` (this may introduce slight latency)
* Attach a MIDI keyboard via USB
* Boot
* Stat playing


## Pinout

All devices on Raspberry Pi GPIOs are optional.

__CAUTION:__ All GPIO numbers are [chip numbers](https://pinout.xyz/), not header positions.

|GPIO | Device |  | Function | Direction | Commant|
|---|---|---|---|---|---|
|14 | UART |  | TXD |  | OUT |  | serial MIDI|
|15 | UART |  | RXD |  | IN |  | serial MIDI|
|18 | DAC |  | CLK |  | OUT|
|19 | DAC |  | FS |  | OUT|
|21 | DAC |  | DOUT |  | OUT|
|02 | I2C |  | SDA |  | IN/OUT |  | used by some DACs|
|03 | I2C |  | SCL |  | OUT |  | used by some DACs|
|17 | LCD |  | EN |  | OUT |  | default setting|
|27 | LCD |  | RS |  | OUT |  | default setting|
|16 | LCD |  | RW |  | OUT |  | default setting, optional|
|22 | LCD |  | D4 |  | OUT |  | default setting|
|23 | LCD |  | D5 |  | OUT |  | default setting|
|24 | LCD |  | D6 |  | OUT |  | default setting|
|25 | LCD |  | D7 |  | OUT |  | default setting|
|05 | ROTARY ENCODER | CLK (ENC A) | IN |  | default setting|
|06 | ROTARY ENCODER | DT (ENC B) | IN |  | default setting|
|26 | ROTARY ENCODER | SW |  | IN |  | default setting|
|12 | PWM AUDIO | PWM0 |  | OUT |  | on Raspberry Pi Zero|
|13 | PWM AUDIO | PWM1 |  | OUT |  | on Raspberry Pi Zero|
|07 | SPI |  | /CE1 |  | OUT |  | reserved|
|08 | SPI |  | /CE0 |  | OUT |  | reserved|
|09 | SPI |  | MISO |  | IN |  | reserved|
|10 | SPI |  | MOSI |  | OUT |  | reserved|
|11 | SPI |  | SCLK |  | OUT |  | reserved|
|04 | NONE |  |  |  |  |  | can generate clock signal|
|20 | NONE |  |  |  |  |  | may be used for DAC DIN|

## Building locally

E.g., to build for Raspberry Pi 4 on a Ubuntu 20.04 build system, you can use the following example. See [`build.yml`](../../tree/main/.github/workflows/build.yml) for complete build steps that create versions for Raspberry Pi 1, 2, 3,and 4 in 32-bit and 64-bit as required.

```
git clone https://github.com/probonopd/MiniDexed
cd MiniDexed

# Recursively pull git submodules
git submodule update --init --recursive

# Install toolchain
wget -q https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
tar xf gcc-arm-*-*.tar.xz 
export PATH=$(readlink -f ./gcc-*/bin/):$PATH

# Build dependencies and MiniDexed
RPI=4 ./build.sh

# Get Raspberry Pi boot files
cd ./circle-stdlib/libs/circle/boot
make
make armstub64
cd -

# Make zip that contains Raspberry Pi 4 boot files. The contents can be copied to a FAT32 formatted partition on a microSD card
mkdir -p sdcard
cd sdcard
../getsysex.sh
cd ..
cp -r ./circle-stdlib/libs/circle/boot/* sdcard
mv sdcard/config64.txt sdcard/config.txt
rm -rf sdcard/config32.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
cp ./src/*img sdcard/
zip -r MiniDexed_Raspberry_Pi_${RPI}.zip sdcard/*

# Optionally, create a RPi image. This can be written to a microSD card using tools like Etcher or dd
sudo apt install --yes  mount parted
IMG="`date +%Y-%m-%d`_minidexed-RPi${RPI}.img"
dd of="${IMG}" seek=50MiB bs=1 count=0
sudo parted "${IMG}" mktable msdos
sudo parted "${IMG}" mkpart primary fat32 2048s 100%
DEV=`sudo losetup --find --partscan --show "${IMG}"`
sudo mkfs.vfat -F 32 -n BOOT "${DEV}p1"
mkdir boot
sudo mount "${DEV}p1" boot
sudo cp -R sdcard/* boot
sudo umount boot
sudo losetup -d "${DEV}"
rm -r boot
```

## Acknowledgements

* [asb2m10](https://github.com/asb2m10/dexed) for the [Dexed](https://github.com/asb2m10/dexed) sound engine
* [dcoredump](https://github.com/dcoredump) for https://codeberg.org/dcoredump/Synth_Dexed, a port of Dexed for embedded systems
* [rsta2](https://github.com/rsta2) for https://github.com/rsta2/circle, the library to run code on bare betal Raspberry Pi (without a Linux kernel or operating system) and for the initial MiniDexed code 
* [smuehlst](https://github.com/smuehlst) for https://github.com/smuehlst/circle-stdlib, a version with Standard C and C++ library support
