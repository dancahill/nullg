# Makefile for NullLogic Groupware HTTPd

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = config.o domains.o format.o log.o main.o md5.o modctl.o sanity.o server.o sql.o ssl.o tcp.o
TARGET  = $(BIN_TARGETDIR)/nullgroupware

all: _nullgroupware

_nullgroupware: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS_T)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
