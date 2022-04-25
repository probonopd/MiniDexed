# MiniDexed ![](https://github.com/probonopd/MiniDexed/actions/workflows/build.yml/badge.svg)

![minidexed](https://user-images.githubusercontent.com/2480569/161813414-bb156a1c-efec-44c0-802a-8926412a08e0.jpg)

MiniDexed is a FM synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer running on a bare metal Raspberry Pi (without a Linux kernel or operating system). On Raspberry Pi 2 and larger, it can run 8 tone generators, not unlike the TX816/TX802 (8 DX7 instances without the keyboard in one box). [Featured on HACKADAY](https://hackaday.com/2022/04/19/bare-metal-gives-this-pi-some-classic-synths/).

## Features

- [x] Uses [Synth_Dexed](https://codeberg.org/dcoredump/Synth_Dexed) with [circle-stdlib](https://github.com/smuehlst/circle-stdlib)
- [x] SD card contents can be downloaded from [GitHub Releases](../../releases)
- [x] Runs on all Raspberry Pi models (except Pico); see below for details
- [x] Produces sound on the headphone jack, HDMI display or [audio extractor](https://github.com/probonopd/MiniDexed/wiki/Hardware#hdmi-to-audio) (better), or a [dedicated DAC](https://github.com/probonopd/MiniDexed/wiki/Hardware#i2c-dac) (best)
- [x] Supports multiple voices through Program Change and Bank Change LSB/MSB MIDI messages
- [x] Loads `.syx` files from SD card (e.g., using `getsysex.sh` or from [Dexed_cart_1.0.zip](http://hsjp.eu/downloads/Dexed/Dexed_cart_1.0.zip))
- [x] Menu structure on optional [HD44780 display](https://www.berrybase.de/sensoren-module/displays/alphanumerische-displays/alphanumerisches-lcd-16x2-gr-252-n/gelb) and rotary encoder
- [x] Runs up to 8 Dexed instances simultaneously (like in a TX816) and mixes their output together
- [x] Allows for each Dexed instance to be detuned and stereo shifted
- [x] Allows to configure multiple Dexed instances through `performance.ini` files
- [x] Compressor effect
- [x] Reverb effect

## Introduction

Video about this project by [Floyd Steinberg](https://www.youtube.com/watch?v=Z3t94ceMHJo):

[![](https://i.ytimg.com/vi/Z3t94ceMHJo/sddefault.jpg)](https://www.youtube.com/watch?v=Z3t94ceMHJo)

## System Requirements

* Raspberry Pi 1, 2, 3, 4, or 400 (Zero and Zero 2 can be used but need HDMI or a supported i2s DAC for audio out). On Raspberry Pi 1 and on Raspberry Pi Zero there will be severely limited functionality (only one tone generator instead of 8)
* A [PCM5102A or PCM5122 based DAC](https://github.com/probonopd/MiniDexed/wiki/Hardware#i2c-dac), HDMI display or [audio extractor](https://github.com/probonopd/MiniDexed/wiki/Hardware#hdmi-to-audio) for good sound quality. If you don't have this, you can use the headphone jack on the Raspberry Pi but on anything but the Raspberry 4 the sound quality will be seriously limited
* Optionally (but highly recommended), an [LCDC1602 Display](https://www.berrybase.de/en/sensors-modules/displays/alphanumeric-displays/alphanumerisches-lcd-16x2-gr-252-n/gelb) (not i2c) and a [KY-040 rotary encoder](https://www.berrybase.de/en/components/passive-components/potentiometer/rotary-encoder/drehregler/rotary-encoder-mit-breakoutboard-ohne-gewinde-und-mutter)

## Usage

* In the case of Raspberry Pi 4, Update the firmware and bootloader to the latest version (not doing this may cause USB reliability issues)
* Download from [GitHub Releases](../../releases)
* Unzip
* Put the files into the root directory of a FAT32 formatted partition on SD/microSD card (Note for small SD cards which are no longer sold: If less than 65525 clusters, you may need to format as FAT16.)
* Put SD/microSD card into Raspberry Pi 1, 2, 3 or 4, or 400 (Zero and Zero 2 can be used but need HDMI or a supported i2c DAC for audio out)
* Attach headphones to the headphone jack using `SoundDevice=pwm` in `minidexed.ini` (default) (poor audio quality)
* Alternatively, attach a  PCM5102A or PCM5122 based DAC and select i2c sound output using `SoundDevice=i2s` in `minidexed.ini` (best audio quality)
* Alternatively, attach a HDMI display with sound and select HDMI sound output using `SoundDevice=hdmi` in `minidexed.ini` (this may introduce slight latency)
* Attach a MIDI keyboard via USB (alternatively you can build a circuit that allows you to attach a "traditional" MIDI keyboard using a DIN connector, or use a DIN-MIDI-to-USB adapter)
* Boot
* Start playing
* If the system seems to become unresponsive after a few seconds, remove `usbspeed=full` from `cmdline.txt` and repeat ([details](https://github.com/probonopd/MiniDexed/issues/39))
* See the Wiki for [Menu](https://github.com/probonopd/MiniDexed/wiki/Menu) operation
* If something is unclear or does not work, don't hesitate to [ask](https://github.com/probonopd/MiniDexed/discussions/)!

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

## Building

Please see the [wiki](https://github.com/probonopd/MiniDexed/wiki/Development#building-locally) on how to compile the code yourself.

## Contributing

This project lives from the contributions of skilled C++ developers, testers, writers, etc. Please see https://github.com/probonopd/MiniDexed/issues.

## Discussions

We are happy to hear from you. Please join the discussions on https://github.com/probonopd/MiniDexed/discussions.

## Documentation

Project documentation is at https://github.com/probonopd/MiniDexed/wiki.

## Acknowledgements

This project stands on the shoulders of giants. Special thanks to:

* [raphlinus](https://github.com/raphlinus) for the [MSFA](https://github.com/google/music-synthesizer-for-android) sound engine
* [asb2m10](https://github.com/asb2m10/dexed) for the [Dexed](https://github.com/asb2m10/dexed) software
* [dcoredump](https://github.com/dcoredump) for https://codeberg.org/dcoredump/Synth_Dexed, a port of Dexed for embedded systems
* [rsta2](https://github.com/rsta2) for https://github.com/rsta2/circle, the library to run code on bare metal Raspberry Pi (without a Linux kernel or operating system) and for the bulk of the MiniDexed code 
* [smuehlst](https://github.com/smuehlst) for https://github.com/smuehlst/circle-stdlib, a version with Standard C and C++ library support
