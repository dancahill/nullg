# Makefile for NullLogic Groupware sendmail

include ../../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = main.o
TARGET  = sendmail

all: _sendmail

_sendmail: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
