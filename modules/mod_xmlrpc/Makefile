# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS  = mod_xmlrpc_auth.o mod_xmlrpc_contacts.o mod_xmlrpc_main.o
TARGET   = $(TARGETDIR)/mod_xmlrpc.so

all: mod_xmlrpc

mod_xmlrpc: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
