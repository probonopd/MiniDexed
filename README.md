# MiniDexed

[Dexed](https://asb2m10.github.io/dexed/) is a synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer. MiniDexed is a port to run it on a bare metal Raspberry Pi (without a Linux kernel or operating system). __Currently it is not functional yet. Contributions are highly welcome.__

## TODO

 Contributions are highly welcome.

- [ ] Get Dexed to build with [circle-stdlib](https://github.com/smuehlst/circle-stdlib)
- [ ] Get it to run on a Raspberry Pi 4 without crashing
- [ ] Get it to produce some sound on the headphone jack
- [ ] Get it to react to USB MIDI
- [ ] Get it to react to MIDI via Raspberry Pi 4 GPIO
- [ ] Get 8 Dexed instances to run simultaneously (like in a TX816) and mix their output together
- [ ] Add functionality for loading `.syx` files (e.g., from [Dexed_cart_1.0.zip](http://hsjp.eu/downloads/Dexed/Dexed_cart_1.0.zip))
- [ ] Add a way to configure each Dexed instance
- [ ] Allow for each Dexed instance to be stereo shifted
- [ ] Add reverb effect

I am wondering whether Dexed could be ported to Circle, in order to recreate basically an open source equivalent of the TX802 (8 DX7 instances without the keyboard in one box).

## Acknowledgements

* [dcoredump](https://github.com/dcoredump)
* [rsta2](https://github.com/rsta2)
* https://github.com/rsta2/circle, the library to run code on bare betal Raspberry Pi (without a Linux kernel or operating system)
* https://github.com/smuehlst/circle-stdlib, a version with Standard C and C++ Library Support
* https://codeberg.org/dcoredump/MicroDexed, a Dexed port to a microcontroller
