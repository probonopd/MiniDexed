#!/bin/bash

set -e 
set -x

export RPI=4

export PATH=$(readlink -f ./gcc-*/bin/):$PATH

./incrementalBuild.sh
cp ./src/kernel*.img ./kernels/

# Make zip that contains Raspberry Pi 4 boot files. The contents can be copied to a FAT32 formatted partition on a microSD card
cp -r ./circle-stdlib/libs/circle/boot/* sdcard
rm -rf sdcard/config*.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
cp ./src/config.txt ./src/minidexed.ini ./src/*img ./src/performance.ini sdcard/
echo "usbspeed=full" > sdcard/cmdline.txt
cd sdcard
cp ../kernels/* . || true
zip -r ../MiniDexed_$GITHUB_RUN_NUMBER_$(date +%Y-%m-%d).zip *
cd -

echo "DONE!!!"