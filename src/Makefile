#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src
CMSIS_DIR = ../CMSIS_5/CMSIS

OBJS = main.o kernel.o minidexed.o config.o userinterface.o uimenu.o \
       mididevice.o midikeyboard.o serialmididevice.o pckeyboard.o \
       sysexfileloader.o performanceconfig.o perftimer.o \
       effect_base.o effect_chorus.o effect_delay.o effect_mverb.o \
       effect_talreverb3.o effect_ds1.o effect_bigmuff.o \
       moddistortion/Distortion_DS1.o moddistortion/Distortion_BigMuff.o \
       moddistortion/HyperbolicTables.o moddistortion/OverSample.o \
       effect_compressor.o effect_platervbstereo.o uibuttons.o midipin.o \
       midi_arp.o modarpeggiator/common/clock.o \
       modarpeggiator/common/midiHandler.o modarpeggiator/common/pattern.o \
       modarpeggiator/utils.o modarpeggiator/arpeggiator.o

OPTIMIZE = -O3

include ./Synth_Dexed.mk
include ./Rules.mk

EXTRACLEAN += moddistortion/*.[od] modarpeggiator/*.[od] modarpeggiator/common/*.[od]