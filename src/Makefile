#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src
CMSIS_DIR = ../CMSIS_5/CMSIS

OBJS := main.o
OBJS += kernel.o
OBJS += minidexed.o
OBJS += config.o
OBJS += userinterface.o
OBJS += uimenu.o
OBJS += mididevice.o
OBJS += midikeyboard.o
OBJS += serialmididevice.o
OBJS += pckeyboard.o
OBJS += sysexfileloader.o
OBJS += performanceconfig.o
OBJS += perftimer.o
OBJS += effect_compressor.o
OBJS += effect_platervbstereo.o
OBJS += fx.o
OBJS += fx_components.o
OBJS += fx_svf.o
OBJS += fx_tube.o
OBJS += fx_chorus.o
OBJS += fx_flanger.o
OBJS += fx_orbitone.o
OBJS += fx_phaser.o
OBJS += fx_delay.o
OBJS += fx_reverberator.o
OBJS += fx_rack.o
OBJS += uibuttons.o
OBJS += midipin.o

OPTIMIZE = -O3

include ./Synth_Dexed.mk
include ./Rules.mk
