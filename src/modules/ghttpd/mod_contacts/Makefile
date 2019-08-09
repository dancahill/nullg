# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_contacts_db.o mod_contacts_lang.o mod_contacts_main.o mod_contacts_vcard.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_contacts.so

all: mod_contacts

mod_contacts: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
