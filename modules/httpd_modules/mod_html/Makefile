# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_html_lang.o mod_html_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_html.so

all: mod_html

mod_html: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
