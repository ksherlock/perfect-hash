#
# Makefile for perfect hash program
#
.SUFFIXES:
.SUFFIXES: .o .c
CFLAGS ?= -std=c89 -Wno-return-type
#
#
# Specify -g in both the .c.o line and the perf: line to provide output
# for sdb.
#
.c.o:
	cc $(CFLAGS) -c  $*.c
#
OBJECTS= perfect.o 
#
HEADERS= perf.h
#
#
perf: $(OBJECTS) Makefile
	cc -o  perf $(OBJECTS)
	cc $(CFLAGS) -o status status.c
#
$(OBJECTS) : $(HEADERS)
