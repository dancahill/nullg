# Makefile for NullLogic GroupServer module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = smtpd_auth.o smtpd_bounce.o smtpd_client.o smtpd_conf.o smtpd_dns.o smtpd_filter.o smtpd_lang.o smtpd_main.o smtpd_server.o
TARGET  = $(LIB_TARGETDIR)/core/smtpd.so

all: core_smtpd

core_smtpd: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
