# Makefile for NullLogic Groupware dbutil
CC       = gcc
CFLAGS   = -Wall -O2 -I../include
LDFLAGS  = -s -ldl
OBJECTS  = dbio.o main.o md5.o sql.o
TARGET = ../distrib/bin/dbutil

all: dbutil

dbutil: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
