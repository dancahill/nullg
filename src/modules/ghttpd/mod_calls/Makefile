# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_calls_db.o mod_calls_lang.o mod_calls_main.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_calls.so

all: mod_calls

mod_calls: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
