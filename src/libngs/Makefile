# Makefile for NullLogic GroupServer Core Library
include ../Rules.mak

CC      = gcc
#CFLAGS  = -g -Wall -O2 -I../../include
CFLAGS  = -g $(BIN_CFLAGS)
OBJECTS = config.o dirlist.o format.o lib.o log.o md5.o sql.o sys.o
TARGET  = ../../lib/libngs.a

all: _lib

_lib: $(OBJECTS)
	ar cru $(TARGET) $(OBJECTS)
	ranlib $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
