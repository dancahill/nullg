# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = smtpq_bounce.o smtpq_client.o smtpq_conf.o smtpq_main.o smtpq_queue.o
TARGET  = $(LIB_TARGETDIR)/srv_smtpq.so

all: srv_smtpq

srv_smtpq: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
