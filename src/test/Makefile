BINDIR := bin
OBJDIR := objects
OUTPUT_FOLDER := results

EXE := $(BINDIR)/all_tests.bin
BETA := $(BINDIR)/beta.bin

CXX		 =	g++
CXXFLAGS = 	-g -Wall -std=c++20 -MMD -MP
DEFINES  = 	-DCPU=x86 -DDEBUG -DOUTPUT_FOLDER="\"$(OUTPUT_FOLDER)\""
INCLUDES = 	-I../../CMSIS_5/CMSIS/DSP/Include/ \
			-I../../CMSIS_5/CMSIS/Core/Include/ \
			-I../../Synth_Dexed/src/

CPPCHECK 	= cppcheck
CHECKFLAGS 	= 	-q -j 8 --enable=all --force --language=c++ \
				$(INCLUDES) --platform=unix64 \
				--error-exitcode=0 \
				--suppressions-list=cppcheck-suppression-list.txt

-include $(TST_OBJS:.o=.d)
-include $(FX__OBJS:.o=.d)

LD 	 := g++
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
FX__SRCS += ../fx_shimmer_helper.cpp
FX__SRCS += ../fx_diffuser.cpp
FX__SRCS += ../fx_pitch_shifter.cpp
FX__SRCS += ../fx_reverberator.cpp
FX__SRCS += ../fx_shimmer_reverb.cpp
FX__SRCS += ../fx_dry.cpp
FX__SRCS += ../fx_rack.cpp

TST_SRCS := $(filter-out waveplay.cpp $(wildcard beta*.cpp), $(wildcard *.cpp))

BETASRCS := $(wildcard beta*.cpp)
BETASRCS += arm_functions.cpp
BETASRCS += wavein.cpp
BETASRCS += waveout.cpp
BETASRCS += test_fx_helper.cpp

FX__OBJS = $(patsubst ../%, $(OBJDIR)/%, $(FX__SRCS:.cpp=.o))
TST_OBJS = $(TST_SRCS:%.cpp=$(OBJDIR)/%.o)
BETAOBJS = $(BETASRCS:%.cpp=$(OBJDIR)/%.o)

all: $(EXE) test

build: $(EXE)

cpp-check: $(FX__SRCS) $(TST_SRCS) $(BETASRCS)
	flawfinder ../../src
	$(CPPCHECK) $(CHECKFLAGS) $^

test: $(EXE) $(OUTPUT_FOLDER)
	rm -rf $(OUTPUT_FOLDER)/*
	./$(EXE)

test-debug: $(EXE) $(OUTPUT_FOLDER)
	rm -rf $(OUTPUT_FOLDER)/*
	valgrind --leak-check=full --leak-resolution=high --show-leak-kinds=all --xtree-leak=yes --show-mismatched-frees=yes --error-limit=no --log-file=$(OUTPUT_FOLDER)/valgrind-analysis-results.txt ./$(EXE)

$(BINDIR):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

$(OUTPUT_FOLDER):
	mkdir -p $@

$(OBJDIR)/%.o: %.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: ../%.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(EXE): $(BINDIR) $(TST_OBJS) $(FX__OBJS)
	$(LD) $(CXXFLAGS) $(call wildcard,$(TST_OBJS)) $(call wildcard,$(FX__OBJS)) -o $@ $(LIBS)

$(BETA): $(BINDIR) $(BETAOBJS) $(FX__OBJS)
	$(LD) $(CXXFLAGS) $(BETAOBJS) $(call wildcard,$(FX__OBJS)) -o $@ $(LIBS)

clean:
	rm -rf $(OBJDIR) $(BINDIR) $(OUTPUT_FOLDER)
