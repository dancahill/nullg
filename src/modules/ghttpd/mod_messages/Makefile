# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_messages_lang.o mod_messages_main.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_messages.so

all: mod_messages

mod_messages: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
