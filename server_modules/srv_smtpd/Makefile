# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = auth.o main.o smtpc.o smtpd.o
TARGET  = $(LIB_TARGETDIR)/srv_smtpd.so

all: srv_http

srv_http: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
