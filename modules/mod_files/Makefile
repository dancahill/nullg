# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_files_conf.o mod_files_db.o mod_files_main.o
TARGET  = $(MOD_TARGETDIR)/mod_files.so

all: mod_files

mod_files: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
