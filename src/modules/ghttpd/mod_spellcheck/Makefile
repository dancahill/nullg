# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_spellcheck_main.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_spellcheck.so

all: mod_spellcheck

mod_spellcheck: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
