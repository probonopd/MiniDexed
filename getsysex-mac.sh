#!/bin/sh

GETSYSEX_SCRIPT=getsysex.sh

cat $GETSYSEX_SCRIPT | sed 's/wget -c \(.*\) -O \(.*\)/curl -o \2 \1/' | bash