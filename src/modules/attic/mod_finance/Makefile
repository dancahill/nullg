# Makefile for NullLogic GroupServer module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_finance_accounts.o mod_finance_db.o mod_finance_inventory.o mod_finance_invoices.o mod_finance_invoiceitems.o mod_finance_journal.o mod_finance_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_finance.so

all: mod_finance

mod_finance: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
