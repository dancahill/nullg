# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_accounting_accounts.o mod_accounting_db.o mod_accounting_journal.o mod_accounting_lang.o mod_accounting_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_accounting.so

all: mod_accounting

mod_accounting: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
