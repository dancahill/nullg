# Makefile for NullLogic GroupServer dbutil

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = main.o
TARGET  = $(BIN_TARGETDIR)/nullsd-dbutil

all: dbutil

dbutil: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
