#!/bin/bash

set -e 
set -x

if [ -z "${RPI}" ] ; then
  echo "\$RPI missing, exting"
  exit 1
fi

if [ "${RPI}" -gt "2" ]; then
    export TOOLCHAIN_PREFIX="aarch64-none-elf-"
else
    export TOOLCHAIN_PREFIX="arm-none-eabi-"
fi

# Build circle-stdlib library
cd circle-stdlib/
make mrproper || true
./configure -r ${RPI} --prefix "${TOOLCHAIN_PREFIX}"
echo "DEFINE += -DUSE_PWM_AUDIO_ON_ZERO" >> libs/circle/Config.mk
echo "DEFINE += -DSAVE_VFP_REGS_ON_IRQ" >> libs/circle/Config.mk
echo "DEFINE += -DREALTIME" >> libs/circle/Config.mk
make -j

# Build additional libraries
cd libs/circle/addon/display/
make clean || true
make -j
cd ../Properties/
make clean || true
make -j
cd ../../../..

cd ..

# Build MiniDexed
cd src
make clean || true
make -j
ls *.img
cd ..
