# Makefile for NullLogic GroupServer Core

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = config.o domains.o format.o language.o main.o modctl.o nsp.o sanity.o server.o sql.o ssl.o tcp.o
TARGET  = $(BIN_TARGETDIR)/nullg

all: _nullg

_nullg: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS_T) -lresolv

clean:
	rm -f $(TARGET) $(OBJECTS) *~
