#!/bin/bash

echo "Building stub..."

if [ -z "${RPI}" ] ; then
  echo "\$RPI missing, exiting"
  exit 1
fi

echo "Building stub for RPI=${RPI}..."

cd external/circle-stdlib/libs/circle/boot

make
if [ "${RPI}" -gt 2 ]
then
	make armstub64
fi

cd -

echo "  Done."
