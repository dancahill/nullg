# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS  = mod_calls_db.o mod_calls_main.o
TARGET   = $(TARGETDIR)/mod_calls.so

all: mod_calls

mod_calls: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
