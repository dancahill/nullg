# Makefile for NullLogic Groupware FastCGI
CC       = gcc
CFLAGS   = -Wall -O2
LDFLAGS  = -s
OBJECTS  = main.o
TARGET   = gw.cgi

all: fastcgi

fastcgi: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
