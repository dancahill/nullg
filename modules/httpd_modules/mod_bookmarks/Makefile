# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_bookmarks_db.o mod_bookmarks_export.o mod_bookmarks_lang.o mod_bookmarks_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_bookmarks.so

all: mod_bookmarks

mod_bookmarks: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
