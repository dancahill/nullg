# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_profile_db.o mod_profile_lang.o mod_profile_main.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_profile.so

all: mod_profile

mod_profile: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
