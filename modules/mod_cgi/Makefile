# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_cgi_main.o
TARGET  = $(MOD_TARGETDIR)/mod_cgi.so

all: mod_cgi

mod_cgi: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
