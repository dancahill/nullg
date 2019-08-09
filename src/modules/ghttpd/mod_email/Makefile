# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_email_accounts.o mod_email_address.o mod_email_codec.o mod_email_conf.o mod_email_db.o mod_email_files.o mod_email_filters.o mod_email_folders.o  mod_email_html.o mod_email_lang.o mod_email_main.o mod_email_mime.o mod_email_search.o mod_email_server.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_email.so

all: mod_email

mod_email: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
