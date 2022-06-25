#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src
CMSIS_DIR = ../CMSIS_5/CMSIS
U8G2_DIR = ../u8g2

BUILD_DIR = ../build
BUILD_OBJ_DIR = $(BUILD_DIR)/objs

OBJS = main.o \ 
	kernel.o \ 
	minidexed.o \ 
	config.o \ 
	userinterface.o \ 
	uimenu.o \ 
       mididevice.o \ 
	midikeyboard.o \ 
	serialmididevice.o \ 
	pckeyboard.o \ 
       sysexfileloader.o \ 
	performanceconfig.o \ 
	perftimer.o \ 
       effect_compressor.o \ 
	effect_platervbstereo.o \
       ui.o \
       screens.o \ 
       userinterfaceext.o

OPTIMIZE = -O3

include ./Synth_Dexed.mk
include ./Rules.mk
