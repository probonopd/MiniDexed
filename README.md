# MiniDexed ![](https://github.com/probonopd/MiniDexed/actions/workflows/build.yml/badge.svg)

![minidexed](https://user-images.githubusercontent.com/2480569/161813414-bb156a1c-efec-44c0-802a-8926412a08e0.jpg)

MiniDexed is a FM synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer running on a bare metal Raspberry Pi (without a Linux kernel or operating system). On Raspberry Pi 2 and larger, it can run 8 tone generators, not unlike the TX816/TX802 (8 DX7 instances without the keyboard in one box). [Featured by HACKADAY](https://hackaday.com/2022/04/19/bare-metal-gives-this-pi-some-classic-synths/) and [adafruit](https://blog.adafruit.com/2022/04/25/free-yamaha-dx7-synth-emulator-on-a-raspberry-pi/).

## Features

- [x] Uses [Synth_Dexed](https://codeberg.org/dcoredump/Synth_Dexed) with [circle-stdlib](https://github.com/smuehlst/circle-stdlib)
- [x] SD card contents can be downloaded from [GitHub Releases](../../releases)
- [x] Runs on all Raspberry Pi models (except Pico); see below for details
- [x] Produces sound on the headphone jack, HDMI display or [audio extractor](https://github.com/probonopd/MiniDexed/wiki/Hardware#hdmi-to-audio) (better), or a [dedicated DAC](https://github.com/probonopd/MiniDexed/wiki/Hardware#i2c-dac) (best)
- [x] Supports multiple voices through Program Change and Bank Change LSB/MSB MIDI messages
- [x] Loads voices from `.syx` files from SD card (e.g., using `getsysex.sh` or from [Dexed_cart_1.0.zip](http://hsjp.eu/downloads/Dexed/Dexed_cart_1.0.zip))
- [x] Menu structure on optional [HD44780 display](https://www.berrybase.de/sensoren-module/displays/alphanumerische-displays/alphanumerisches-lcd-16x2-gr-252-n/gelb) and rotary encoder
- [x] Runs up to 8 Dexed instances simultaneously (like in a TX816) and mixes their output together
- [x] Allows for each Dexed instance to be detuned and stereo shifted
- [x] Allows to configure multiple Dexed instances through `performance.ini` files
- [x] Compressor effect
- [x] Reverb effect
- [x] Voices can be edited over MIDI, e.g., using the [synthmata](https://synthmata.github.io/volca-fm/) online editor (requires [additional hardware](https://github.com/probonopd/MiniDexed/wiki/Hardware#usb-midi-device))

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
* Optionally, put voices in `.syx` files onto the SD card (e.g., using `getsysex.sh`)
* See the Wiki for [Menu](https://github.com/probonopd/MiniDexed/wiki/Menu) operation
* If something is unclear or does not work, don't hesitate to [ask](https://github.com/probonopd/MiniDexed/discussions/)!

## Pinout

All devices on Raspberry Pi GPIOs are **optional**.

![](https://user-images.githubusercontent.com/2480569/166105580-da11481c-8fc7-4375-8ab1-3031ab5c6ad0.png)

Please the the [wiki](https://github.com/probonopd/MiniDexed/wiki) for more information.

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
