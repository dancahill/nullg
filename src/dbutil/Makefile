# Makefile for NullLogic Groupware dbutil

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = config.o dbio.o dbiov2.o main.o md5.o sql.o
TARGET  = $(BIN_TARGETDIR)/nullgw-dbutil

all: dbutil

dbutil: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS) -L../../lib -lm -lnsp

clean:
	rm -f $(TARGET) $(OBJECTS) *~
