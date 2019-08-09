# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_weblog_db.o mod_weblog_lang.o mod_weblog_main.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_weblog.so

all: mod_weblog

mod_weblog: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
