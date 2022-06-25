#!/bin/bash

usage()
{
   # Display Help
   echo "This script launch the build of the code."
   echo
   echo "options:"
   echo "-h				Print this Help."
   echo "-RPI <version>	Set Raspberry PI version"
   echo "-clean			Run clean before building"
   echo "-full          Run full build"
   echo "-all           Run the entire build"
}

export clean=0;
export minimal=1

while true
do
    case "$1" in
		-h|--help) usage ; exit 0;;
		-RPI) export RPI=$2 ; shift;;
        -clean) export clean=1 ; shift;;
        -all) export minimal=0 ; shift;;
		--) shift ; break;;
	   	*) break;;
    esac
done


if [ -z "${RPI}" ] ; then
  echo "\$RPI missing, exting"
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

if [ ${minimal} -eq 1 ]
then

    # Build circle-stdlib library
    cd circle-stdlib/
    if [ ${clean} -eq 1 ]
    then
        make mrproper || true
    fi
    ./configure -r ${RPI} --prefix "${TOOLCHAIN_PREFIX}" ${OPTIONS} -o KERNEL_MAX_SIZE=0x400000
    make -j


    # Build additional libraries
    cd libs/circle/addon/display/
    if [ ${clean} -eq 1 ]
    then
        make clean || true
    fi
    make -j

    cd ../sensor/
    if [ ${clean} -eq 1 ]
    then
        make clean || true
    fi
    make -j

    cd ../Properties/
    if [ ${clean} -eq 1 ]
    then
        make clean || true
    fi
    make -j

    cd ../../../..

    cd ..
fi

# Build MiniDexed
cd src
if [ ${clean} -eq 1 ]
then
    make clean || true
fi
make -j
ls *.img
cd ..
