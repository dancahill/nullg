# Makefile for NullLogic GroupServer module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_forums_db.o mod_forums_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_forums.so

all: mod_forums

mod_forums: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
