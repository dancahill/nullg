# Makefile for NullLogic Groupware
CC       = gcc
CFLAGS   = -Wall -O2 -s -I./include
#CFLAGS   = -Wall -O -g -I./include
LDFLAGS  = -ldl -lpthread
OBJECTS  = \
	base/auth.o base/cgi.o base/config.o base/connio.o base/dbio.o \
	base/fileio.o base/format.o base/html.o base/http.o base/logging.o \
	base/main.o base/md5.o base/sanity.o base/server.o base/sql.o \
	base/time.o \
	mods/admin.o mods/bookmarks.o mods/calendar.o mods/calls.o \
	mods/contacts.o mods/files.o mods/forums.o mods/messages.o \
	mods/notes.o mods/orders.o mods/products.o mods/profile.o \
	mods/searches.o mods/tasks.o mods/wmcodec.o mods/wmmain.o \
	mods/wmserver.o mods/xml-rpc.o

all: groupware

groupware: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o ../distrib/bin/groupware
	@echo Make groupware is complete.

clean:
	rm -f ../distrib/bin/groupware $(OBJECTS)
