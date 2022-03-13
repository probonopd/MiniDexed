#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src
CMSIS_CORE_INCLUDE_DIR = ../CMSIS_5/CMSIS/Core/Include
CMSIS_DSP_INCLUDE_DIR = ../CMSIS_5/CMSIS/DSP/Include
CMSIS_DSP_PRIVATE_INCLUDE_DIR = ../CMSIS_5/CMSIS/DSP/PrivateInclude
CMSIS_DSP_SOURCE_DIR = ../CMSIS_5/CMSIS/DSP/Source

OBJS = main.o kernel.o minidexed.o config.o userinterface.o \
       mididevice.o midikeyboard.o serialmididevice.o pckeyboard.o \
       sysexfileloader.o perftimer.o ky040.o \
       $(SYNTH_DEXED_DIR)/PluginFx.o \
       $(SYNTH_DEXED_DIR)/dexed.o \
       $(SYNTH_DEXED_DIR)/dx7note.o \
       $(SYNTH_DEXED_DIR)/env.o \
       $(SYNTH_DEXED_DIR)/exp2.o \
       $(SYNTH_DEXED_DIR)/fm_core.o \
       $(SYNTH_DEXED_DIR)/fm_op_kernel.o \
       $(SYNTH_DEXED_DIR)/freqlut.o \
       $(SYNTH_DEXED_DIR)/lfo.o \
       $(SYNTH_DEXED_DIR)/pitchenv.o \
       $(SYNTH_DEXED_DIR)/porta.o \
       $(SYNTH_DEXED_DIR)/sin.o \
       $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/SupportFunctions.o

INCLUDE += -I $(SYNTH_DEXED_DIR)
INCLUDE += -I $(CMSIS_CORE_INCLUDE_DIR)
INCLUDE += -I $(CMSIS_DSP_INCLUDE_DIR)
INCLUDE += -I $(CMSIS_DSP_PRIVATE_INCLUDE_DIR)
CXXFLAGS += -DARM_MATH_NEON

EXTRACLEAN = $(SYNTH_DEXED_DIR)/*.o $(SYNTH_DEXED_DIR)/*.d $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/*.o $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/*.d

include ./Rules.mk
