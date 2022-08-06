export RPI=3

export CLEAN="true"
export BUILDCIRCLE="true"
export PATH=$(readlink -f ./gcc-*/bin/):$PATH

# Build dependencies and MiniDexed
./build.sh
cp ./src/kernel*.img ./kernels/
if [[ $? -ne 0 ]] ; then
exit
fi
# Get Raspberry Pi boot files
cd ./circle-stdlib/libs/circle/boot
make
if [ "${RPI}" -gt 2 ]
then
	make armstub64
fi
cd -

# Make zip that contains Raspberry Pi 4 boot files. The contents can be copied to a FAT32 formatted partition on a microSD card
#cd sdcard
#../getsysex.sh
#cd ..
cp -r ./circle-stdlib/libs/circle/boot/* sdcard
if [[ $? -ne 0 ]] ; then
exit
fi
rm -rf sdcard/config*.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
cp ./src/config.txt ./src/*img sdcard/
if [[ $? -ne 0 ]] ; then
exit
fi
echo "usbspeed=full" > sdcard/cmdline.txt
cd sdcard
cp ../kernels/* . || true
if [[ $? -ne 0 ]] ; then
exit
fi
zip -r ../MiniDexed_$(date +%Y-%m-%d).zip *
cd -
