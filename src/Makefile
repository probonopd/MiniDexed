#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src
CMSIS_DIR = ../CMSIS_5/CMSIS

OBJS = main.o kernel.o minidexed.o config.o userinterface.o uimenu.o \
       mididevice.o midikeyboard.o serialmididevice.o pckeyboard.o \
       sysexfileloader.o performanceconfig.o perftimer.o \
       uibuttons.o midipin.o effect_compressor.o \
       effect_audio/effect_base.o \
       effect_audio/effect_chorus.o \
       effect_audio/effect_delay.o \
       effect_audio/effect_platervbstereo.o \
       effect_audio/effect_talreverb3.o \
       effect_audio/effect_mverb.o \
       effect_audio/effect_ds1.o \
       effect_audio/effect_bigmuff.o \
       effect_audio/moddistortion/Distortion_DS1.o \
       effect_audio/moddistortion/Distortion_BigMuff.o \
       effect_audio/moddistortion/HyperbolicTables.o \
       effect_audio/moddistortion/OverSample.o \
       effect_midi/midi_arp.o \
       effect_midi/modarpeggiator/common/clock.o \
       effect_midi/modarpeggiator/common/midiHandler.o \
       effect_midi/modarpeggiator/common/pattern.o \
       effect_midi/modarpeggiator/utils.o \
       effect_midi/modarpeggiator/arpeggiator.o

OPTIMIZE = -O3

include ./Synth_Dexed.mk
include ./Rules.mk

EXTRACLEAN += effect_audio/moddistortion/*.[od] effect_midi/modarpeggiator/*.[od] effect_midi/modarpeggiator/common/*.[od]