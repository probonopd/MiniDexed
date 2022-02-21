#!/bin/bash

set -e 
set -x

if [ -z "${RPI}" ] ; then
  echo "${RPI} missing, exting"
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
( cd build/circle-newlib/ ; make mrproper ) || true
( cd build/circle-newlib/aarch64-none-circle/newlib ; rm ./config.cache ; make clean ) || true
./configure -r ${RPI} --prefix "${TOOLCHAIN_PREFIX}"
make -j$(nproc)
cd ..

# Build MiniDexed
cd src
make -j$(nproc)
ls *.img
cd ..
