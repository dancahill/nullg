# Makefile for NullLogic Groupware POP3d

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = auth.o config.o format.o io.o log.o main.o md5.o pop3.o sanity.o \
	server.o sql.o
TARGET  = $(BIN_TARGETDIR)/nullgw-pop3d

all: nullgw-pop3d

nullgw-pop3d: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS_T)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
