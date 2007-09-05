# Makefile for NullLogic GroupServer xmlsync

CC      = gcc
CFLAGS  = -Wall -O2 -fPIC -DLINUX -I../../include
LDFLAGS = -s -shared
OBJECTS = xmlsync.o
TARGET  = ../bin/xmlsync.so

all: mod_admin

mod_admin: $(OBJECTS)
	@mkdir -p ../bin
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
