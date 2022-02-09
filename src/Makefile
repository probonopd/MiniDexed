#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
SYNTH_DEXED_DIR = ../Synth_Dexed/src

OBJS = main.o kernel.o miniorgan.o \
	$(SYNTH_DEXED_DIR)/synth_dexed.o \

INCLUDE += -I $(SYNTH_DEXED_DIR) -I tuning-library

EXTRACLEAN = $(SYNTH_DEXED_DIR)/*.o $(SYNTH_DEXED_DIR)/*.d

include ./Rules.mk

%.o: %.cc
	@echo "  CPP   $@"
	@$(CPP) $(CPPFLAGS) -c -o $@ $<

%.d: %.cc
	@$(CPP) $(CPPFLAGS) -M -MG -MT $*.o -MT $@ -MF $@ $<
