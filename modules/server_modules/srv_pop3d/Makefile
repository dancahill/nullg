# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = pop3d_auth.o pop3d_conf.o pop3d_main.o pop3d_server.o
TARGET  = $(LIB_TARGETDIR)/srv_pop3d.so

all: srv_pop3d

srv_pop3d: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
