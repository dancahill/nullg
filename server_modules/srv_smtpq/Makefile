# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = main.o smtpclient.o
TARGET  = $(LIB_TARGETDIR)/srv_smtpq.so

all: srv_smtpq

srv_smtpq: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
