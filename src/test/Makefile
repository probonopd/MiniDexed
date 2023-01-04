CXX := gcc
# CXXFLAGS := -O2
CXXFLAGS := -g 
DEFINES := -DCPU=x86
INCLUDES := -I../../CMSIS_5/CMSIS/DSP/Include/ -I../../CMSIS_5/CMSIS/Core/Include/
GCC := $(CXX) $(INCLUDES) $(CXXFLAGS)

LD := gcc
LIBS := -lm -lstdc++

OBJS :=	\
		wavein.o \
		waveout.o \
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
		fx_rack.o \
		fxrack_test.o

test: fxrack_test
	./fxrack_test

# %.o: ../%.cpp
#  	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

wavein.o: wavein.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

waveout.o: waveout.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

# waveplay.o: waveplay.cpp
# 	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx.o: ../fx.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx_components.o: ../fx_components.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx_svf.o: ../fx_svf.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx_tube.o: ../fx_tube.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

../fx_chorus.cpp: ../fx_engine.hpp
	touch ../fx_chorus.cpp

fx_chorus.o: ../fx_chorus.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx_phaser.o: ../fx_phaser.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

../fx_orbitone.cpp: ../fx_engine.hpp
	touch ../fx_orbitone.cpp

fx_orbitone.o: ../fx_orbitone.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx_flanger.o: ../fx_flanger.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx_delay.o: ../fx_delay.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

../fx_shimmer_reverb.cpp: ../fx_engine.hpp
	touch ../fx_shimmer_reverb.cpp

fx_shimmer_reverb.o: ../fx_shimmer_reverb.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fx_rack.o: ../fx_rack.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@

fxrack_test.o: fxrack_test.cpp
	$(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -c $^ -o $@


fxrack_test: $(OBJS)
	$(LD) $(OBJS) -o fxrack_test $(LIBS)

clean:
	rm -f *.o fxrack_test
