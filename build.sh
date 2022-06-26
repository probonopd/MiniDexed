#!/bin/bash

set +x

usage()
{
   # Display Help
   echo "This script launch the build of the code."
   echo
   echo "options:"
   echo "  -h               Print this Help."
   echo "  -d|--debug       Set the debug mode"
   echo "  -RPI <version>   Set Raspberry PI version"
   echo "  -clean           Run clean before building"
   echo "  -full            Run full build"
   echo "  -all             Run the entire build"
}

export PATH=$(readlink -f ./gcc-*/bin/):$PATH

export CLEAN=0;
export INCREMENTAL_BUILD=1

while true
do
    case "$1" in
		-h|--help) usage ; shift ; exit 0;;
        -d|--debug) set -x ; shift;;
		-RPI) export RPI=$2 ; shift 2;;
        -clean) export CLEAN=1 ; shift;;
        -all) export INCREMENTAL_BUILD=0 ; shift;;
		--) shift ; break;;
	   	*)  if [ $1 ]
            then
                echo "Error processing param #$1#"
                exit 1
            else
                break
            fi;;
    esac
done

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

if [ ${INCREMENTAL_BUILD} -eq 0 ]
then

    # Build circle-stdlib library
    cd circle-stdlib/
    if [ ${CLEAN} -eq 1 ]
    then
        make mrproper || true
    fi
    ./configure -r ${RPI} --prefix "${TOOLCHAIN_PREFIX}" ${OPTIONS} -o KERNEL_MAX_SIZE=0x400000
    make -j


    # Build additional libraries
    cd libs/circle/addon/display/
    if [ ${CLEAN} -eq 1 ]
    then
        make clean || true
    fi
    make -j

    cd ../sensor/
    if [ ${CLEAN} -eq 1 ]
    then
        make clean || true
    fi
    make -j

    cd ../Properties/
    if [ ${CLEAN} -eq 1 ]
    then
        make clean || true
    fi
    make -j

    cd ../../../..

    cd ..
fi

# Build MaxiDexed
cd src
if [ ${CLEAN} -eq 1 ]
then
    make clean || true
fi
make -j
ls *.img
cd ..
