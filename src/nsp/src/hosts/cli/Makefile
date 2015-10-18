# Makefile for NullLogic Embedded Scripting Language
CC      = gcc
CFLAGS  = -Wall -g -O2 -I../../../include
LDFLAGS = -rdynamic -L../../../lib -lnsp -ldl -lm
#CFLAGS  = -Wall -pg -g -O2 -I../../../include
#LDFLAGS = -L../../../lib -lnsp -pg
OBJECTS = main.o
TARGET  = ../../../bin/nsp

-include ../../preconf.mak

all: _nsp

_nsp: $(OBJECTS)
	$(CC) -o $(TARGET) -s $(OBJECTS) $(LDFLAGS)
	$(CC) -o $(TARGET)_d $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET)_d $(TARGET)_s $(OBJECTS) *~
