# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(SRVMOD_CFLAGS)
OBJECTS = auth.o main.o smtp.o smtpclient.o
TARGET  = $(SRVMOD_TARGETDIR)/srv_smtpd.so

all: srv_http

srv_http: $(OBJECTS)
	$(CC) $(SRVMOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
