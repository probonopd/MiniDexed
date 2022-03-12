#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src

OBJS = main.o kernel.o minidexed.o config.o userinterface.o \
       mididevice.o midikeyboard.o serialmididevice.o pckeyboard.o \
       sysexfileloader.o perftimer.o ky040.o \
       $(SYNTH_DEXED_DIR)/synth_dexed.o

INCLUDE += -I $(SYNTH_DEXED_DIR)

EXTRACLEAN = $(SYNTH_DEXED_DIR)/*.o $(SYNTH_DEXED_DIR)/*.d

include ./Rules.mk
