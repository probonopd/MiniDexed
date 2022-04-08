#
# Synth_Dexed.mk
#

CMSIS_CORE_INCLUDE_DIR = $(CMSIS_DIR)/Core/Include
CMSIS_DSP_INCLUDE_DIR = $(CMSIS_DIR)/DSP/Include
CMSIS_DSP_PRIVATE_INCLUDE_DIR = $(CMSIS_DIR)/DSP/PrivateInclude
CMSIS_DSP_SOURCE_DIR = $(CMSIS_DIR)/DSP/Source

OBJS += \
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
       $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/SupportFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/BasicMathFunctions/BasicMathFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/FastMathFunctions/FastMathFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/FilteringFunctions/FilteringFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/CommonTables/CommonTables.o

INCLUDE += -I $(SYNTH_DEXED_DIR)
INCLUDE += -I $(CMSIS_CORE_INCLUDE_DIR)
INCLUDE += -I $(CMSIS_DSP_INCLUDE_DIR)
INCLUDE += -I $(CMSIS_DSP_PRIVATE_INCLUDE_DIR)
CXXFLAGS += -DARM_MATH_NEON -DHAVE_NEON

EXTRACLEAN = $(SYNTH_DEXED_DIR)/*.[od] $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/BasicMathFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/FastMathFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/FilteringFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/CommonTables/*.[od]
