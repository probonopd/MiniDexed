# MiniDexed ![](https://github.com/probonopd/MiniDexed/actions/workflows/build.yml/badge.svg)

![minidexed](https://user-images.githubusercontent.com/2480569/161813414-bb156a1c-efec-44c0-802a-8926412a08e0.jpg)

MiniDexed is a FM synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer running on a bare metal Raspberry Pi (without a Linux kernel or operating system). On Pi Raspberry 2 and larger, it can run 8 tone generators, basically creating an open source equivalent of the TX816/TX802 (8 DX7 instances without the keyboard in one box).

## Features

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
- [x] Support selecting patches using MIDI Bank Change and Program Change messages
- [x] Get 8 Dexed instances to run simultaneously (like in a TX816) and mix their output together
- [x] Allow for each Dexed instance to be detuned and stereo shifted
- [x] Add a way to configure multiple Dexed instances through `performance.ini` files
- [ ] Add a way to configure multiple Dexed instances through Performance `.syx` files
- [ ] Add a way to configure multiple Dexed instances through Performance sysex messages
- [x] Add compressor effect
- [x] Add reverb effect
- [ ] Make it possible to assign voice parameters to sliders and knobs on MIDI controllers

## System Requirements

* Raspberry Pi 1, 2, 3, 4, or 400 (Zero and Zero 2 can be used but need HDMI or a supported i2c DAC for audio out). On Raspberry Pi 1 and on Raspberry Pi Zero there will be severely limited functionality (only one tone generator instead of 8)
* A PCM5102A or PCM5122 based DAC or HDMI display or audio extractor for good sound quality. If you don't have this, you can use the headphone jack on the Raspberry Pi but on anything but the Raspberry 4 the sound quality will be seriously limited
* Optionally (but highly recommended), an [alphanumeric 1602 LCD Display](https://www.berrybase.de/en/sensors-modules/displays/alphanumeric-displays/alphanumerisches-lcd-16x2-gr-252-n/gelb) and a [KY-040 rotary encoder](https://www.berrybase.de/en/components/passive-components/potentiometer/rotary-encoder/drehregler/rotary-encoder-mit-breakoutboard-ohne-gewinde-und-mutter)

## Usage

* In the case of Raspberry Pi 4, Update the firmware and bootloader to the latest version (not doing this may cause USB reliability issues)
* Download from [GitHub Releases](../../releases)
* Unzip
* Put the files into the root directory of a FAT32 formatted partition on SD/microSD card
* Put SD/microSD card into Raspberry Pi 1, 2, 3 or 4, or 400 (Zero and Zero 2 can be used but need HDMI or a supported i2c DAC for audio out)
* Attach headphones to the headphone jack using `SoundDevice=pwm` in `minidexed.ini` (default) (poor audio quality)
* Alternatively, attach a  PCM5102A or PCM5122 based DAC and select i2c sound output using `SoundDevice=i2s` in `minidexed.ini` (best audio quality)
* Alternatively, attach a HDMI display with sound and select HDMI sound output using `SoundDevice=hdmi` in `minidexed.ini` (this may introduce slight latency)
* Attach a MIDI keyboard via USB
* Boot
* Start playing
* See the Wiki for [Menu](https://github.com/probonopd/MiniDexed/wiki/Menu) operation


## Pinout

All devices on Raspberry Pi GPIOs are **optional**.

__CAUTION:__ All GPIO numbers are [chip numbers](https://pinout.xyz/), not header positions.

|GPIO | Device |  | Function | Direction | Comment|
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

## Downloading

Compiled versions are available on [GitHub Releases](../../releases). Just download and put on a FAT32 formatted SD card.

## Building locally

If you need to build the source code yoursel, you can use the following example, e.g., to build for Raspberry Pi 4 on a Ubuntu 20.04 build system. See [`build.yml`](../../tree/main/.github/workflows/build.yml) for complete build steps that create versions for Raspberry Pi 1, 2, 3,and 4 in 32-bit and 64-bit as required.

```
# Choose your RPi
export RPI=4

git clone https://github.com/probonopd/MiniDexed
cd MiniDexed
mkdir -p kernels sdcard

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
zip -r ../MiniDexed_$GITHUB_RUN_NUMBER_$(date +%Y-%m-%d).zip *
cd -

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

# Write to SD card
sudo dd if="${IMG}" of=/dev/mmcblk0 bs=512k status=progress && sync
```

## Acknowledgements

* [asb2m10](https://github.com/asb2m10/dexed) for the [Dexed](https://github.com/asb2m10/dexed) sound engine
* [dcoredump](https://github.com/dcoredump) for https://codeberg.org/dcoredump/Synth_Dexed, a port of Dexed for embedded systems
* [rsta2](https://github.com/rsta2) for https://github.com/rsta2/circle, the library to run code on bare betal Raspberry Pi (without a Linux kernel or operating system) and for the initial MiniDexed code 
* [smuehlst](https://github.com/smuehlst) for https://github.com/smuehlst/circle-stdlib, a version with Standard C and C++ library support
