# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_messages.o
TARGET  = $(MOD_TARGETDIR)/mod_messages.so

all: mod_messages

mod_messages: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
