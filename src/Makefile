#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src

OBJS = main.o kernel.o minidexed.o pckeyboard.o $(SYNTH_DEXED_DIR)/synth_dexed.o \
       perftimer.o

INCLUDE += -I $(SYNTH_DEXED_DIR)

EXTRACLEAN = $(SYNTH_DEXED_DIR)/*.o $(SYNTH_DEXED_DIR)/*.d

include ./Rules.mk

%.o: %.cc
	@echo "  CPP   $@"
	@$(CPP) $(CPPFLAGS) -c -o $@ $<

%.d: %.cc
	@$(CPP) $(CPPFLAGS) -M -MG -MT $*.o -MT $@ -MF $@ $<
