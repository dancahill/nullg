# Makefile for NullLogic Groupware dbutil

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = dbio.o main.o md5.o sql.o
TARGET  = $(BIN_TARGETDIR)/dbutil

all: dbutil

dbutil: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
