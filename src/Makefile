#
# Makefile
#

CIRCLE_STDLIB_DIR = ../circle-stdlib
MSFA_DIR = msfa

OBJS	= main.o kernel.o miniorgan.o \
	  $(MSFA_DIR)/dx7note.o $(MSFA_DIR)/env.o $(MSFA_DIR)/exp2.o $(MSFA_DIR)/fm_core.o \
	  $(MSFA_DIR)/fm_op_kernel.o $(MSFA_DIR)/freqlut.o $(MSFA_DIR)/lfo.o \
	  $(MSFA_DIR)/pitchenv.o $(MSFA_DIR)/sin.o $(MSFA_DIR)/tuning.o

INCLUDE	+= -I $(MSFA_DIR) -I tuning-library

EXTRACLEAN = $(MSFA_DIR)/*.o $(MSFA_DIR)/*.d

include ./Rules.mk

%.o: %.cc
	@echo "  CPP   $@"
	@$(CPP) $(CPPFLAGS) -c -o $@ $<

%.d: %.cc
	@$(CPP) $(CPPFLAGS) -M -MG -MT $*.o -MT $@ -MF $@ $<
