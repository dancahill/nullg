# Makefile for NullLogic Groupware smtpc

include ../../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = dns.o main.o smtpc.o
TARGET  = ../$(BIN_TARGETDIR)/nullgw-smtpc

all: smtpc

smtpc: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS) -lresolv

clean:
	rm -f $(TARGET) $(OBJECTS) *~
