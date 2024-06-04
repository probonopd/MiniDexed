#!/bin/bash

echo "Zipping boot files..."

if [ -z "${RPI}" ] ; then
  echo "\$RPI missing, exiting"
  exit 1
fi

DEST="build/sdcard"

mkdir -p ${DEST}

cp -r ./external/circle-stdlib/libs/circle/boot/* ${DEST}
rm -rf \
  ${DEST}/README \
  ${DEST}/config*.txt \
  ${DEST}/Makefile \
  ${DEST}/armstub \
  ${DEST}/COPYING.linux

cp \
  build/kernels/* \
  src/config.txt \
  src/minidexed.ini \
  src/performance.ini \
  ${DEST}

echo "usbspeed=full" > ${DEST}/cmdline.txt

cp build/kernels/* ${DEST}

zip -r build/MiniDexed_rpi_${RPI}_$(date +%Y-%m-%d).zip ${DEST}/*

echo "  Done."
