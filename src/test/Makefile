OBJDIR := objects
OUTPUT_FOLDER = results
EXECUTABLE := all_test.bin

CXX := g++
# CXXFLAGS := -O2
CXXFLAGS := -g -std=c++20
DEFINES := -DCPU=x86 -DDEBUG=1 -DOUTPUT_FOLDER=$(OUTPUT_FOLDER)
INCLUDES := -I../../CMSIS_5/CMSIS/DSP/Include/ \
			-I../../CMSIS_5/CMSIS/Core/Include/ \
			-I../../Synth_Dexed/src
			# -I../../circle-stdlib/libs/circle/include \
			# -I../../circle-stdlib/libs/circle/addon \
GCC := $(CXX) $(INCLUDES) $(CXXFLAGS)

LD := g++
LIBS := -lm -lstdc++ -lgtest -lpthread

SRCS = $(filter-out waveplay.cpp, $(wildcard *.cpp))
SRCS += ../fx.cpp
SRCS += ../fx_svf.cpp
SRCS += ../fx_tube.cpp
SRCS += ../fx_chorus.cpp
SRCS += ../fx_phaser.cpp
SRCS += ../fx_orbitone.cpp
SRCS += ../fx_flanger.cpp
SRCS += ../fx_delay.cpp
SRCS += ../effect_platervbstereo.cpp
SRCS += ../fx_shimmer_reverb.cpp
SRCS += ../fx_dry.cpp
SRCS += ../fx_rack.cpp

OBJS = $(SRCS:.cpp=.o)
# OBJS = $(addprefix $(OBJDIR)/,$(OBJ))

all: $(EXECUTABLE)

test: all
	rm -f $(OUTPUT_FOLDER)/*
	./$(EXECUTABLE)

clean-all: clean test

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.cpp $(OBJDIR)
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $< -o $@
	# mv -f $@ $(OBJDIR)

test_mixing_console.cpp: ../mixing_console.h ../mixing_console.cpp
	touch $@

$(EXECUTABLE): $(OBJS)
	echo "***************************"
	echo $(OBJS)
	echo "***************************"
	$(LD) $(OBJS) -o $@ $(LIBS)

$(OUTPUT_FOLDER):
	mkdir -p $@

clean:
	rm -rf $(OUTPUT_FOLDER) *.o $(EXECUTABLE)
