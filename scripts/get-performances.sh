#!/bin/bash

echo "Fetching performances..."

DEST="build/sdcard"

mkdir -p ${DEST}

rm -rf external/soundplantage

git clone https://github.com/Banana71/Soundplantage --depth 1 external/soundplantage > /dev/null 2>&1

cp -r \
  external/soundplantage/performance \
  external/soundplantage/*.pdf \
  $DEST

echo "  Done."