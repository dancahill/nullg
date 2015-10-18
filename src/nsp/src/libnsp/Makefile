# Makefile for NullLogic Embedded Scripting Language
CC      = gcc
CFLAGS  = -Wall -g -O3 -I../../include
#CFLAGS  = -Wall -pg -g -O3 -I../../include
#CFLAGS  = -ansi -pedantic -D_GNU_SOURCE -Wall -g -O3 -I../../include
#CFLAGS  = -ansi -pedantic -D_BSD_SOURCE -Wall -g -O3 -I../../include
OBJECTS = block.o compile.o debug.o exec.o libc.o libn.o objects.o opcodes.o parser.o
#OBJECTS = merged.o
TARGET  = ../../lib/libnsp.a

.SILENT:

all: _libnsp

_libnsp: $(OBJECTS)
	@ar cru $(TARGET) $(OBJECTS)
	@ranlib $(TARGET)

clean:
	@rm -f $(TARGET) $(OBJECTS) *~

.c.o:
	@echo "  CC $@" && $(CC) $(CFLAGS) -c $< -o $@
#	@echo "$(CC) $(CFLAGS) -c $< -o $@" && $(CC) $(CFLAGS) -c $< -o $@
