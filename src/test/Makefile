OBJDIR := objects
OUTPUT_FOLDER = results
EXE := all_tests.bin
BETA := beta.bin

CXX := g++
CXXFLAGS = 	-g -std=c++20 -MMD -MP
DEFINES  = 	-DCPU=x86 -DDEBUG -DOUTPUT_FOLDER="\"$(OUTPUT_FOLDER)\""
INCLUDES = 	-I../../CMSIS_5/CMSIS/DSP/Include/ \
			-I../../CMSIS_5/CMSIS/Core/Include/ \
			-I../../Synth_Dexed/src/

-include $(TST_OBJS:.o=.d)
-include $(FX__OBJS:.o=.d)

LD := g++
LIBS := -lm -lstdc++ -lgtest -lpthread

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
FX__SRCS += ../fx_rack.cpp

TST_SRCS := $(filter-out waveplay.cpp $(wildcard beta*.cpp), $(wildcard *.cpp))

FX__OBJS = $(patsubst ../%, $(OBJDIR)/%, $(FX__SRCS:.cpp=.o))
TST_OBJS = $(TST_SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(EXE) test

test: $(EXE) $(OUTPUT_FOLDER)
	rm -rf $(OUTPUT_FOLDER)/*
	./$(EXE)

test-debug: $(EXE) $(OUTPUT_FOLDER)
	rm -rf $(OUTPUT_FOLDER)/*
	valgrind --leak-check=full --leak-resolution=high --show-leak-kinds=all --xtree-leak=yes --show-mismatched-frees=yes --error-limit=no --log-file=$(OUTPUT_FOLDER)/valgrind-analysis-results.txt ./$(EXE)

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
