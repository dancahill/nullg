# Makefile for Null Groupware dbutil
CC       = gcc
CFLAGS   = -Wall -O2 -s -I../groupware/include
LDFLAGS  = -ldl
OBJECTS  = dbio.o main.o md5.o sql.o

all: dbutil

dbutil: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o ../distrib/bin/dbutil
	@echo Make dbutil is complete.

clean:
	rm -f ../distrib/bin/dbutil $(OBJECTS)
