# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS = mod_mail_addr.o mod_mail_codec.o mod_mail_folders.o mod_mail_html.o mod_mail_main.o mod_mail_mime.o mod_mail_server.o
TARGET  = $(TARGETDIR)/mod_mail.so

all: mod_mail

mod_mail: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
