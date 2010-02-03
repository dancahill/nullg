# Makefile for NullLogic GroupServer confutil

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = main.o
TARGET  = $(BIN_TARGETDIR)/nullsd-confutil

all: confutil

confutil: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
