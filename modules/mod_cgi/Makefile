# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS  = mod_cgi.o
TARGET   = $(TARGETDIR)/mod_cgi.so

all: mod_cgi

mod_cgi: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
