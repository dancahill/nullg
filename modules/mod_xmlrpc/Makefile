# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS  = mod_xmlrpc.o
TARGET   = $(TARGETDIR)/mod_xmlrpc.so

all: mod_xmlrpc

mod_xmlrpc: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
