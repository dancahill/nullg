# Makefile for NullLogic GroupServer module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_notes_db.o mod_notes_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_notes.so

all: mod_notes

mod_notes: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
