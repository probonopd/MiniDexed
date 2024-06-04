#
# Makefile
#

CIRCLE_STDLIB_DIR = ../external/circle-stdlib
SYNTH_DEXED_DIR = ../external/Synth_Dexed/src
CMSIS_DIR = ../external/CMSIS_5/CMSIS

OBJS = main.o kernel.o minidexed.o config.o userinterface.o uimenu.o \
       mididevice.o midikeyboard.o serialmididevice.o pckeyboard.o \
       sysexfileloader.o performanceconfig.o perftimer.o \
       effect_compressor.o effect_platervbstereo.o uibuttons.o midipin.o

OPTIMIZE = -O3

include ./Synth_Dexed.mk
include ./Rules.mk
