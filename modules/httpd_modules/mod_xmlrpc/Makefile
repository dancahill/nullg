# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_xmlrpc_auth.o mod_xmlrpc_contacts.o mod_xmlrpc_lang.o mod_xmlrpc_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_xmlrpc.so

all: mod_xmlrpc

mod_xmlrpc: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
