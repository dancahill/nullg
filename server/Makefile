# Makefile for NullLogic Groupware
CC       = gcc
CFLAGS   = -Wall -O2 -I../include
#CFLAGS   = -Wall -O -g -I../include
LDFLAGS  = -s -ldl -lpthread
OBJECTS  = \
	auth.o config.o format.o html.o http.o io.o log.o main.o md5.o \
	modctl.o sanity.o server.o sql.o
TARGET = ../distrib/bin/groupware

all: server

server: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
