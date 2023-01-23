OBJDIR := objects
OUTPUT_FOLDER = results
EXE := all_test.bin

CXX := g++
CXXFLAGS = 	-g -std=c++20
DEFINES  = 	-DCPU=x86 -DDEBUG -DOUTPUT_FOLDER="\"$(OUTPUT_FOLDER)\""
INCLUDES = 	-I../../CMSIS_5/CMSIS/DSP/Include/ \
			-I../../CMSIS_5/CMSIS/Core/Include/ \
			-I../../Synth_Dexed/src/
			# -I../../circle-stdlib/libs/circle/include \
			# -I../../circle-stdlib/libs/circle/addon \

LD := g++
LIBS := -lm -lstdc++ -lgtest -lpthread

TST_SRCS := $(filter-out waveplay.cpp, $(wildcard *.cpp))
FX__SRCS := ../fx.cpp
FX__SRCS += ../fx_components.cpp
FX__SRCS += ../fx_svf.cpp
FX__SRCS += ../fx_tube.cpp
FX__SRCS += ../fx_chorus.cpp
FX__SRCS += ../fx_phaser.cpp
FX__SRCS += ../fx_orbitone.cpp
FX__SRCS += ../fx_flanger.cpp
FX__SRCS += ../fx_delay.cpp
FX__SRCS += ../effect_platervbstereo.cpp
FX__SRCS += ../fx_shimmer_reverb.cpp
FX__SRCS += ../fx_dry.cpp
FX__SRCS += ../fx_rack.cpp

TST_OBJS = $(TST_SRCS:%.cpp=$(OBJDIR)/%.o)
FX__OBJS = $(patsubst ../%, $(OBJDIR)/%, $(FX__SRCS:.cpp=.o))

all: $(EXE) test

test: $(EXE) $(OUTPUT_FOLDER)
	rm -f $(OUTPUT_FOLDER)/*
	./$(EXE)

$(OBJDIR):
	mkdir -p $@

$(OUTPUT_FOLDER):
	mkdir -p $@

$(OBJDIR)/%.o: %.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: ../%.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(EXE): $(TST_OBJS) $(FX__OBJS)
	$(LD) $(CXXFLAGS) $(call wildcard,$(TST_OBJS)) $(call wildcard,$(FX__OBJS)) -o $@ $(LIBS)

clean:
	rm -rf *.o $(OBJDIR) $(EXE) $(OUTPUT_FOLDER)
