# Makefile for NullLogic GroupServer Core

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = config.o domains.o format.o language.o main.o modctl.o sanity.o server.o sql.o ssl.o tcp.o
TARGET  = $(BIN_TARGETDIR)/nullgsd

all: _nullgsd

_nullgsd: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS_T)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
