#!/bin/bash

RPI=${RPI:-4}
DEST="external/gcc"
TARGET_GCC="gcc-arm-10.3-2021.07"

echo "Installing toolchain for RPI=${RPI}..."

# Check if gcc is already installed
if [ -d ${DEST} ]; then
  echo "  Toolchain already installed."
  exit 0
fi

mkdir -p ${DEST}
mkdir -p tmp

TARGET="${TARGET_GCC}-x86_64-aarch64-none-elf"
URL="https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/${TARGET}.tar.xz"

if [ "${RPI}" -le 2 ]; then
  TARGET="${TARGET_GCC}-x86_64-arm-none-eabi"
  URL="https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/${TARGET}.tar.xz"
fi 

if [ ! -f tmp/gcc-arm-*-*.tar.xz ]; then
  echo "  Downloading toolchain..."
  wget -q -P tmp ${URL}
  tar -xf tmp/gcc-arm-*-*.tar.xz -C ${DEST} --strip-components 1
fi

echo "  Done."
