#
# Makefile
#

MAXIDEXED_SRC_DIR = ../src

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src
CMSIS_DIR = ../CMSIS_5/CMSIS
U8G2_DIR = ../u8g2

BUILD_DIR = ../build

MAXIDEXED_BUILD_DIR = $(BUILD_DIR)/maxi-dexed

CIRCLE_STDLIB_BUILD_DIR = $(BUILD_DIR)/circle-stdlib
SYNTH_DEXED_BUILD_DIR = $(BUILD_DIR)/Synth_Dexed/src
CMSIS_BUILD_DIR = $(BUILD_DIR)/CMSIS_5/CMSIS
U8G2_BUILD_DIR = $(BUILD_DIR)/u8g2

SOURCES = $(wildcard $(MAXIDEXED_SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(MAXIDEXED_SRC_DIR)/%.cpp,$(MAXIDEXED_BUILD_DIR)/%.o,$(SOURCES))

dir:
	mkdir -p $(MAXIDEXED_BUILD_DIR)

OPTIMIZE = -O3

include ./Synth_Dexed.mk
include ./Rules.mk
