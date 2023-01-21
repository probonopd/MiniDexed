CXX := g++
# CXXFLAGS := -O2
CXXFLAGS := -g -std=c++20
DEFINES := -DCPU=x86 -DDEBUG=1
INCLUDES := -I../../CMSIS_5/CMSIS/DSP/Include/ \
			-I../../CMSIS_5/CMSIS/Core/Include/ \
			-I../../Synth_Dexed/src
			# -I../../circle-stdlib/libs/circle/include \
			# -I../../circle-stdlib/libs/circle/addon \
GCC := $(CXX) $(INCLUDES) $(CXXFLAGS)

LD := g++
LIBS := -lm -lstdc++ -lgtest -lpthread

OBJS :=	\
		fx.o \
		fx_components.o \
		fx_svf.o \
		fx_tube.o \
		fx_chorus.o \
		fx_phaser.o \
		fx_orbitone.o \
		fx_flanger.o \
		fx_delay.o \
		fx_shimmer_reverb.o \
		fx_dry.o \
		fx_rack.o \
		effect_platervbstereo.o \
		arm_functions.o \
		wavein.o \
		waveout.o \
		fxrack_test.o

OBJS2:=	\
		fx.o \
		fx_components.o \
		fx_svf.o \
		fx_tube.o \
		fx_chorus.o \
		fx_phaser.o \
		fx_orbitone.o \
		fx_flanger.o \
		fx_delay.o \
		fx_shimmer_reverb.o \
		fx_dry.o \
		fx_rack.o \
		effect_platervbstereo.o \
		arm_functions.o \
		wavein.o \
		waveout.o \
		all_tests.o

test: all_tests fxrack_test
	./all_tests
	./fxrack_test

../%.cpp: ../%.h
	touch $@

%.o: ../%.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

wavein.o: wavein.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

waveout.o: waveout.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

arm_functions.o: arm_functions.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# # waveplay.o: waveplay.cpp
# # 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx.o: ../fx.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx_components.o: ../fx_components.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx_svf.o: ../fx_svf.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx_tube.o: ../fx_tube.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# ../fx_chorus.cpp: ../fx_engine.hpp
# 	touch ../fx_chorus.cpp

# fx_chorus.o: ../fx_chorus.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx_phaser.o: ../fx_phaser.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# ../fx_orbitone.cpp: ../fx_engine.hpp
# 	touch ../fx_orbitone.cpp

# fx_orbitone.o: ../fx_orbitone.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx_flanger.o: ../fx_flanger.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx_delay.o: ../fx_delay.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# ../fx_shimmer_reverb.cpp: ../fx_engine.hpp
# 	touch ../fx_shimmer_reverb.cpp

# fx_shimmer_reverb.o: ../fx_shimmer_reverb.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# fx_rack.o: ../fx_rack.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# effect_platervbstereo.o: ../effect_platervbstereo.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fxrack_test.cpp: ../mixing_console.h ../mixing_console.cpp
	touch $@

fxrack_test.o: fxrack_test.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

all_tests.o: all_tests.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@


fxrack_test: $(OBJS)
	$(LD) $(OBJS) -o fxrack_test $(LIBS)

all_tests: $(OBJS2)
	$(LD) $(OBJS2) -o all_tests $(LIBS)

clean:
	rm -f *.o fxrack_test
