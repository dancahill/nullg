# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS  = mod_contacts_db.o mod_contacts_main.o mod_contacts_vcard.o
TARGET   = $(TARGETDIR)/mod_contacts.so

all: mod_contacts

mod_contacts: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
