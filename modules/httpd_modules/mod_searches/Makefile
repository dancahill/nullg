# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_searches_contacts.o mod_searches_db.o mod_searches_main.o mod_searches_sql.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_searches.so

all: mod_searches

mod_searches: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
