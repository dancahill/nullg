# Makefile for NullLogic Groupware
CC       = gcc
CFLAGS   = -Wall -O2 -s -I../include
#CFLAGS   = -Wall -O -g -I../include
LDFLAGS  = -ldl -lpthread
OBJECTS  = \
	auth.o cgi.o config.o db.o format.o html.o http.o io.o logging.o \
	main.o md5.o modctl.o sanity.o server.o sql.o xmlrpc.o

all: groupware

groupware: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o ../distrib/bin/groupware

clean:
	rm -f ../distrib/bin/groupware $(OBJECTS)
