# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = pop3d_auth.o pop3d_conf.o pop3d_lang.o pop3d_main.o pop3d_server.o
TARGET  = $(LIB_TARGETDIR)/core/pop3d.so

all: core_pop3d

core_pop3d: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
