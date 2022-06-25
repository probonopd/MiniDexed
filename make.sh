#!/bin/bash

set +x

# this script install the dev environment and assume that the repository is alreadw cloned
usage()
{
   # Display Help
   echo "usage: make-sh [ <option> ... ]"
   echo "This script install the dev environment and assume that the repository is already cloned."
   echo
   echo "options:"
   echo "  -d, --debug     Execute with debugging script"
   echo "  -h, --help      Print this help"
   echo "  -r <version>, -RPI <version>, --raspberrypi <version"
   echo "                  Set Raspberry PI version"
   echo "  -prepenv        Prepare the development environment"
   echo "  -build          Run build"
   echo "  -FETCH_SYSEX          Fetch all FETCH_SYSEX"
   echo "  -zip            Create the resulting zip file"
   echo "  -opt            Run RUN_OPTIONALITIES"
   echo
}

export RPI=4
export SETUP_ENV=0
export BUILD=0
export CREATE_ZIP=0
export FETCH_SYSEX=0
export RUN_OPTIONALITIES=0

while true ; do
    case "$1" in
	-d|--debug) set -x ; shift;;
	-h|--help) usage ; exit 0;;
	-r|-RPI|--raspberrypi) export RPI=$2 ; shift 2;;
	-prepenv) export SETUP_ENV=1 ; shift;;
	-build) export BUILD=1 ; shift;;
	-FETCH_SYSEX) export FETCH_SYSEX=1 ; shift;;
	-zip) export CREATE_ZIP=1 ; shift;;
	-opt) export RUN_OPTIONALITIES=1 ; shift;;
	--) shift ; break ;;
	*) break ;;
	# *) echo "Internal error! Remaining params: #$*#" ; exit 1;;
    esac
done

echo "Raspberry PI=$RPI"
echo "Setup environment=$SETUP_ENV"
echo "Run build=$BUILD"
echo "Fetch FETCH_SYSEX=$FETCH_SYSEX"
echo "Prepare zip file=$CREATE_ZIP"
echo "Run optional steps=$RUN_OPTIONALITIES"

if [ ${SETUP_ENV} -eq 1 ]
then
	mkdir -p kernels sdcard

	# Recursively pull git submodules
	git submodule update --init --recursive

	# Install toolchain
	if [ "${RPI}" -gt 2 ]
	then
		if [ -f gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz ]
		then
			rm -f gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
		fi

		wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
	else
		if [ -f gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz ]
		then
			rm -f gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz
		fi

		wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz
	fi
	tar xvf gcc-arm-*-*.tar.xz 
fi

if [ ${BUILD} -eq 1 ]
then
	export PATH=$(readlink -f ./gcc-*/bin/):$PATH

	# Build dependencies and MiniDexed
	./build.sh -clean -all
	cp ./src/kernel*.img ./kernels/

	if [ ${SETUP_ENV} -eq 1 ]
	then
		# Get Raspberry Pi boot files
		cd ./circle-stdlib/libs/circle/boot
		make
		if [ "${RPI}" -gt 2 ]
		then
			make armstub64
		fi
		cd -
	fi
fi

if [ ${CREATE_ZIP} -eq 1 ]
then
	# Make zip that contains Raspberry Pi 4 boot files. The contents can be copied to a FAT32 formatted partition on a microSD card
	if [ ${FETCH_SYSEX} -eq 1]
	then
		cd sdcard
		../getFETCH_SYSEX.sh
		cd ..
	fi

	cp -r ./circle-stdlib/libs/circle/boot/* sdcard
	rm -rf sdcard/config*.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
	cp ./src/config.txt ./src/minidexed.ini ./src/*img ./src/performance.ini sdcard/
	echo "usbspeed=full" > sdcard/cmdline.txt
	cd sdcard
	cp ../kernels/* . || true
	zip -r ../MaxiDexed_$GITHUB_RUN_NUMBER_$(date +%Y-%m-%d).zip *
	cd -
fi

if [ ${RUN_OPTIONALITIES} -eq 1 ]
then
	# Optionally, create a RPi image. This can be written to a microSD card using tools like Etcher or dd
	sudo apt install --yes  mount parted
	IMG="`date +%Y-%m-%d`_minidexed-RPi${RPI}.img"
	dd of="${IMG}" seek=50MiB bs=1 count=0
	sudo parted "${IMG}" mktable msdos
	sudo parted "${IMG}" mkpart primary fat32 2048s 100%
	DEV=`sudo losetup --find --partscan --show "${IMG}"`
	sudo mkfs.vfat -F 32 -n BOOT "${DEV}p1"
	mkdir boot
	sudo mount "${DEV}p1" boot
	sudo cp -R sdcard/* boot
	sudo umount boot
	sudo losetup -d "${DEV}"
	rm -r boot

	# Write to SD card
	sudo dd if="${IMG}" of=/dev/mmcblk0 bs=512k status=progress && sync
fi