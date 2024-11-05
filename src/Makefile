#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src
CMSIS_DIR = ../CMSIS_5/CMSIS
NET_DIR = ./net

OBJS = main.o kernel.o minidexed.o config.o userinterface.o uimenu.o \
       mididevice.o udpmididevice.o midikeyboard.o serialmididevice.o pckeyboard.o \
       sysexfileloader.o performanceconfig.o perftimer.o \
       effect_compressor.o effect_platervbstereo.o uibuttons.o midipin.o \
       net/ftpdaemon.o net/ftpworker.o net/applemidi.o net/udpmidi.o

OPTIMIZE = -O3

include ./Synth_Dexed.mk
include ./Rules.mk
