# Makefile for NullLogic GroupServer module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = smtpq_bounce.o smtpq_client.o smtpq_conf.o smtpq_dns.o smtpq_lang.o smtpq_main.o smtpq_queue.o
TARGET  = $(LIB_TARGETDIR)/core/smtpq.so

all: core_smtpq

core_smtpq: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
