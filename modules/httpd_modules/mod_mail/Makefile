# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_mail_accounts.o mod_mail_address.o mod_mail_codec.o mod_mail_conf.o mod_mail_db.o mod_mail_files.o mod_mail_filters.o mod_mail_folders.o  mod_mail_html.o mod_mail_main.o mod_mail_mime.o mod_mail_search.o mod_mail_server.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_mail.so

all: mod_mail

mod_mail: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
