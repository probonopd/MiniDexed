#!/bin/bash

set -e 
set -x

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
source USBID.sh
if [ "${USB_VID}" ] ; then
	OPTIONS="${OPTIONS} -o USB_GADGET_VENDOR_ID=${USB_VID}"
fi
if [ "${USB_DID}" ] ; then
	OPTIONS="${OPTIONS} -o USB_GADGET_DEVICE_ID=${USB_DID}"
fi

# Patch Circle so that descriptive strings are used in dmesg and in DAWs
sed -i -e 's|"Circle"|"probonopd"|g' circle-stdlib/libs/circle/lib/usb/gadget/usbmidigadget.cpp
sed -i -e 's|"MIDI Gadget"|"MiniDexed"|g' circle-stdlib/libs/circle/lib/usb/gadget/usbmidigadget.cpp

# Disable the MIDI Output as it is currently unused
sed -i -e 's|m_pEP\[EPOut\]->AttachInterface|// m_pEP[EPOut]->AttachInterface|g' circle-stdlib/libs/circle/lib/usb/gadget/usbmidigadget.cpp

# Build circle-stdlib library
cd circle-stdlib/
make mrproper || true
./configure -r ${RPI} --prefix "${TOOLCHAIN_PREFIX}" ${OPTIONS} -o KERNEL_MAX_SIZE=0x400000
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

cd ..

# Build MiniDexed
cd src
make clean || true
make -j
ls *.img
cd ..
