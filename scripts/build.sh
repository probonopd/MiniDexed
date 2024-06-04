#!/bin/bash

if [ -z "${RPI}" ] ; then
  echo "\$RPI missing, exiting"
  exit 1
fi

if [ "${RPI}" -gt "2" ]; then
    export TOOLCHAIN_PREFIX="aarch64-none-elf-"
else
    export TOOLCHAIN_PREFIX="arm-none-eabi-"
fi

# Define system options
OPTIONS="-o USE_PWM_AUDIO_ON_ZERO -o SAVE_VFP_REGS_ON_IRQ -o REALTIME -o SCREEN_DMA_BURST_LENGTH=1"
if [ "${RPI}" -gt "1" ]; then
    OPTIONS="${OPTIONS} -o ARM_ALLOW_MULTI_CORE"
fi

# USB Vendor and Device ID for use with USB Gadget Mode
source scripts/USBID.sh
if [ "${USB_VID}" ] ; then
	OPTIONS="${OPTIONS} -o USB_GADGET_VENDOR_ID=${USB_VID}"
fi
if [ "${USB_DID}" ] ; then
	OPTIONS="${OPTIONS} -o USB_GADGET_DEVICE_ID_BASE=${USB_DID}"
fi

# Build circle-stdlib library
echo "Building circle-stdlib with RPI ${RPI} ${TOOLCHAIN_PREFIX}..."
cd external/circle-stdlib/
make mrproper || true
./configure -r ${RPI} -p "${TOOLCHAIN_PREFIX}" ${OPTIONS} -o "KERNEL_MAX_SIZE=0x400000"
make -j

# Build additional libraries
cd libs/circle/addon/display/
make clean || true
make -j
cd ../sensor/
make clean || true
make -j
cd ../Properties/
make clean || true
make -j
cd ../../../..

cd ../..

# Build
cd src
make clean || true
make -j
ls *.img
cd ..

mkdir -p build/kernels
mkdir -p build/sdcard

cp src/*.img build/kernels/
cp src/*.img build/sdcard/
