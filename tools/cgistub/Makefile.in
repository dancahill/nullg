# Makefile for NullLogic Groupware FastCGI
CC       = gcc
CFLAGS   = -Wall -O2
LDFLAGS  = -s
OBJECTS  = main.o
TARGET = ../distrib/bin/fastcgi

all: fastcgi

fastcgi: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
