# Makefile for NullLogic Groupware module

include ../../../Rules.mak

PHPBASE = /usr/include/php4
CFLAGS  = $(MOD_CFLAGS) -I$(PHPBASE) -I$(PHPBASE)/main -I$(PHPBASE)/regex -I$(PHPBASE)/TSRM -I$(PHPBASE)/Zend
OBJECTS = mod_php_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_php.so

all: mod_php

mod_php: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
